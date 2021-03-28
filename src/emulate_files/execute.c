#include "execute.h"

//execute instructions

//checks whether an instruction should run based on the condition and the current CPSR.
int check_cond(int cond, arm_state *state){
  if (cond==14) return 1;
  unsigned int flag =(*(state->registers))[CPSR];
  int n = getBits(flag,31,1);
  int z = getBits(flag,30,1);
  //int c = getBits(flag,29,1);
  int v = getBits(flag,28,1);
  switch (cond){
  case 0: return z==1;
  case 1: return z==0;
  case 10: return n==v;
  case 11: return n!=v;
  case 12: return (z==0)&&(n==v);
  case 13: return (z==1)||(n!=v);
  default: return 0;
  }
}

//executed data processing instructions
int execute_dp(arm_state *state){
  DataProc instruction =(state->pipeline).decoded.dp;
  //if condition is false, then return 0 and ignore.
  if (!check_cond(instruction.cond,state)){
    return 0;
  }
  unsigned int first_operand = (*(state->registers))[instruction.Rn];
  int result;
  unsigned int second_operand;
  if (instruction.I){//op2 is an immediate value
    second_operand = extend(instruction.Op2);
  }
  else{
    second_operand =shiftRegister(state);
  }

  void (*flag_update)(unsigned int, unsigned int, arm_state*) = no_update;
  switch (instruction.Opcode){
  //logical operations
  case 0:
  case 8:
    result= first_operand & second_operand;
    break;
  case 1:
  case 9:
    result= first_operand ^ second_operand;
    break;
  case 12:
    result = first_operand | second_operand;
    break;
  case 13:
    result = second_operand;
    break;
  //arithmetic operations
  case 4:
    result = first_operand + second_operand;
    flag_update = update_cflag_add;
    break;
  case 2:
  case 10:
    result = first_operand - second_operand;
    flag_update =update_cflag_sub;
    break;
  case 3:
    result = second_operand - first_operand;
    if (instruction.S) update_cflag_sub(second_operand,first_operand,state);
    break;
  default:
    perror("error in dp switch case\n");
    exit(EXIT_FAILURE);
    break;
  }
  //if opcode is 0,1,12,4,2  or 3, then save the result to the return register
  if (instruction.Opcode < 5 || instruction.Opcode>11)
    (*(state->registers))[instruction.Rd] = result;
  //update flags if need be;
  if (instruction.S){
    update_zflag(result,state);
    update_nflag(result,state);
    flag_update(first_operand,second_operand,state);
  }
  return 0;
}

//execute a multiplication instruction.
int execute_mult(arm_state *state){
  Mult instr = (state->pipeline).decoded.m;
  //if the condition is false, then ignore the instruction.
  if (!check_cond(instr.cond, state)) return 0;
  
  //get the registers used in the operation 
  unsigned int Rm = (*(state->registers))[instr.Rm];
  unsigned int Rs = (*(state->registers))[instr.Rs];
  unsigned int Rn = (*(state->registers))[instr.Rn];
  
  //perform the calculation and store result
  unsigned int result = Rm*Rs + instr.A*Rn; 
  (*(state->registers))[instr.Rd] = result;
  
  //modify the CPSR flags if need be.
  if (instr.S) {
    modifyBit((*(state->registers))[CPSR], 31,getBits(result,31,1));
    modifyBit((*(state->registers))[CPSR], 30, !result);
  }
  return 0;
}

//execute a Single Data Transfer instruction
int execute_sdt(arm_state *state){
  SingleDT instr = (state->pipeline).decoded.sdt;
  unsigned int address;
  unsigned int rm;
  //if condition is false then ignore the instruction.
  if (!check_cond(instr.cond,state)) return 0;
  if (instr.Rd==PC){
    return 0;
  }

  if (instr.I){
    rm = (int) (*(state->registers))[getBits(instr.Offset,0,4)];
    int integer = getBits(instr.Offset,7,5);
    int type = getBits(instr.Offset,5,2);
    if (integer){
      switch (type){
      case 0://lsl
	      rm = rm << integer;
        break;
      case 1://lsr
	      rm = rm >> integer;
        break;
      case 2: //asr
	      rm = rm >> integer;
        int msb = getBits(rm,31,1);
        if (msb){
	        unsigned int mask = 0x0000FFFF << (32 - integer);
	        rm = (rm >> integer) | mask;
	      }
	      break;
      case 3://ror
	      rm = (rm >> integer) | (rm << (32-integer));
        break;
      default:
	      perror("type error in singleDT");
        exit(EXIT_FAILURE);
      }
    }
  }

  if (!instr.I){
    rm = extend(instr.Offset);
  }
  
  if (instr.P){ //Pre-indexing
    if (instr.U){
      address = (*(state->registers))[instr.Rn] + rm;
    }
    else {
      address = (*(state->registers))[instr.Rn] - rm;
    }
  }

  if (!instr.P){ //Post-indexing
    address = (*(state->registers))[instr.Rn];
    if (instr.U){
      (*(state->registers))[instr.Rn] += rm;
    }
    else {
      (*(state->registers))[instr.Rn] -= rm;
    }
  }

  if (instr.L){
    int error = 0;
    //printf("Address : %u",address);
    unsigned int val = get_val_from_mem(state,address, &error);
    //printf("loading  number %x , from address %x, into register %d\n",val,address,instr.Rd);
    if (!error){
      (*(state->registers))[instr.Rd] = val;
    }
  }

  else{
    unsigned int val = (*(state->registers))[instr.Rd];
    //printf("storing number %x, from register %d, into address %x",val,instr.Rd,address);
    store_val_to_mem(state,val,address);
  }
  return 0;
}

//execute a branch instruction
int execute_b(arm_state *state){
  Branch instr = (state->pipeline).decoded.b;
  if (!check_cond(instr.cond,state)) return 0;
  int result = instr.Offset<<2;
  int msb = getBits(result,25,1);
  if (msb==1){
    int mask = -67108864;
    result = result | mask;
  }
  (*(state->registers))[PC] += result;
  return 1;
}

//execute an instruction using switch/case.
int execute(arm_state *state){
  Instruction instr = (state->pipeline).decoded;

  switch(instr.type){
  case IGNORE:
    return 0;
  case HALT:
    state->halted = 1;
    return 0;
  case DATAPROC:
    return execute_dp(state);
  case MULT:
    return execute_mult(state);
  case SINGLEDT:
    return execute_sdt(state);
  case BRANCH:
    return execute_b(state);
  default:
    printf("error with execute switch\n");
    return 0;
  }
}
