#pragma once
#include <vector>
#include <string>

typedef enum {
    CL_LIST=0,
    CL_DECOMPRESS,
    CL_EXTREME,
    CL_SLOW,
    CL_FAST,  // not used
    CL_STORE,
    CL_THREADS,
    CL_WIKI,
    CL_EDICT,
    CL_DICTF,
    CL_RECUR,
    CL_VERBOSE,
    CL_HELP,
    CL_PARSER,
    CL_UNK,
    CL_LAST
} CliType;

struct CliCommand {
    CliType size;
    std::string cmd;
    std::string valstr;
    uint64_t    val;
    CliType     type;
    CliCommand():cmd(""),valstr(""),val(9),type(CL_UNK) {
    }
};

class CLI {
    std::vector<CliCommand> cmd;
    bool isCommand;
    size_t cmdIndex;
public:
    std::vector<std::string> files;
        CLI():isCommand(false),cmdIndex(0) { }
        bool CError(std::string err) {
            printf("Bad command line parameter: %s\n\n",err.c_str());
            return false;
        }
        CliCommand GetCommand(CliType c) {
            for (uint64_t j=0; j<cmd.size(); j++) {
                if (cmd[j].type==c) {
                    return cmd[j];
                }
            }
            CliCommand ce;
            ce.type=CL_UNK;
            return ce;
        }
        CliCommand GetCommand() {
            if (cmdIndex<cmd.size()) return cmd[cmdIndex++];
            else {
                cmdIndex=0;
                CliCommand ce;
                ce.type=CL_UNK;
                return ce;
            }
        }
        ParserType isValidParser(std::string &p) {
            if (p=="P_BMP") return P_BMP;
            else if (p=="P_TXT") return P_TXT;
            else if (p=="P_DECA") return P_DECA;
            else if (p=="P_MRB") return P_MRB;
            else if (p=="P_EXE") return P_EXE;
            else if (p=="P_NES") return P_NES;
            else if (p=="P_MZIP") return P_MZIP;
            else if (p=="P_JPG") return P_JPG;
            else if (p=="P_PNM") return P_PNM;
            else if (p=="P_PLZW") return P_PLZW;
            else if (p=="P_GIF") return P_GIF;
            else if (p=="P_DBS") return P_DBS;
            else if (p=="P_AIFF") return P_AIFF;
            else if (p=="P_A85") return P_A85;
            else if (p=="P_B641") return P_B641;
            else if (p=="P_B642") return P_B642;
            else if (p=="P_MOD") return P_MOD;
            else if (p=="P_SGI") return P_SGI;
            else if (p=="P_TGA") return P_TGA;
            else if (p=="P_ICD") return P_ICD;
            else if (p=="P_MDF") return P_MDF;
            else if (p=="P_UUE") return P_UUE;
            else if (p=="P_TIFF") return P_TIFF;
            else if (p=="P_TAR") return P_TAR;
            else if (p=="P_PNG") return P_PNG;
            else if (p=="P_ZIP") return P_ZIP;
            else if (p=="P_GZIP") return P_GZIP;
            else if (p=="P_BZIP2") return P_BZIP2;
            else if (p=="P_SZDD") return P_SZDD;
            else if (p=="P_MSCF") return P_MSCF;
            else if (p=="P_ZLIB") return P_ZLIB;
            else if (p=="P_ZLIBP") return P_ZLIBP;
            else if (p=="P_ISO9960") return P_ISO9960;
            else if (p=="P_ISCAB") return P_ISCAB;
            else if (p=="P_PBIT") return P_PBIT;
            else return P_LAST;
        }
        bool Parse(int argc, char** argv) {
            // load commands
            argv++, argc--;
            if (argc==0) return false;
            bool opDone=false;
            for (uint64_t j=0; j<argc; j++) {
                char* aopt=&argv[j][0];
                if (aopt[0]=='-' && opDone==false) {  // options
                    CliCommand c;
                    const std::string command=aopt;
                    const std::string opt=command.substr(0,2);
                    c.cmd=opt;
                    c.valstr=command;
                    c.val=0;
                    //printf("%d \n",command.size());
                    if (opt=="-d") {       // decompress
                       if (command.size()!=2) return CError(command);
                        c.type=CL_DECOMPRESS;
                    } else if (opt=="-l") { // list
                        if (command.size()!=2) return CError(command);
                        c.type=CL_LIST;
                    } else if (opt=="-w") { // wiki transform
                        if (command.size()!=2) return CError(command);
                        c.type=CL_WIKI;
                    } else if (opt=="-e") { // external dict
                        c.type=CL_EDICT;
                        c.valstr=&argv[j][2];
                        if (command.size()==2 || c.valstr.size()==0 ) return CError(command);
                        FILE *f=fopen(c.valstr.c_str(),"rb");
                        if (f==NULL) return CError(command+"\n File not found.");
                        else fclose(f);
                    } else if (opt=="-p") { // user defined parser
                        c.type=CL_PARSER;
                        c.valstr=&argv[j][2];
                        if (command.size()==2 || c.valstr.size()==0 ) return CError(command);
                        ParserType pt=isValidParser(c.valstr);
                        if (pt==P_LAST) return CError(command);
                        c.val=pt;
                    } else if (opt=="-q") { // word freq
                        c.val=atol(&argv[j][2]);
                        if (c.val<0) return CError(command);
                        c.type=CL_DICTF;
                    } else if (opt=="-s") { // slow
                        c.val=atol(&argv[j][2]);
                        if (c.val>15 || c.val<0) return CError(command);
                        if (c.val==0) c.type=CL_STORE; else c.type=CL_SLOW;
                    } else if (opt=="-x") { //extreme
                        c.val=atol(&argv[j][2]);
                        if (c.val>15 || c.val<0) return CError(command);
                        if (c.val==0) c.type=CL_STORE; else c.type=CL_EXTREME;
                    } else if (opt=="-v") { // verbose
                        c.val=atol(&argv[j][2]);
                        if (c.val>3 || c.val<0) return CError(command);
                        c.type=CL_VERBOSE;
                    } else if (opt=="-r") { // recursion
                        c.val=atol(&argv[j][2]);
                        if (c.val>9 || c.val<0) return CError(command);
                        c.type=CL_RECUR;
                    } else if (opt=="-h") { // help
                        if (command.size()!=2) return CError(command);
                        c.type=CL_HELP;
                    } else if (opt=="-t") { // threads
                        c.val=atol(&argv[j][2]);
                        if (c.val>4 || c.val<1) return CError(command);
                        c.type=CL_THREADS;
                    } else {
                        return CError(command);
                    }
                    //printf("Command %d: option %s (%s), %d\n",c.type,c.cmd.c_str(),c.valstr.c_str(),c.val);
                    cmd.push_back(c);
                } else {  // input &| archive
                    opDone=true;
                    std::string file=&argv[j][0];
                    //printf("File: %s\n",file.c_str());
                    files.push_back(file);
                }
            }
            if (files.size()==0) {
                CliCommand c=GetCommand(CL_HELP);
                if (c.type==CL_UNK) {
                    c.type=CL_HELP;
                    cmd.push_back(c);
                }
                return true;
            }
            
            // Speacial case for compression/decompression
            if (files.size()==1) { // input | archive
                bool isDecompress=false;
                bool isArchive=false;
                CliCommand c;
                // Is there a decompress command?
                for (uint64_t j=0; j<cmd.size(); j++) {
                    if (cmd[j].type==CL_DECOMPRESS) {
                        isDecompress=true;
                        break;
                    }
                }
                size_t lastdot=files[0].find_last_of(".");
                    if (lastdot!=std::string::npos && files[0].size()!=lastdot) {
                        std::string ext=files[0].substr(lastdot + 1);
                        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
                        if (ext==std::string(PROGNAME)) {
                            isArchive=true;
                        }
                }
                // Test if input is archive and if so add decompress command
                // Otherwise compress with default level
                if (isArchive) {
                    // This overrides all command line options
                    // and defaults to decompressin/compare
                    while (cmd.size()) cmd.pop_back();
                    c.type=CL_DECOMPRESS;
                    c.cmd="-d";
                    c.valstr="";
                    c.val=0;
                    cmd.push_back(c);
                } else if (isArchive==false && argc==1) {
                    // Input not archive. Compress with default level 8
                    c.type=CL_SLOW;
                    c.cmd="-s";
                    c.valstr="";
                    c.val=8;
                    cmd.push_back(c);
                } else if (isArchive==false && isDecompress) {
                    return CError("Input not a "+std::string(PROGNAME)+" archive.");
                }
            }
            //printf("Commands %d, files %d\n",cmd.size(),files.size());
            return true; // all good
        }
};
