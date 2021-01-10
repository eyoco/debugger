#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>

#include "instruction.h"
#include "printRoutines.h"

/* Reads one byte from memory, at the specified address. Stores the
   read value into *value. Returns 1 in case of success, or 0 in case
   of failure (e.g., if the address is beyond the limit of the memory
   size). */
int memReadByte(machine_state_t *state, uint64_t address, uint8_t *value) {

  /* THIS PART TO BE COMPLETED BY THE STUDENT */
  // uint8_t value0, value1, value2, value3, value4, value5, value6, value7;
  if(address < state-> programSize){
    *value = state -> programMap[address];
    return 1;
  } else {
    return 0;
  }
}

/* Reads one quad-word (64-bit number) from memory in little-endian
   format, at the specified starting address. Stores the read value
   into *value. Returns 1 in case of success, or 0 in case of failure
   (e.g., if the address is beyond the limit of the memory size). */
int memReadQuadLE(machine_state_t *state, uint64_t address, uint64_t *value) {

  /* THIS PART TO BE COMPLETED BY THE STUDENT */
  if(address <= state -> programSize){
    uint64_t value0, value1, value2, value3, value4, value5, value6, value7;
    value0 = state -> programMap[address];
    value1 = state -> programMap[address+1];
    value2 = state -> programMap[address+2];
    value3 = state -> programMap[address+3];
    value4 = state -> programMap[address+4];
    value5 = state -> programMap[address+5];
    value6 = state -> programMap[address+6];
    value7 = state -> programMap[address+7];
    *value = (value7 << 56) | (value6 << 48) | (value5 << 40) | (value4 << 32) | (value3 << 24) | (value2 << 16) | (value1 << 8) | (value0 << 0);
    return 1;
  } else {
    return 0;
  }
}

/* Stores the specified one-byte value into memory, at the specified
   address. Returns 1 in case of success, or 0 in case of failure
   (e.g., if the address is beyond the limit of the memory size). */
int memWriteByte(machine_state_t *state,  uint64_t address, uint8_t value) {

  /* THIS PART TO BE COMPLETED BY THE STUDENT */

  if(address <= state -> programSize){
    state -> programMap[address] = value;
    return 1;
  } else {
    return 0;
  }
}

/* Stores the specified quad-word (64-bit) value into memory, at the
   specified start address, using little-endian format. Returns 1 in
   case of success, or 0 in case of failure (e.g., if the address is
   beyond the limit of the memory size). */
int memWriteQuadLE(machine_state_t *state, uint64_t address, uint64_t value) {

  /* THIS PART TO BE COMPLETED BY THE STUDENT */
  // uncomplete
  if(address <= state -> programSize){
    //byte * bytearray;
    unsigned char * bytearray;
    bytearray = &value;
    state -> programMap[address] = bytearray[0];
    state -> programMap[address+1] = bytearray[1];
    state -> programMap[address+2] = bytearray[2];
    state -> programMap[address+3] = bytearray[3];
    state -> programMap[address+4] = bytearray[4];
    state -> programMap[address+5] = bytearray[5];
    state -> programMap[address+6] = bytearray[6];
    state -> programMap[address+7] = bytearray[7];
      return 1;
    } else {
      return 0;
    }
}

/* Fetches one instruction from memory, at the address specified by
   the program counter. Does not modify the machine's state. The
   resulting instruction is stored in *instr. Returns 1 if the
   instruction is a valid non-halt instruction, or 0 (zero)
   otherwise. */
int fetchInstruction(machine_state_t *state, y86_instruction_t *instr) {

  /* THIS PART TO BE COMPLETED BY THE STUDENT */
  uint8_t value;
  uint64_t valueQuad; 
  if(memReadByte(state, state -> programCounter, &value)){
    instr -> ifun = value & 0xF;
    instr -> icode = value >> 4;
    if (instr -> icode == 0){
      return 0;
    }
    if (instr -> icode == 1 || instr -> icode == 9){
      instr -> valP = state -> programCounter + 1;
      instr -> location = state -> programCounter;

      return 1;
    } else if (instr -> icode == 2 || instr -> icode == 6 || instr -> icode == 0xA || instr -> icode == 0xB){
      // offset = 2;
      instr -> valP = state -> programCounter + 2;
      instr -> location = state -> programCounter;
      if (memReadByte(state, state -> programCounter + 1, &value)){
        instr -> rA = value >> 4;
        instr -> rB = value & 0xF; //?
        return 1;
      }
      return 0;
    } else if (instr -> icode == 7 || instr -> icode == 8) {
      // offset = 9;
      instr -> valP = state -> programCounter + 9;
      instr -> location = state -> programCounter;
      if (memReadQuadLE(state, state -> programCounter + 1, &valueQuad)){
        instr -> valC = valueQuad;
        return 1;
      }
      return 0;
    } else {
      // offset = 10;
      instr -> valP = state -> programCounter + 10;
      instr -> location = state -> programCounter;
      if (memReadByte(state, state -> programCounter + 1, &value)){
        instr -> rA = value >> 4;
        instr -> rB = value & 0xF;
        if (memReadQuadLE(state, state -> programCounter + 2, &valueQuad)){
          instr -> valC = valueQuad;
          return 1;
        }
      }
      return 0;
    }
  } 
  return 0;
}

int executeInstruction(machine_state_t *state, y86_instruction_t *instr) {

  /* THIS PART TO BE COMPLETED BY THE STUDENT */
  if(instr -> icode == I_HALT){
    state -> programCounter = instr -> valP;
  } else if(instr -> icode == I_NOP){
    state -> programCounter = instr -> valP;
  } else if(instr -> icode == I_RRMVXX){
    if(instr -> ifun == C_NC){
      state -> registerFile[instr -> rB] = state -> registerFile[instr -> rA];
      state -> programCounter = instr -> valP;
    } else if(instr -> ifun == C_LE){
      if(instr -> rB <= 0){
        state -> registerFile[instr -> rB] = state -> registerFile[instr -> rA];
        state -> programCounter = instr -> valP;
      }
    } else if(instr -> ifun == C_L){
      if(instr -> rB < 0){
        state -> registerFile[instr -> rB] = state -> registerFile[instr -> rA];
        state -> programCounter = instr -> valP;
      }
    } else if(instr -> ifun == C_E){
      if(instr -> rB == 0){
        state -> registerFile[instr -> rB] = state -> registerFile[instr -> rA];
        state -> programCounter = instr -> valP;
      }
    } else if(instr -> ifun == C_NE){
      if(instr -> rB != 0){
        state -> registerFile[instr -> rB] = state -> registerFile[instr -> rA];
        state -> programCounter = instr -> valP;
      }
    } else if(instr -> ifun == C_GE){
      if(instr -> rB >= 0){
        state -> registerFile[instr -> rB] = state -> registerFile[instr -> rA];
        state -> programCounter = instr -> valP;
      }
    } else if(instr -> ifun == C_G){
      if(instr -> rB > 0){
        state -> registerFile[instr -> rB] = state -> registerFile[instr -> rA];
        state -> programCounter = instr -> valP;
      }
    }
  } else if(instr -> icode == I_IRMOVQ){
    state -> registerFile[instr -> rB] = instr -> valC;
    state -> programCounter = instr -> valP;
  } else if(instr -> icode ==I_RMMOVQ){
    state -> programMap[state -> registerFile[instr -> rA] + instr -> valC] = state -> registerFile[instr -> rA];
    state -> programCounter = instr -> valP;
  } else if(instr -> icode == I_MRMOVQ){
    // state -> registerFile[instr -> rB] = state -> programMap[state -> registerFile[instr -> rA] + instr -> valC];
    if(memReadQuadLE(state, state -> registerFile[instr -> rA] + instr -> valC, &(state -> registerFile[instr -> rB])) == 1){
    	state -> programCounter = instr -> valP;
    	return 1;
    } else {
    	return 0;
    }
  } else if(instr -> icode == I_OPQ){
    if(instr -> ifun == A_ADDQ){
      state -> registerFile[instr -> rB] = state -> registerFile[instr -> rB] + state -> registerFile[instr -> rA];
      state -> programCounter = instr -> valP;
    } else if(instr -> ifun == A_SUBQ){
      state -> registerFile[instr -> rB] = state -> registerFile[instr -> rB] - state -> registerFile[instr -> rA];
      state -> programCounter = instr -> valP;
    } else if(instr -> ifun == A_ANDQ){
      state -> registerFile[instr -> rB] = state -> registerFile[instr -> rB] & state -> registerFile[instr -> rA];
      state -> programCounter = instr -> valP;
    } else if(instr -> ifun == A_XORQ){
      state -> registerFile[instr -> rB] = state -> registerFile[instr -> rB] ^ state -> registerFile[instr -> rA];
      state -> programCounter = instr -> valP;
    } else if(instr -> ifun == A_MULQ){
      state -> registerFile[instr -> rB] = state -> registerFile[instr -> rB] * state -> registerFile[instr -> rA];
      state -> programCounter = instr -> valP;
    } else if(instr -> ifun == A_DIVQ){
      state -> registerFile[instr -> rB] = state -> registerFile[instr -> rB] / state -> registerFile[instr -> rA];
      state -> programCounter = instr -> valP;
    } else if(instr -> ifun == A_MODQ){
      state -> registerFile[instr -> rB] = state -> registerFile[instr -> rB] % state -> registerFile[instr -> rA];
      state -> programCounter = instr -> valP;
    }
  } else if(instr -> icode == I_JXX){
    if(instr -> ifun == C_NC){
      state -> programCounter = instr -> valC;
    } else if(instr -> ifun == C_LE){
      if(state -> registerFile[instr -> rB] <= 0){
        state -> programCounter = instr -> valC;
      } else{
      	state -> programCounter = instr -> valP;
      }
    } else if(instr -> ifun == C_L){
      if(state -> registerFile[instr -> rB] < 0){
        state -> programCounter = instr -> valC;
      } else{
      	state -> programCounter = instr -> valP;
      }
    } else if(instr -> ifun == C_E){
      if(state -> registerFile[instr -> rB] == 0){
        state -> programCounter = instr -> valC;
      } else{
      	state -> programCounter = instr -> valP;
      }
    } else if(instr -> ifun == C_NE){
      if(state -> registerFile[instr -> rB] != 0){
        state -> programCounter = instr -> valC;
      } else{
        state -> programCounter = instr -> valP;
      }
    } else if(instr -> ifun == C_GE){
      if(state -> registerFile[instr -> rB] >= 0){
        state -> programCounter = instr -> valC;
      } else{
      	state -> programCounter = instr -> valP;
      }
    } else if(instr -> ifun == C_G){
      if(state -> registerFile[instr -> rB] > 0){
        state -> programCounter = instr -> valC;
      } else{
      	state -> programCounter = instr -> valP;
      }
    }
  } else if(instr -> icode == I_CALL){
    state -> programMap[state -> registerFile[R_RSP] - 8] = state -> programCounter;
    state -> registerFile[R_RSP] = state -> registerFile[R_RSP] - 8;
    state -> programCounter = instr -> valC;
  } else if(instr -> icode == I_RET){
    state -> programCounter = state -> programMap[state -> registerFile[R_RSP]];
    state -> registerFile[R_RSP] = state -> registerFile[R_RSP] + 8;
    state -> programCounter = instr -> valP;
  } else if(instr -> icode == I_PUSHQ){
    state -> programMap[state -> registerFile[R_RSP] - 8] = state -> registerFile[instr -> rA];
    state -> registerFile[R_RSP] = state -> registerFile[R_RSP] - 8;
    state -> programCounter = instr -> valP;
  } else if(instr -> icode == I_POPQ){
    state -> registerFile[instr -> rA] = state -> programMap[state -> registerFile[R_RSP]];
    state -> registerFile[R_RSP] = state -> registerFile[R_RSP] + 8;
    state -> programCounter = instr -> valP;
  } else if(instr -> icode == I_INVALID || instr -> icode == I_TOO_SHORT){
    return 0;
  }
  return 1;
}

int memReadByte(machine_state_t *state,	uint64_t address, uint8_t *value);
int memReadQuadLE(machine_state_t *state, uint64_t address, uint64_t *value);
int memWriteByte(machine_state_t *state,  uint64_t address, uint8_t value);
int memWriteQuadLE(machine_state_t *state, uint64_t address, uint64_t value);