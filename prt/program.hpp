#pragma once
#include "cli.hpp"
#include "job.hpp"
#include "segment.hpp"
#include "../filters/textfilter.hpp"
#include "../predictors/predictors.hpp"
#include "../predictors/predictor.hpp"
#include "../predictors/predictordec.hpp"
#include "../predictors/predictorjpeg.hpp"
#include "../predictors/predictorexe.hpp"
#include "../predictors/predictorimg4.hpp"
#include "../predictors/predictorimg8.hpp"
#include "../predictors/predictorimg24.hpp"
#include "../predictors/predictortext.hpp"
#include "../predictors/predictorimg1.hpp"
#include "../predictors/predictoraudio.hpp"

#include "coder.hpp"
#include "../stream/streams.hpp"
#include "../analyzer/codec.hpp"

class Program {
public:
    CLI &cli;
    std::vector<Job> jobs;
    Segment segment; //for file segments type size info(if not -1)
    Mode mode;
    const std::string progname;
    Streams streams;
    std::string archiveName;
    File* archive;               // compressed file
    int files;                   // number of files to compress/decompress
    Array<const char*> fname{1};   // file names (resized to files)
    Array<uint64_t> fsize{1};           // file lengths (resized to files)
    uint16_t streambit;     
    ~Program();
    Program(CLI &c, const std::string);
    void List();
    void Compress();
    void CompressStreams(File *archive, uint16_t &streambit);
    void Decompress();
    void DecompressStreams(File *archive);
};
