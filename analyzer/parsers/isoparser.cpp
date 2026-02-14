#include "isoparser.hpp"

ISO9960Parser::ISO9960Parser():isoF(0) {
    priority=2;
    Reset();
    inpos=0;
    name="ISO9960";
}

ISO9960Parser::~ISO9960Parser() {
}

// loop over input block byte by byte and report state
DetectState ISO9960Parser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && rec==false && len<(25*2048)) return DISABLE; // min 25 sectors
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
        if (rec && state==END && isoFiles==0) state=NONE,priority=2,rec=false; // revusive mode ended
    }
    
    while (inSize<len && rec==false) {
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && i>0x8000 &&(buf1&0xffffff)==0x014344 && buf0==0x30303101) { 
            state=INFO;
            iso=relAdd=i-(0x8000+6);
            jstart=relAdd;
            sectcount=16;
            if (inSize>=7) for (size_t j=7; j>0; j--) sector[7-j]=data[inSize+1-j];
            sectorpos=7;
        }else if (state==INFO && sectorpos<2048) {
            sector[sectorpos++]=c;
            
            if (state==INFO && sectorpos==2048 && volterm==false) {
                i9660_vd &vd=(i9660_vd&)sector[0];
                if (vd.type==1 && rootdir==0) {
                    // Primary Volume Descriptor
                    //printf("Primary Volume Descriptor, sector %d \n",sectcount);
                    uint16_t &lbs=(uint16_t&)vd.logical_block_size.le[0];
                    //printf("Sector size %d\n",lbs); //2048
                    uint32_t &rts=(uint32_t&)vd.root_dir.sector.le[0];
                    //printf("Root dir LBA %d\n",rts);
                    //uint32_t &rtss=(uint32_t&)vd.root_dir.size.le[0];
                    //printf("Root size %d\n",rtss);
                    rootdir=rts;
                    isoFiles=0;
                    if (lbs!=2048 || last==true) sectcount=sectorpos=rootdir=rootdirsup=0,state=NONE;
                } else if (vd.type==2 && rootdirsup==0) {
                    // Supplementary Volume Descriptor
                    //printf("Supplementary Volume Descriptor, sector %d \n",sectcount);
                    uint16_t &lbs=(uint16_t&)vd.logical_block_size.le[0];
                    //printf("Sector size %d\n",lbs); //2048
                    uint32_t &rts=(uint32_t&)vd.root_dir.sector.le[0];
                    //printf("Root dir LBA %d\n",rts);
                    //uint32_t &rtss=(uint32_t&)vd.root_dir.size.le[0];
                    //printf("Root size %d\n",rtss);
                    rootdirsup=rts;
                    isoFiles=0;
                    if (lbs!=2048 || last==true) sectcount=sectorpos=rootdir=rootdirsup=0,state=NONE;
                } else if (vd.type==1||vd.type==2||vd.type==3||vd.type==0||vd.type==255) {
                    if (vd.type==255) volterm=true;
                } else {
                    sectcount=sectorpos=rootdir=rootdirsup=0,state=NONE;
                }
                sectorpos%=2048;
                sectcount++;
            } else if (state==INFO && sectorpos==2048 && sectcount>=rootdir && sectcount!=rootdirsup) {
                int dirlenght=0;
                bool wrongs=false; // is wrong sector?
                // Ignore Supplementary root dir
                if (sectorl.size()>0 && rootdirsup) {
                    std::set<uint32_t>::iterator pos;
                    pos=sectorl.find(sectcount);
                    if (pos==sectorl.end() && sectcount>=rootdirsup) {
                        wrongs=true;
                    } else {
                        sectorl.erase(sectcount);
                    }
                }
                // ISO9660 Extensions - SUSP 
                //CE: Continuation area
                //PD: Padding field
                //SP: Sharing protocol indicator
                //ST: Sharing protocol terminator
                //ER: Extensions reference
                //ES: Extension selector
                // ignore sectors if SUSP
                // maybe ignere if >255 ?
                uint16_t &rr=(uint16_t&)sector[0];
                if (rr==0x4543 || rr== 0x4450 || rr== 0x5053 || rr== 0x5453 || rr== 0x5245 || rr== 0x5345) wrongs=true;
                if (wrongs==false) {
                    do {
                        i9660_dir &dent=(i9660_dir&)sector[dirlenght];
                        if (dent.length==0 || (uint32_t&)dent.sector.le[0]==0) {
                            dirlenght+=12+sizeof(i9660_dir);
                            // Spans multile sectors?
                            if (dirlenght>=2048) sectorl.insert(sectcount+1);
                            break;
                        } 
                        
                        // Add files
                        if ((dent.flags&2)!=2 && (uint32_t&)dent.size.le[0]>8 && dent.xattr_length==0) {
                            // Process files with lenght >8 and ignore any file with xattr_length
                            // Get file name, it may end with ;1, if so remove it. If '.' is left then also remove
                            uint32_t nlen=dent.name_len;
                            std::string fname="";
                            char *name=(char*)&dent.name;
                            for (int j=0; j<nlen; j++) fname+=name[j];
                            //printf("%d %d %s\n",(uint32_t&)dent.sector.le[0],(uint32_t&)dent.size.le[0],fname.c_str());
                            if (fname.size()>3 && fname.substr(fname.size()-2,2)==";1") fname.pop_back(),fname.pop_back();
                            if (fname.size()>2 && fname.substr(fname.size()-1,1)==".") fname.pop_back();
                            ISOfile tf;
                            tf.start=(uint32_t&)dent.sector.le[0];
                            tf.start=tf.start*2048+(isoFiles==0?iso:0);
                            tf.size=(uint32_t&)dent.size.le[0];
                            ParserType etype=GetTypeFromExt(fname);
                            tf.p=etype;
                            isoF.push_back(tf);
                            isoFiles++;
                        }
                        /*
                        int32_t nlen=dent.name_len;
                        std::string fname="";
                        char *name=(char*)&dent.name;
                        for (int j=0; j<nlen; j++) fname+=name[j];
                        if ((dent.flags&2)==2)    
                        printf("%s %d %d %d %s\n",(dent.flags&2)!=2?"F":"D",dent.length,(uint32_t&)dent.sector.le[0],(uint32_t&)dent.size.le[0],fname.c_str());
                        */
                        // Collect info about directorys
                        if ((dent.flags&2)==2 && dent.name!=1&& dent.name!=0 && sectcount!=(uint32_t&)dent.sector.le[0] && dent.xattr_length==0) {
                            // add subdir
                            sectorl.insert((uint32_t&)dent.sector.le[0]);
                        } else if ((dent.flags&2)==2 && rootdirsup==0) {
                            // remove subdir
                            std::set<uint32_t>::iterator pos;
                            uint32_t elf=(uint32_t&)dent.sector.le[0];
                            // Is current sector subdir is same then remove
                            pos=sectorl.find(elf);
                            if (pos!=sectorl.end()) {
                                sectorl.erase(elf);
                            }
                        }
                        dirlenght=dirlenght+dent.length;
                        if (dent.length==0) dirlenght=0;
                    } while (dirlenght);
                    // To be sure we remove current sector from dir list.
                    std::set<uint32_t>::iterator pos;
                    pos=sectorl.find(sectcount);
                    if (pos!=sectorl.end()) {
                        sectorl.erase(sectcount);
                    }
                    //printf(" Dirs: %d, files: %d\n",sectorl.size(),isoF.size());
                }
                if (state==INFO) jend=i;
                if (sectorl.size()==0) {
                    // All dirs processed, signal end
                    sectcount=sectorpos=rootdir=rootdirsup=0,state=NONE;
                    jstart=0;
                    jend=0;
                    state=END;
                    rec=true;
                    // sort
                    // file sectors my be out of order
                    int n=isoF.size();
                    //printf("Total files: %d\n",n);
                    pinfo="ISO9960 - files " +itos(n);
                    for (int i=0; i<n-1; i++) {
                        for (int j=0; j<n-i-1; j++) {
                            if (isoF[j].start>isoF[j+1].start){
                                ISOfile tmp=isoF[j+1];
                                isoF[j+1]=isoF[j];
                                isoF[j]=tmp;
                            }
                        }
                    }
                }
                sectorpos=0;
                sectcount++;
            } else if (state==INFO && sectorpos==2048) {
                sectorpos%=2048;
                sectcount++;
            }
        }

        inSize++;
        i++;
    }
    if (rec && state==END && isoFiles) { 
        // recursive mode, report all iso file ranges
        // file extension based type parser set in info
        ISOfile isofile=isoF[isoF.size()-isoFiles];
        jstart=isofile.start-(relAdd-iso);
        jend=jstart+isofile.size;
        //printf("File %d %d %d\n",isoF.size()-isoFiles,jstart,jend);
        relAdd+=jend-iso;
        relAdd-=iso;
        type=RECE;
        info=isofile.p;
        iso=0;
        if ((isoF.size()-isoFiles)==1) pinfo="";
        isoFiles--;
        if (isoFiles==0) isoF.clear(),rec=false;
        return state;        
    }
    if (state==INFO) {jend=i+1; return INFO;}
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType ISO9960Parser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=rec;
    return t;
}

void ISO9960Parser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    iso=0,sectcount=0,rootdir=0,rootdirsup=0;
    relAdd=0;
    priority=2;
    isoFiles=0; 
    isoF.clear();
    info=i=inSize=0;
    rec=false;
    volterm=false;
}
