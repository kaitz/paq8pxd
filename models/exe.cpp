#include "exe.hpp"


bool exeModel1::IsInvalidX64Op(U8 Op){
  for (int i=0; i<19; i++){
    if (Op == InvalidX64Ops[i])
      return true;
  }
  return false;
}

bool exeModel1::IsValidX64Prefix(U8 Prefix){
  for (int i=0; i<8; i++){
    if (Prefix == X64Prefixes[i])
      return true;
  }
  return ((Prefix>=0x40 && Prefix<=0x4F) || (Prefix>=0x64 && Prefix<=0x67));
}

void exeModel1::ProcessMode(Instruction &Op, ExeState &State){
  if ((Op.Flags&fMODE)==fAM){
    Op.Data|=AddressMode;
    Op.BytesRead = 0;
    switch (Op.Flags&fTYPE){
      case fDR : Op.Data|=(2<<TypeShift);
      case fDA : Op.Data|=(1<<TypeShift);
      case fAD : {
        State = Read32;
        break;
      }
      case fBR : {
        Op.Data|=(2<<TypeShift);
        State = Read8;
      }
    }
  }
  else{
    switch (Op.Flags&fTYPE){
      case fBI : State = Read8; break;
      case fWI : {
        State = Read16;
        Op.Data|=(1<<TypeShift);
        Op.BytesRead = 0;
        break;
      }
      case fDI : {
        // x64 Move with 8byte immediate? [REX.W is set, opcodes 0xB8+r]
        Op.imm8=((Op.REX & REX_w)>0 && (Op.Code&0xF8)==0xB8);
        if (!Op.o16 || Op.imm8){
          State = Read32;
          Op.Data|=(2<<TypeShift);
        }
        else{
          State = Read16;
          Op.Data|=(3<<TypeShift);
        }
        Op.BytesRead = 0;
        break;
      }
      default: State = Start; /*no immediate*/
    }
  }
}

void exeModel1::ProcessFlags2(Instruction &Op, ExeState &State){
  //if arriving from state ExtraFlags, we've already read the ModRM byte
  if ((Op.Flags&fMODE)==fMR && State!=ExtraFlags){
    State = ReadModRM;
    return;
  }
  ProcessMode(Op, State);
}

void exeModel1::ProcessFlags(Instruction &Op, ExeState &State){
  if (Op.Code==OP_CALLF || Op.Code==OP_JMPF || Op.Code==OP_ENTER){
    Op.BytesRead = 0;
    State = Read16_f;
    return; //must exit, ENTER has ModRM too
  }
  ProcessFlags2(Op, State);
}

void exeModel1::CheckFlags(Instruction &Op, ExeState &State){
  //must peek at ModRM byte to read the REG part, so we can know the opcode
  if (Op.Flags==fMEXTRA)
    State = ExtraFlags;
  else if (Op.Flags==fERR){
    memset(&Op, 0, sizeof(Instruction));
    State = Error;
  }
  else
    ProcessFlags(Op, State);
}

void exeModel1::ReadFlags(Instruction &Op, ExeState &State){
  Op.Flags = Table1[Op.Code];
  Op.Category = TypeOp1[Op.Code];
  CheckFlags(Op, State);
}

void exeModel1::ProcessModRM(Instruction &Op, ExeState &State){
  if ((Op.ModRM & ModRM_mod)==0x40)
    State = Read8_ModRM; //register+byte displacement
  else if ((Op.ModRM & ModRM_mod)==0x80 || (Op.ModRM & (ModRM_mod|ModRM_rm))==0x05 || (Op.ModRM<0x40 && (Op.SIB & SIB_base)==0x05) ){
    State = Read32_ModRM; //register+dword displacement
    Op.BytesRead = 0;
  }
  else
    ProcessMode(Op, State);
}

void exeModel1::ApplyCodeAndSetFlag(Instruction &Op, U32 Flag){
  Op.Data&=ClearCodeMask; \
  Op.Data|=(Op.Code<<CodeShift)|Flag;
}

inline U32 exeModel1::OpN(OpCache &Cache, U32 n){
  return Cache.Op[ (Cache.Index-n)&(CacheSize-1) ];
}

inline U32 exeModel1::OpNCateg(U32 &Mask, U32 n){
  return ((Mask>>(CategoryShift*(n-1)))&CategoryMask);
}

inline int exeModel1::pref(int i) { return (buf(i)==0x0f)+2*(buf(i)==0x66)+3*(buf(i)==0x67); }

// Get context at buf(i) relevant to parsing 32-bit x86 code
U32 exeModel1::execxt(int i, int x) {
  int prefix=0, opcode=0, modrm=0, sib=0;
  if (i) prefix+=4*pref(i--);
  if (i) prefix+=pref(i--);
  if (i) opcode+=buf(i--);
  if (i) modrm+=buf(i--)&(ModRM_mod|ModRM_rm);
  if (i&&((modrm&ModRM_rm)==4)&&(modrm<ModRM_mod)) sib=buf(i)&SIB_scale;
  return prefix|opcode<<4|modrm<<12|x<<20|sib<<(28-6);
}

  exeModel1::exeModel1(BlockData& bd,U32 val):x(bd),buf(bd.buf),N1(10), N2(10),cm(CMlimit(MEM()*4), N1+N2,M_EXE,
  CM_MR+
  CM_RUN0+
  CM_MAIN1+
  CM_MAIN2+
  CM_MAIN3+
  CM_MAIN4+
  CM_M12+
  CM_M6
  ),iMap(20,1),
  pState (Start), State( Start), TotalOps(0), OpMask(0),OpCategMask(0), Context(0),BrkPoint(0),
  BrkCtx(0),Valid(false) {
      memset(&Cache, 0, sizeof(OpCache));
      memset(&Op, 0, sizeof(Instruction));
      memset(&StateBH, 0, sizeof(StateBH));
  }

int exeModel1::p(Mixer& m,int val1,int val2){
    if (x.filetype==DBASE ||x.filetype==HDR || x.filetype==ARM|| x.filetype==IMGUNK){
        for (int i=0; i<inputs(); i++) m.add(0); 
        return false;
    }
  if (x.bpos==0 ) {
    pState = State;
    U8 B = (U8)x.c4;
    Op.Size++;
    switch (State){
      case Start: case Error: {
        // previous code may have just been a REX prefix
        bool Skip = false;
        if (Op.MustCheckREX){
          Op.MustCheckREX = false;
          // valid x64 code?
          if (!IsInvalidX64Op(B) && !IsValidX64Prefix(B)){
            Op.REX = Op.Code;
            Op.Code = B;
            Op.Data = PrefixREX|(Op.Code<<CodeShift)|(Op.Data&PrefixMask); 
            Skip = true;
          }
        }
        
        Op.ModRM = Op.SIB = Op.REX = Op.Flags = Op.BytesRead = 0;       
        if (!Skip){
          Op.Code = B;
          // possible REX prefix?
          Op.MustCheckREX = ((Op.Code&0xF0)==0x40) && (!(Op.Decoding && ((Op.Data&PrefixMask)==1)));
          
          // check prefixes
          Op.Prefix = (Op.Code==ES_OVERRIDE || Op.Code==CS_OVERRIDE || Op.Code==SS_OVERRIDE || Op.Code==DS_OVERRIDE) + //invalid in x64
                      (Op.Code==FS_OVERRIDE)*2 +
                      (Op.Code==GS_OVERRIDE)*3 +
                      (Op.Code==AD_OVERRIDE)*4 +
                      (Op.Code==WAIT_FPU)*5 +
                      (Op.Code==LOCK)*6 +
                      (Op.Code==REP_N_STR || Op.Code==REP_STR)*7;

          if (!Op.Decoding){
            TotalOps+=(Op.Data!=0)-(Cache.Index && Cache.Op[ Cache.Index&(CacheSize-1) ]!=0);
            OpMask = (OpMask<<1)|(State!=Error);
            OpCategMask = (OpCategMask<<CategoryShift)|(Op.Category);
            Op.Size = 0;
            
            Cache.Op[ Cache.Index&(CacheSize-1) ] = Op.Data;
            Cache.Index++;

            if (!Op.Prefix)
              Op.Data = Op.Code<<CodeShift;
            else{
              Op.Data = Op.Prefix;
              Op.Category = TypeOp1[Op.Code];
              Op.Decoding = true;
              BrkCtx = hash(1+(BrkPoint = 0), Op.Prefix, OpCategMask&CategoryMask);
              break;
            }
          }
          else{
            // we only have enough bits for one prefix, so the
            // instruction will be encoded with the last one
            if (!Op.Prefix){
              Op.Data|=(Op.Code<<CodeShift);
              Op.Decoding = false;
            }
            else{
              Op.Data = Op.Prefix;
              Op.Category = TypeOp1[Op.Code];
              BrkCtx = hash(1+(BrkPoint = 1), Op.Prefix, OpCategMask&CategoryMask);
              break;
            }
          }
        }

        if ((Op.o16=(Op.Code==OP_OSIZE)))
          State = Pref_Op_Size;
        else if (Op.Code==OP_2BYTE)
          State = Pref_MultiByte_Op;
        else
          ReadFlags(Op, State);
        BrkCtx = hash(1+(BrkPoint = 2), State, Op.Code, (OpCategMask&CategoryMask), OpN(Cache,1)&((ModRM_mod|ModRM_reg|ModRM_rm)<<ModRMShift));
        break;
      }
      case Pref_Op_Size : {
        Op.Code = B;
        ApplyCodeAndSetFlag(Op, OperandSizeOverride);
        ReadFlags(Op, State);
        BrkCtx = hash(1+(BrkPoint = 3), State);
        break;
      }
      case Pref_MultiByte_Op : {
        Op.Code = B;        
        Op.Data|=MultiByteOpcode;

        if (Op.Code==0x38)
          State = Read_OP3_38;
        else if (Op.Code==0x3A)
          State = Read_OP3_3A;
        else{
          ApplyCodeAndSetFlag(Op);
          Op.Flags = Table2[Op.Code];
          Op.Category = TypeOp2[Op.Code];
          CheckFlags(Op, State);
        }
        BrkCtx = hash(1+(BrkPoint = 4), State);
        break;
      }
      case ParseFlags : {
        ProcessFlags(Op, State);
        BrkCtx = hash(1+(BrkPoint = 5), State);
        break;
      }
      case ExtraFlags : case ReadModRM : {
        Op.ModRM = B;
        Op.Data|=(Op.ModRM<<ModRMShift)|HasModRM;
        Op.SIB = 0;
        if (Op.Flags==fMEXTRA){
          Op.Data|=HasExtraFlags;
          int i = ((Op.ModRM>>3)&0x07) | ((Op.Code&0x01)<<3) | ((Op.Code&0x08)<<1);
          Op.Flags = TableX[i];
          Op.Category = TypeOpX[i];
          if (Op.Flags==fERR){
            memset(&Op, 0, sizeof(Instruction));
            State = Error;
            BrkCtx = hash(1+(BrkPoint = 6), State);
            break;
          }
          ProcessFlags(Op, State);
          BrkCtx = hash(1+(BrkPoint = 7), State);
          break;
        }

        if ((Op.ModRM & ModRM_rm)==4 && Op.ModRM<ModRM_mod){
          State = ReadSIB;
          BrkCtx = hash(1+(BrkPoint = 8), State);
          break;
        }

        ProcessModRM(Op, State);
        BrkCtx = hash(1+(BrkPoint = 9), State, Op.Code );
        break;
      }
      case Read_OP3_38 : case Read_OP3_3A : {
        Op.Code = B;
        ApplyCodeAndSetFlag(Op, Prefix38<<(State-Read_OP3_38));
        if (State==Read_OP3_38){
          Op.Flags = Table3_38[Op.Code];
          Op.Category = TypeOp3_38[Op.Code];
        }
        else{
          Op.Flags = Table3_3A[Op.Code];
          Op.Category = TypeOp3_3A[Op.Code];
        }
        CheckFlags(Op, State);
        BrkCtx = hash(1+(BrkPoint = 10), State);
        break;
      }
      case ReadSIB : {
        Op.SIB = B;
        Op.Data|=((Op.SIB & SIB_scale)<<SIBScaleShift);
        ProcessModRM(Op, State);
        BrkCtx = hash(1+(BrkPoint = 11), State, Op.SIB&SIB_scale);
        break;
      }
      case Read8 : case Read16 : case Read32 : {
        if (++Op.BytesRead>=((State-Read8)<<int(Op.imm8+1))){
          Op.BytesRead = 0;
          Op.imm8 = false;
          State = Start;
        }
        BrkCtx = hash(1+(BrkPoint = 12), State, Op.Flags&fMODE, Op.BytesRead, ((Op.BytesRead>1)?(buf(Op.BytesRead)<<8):0)|((Op.BytesRead)?B:0) );
        break;
      }
      case Read8_ModRM : {
        ProcessMode(Op, State);
        BrkCtx = hash(1+(BrkPoint = 13), State);
        break;
      }
      case Read16_f : {
        if (++Op.BytesRead==2){
          Op.BytesRead = 0;
          ProcessFlags2(Op, State);
        }
        BrkCtx = hash(1+(BrkPoint = 14), State);
        break;
      }
      case Read32_ModRM : {
        Op.Data|=RegDWordDisplacement;
        if (++Op.BytesRead==4){
          Op.BytesRead = 0;
          ProcessMode(Op, State);
        }
        BrkCtx = hash(1+(BrkPoint = 15), State);
        break;
      }
    }

    Valid = (TotalOps>2*MinRequired) && ((OpMask&((1<<MinRequired)-1))==((1<<MinRequired)-1));
    Context = State+16*Op.BytesRead+16*(Op.REX & REX_w);
    StateBH[Context] = (StateBH[Context]<<8)|B;

    if (Valid || val1){
      int mask=0, count0=0;
      int i=0;
      while (i<N1){
        if (i>1) mask=mask*2+(buf(i-1)==0), count0+=mask&1;
        int j=(i<4)?i+1:5+(i-4)*(2+(i>6));
        cm.set(hash(execxt(j, buf(1)*(j>6)), ((1<<N1)|mask)*(count0*N1/2>=i), (0x08|(x.blpos&0x07))*(i<4)));
        i++;
      }

      cm.set(BrkCtx);
      mask = PrefixMask|(0xF8<<CodeShift)|MultiByteOpcode|Prefix38|Prefix3A;
      cm.set(hash(OpN(Cache, 1)&(mask|RegDWordDisplacement|AddressMode), State+16*Op.BytesRead, Op.Data&mask, Op.REX, Op.Category));
      
      mask = 0x04|(0xFE<<CodeShift)|MultiByteOpcode|Prefix38|Prefix3A|((ModRM_mod|ModRM_reg)<<ModRMShift);
      cm.set(hash(
        OpN(Cache, 1)&mask, OpN(Cache, 2)&mask, OpN(Cache, 3)&mask,
        Context+256*((Op.ModRM & ModRM_mod)==ModRM_mod),
        Op.Data&((mask|PrefixREX)^(ModRM_mod<<ModRMShift))
      ));
     
      mask = 0x04|CodeMask;
      cm.set(hash(OpN(Cache, 1)&mask, OpN(Cache, 2)&mask, OpN(Cache, 3)&mask, OpN(Cache, 4)&mask, (Op.Data&mask)|(State<<11)|(Op.BytesRead<<15)));

      mask = 0x04|(0xFC<<CodeShift)|MultiByteOpcode|Prefix38|Prefix3A;
      cm.set(hash(State+16*Op.BytesRead, Op.Data&mask, Op.Category*8 + (OpMask&0x07), Op.Flags, ((Op.SIB & SIB_base)==5)*4+((Op.ModRM & ModRM_reg)==ModRM_reg)*2+((Op.ModRM & ModRM_mod)==0)));

      mask = PrefixMask|CodeMask|OperandSizeOverride|MultiByteOpcode|PrefixREX|Prefix38|Prefix3A|HasExtraFlags|HasModRM|((ModRM_mod|ModRM_rm)<<ModRMShift);
      cm.set(hash(Op.Data&mask, State+16*Op.BytesRead, Op.Flags));

      mask = PrefixMask|CodeMask|OperandSizeOverride|MultiByteOpcode|Prefix38|Prefix3A|HasExtraFlags|HasModRM;
      cm.set(hash(OpN(Cache, 1)&mask, State, Op.BytesRead*2+((Op.REX&REX_w)>0), Op.Data&((U16)(mask^OperandSizeOverride))));

      mask = 0x04|(0xFE<<CodeShift)|MultiByteOpcode|Prefix38|Prefix3A|(ModRM_reg<<ModRMShift);
      cm.set(hash(OpN(Cache, 1)&mask, OpN(Cache, 2)&mask, State+16*Op.BytesRead, Op.Data&(mask|PrefixMask|CodeMask)));

      cm.set(hash(++i,State+16*Op.BytesRead));

      cm.set(hash(
        (0x100|B)*(Op.BytesRead>0),
        State+16*pState+256*Op.BytesRead,
        ((Op.Flags&fMODE)==fAM)*16 + (Op.REX & REX_w) + (Op.o16)*4 + ((Op.Code & 0xFE)==0xE8)*2 + ((Op.Data & MultiByteOpcode)!=0 && (Op.Code & 0xF0)==0x80)
      ));
      
    }
  }
  if (Valid || val1){
    int  cnx=m.nx;
    cm.mix(m, 1,4);
    int count=(m.nx-cnx)/(N1+N2);
    m.nx=cnx;
    for (int i=count*(N1+N2);i!=0;--i) m.sp(5); // increase all

    iMap.set(hash(BrkCtx, x.bpos));
    if(x.filetype==EXE ) iMap.mix(m,1,4); 
    else m.add(0),m.add(0);
    x.x86_64.state = 0x80u | (static_cast<std::uint8_t>(State) << 3u) | Op.BytesRead;
  }else{
      x.x86_64.state=0;
      for (int i=0; i<inputs(); ++i)
        m.add(0);
  }
  U8 s = ((StateBH[Context]>>(28-x.bpos))&0x08) |
         ((StateBH[Context]>>(21-x.bpos))&0x04) |
         ((StateBH[Context]>>(14-x.bpos))&0x02) |
         ((StateBH[Context]>>( 7-x.bpos))&0x01) |
         ((Op.Category==OP_GEN_BRANCH)<<4)|
         (((x.c0&((1<<x.bpos)-1))==0)<<5);

  m.set((Context*4+(s>>4)), 1024);
  m.set((State*64+x.bpos*8+(Op.BytesRead>0)*4+(s>>4)), 1024);
  m.set( (BrkCtx&0x1FF)|((s&0x20)<<4), 1024 );
  m.set(hash(Op.Code, State, OpN(Cache, 1)&CodeMask)&0x1FFF, 8192);
  m.set(hash(State, x.bpos, Op.Code, Op.BytesRead)&0x1FFF, 8192);
  m.set(hash(State, (x.bpos<<2)|(x.c0&3), OpCategMask&CategoryMask, ((Op.Category==OP_GEN_BRANCH)<<2)|(((Op.Flags&fMODE)==fAM)<<1)|(Op.BytesRead>0))&0x1FFF, 8192);
  
  return Valid;
}




