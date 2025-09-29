#include "dec.hpp"

  decModel1::decModel1(BlockData& bd,U32 val):x(bd),buf(bd.buf),
   cache(8), op{0}, state(OpCode), count(0u), lastRc(0xFF), last{0},
    maps0 {
      { 1, 6, 256, 255}, // OpCode
      { 1, 5, 256, 255}, // Bra_Ra
      { 2, 7, 256, 255}, // Bra_Displacement
      { 2, 3, 256, 255}, // F_P_Function
      { 1, 5, 256, 255}, // F_P_Ra
      { 1, 5, 256, 255}, // F_P_Rb
      { 1, 5, 256, 255}, // F_P_Rc
      { 1, 5, 256, 255}, // Mem_Ra
      { 1, 5, 256, 255}, // Mem_Rb
      { 2, 4, 256, 255}, // Mem_Displacement
      { 2, 4, 512, 255}, // Mfc_Function
      { 1, 5, 512, 255}, // Mfc_Ra
      { 1, 5, 512, 255}, // Mfc_Rb
      { 1, 5, 512, 255}, // Mbr_Ra
      { 1, 5, 256, 255}, // Mbr_Rb
      { 2, 4, 256, 255}, // Mbr_Displacement
      { 1, 1, 256, 255}, // Opr_Bit
      { 1, 7, 256, 255}, // Opr_Function
      { 1, 5, 256, 255}, // Opr_Ra
      { 1, 5, 256, 255}, // Opr_Rb
      { 1, 3, 512, 255}, // Opr_Unused
      { 1, 8, 256, 255}, // Opr_Literal
      { 1, 5, 256, 255}, // Opr_Rc
      { 4, 3, 512, 255}, // Pcd_Function
      { 4, 3, 512, 255}  // Nop_Skip
    },
    maps1 {
      { 11, 6, 256, 255}, // OpCode
      {  5, 5, 256, 255}, // Bra_Ra
      { 14, 7, 256, 255}, // Bra_Displacement
      { 14, 3, 256, 255}, // F_P_Function
      {  5, 5, 256, 255}, // F_P_Ra
      { 10, 5, 256, 255}, // F_P_Rb
      { 10, 5, 256, 255}, // F_P_Rc
      {  5, 5, 256, 255}, // Mem_Ra
      {  5, 5, 256, 255}, // Mem_Rb
      { 16, 4, 256, 255}, // Mem_Displacement
      {  1, 5, 256, 255}, // Mbr_Rb
      {  3, 4, 256, 255}, // Mbr_Displacement
      {  3, 1, 256, 255}, // Opr_Bit
      {  4, 7, 256, 255}, // Opr_Function
      {  5, 5, 256, 255}, // Opr_Ra
      { 10, 5, 256, 255}, // Opr_Rb
      { 10, 8, 256, 255}, // Opr_Literal
      {  6, 5, 256, 255}  // Opr_Rc
    },
    maps2 {
      { 16, 6, 256, 255}, // OpCode
      { 10, 5, 256, 255}, // Bra_Ra
      { 15, 7, 256, 255}, // Bra_Displacement
      { 10, 5, 256, 255}, // Mem_Ra
      { 10, 5, 256, 255}, // Mem_Rb
      { 18, 4, 256, 255}, // Mem_Displacement
      { 11, 1, 256, 255}, // Opr_Bit
      { 16, 7, 256, 255}, // Opr_Function
      {  5, 5, 256, 255}, // Opr_Ra
      { 10, 5, 256, 255}, // Opr_Rb
      { 14, 8, 256, 255}, // Opr_Literal
      { 16, 5, 256, 255}  // Opr_Rc
    },
    maps3{
      { 16, 6, 256, 255}, // OpCode
      { 16, 5, 256, 255}, // Bra_Ra
      { 15, 5, 256, 255}, // Mem_Ra
      { 15, 5, 256, 255}, // Mem_Rb
      { 18, 4, 256, 255}, // Mem_Displacement
      { 16, 7, 256, 255}, // Opr_Function
      { 10, 5, 256, 255}, // Opr_Ra
      { 15, 5, 256, 255},  // Opr_Rb
      { 11, 5, 256, 255}  // Opr_Rc
    },
    maps4{
      { 16, 6, 256, 255}, // OpCode
      { 16, 5, 256, 255}, // Bra_Ra
      { 16, 5, 256, 255}, // Mem_Ra
      { 16, 5, 256, 255}, // Mem_Rb
      { 16, 5, 256, 255}, // Opr_Ra
      { 16, 5, 256, 255}  // Opr_Rc
    },
    maps5{
      { 17, 6, 256, 255}, // OpCode
      { 17, 5, 256, 255}, // Mem_Ra
      { 16, 5, 256, 255}  // Mem_Rb
    },
    maps6{
      { 17, 6, 256, 255}, // OpCode
      { 17, 5, 256, 255}, // Mem_Ra
      { 17, 5, 256, 255}  // Mem_Rb
    },
    maps7{
      { 18, 6, 256, 255}, // OpCode
      { 18, 5, 256, 255}, // Mem_Ra
    },
    maps8{
      { 18, 6, 256, 255}  // OpCode
    },
    maps9{
      { 17, 6, 256, 255}  // OpCode
    },
    maps10{
      { 17, 6, 256, 255}  // OpCode
    }
   
   {
 }
 
int  decModel1::p(Mixer& m,int val1,int val2){  
  if ((x.blpos  == 0u) && (x.bpos == 0)) {
      state = State::OpCode;
      for (std::uint32_t i = 0u; i < nMaps - 1u; i++) {
        if (((maps_mask[i] >> State::OpCode) & 1u) != 0u)
          maps[i][map_state(i, State::OpCode)].set(0u);
      }
      count = 0u;
      cache.fill(op = { 0 });
    }
    else {
      count++;
      //INJECT_SHARED_y
      switch (state) {
        case State::OpCode: {
          op.Opcode += op.Opcode + x.y;
          if (count == 6u) {
            op.relOpcode = DECAlpha::rel_op[op.Opcode];
            switch (op.Format = DECAlpha::formats[op.Opcode]) {
              case DECAlpha::Bra: {
                state = State::Bra_Ra;
                maps1[map_state(0u, State::Bra_Ra)].set_direct(cache(1).Rc);
                maps2[map_state(1u, State::Bra_Ra)].set_direct((cache(1).Ra << 5u) | cache(1).Rc);
                maps3[map_state(2u, State::Bra_Ra)].set_direct((cache(1).Opcode << 10u) | (cache(1).Ra << 5u) | cache(1).Rc);
                maps4[map_state(3u, State::Bra_Ra)].set(hash(cache(1).Ra, cache(2).Ra, cache(3).Ra, cache(4).Ra));
                break;
              }
              case DECAlpha::F_P: {
                state = State::F_P_Function;
                maps1[map_state(0u, State::F_P_Function)].set_direct(op.Opcode);
                break;
              }
              case DECAlpha::Mem: {
                state = State::Mem_Ra;
                maps1[map_state(0u, State::Mem_Ra)].set_direct(cache(1).Ra);
                maps2[map_state(1u, State::Mem_Ra)].set_direct((cache(1).Ra << 5u) | cache(2).Ra);
                maps3[map_state(2u, State::Mem_Ra)].set_direct((cache(1).Ra << 10u) | (cache(2).Ra << 5u) | cache(3).Ra);
                maps4[map_state(3u, State::Mem_Ra)].set(hash(cache(1).Ra, cache(2).Ra, cache(3).Ra, cache(4).Ra));
                maps5[map_state(4u, State::Mem_Ra)].set(hash(op.Opcode, cache(1).Opcode, cache(1).Ra, cache(2).Opcode, cache(2).Ra));
                maps6[map_state(5u, State::Mem_Ra)].set(hash(op.Opcode, cache(1).Ra, cache(2).Ra, cache(2).Rb,  cache(3).Ra, cache(4).Ra));
                maps7[map_state(6u, State::Mem_Ra)].set(hash(op.Opcode, cache(1).Ra, cache(1).Rb, cache(2).Ra,  cache(3).Ra, cache(4).Ra, cache(5).Ra));
                break;
              }
              case DECAlpha::Mfc: {
                state = State::Mfc_Function;
                break;
              }
              case DECAlpha::Mbr: {
                state = State::Mbr_Ra; // usually R26, R27 or R31
                break;
              }
              case DECAlpha::Opr: {
                state = State::Opr_Bit;
                maps1[map_state(0u, State::Opr_Bit)].set_direct(op.relOpcode);
                maps2[map_state(1u, State::Opr_Bit)].set(hash(op.Opcode, cache(1).Opcode, cache(1).Function));
                break;
              }
              case DECAlpha::Pcd: {
                state = State::Pcd_Function;
                break;
              }
              case DECAlpha::Nop: {
                state = State::Nop_Skip;
                break;
              }
            }
            count = 0u;
          }
          break;
        }
        case State::Bra_Ra: {
          op.Ra += op.Ra + x.y;
          if (count == 5u) {
            state = State::Bra_Displacement;
            count = 0u;
            maps1[map_state(0u, State::Bra_Displacement)].set_direct(cache(1).Opcode);
            maps2[map_state(1u, State::Bra_Displacement)].set_direct((cache(1).Opcode << 6u) | cache(2).Opcode);
          }
          break;
        }
        case State::Bra_Displacement: {
          op.Displacement += op.Displacement + x.y;
          if ((count % 7u) == 0) {
            if (count < 21u) {
              maps0[state].set(count / 7u);
              maps1[map_state(0u, State::Bra_Displacement)].set(hash(count, cache(1).Opcode, op.Displacement) + 0x40u);
              maps2[map_state(1u, State::Bra_Displacement)].set(hash(count, cache(1).Opcode, cache(2).Opcode, op.Displacement) + 0x1000u);
            }
            else {
              state = State::OpCode;
              count = 0u;
            }
          }
          break;
        }
        case State::F_P_Function: {
          op.Function = op.Function + x.y;
          if (count == 11u) {
            state = State::F_P_Ra;
            count = 0u;
            maps1[map_state(0u, State::F_P_Ra)].set_direct(cache(1).Rc);
          }
          else if ((count % 3u) == 0u) {
            maps0[state].set(count / 3u);
            maps1[map_state(0u, State::F_P_Function)].set(hash(count, op.Opcode, op.Function) + 0x40u);
          }
          break;
        }
        case State::F_P_Ra: {
          op.Ra += op.Ra + x.y;
          if (count == 5u) {
            state = State::F_P_Rb;
            count = 0u;
            maps1[map_state(0u, State::F_P_Rb)].set_direct((op.Ra << 5u) | cache(1).Rc);
          }
          break;
        }
        case State::F_P_Rb: {
          op.Rb += op.Rb + x.y;
          if (count == 5u) {
            state = State::F_P_Rc;
            count = 0u;
            maps1[map_state(0u, State::F_P_Rc)].set_direct((op.Ra << 5u) | op.Rb);
          }
          break;
        }
        case State::F_P_Rc: {
          op.Rc += op.Rc + x.y;
          if (count == 5u) {
            lastRc = op.Rc;
            state = State::OpCode;
            count = 0u;
          }
          break;
        }
        case State::Mem_Ra: {
          op.Ra += op.Ra + x.y;
          if (count == 5u) {
            state = State::Mem_Rb;
            count = 0u;
            maps1[map_state(0u, State::Mem_Rb)].set_direct(op.relOpcode);
            maps2[map_state(1u, State::Mem_Rb)].set_direct((op.relOpcode << 5u) | cache(1).Rb);           
            maps3[map_state(2u, State::Mem_Rb)].set_direct((op.relOpcode << 10u) | (cache(1).Rb << 5u) | cache(2).Rb);           
            maps4[map_state(3u, State::Mem_Rb)].set(hash(op.Opcode, cache(1).Rb, cache(2).Rb, cache(3).Rb));
            maps5[map_state(4u, State::Mem_Rb)].set(hash(op.Opcode, cache(1).Rb, cache(2).Ra, cache(3).Ra));
            maps6[map_state(5u, State::Mem_Rb)].set(hash(op.Opcode, cache(1).Opcode, cache(1).Ra, cache(1).Rb));
          }
          break;
        }
        case State::Mem_Rb: {
          op.Rb += op.Rb + x.y;
          if (count == 5u) {
            state = State::Mem_Displacement;
            count = 0u;
            maps1[map_state(0u, State::Mem_Displacement)].set_direct(op.Opcode);
            maps2[map_state(1u, State::Mem_Displacement)].set(hash(op.Opcode, op.Rb));
            maps3[map_state(2u, State::Mem_Displacement)].set(hash(op.Opcode, op.Rb, cache(1).Literal));
          }
          break;
        }
        case State::Mem_Displacement: {
          op.Displacement += op.Displacement + x.y;
          if ((count & 3u) == 0) {
            if (count < 16u) {
              maps0[state].set(count / 4u);
              maps1[map_state(0u, State::Mem_Displacement)].set(hash(count, op.Opcode, op.Displacement));
              maps2[map_state(1u, State::Mem_Displacement)].set(hash(count, op.Opcode, op.Rb, op.Displacement));
              maps3[map_state(2u, State::Mem_Displacement)].set(hash(count, op.Opcode, op.Rb, cache(1).Literal, op.Displacement));
            }
            else {
              state = State::OpCode;
              count = 0u;
            }
          }
          break;
        }
        case State::Mfc_Function: {
          op.Function += op.Function + x.y;
          if ((count & 3u) == 0) {
            if (count < 16u)
              maps0[state].set(count / 4u);
            else {
              state = State::Mfc_Ra; // usually R31
              count = 0u;
            }            
          }
          break;
        }
        case State::Mfc_Ra: {
          op.Ra += op.Ra + x.y;
          if (count == 5u) {
            state = State::Mfc_Rb; // usually R31
            count = 0u;
          }
          break;
        }
        case State::Mfc_Rb: {
          op.Rb += op.Rb + x.y;
          if (count == 5u) {
            state = State::OpCode;
            count = 0u;
          }
          break;
        }
        case State::Mbr_Ra: {
          op.Ra += op.Ra + x.y;
          if (count == 5u) {
            state = State::Mbr_Rb;
            count = 0u;
            maps1[map_state(0u, State::Mbr_Rb)].set_direct(op.relOpcode);
          }
          break;
        }
        case State::Mbr_Rb: {
          op.Rb += op.Rb + x.y;
          if (count == 5u) {
            state = State::Mbr_Displacement;
            count = 0u;
            maps1[map_state(0u, State::Mbr_Displacement)].set_direct(op.relOpcode);
          }
          break;
        }
        case State::Mbr_Displacement: {
          op.Displacement += op.Displacement + x.y;
          if ((count & 3u) == 0) {
            if (count < 16u) {
              maps0[state].set(count / 4u);
              maps1[map_state(0u, State::Mbr_Displacement)].set_direct(((count >> 1u) & 0x6u) | op.relOpcode);
            }
            else {
              state = State::OpCode;
              count = 0u;
            }
          }
          break;
        }
        case State::Opr_Bit: {
          op.Bit = x.y;
          state = State::Opr_Function;
          count = 0u;
          maps1[map_state(0u, State::Opr_Function)].set_direct((op.relOpcode << 1u) | op.Bit);
          maps2[map_state(1u, State::Opr_Function)].set(hash(op.Opcode, op.Bit, cache(1).Opcode, cache(1).Function));
          maps3[map_state(2u, State::Opr_Function)].set(hash(op.Opcode, op.Bit, cache(1).Opcode, cache(2).Opcode, cache(3).Opcode));
          break;
        }
        case State::Opr_Function: {
          op.Function += op.Function + x.y;
          if (count == 7u) {
            state = State::Opr_Ra;
            count = 0u;
            maps1[map_state(0u, State::Opr_Ra)].set_direct(cache(1).Ra);
            maps2[map_state(1u, State::Opr_Ra)].set_direct(cache(1).Rc);
            maps3[map_state(2u, State::Opr_Ra)].set_direct((cache(1).Ra << 5u) | cache(2).Ra);
            maps4[map_state(3u, State::Opr_Ra)].set(hash(op.Bit, cache(1).Ra, cache(2).Ra, cache(3).Ra, cache(4).Ra));
          }
          break;
        }
        case State::Opr_Ra: {
          op.Ra += op.Ra + x.y;
          if (count == 5u) {
            count = 0u;
            if (op.Bit == 0u) {
              state = State::Opr_Rb;
              maps1[map_state(0u, State::Opr_Rb)].set_direct((cache(1).Ra << 5u) | cache(1).Rb);
              maps2[map_state(1u, State::Opr_Rb)].set_direct((cache(1).Rb << 5u) | cache(2).Ra);
              maps3[map_state(2u, State::Opr_Rb)].set_direct((op.Ra << 10u) | (cache(1).Ra << 5u) | cache(2).Ra);
            }
            else {
              state = State::Opr_Literal;
              maps1[map_state(0u, State::Opr_Literal)].set_direct((op.relOpcode << 7u) | op.Function);
              maps2[map_state(1u, State::Opr_Literal)].set(hash(op.Opcode, op.Function, cache(1).Opcode, cache(1).Function));
            }
          }
          break;
        }
        case State::Opr_Rb: {
          op.Rb += op.Rb + x.y;
          if (count == 5u) {
            state = State::Opr_Unused;
            count = 0u;
          }
          break;
        }
        case State::Opr_Unused: {
          if (count == 3u) {
            state = State::Opr_Rc;
            count = 0u;
            maps1[map_state(0u, State::Opr_Rc)].set_direct((op.Ra << 1u) | op.Bit);
            maps2[map_state(1u, State::Opr_Rc)].set_direct((op.Ra << 10u) | (op.Rb << 5u) | cache(1).Ra);
            maps3[map_state(2u, State::Opr_Rc)].set_direct((cache(1).Ra << 5u) | cache(2).Ra);
            maps4[map_state(3u, State::Opr_Rc)].set_direct((cache(1).Ra << 10u) | (cache(2).Ra << 5u) | cache(3).Ra);
          }
          break;
        }
        case State::Opr_Literal: {
          op.Literal += op.Literal + x.y;
          if (count == 8u) {
            state = State::Opr_Rc;
            count = 0u;
            maps1[map_state(0u, State::Opr_Rc)].set_direct((op.Ra << 1u) | op.Bit);
            maps2[map_state(1u, State::Opr_Rc)].set_direct(0x8000u | (op.Ra << 10u) | (cache(1).Ra << 5u) | cache(1).Rc);
            maps3[map_state(2u, State::Opr_Rc)].set_direct(0x400u | (cache(1).Ra << 5u) | cache(2).Ra);
            maps4[map_state(3u, State::Opr_Rc)].set_direct(0x8000u | (cache(1).Ra << 10u) | (cache(2).Ra << 5u) | cache(3).Ra);
          }
          break;
        }
        case State::Opr_Rc: {
          op.Rc += op.Rc + x.y;
          if (count == 5u) {
            lastRc = op.Rc;
            state = State::OpCode;
            count = 0u;
          }
          break;
        }
        case State::Pcd_Function:
        case State::Nop_Skip: {
          op.Function += op.Function + x.y;
          if (count == 26u) {
            state = State::OpCode;
            count = 0u;
          }
          else if ((count % 3) == 0u)
            maps0[state].set(count / 3u);
          break;
        }
      }
    }
    if (count == 0u) {
      maps0[state].set(0u);
      if (state == State::OpCode) {
        std::uint64_t ctx = hash(op.Opcode, op.Function);
        maps1[map_state(0u, State::OpCode)].set(ctx);
        maps2[map_state(1u, State::OpCode)].set(hash(ctx, cache(1).Opcode, cache(1).Function));
        maps3[map_state(2u, State::OpCode)].set(ctx = hash(ctx, cache(1).Opcode, cache(2).Opcode));
        maps4[map_state(3u, State::OpCode)].set(ctx = hash(ctx, cache(3).Opcode));
        maps5[map_state(4u, State::OpCode)].set(hash(ctx, cache(4).Opcode));
        maps6[map_state(5u, State::OpCode)].set(ctx = hash(op.Opcode, cache(1).Opcode, cache(2).Opcode, cache(3).Opcode, cache(4).Opcode, cache(5).Opcode));
        maps7[map_state(6u, State::OpCode)].set(ctx = hash(ctx, cache(6).Opcode));
        maps8[map_state(7u, State::OpCode)].set(ctx = hash(ctx, cache(7).Opcode));
        maps9[map_state(8u, State::OpCode)].set(hash(op.Opcode, op.Ra, cache(1).Opcode, cache(1).Ra, cache(2).Opcode, cache(2).Ra));
        maps10[map_state(9u, State::OpCode)].set(hash(op.Opcode, op.Rb, lastRc, cache(2).Opcode, cache(2).Rb, cache(3).Opcode, cache(3).Ra));
        last[op.Format] = op;
        cache.add(op);
        op = { 0 };
      }
    }    
   
    maps0[state].mix1(m);
    for (std::uint32_t i = 0u; i < nMaps - 1u; i++) {
      if (((maps_mask[i] >> state) & 1u) != 0u)
        maps[i][map_state(i, state)].mix1(m);
    }

    std::uint8_t const opcode = (state != State::OpCode) ? op.Opcode : cache(1).Opcode;

    m.set(static_cast<std::uint32_t>(state) * 26u + count, State::Count * 26u);
    m.set((state << 6u) | opcode, State::Count * 64u);
    m.set( (hash(state, count, opcode))&2047, 2048u);
    m.set( (hash(state, count, op.Opcode, cache(1).Opcode))&4095, 4096u);
    m.set( (hash(state, count, cache(1).Opcode, cache(2).Opcode))&4095, 4096u);
    m.set( (hash(state, count, cache(1).Opcode, cache(2).Opcode, cache(3).Opcode))&8191, 8192u);
    m.set( (hash(state, count, op.Opcode, cache(1).Opcode, cache(2).Opcode, cache(3).Opcode))&8191, 8192u);
    m.set( (hash(state, opcode, cache(1).Format, cache(2).Format, cache(3).Format, cache(4).Format))&8191, 8192u);
    m.set( (hash(state, count, op.Opcode, op.Bit, op.Ra))&4095, 4096u);
    m.set( (hash(state, op.Ra, op.Rb, op.Bit))&4095, 4096u);
    m.set( (hash(state, op.Ra, last[DECAlpha::Mem].Ra, cache(1).Format))&4095, 4096u);
x.DEC.state = state;
    x.DEC.bcount = count;
  return 1;
}


