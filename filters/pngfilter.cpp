#include "pngfilter.hpp"
#include "preflate_adapters.hpp"
#include "../preflate/preflate.h"
#include <vector>
#include <cstring>
#include <algorithm>

// Big-Endian read helper
static uint32_t read_be32(const uint8_t* p) {
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

// Big-Endian write helper
static void write_be32(uint8_t* p, uint32_t v) {
    p[0] = (v >> 24) & 0xFF;
    p[1] = (v >> 16) & 0xFF;
    p[2] = (v >> 8) & 0xFF;
    p[3] = v & 0xFF;
}

// Using zlib's crc32 for PNG chunk CRC calculation
#include "../zlib/zlib.h"

PNGFilter::PNGFilter(std::string n, Filetype f) {
    name = n;
    Type = f;
}

PNGFilter::~PNGFilter() {}

void PNGFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    // Read the entire IDAT sequence range provided by parser
    std::vector<uint8_t> input_data(size);
    in->blockread(input_data.data(), size);

    std::vector<uint8_t> zlib_stream;
    std::vector<uint32_t> idat_sizes;

    uint64_t pos = 0;
    while (pos < size) {
        if (pos + 8 > size) break; // Too small for Length+Type

        uint32_t length = read_be32(&input_data[pos]);
        uint32_t type = read_be32(&input_data[pos + 4]);

        // Safety check: ensure we are looking at an IDAT
        // 0x49444154 = "IDAT"
        if (type != 0x49444154) {
             // Parser should ensure we only get IDATs. If not, it's an error.
             break;
        }

        if (pos + 12 + length > size) break; // Incomplete chunk (Length+Type+Data+CRC)

        // Extract data (IDAT chunk data is part of the ZLIB stream)
        const uint8_t* data_ptr = &input_data[pos + 8];
        zlib_stream.insert(zlib_stream.end(), data_ptr, data_ptr + length);
        idat_sizes.push_back(length);

        pos += 12 + length; // Length(4) + Type(4) + Data(Len) + CRC(4)
    }

    // Check if we extracted anything
    if (zlib_stream.empty()) {
        diffFound = 1;
        return;
    }

    // ZLIB Stream format: [Header (2)] + [Deflate Data] + [Adler32 (4)]
    // We need at least 6 bytes for header + adler32
    if (zlib_stream.size() < 6) {
        diffFound = 1;
        return;
    }

    // Extract ZLIB header (first 2 bytes)
    uint8_t zlib_header[2];
    zlib_header[0] = zlib_stream[0];
    zlib_header[1] = zlib_stream[1];

    // Extract raw deflate stream (strip 2-byte header and 4-byte Adler32 footer)
    std::vector<uint8_t> raw_deflate(zlib_stream.begin() + 2, zlib_stream.end() - 4);

    // Decode with preflate
    std::vector<uint8_t> unpacked;
    std::vector<uint8_t> recon_info;
    
    // Always use Raw Deflate for PNG IDAT streams (stripped of ZLIB wrapper)
    bool decode_success = preflate_decode(unpacked, recon_info, raw_deflate);

    if (!decode_success) {
        // If preflate_decode fails, we cannot process this stream.
        diffFound = 1;
        return;
    }
    
    // Write format (Version 1):
    // [1 byte: version=1]
    // [2 bytes: original ZLIB Header]
    // [4 bytes: number of IDAT chunks]
    // [4*N bytes: original IDAT chunk sizes]
    // [4 bytes: recon_info size]
    // [recon_info bytes]
    // [unpacked data]
    
    out->putc(1); // Version 1
    out->putc(zlib_header[0]);
    out->putc(zlib_header[1]);

    out->put32(static_cast<uint32_t>(idat_sizes.size()));
    for (uint32_t sz : idat_sizes) {
        out->put32(sz);
    }
    
    out->put32(static_cast<uint32_t>(recon_info.size()));
    out->blockwrite(recon_info.data(), recon_info.size());
    hdrsize=static_cast<uint32_t>(out->curpos());
    out->blockwrite(unpacked.data(), unpacked.size());
    
    diffFound = 0;
}

uint64_t PNGFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    // Read Version
    int version = in->getc();

    if (version == 1) {
        // Read ZLIB Header
        uint8_t zlib_header[2];
        zlib_header[0] = in->getc();
        zlib_header[1] = in->getc();

        // Read Fragmentation Map (IDAT chunk sizes)
        uint32_t chunk_count = in->get32();

        std::vector<uint32_t> idat_sizes(chunk_count);
        for (uint32_t i = 0; i < chunk_count; i++) {
            idat_sizes[i] = in->get32();
        }

        // Read reconstruction info size
        uint32_t recon_size = in->get32();

        // Read reconstruction info
        std::vector<uint8_t> recon_info(recon_size);
        in->blockread(recon_info.data(), recon_size);

        // Calculate remaining size for unpacked data
        // Total encoded size = 1 (version) + 2 (zlib_header) + 4 (chunk_count) + 4*N (idat_sizes) + 4 (recon_size) + recon_info_size + unpacked_size
        uint64_t header_overhead = 1 + 2 + 4 + (static_cast<uint64_t>(chunk_count) * 4) + 4 + recon_size;
        hdrsize=header_overhead;
        if (header_overhead > size) {
            fsize = 0;
            return 0;
        }

        uint64_t unpacked_size = size - header_overhead;

        // Read unpacked data
        std::vector<uint8_t> unpacked(unpacked_size);
        in->blockread(unpacked.data(), unpacked_size);

        // Reencode: unpacked + recon_info -> raw deflate stream
        std::vector<uint8_t> raw_deflate;

        bool reencode_success = preflate_reencode(raw_deflate, recon_info, unpacked);
        if (!reencode_success) {
            fsize = 0;
            return 0;
        }

        // Reconstruct full ZLIB Stream: [Header] + [Raw Deflate] + [Adler32]
        std::vector<uint8_t> zlib_stream;
        zlib_stream.push_back(zlib_header[0]);
        zlib_stream.push_back(zlib_header[1]);
        zlib_stream.insert(zlib_stream.end(), raw_deflate.begin(), raw_deflate.end());

        // Calculate Adler32 of the *unpacked* data
        uLong adler = adler32(0L, Z_NULL, 0);
        adler = adler32(adler, unpacked.data(), unpacked.size());

        // Append Adler32 checksum (Big Endian)
        zlib_stream.push_back((adler >> 24) & 0xFF);
        zlib_stream.push_back((adler >> 16) & 0xFF);
        zlib_stream.push_back((adler >> 8) & 0xFF);
        zlib_stream.push_back(adler & 0xFF);

        // Re-fragment into IDAT chunks and write to output
        uint64_t zlib_pos = 0;
        uint64_t total_written_bytes = 0;

        for (uint32_t chunk_len : idat_sizes) {
            if (zlib_pos + chunk_len > zlib_stream.size()) {
                 // Error: ZLIB stream shorter than expected chunks
                 fsize = 0;
                 return 0;
            }

            // Write IDAT Length (Big Endian)
            out->put32(chunk_len);

            // Prepare CRC buffer: Type + Data
            // CRC is calculated over the 4-byte chunk type code and the chunk data.
            std::vector<uint8_t> crc_buf;
            uint32_t type_idat = 0x49444154; // "IDAT"

            // Append Type to CRC buffer
            crc_buf.push_back((type_idat >> 24) & 0xFF);
            crc_buf.push_back((type_idat >> 16) & 0xFF);
            crc_buf.push_back((type_idat >> 8) & 0xFF);
            crc_buf.push_back(type_idat & 0xFF);

            // Append Data to CRC buffer
            crc_buf.insert(crc_buf.end(), &zlib_stream[zlib_pos], &zlib_stream[zlib_pos + chunk_len]);

            // Write Type (Big Endian)
            out->put32(type_idat);

            // Write Data
            out->blockwrite(&zlib_stream[zlib_pos], chunk_len);

            // Calculate and Write CRC (Big Endian)
            uLong crc = crc32(0L, Z_NULL, 0);
            crc = crc32(crc, crc_buf.data(), crc_buf.size());
            out->put32(crc);

            zlib_pos += chunk_len;
            total_written_bytes += (4 + 4 + chunk_len + 4); // Length + Type + Data + CRC
        }

        // If there's remaining data in zlib_stream, it's an error or unexpected.
        // For bit-perfect reconstruction, it should be exactly consumed.
        if (zlib_pos != zlib_stream.size()) {
            fsize = 0;
            return 0;
        }

        fsize = total_written_bytes;
        return fsize;

    } else {
        // Unsupported version
        fsize = 0;
        return 0;
    }
}
