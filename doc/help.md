

## Command line parameters
### Usage
Compress with slow method:    

    paq8pxdN -s[level] file             (compresses to file.paq8pxdN)    
    paq8pxdN -s[level] archive files... (creates archive.paq8pxdN)    
    paq8pxdN file                       (level -s8 pause when done)    

N is program version number.

### Commands
#### Command -{s|x}[level]
Compression mode s - slow or x - extreme.   
Level selects memory usage, range 0-15.   
Default level is 8.   
If level is omitted, default is 0 (store).   

Memory usage (level):

            -{s|x}0               store (no compression)
                                  (These values are ~ and may change between versions)
            -{s|x}1...-{s|x}3     uses 393, 398, 409 MB
            -{s|x}4...-{s|x}9     uses 1.2  1.3  1.5  1.9 2.7 4.9 GB
            -{s|x}10...-{s|x}15   uses 7.0  9.0 11.1 27.0   x.x x.x GB

#### Command -d
Decompress archive. If target exist then compare.

#### Command -l
List files in archive.

#### Command -r{n}
Recursion depth. Range 0-9. Default 6.

#### Command -v{n}
Set verbose level to n. Range 0-3. Defaul 0.

#### Command -h
Show help. Use command -v for more info.

#### Command -t{n}
Select number of threads when compressing.    
Valid range 1-4. Default 1.

#### Command -e{name}
Use external dictionary: name.

#### Command -q{frq}
Set minimum frequency for dictionary transform.    
Default 19.

#### Command -w
Preprocces wikipedia xml dump with transform before dictionary transform.

#### Command -p{name}
Enable only parser: name

List of allowed parsers:

            P_BMP,  P_TXT,  P_DECA,  P_MRB,     P_EXE,   P_NES,
            P_MZIP, P_JPG,  P_WAV,   P_PNM,     P_PLZW,  P_GIF,
            P_DBS,  P_AIFF, P_A85,   P_B641,    P_B642,  P_MOD,
            P_SGI,  P_TGA,  P_ICD,   P_MDF,     P_UUE,   P_TIFF,
            P_TAR,  P_PNG,  P_ZIP,   P_GZIP,    P_BZIP2, P_SZDD,
            P_MSCF, P_ZLIB, P_ZLIBP, P_ISO9960, P_ISCAB, P_PBIT,
            P_ARM


          
