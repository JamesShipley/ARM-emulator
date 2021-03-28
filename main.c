#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#define PC 15
#define CPSR 16
//decode structs
typedef struct{int cond,I,Opcode,S,Rn,Rd,Op2;}DataProc;
typedef struct{int cond,A,S,Rd,Rn,Rs,Rm;}Mult;
typedef struct{int cond,I,P,U,L,Rn,Rd,Offset;}SingleDT;
typedef struct{int cond,Offset;}Branch;
enum instruction_type{DATAPROC=1,MULT=2,SINGLEDT=3,BRANCH=4,IGNORE=5,HALT=6};
typedef struct{enum instruction_type type;union{DataProc dp;Mult m;SingleDT sdt;Branch b;};}Instruction; 
//arm structs
typedef struct {
  unsigned int fetched;
  Instruction decoded;
} pipeline;

typedef struct {
  unsigned int **instructions;
  unsigned int **registers;
  unsigned int **memory;
  pipeline pipeline;
  int halted;
} arm_state;
//decode functions

//gets a certain number of bits from the command 
int getBits(unsigned int x, int point, int length){
  return (x>>point)%(1<<length);
}
//prints the command in binary, 8 sets of 4 
void binprint(unsigned int x){
  for (int i=31; i>-1 ;i--){
    if (i%4 ==3 && i) printf(" ");
    printf("%d",getBits(x,i,1));
  }
  printf("\n");
}
//decodes an instruction (unsigned int), returning an Instruction struct
Instruction decode(unsigned int x, arm_state* state){
  if (x==0){
    state->halted = 1;
  }
  int cond = getBits(x,28,4);
  int partial_specifier = getBits(x,26,2);
  if (partial_specifier ==1){
    SingleDT instr = {
		      cond,
		      getBits(x,25,1),
		      getBits(x,24,1),
		      getBits(x,23,1),
		      getBits(x,20,1),
		      getBits(x,16,4),
		      getBits(x,12,4),
		      getBits(x,0,12)
    };
    Instruction  ret;
    ret.type =SINGLEDT;
    ret.sdt = instr;
    return ret;
  }
  
  else if (partial_specifier ==2){
    Branch instr = {
		    cond,
		    getBits(x,0,24)};
    Instruction ret;
    ret.type = BRANCH;
    ret.b = instr;
    return ret;
  }
  
  else if (getBits(x,22,6)==0 && getBits(x,4,4)==9){
    Mult instr = {
		  cond,
		  getBits(x,21,1),
		  getBits(x,20,1),
		  getBits(x,16,4),
		  getBits(x,12,4),
		  getBits(x,8,4),
		  getBits(x,0,4)
    };
    Instruction ret;
    ret.type =MULT;
    ret.m = instr;
    return ret;
  }
  else{
    DataProc instr = {
		      cond,
		      getBits(x,25,1),
		      getBits(x,21,4),
		      getBits(x,20,1),
		      getBits(x,16,4),
		      getBits(x,12,4),
		      getBits(x,0,12)
    };
    Instruction ret;
    ret.type = DATAPROC;
    ret.dp =  instr;
    return ret;
  }
}
//execute instructions
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
unsigned int modifyBit(unsigned int number, int position,int binary){
  int mask = 1<<position;
  return (number &(~mask)) | ((binary << position) & mask);
}
unsigned int shiftRegister(arm_state *state){
  int shift = (state->pipeline).decoded.dp.Op2;
  int s = (state->pipeline).decoded.dp.S;
  int rm = getBits(shift,0,4);
  // assume bit 4 is 0 -- shift by constant amount (not by register)
  int shift_type = getBits(shift,5,2);
  int integer = getBits(shift,7,5);
  unsigned int rm_value = (*(state->registers))[rm];
  int carryout;

  if (shift_type == 0){//lsl
    if (s==1){//update CPSR
      if (integer==0){
	carryout=0;
      }
      else{
	carryout = getBits(rm_value, 32-integer,1);
      }
      //set c flag to carryout
      (*(state->registers))[CPSR] = modifyBit((*(state->registers))[CPSR],29,carryout);
    }
    return rm_value<<integer;
  }
  //from this point, the carryout is computed in the same way for all shift types
  if (s==1){
    if (integer == 0){
      carryout = 0;
    }
    else{
      carryout = getBits(rm_value,integer-1,1);
    }
    (*(state->registers))[CPSR] = modifyBit((*(state->registers))[CPSR],29,carryout);
  }
  if (shift_type ==1){//lsr
    return rm_value >>integer;
  }
  if (shift_type == 2) {//asr
    if (!integer) return rm_value;
    int msb = getBits(rm_value,31,1);
    if (msb==1){
      unsigned int mask = 0x0000FFFF << (32 - integer);
      return (rm_value>>integer) | (mask);
    }
    assert(msb==0);
    return rm_value>>integer;
  }
  assert(shift_type == 3);//ror
  return (rm_value >>integer) | (rm_value <<(32-integer));
}
unsigned int extend(int immediate){
  unsigned int u_immediate = getBits(immediate,0,8);
  int rotate = getBits(immediate,8,4)*2;
  return (u_immediate>>rotate) | (u_immediate << (32-rotate));
}
void update_nflag(unsigned int result, arm_state *state){
  int nbit = getBits(result,31,1);
  (*(state->registers))[CPSR] = modifyBit((*(state->registers))[CPSR],31,nbit);
}
void update_zflag(unsigned int result, arm_state *state){
  (*(state->register))[CPSR] = modifyBit((*(state->register))[CPSR],30,!result);
}
void update_cflag_add(unsigned int first, unsigned int second, arm_state *state){
  int bool = ((first +second)<first);
  //unsigned overflow
  (*(state->registers))[CPSR] = modifyBit((*(state->registers))[CPSR],29,bool);
}
void update_cflag_sub(unsigned int first, unsigned int second, arm_state *state){
  int bool = ((first -second)<= first);
  //unsigned underflow
  (*(state->registers))[CPSR] =modifyBit((*(state->registers))[CPSR],29,bool);
}
void no_update(unsigned int a, unsigned int b, arm_state *state){};

int execute_dp(arm_state *state){
  DataProc instruction =(state->pipeline).decoded.dp;
  //if condition is false, then return 0 and ignore.
  if (!check_cond(instruction.cond,state)){
    printf("ignore instr\n");
    return 0;
  }
  printf("DataProc instr\n");
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
    printf("error in dp\n");
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

int execute_mult(arm_state *state){
  if (0)return 0;//!check_cond())return 0;
  printf("Mult instr\n");
  return 0;
}

int execute_sdt(arm_state *state){
  SingleDT instr = (state->pipeline).decoded.sdt;
  unsigned int address;
  if (!check_cond(instr.cond,state)) return 0;
  printf("SingleDT instr\n");
  if (instr.Rd==PC){
    printf("error, PC cannot be used as Source Register");
    return 0;
  }
  if (instr.I){
    //compute shifted register
  }
  else { 
    printf("Source register : %d\n",instr.Rn);
    if (instr.P){
      if (instr.U){
	address = (*(state->registers))[instr.Rn] + instr.Offset*4;
      }
      else {
	address = (*(state->registers))[instr.Rn] - instr.Offset*4;
      }
    }
    
    else{
      address = (*(state->registers))[instr.Rn];
      if (instr.U){
	(*(state->registers))[instr.Rn] += instr.Offset;
	}

      else {
	(*(state->registers))[instr.Rn] -=instr.Offset;
      }
    }
  }
  printf("memory address computed : %x, %x\n",address,address>>2);
  printf("value in address: %x\n",*(state->memory)[address>>2]);
  printf("destination register: %d\n",instr.Rd);
  (*(state->registers))[instr.Rd] = (*(state->memory))[address>>2];
  return 0;
}

//branch may have error to do
int naive_branch(arm_state *state){
  Branch instr = (state->pipeline).decoded.b;
  if (!check_cond(instr.cond,state))return 0;
  int Offset = instr.Offset<<2;
  if (Offset <0){
    Offset = -Offset;
    (*(state->registers))[PC] -= Offset;
  }
  else{
    (*(state->registers))[PC] += Offset;
  }
  return 0;
}
  
int execute_branch(arm_state *state){
  return naive_branch(state);
  Branch instr = (state->pipeline).decoded.b;
  if (!check_cond(instr.cond,state))return 0;
  if (instr.Offset>>23 == 1){
    //negative
  }
}
int execute_b(arm_state *state){
  Branch instruction = (state->pipeline).decoded.b;
  int result = instruction.Offset<<2;
  int msb = getBits(result,25,1);
  if (msb==1){
    int mask = -67108864;
    result = result | mask;
  }
  (*(state->registers))[PC] += result;
  return 1;
}
//execute functions
int execute(arm_state *state){
  Instruction instr = (state->pipeline).decoded;
  if (instr.type==IGNORE) return 0;
  switch(instr.type){
  case DATAPROC:
    return execute_dp(state);
  case MULT:
    return execute_mult(state);
  case SINGLEDT:
    return execute_sdt(state);
  case BRANCH:
    return execute_b(state);
  default:
    return 0;
  }
  return 0;
}
void printRegisters(arm_state *state){
  unsigned int x;
  printf("Registers:\n");
  for (int i =0; i<=12 ; i++){
    x = (*(state->registers))[i];
    printf("$%d :    %d (%x)\n",i,x,x);
  }
  x = (*(state->registers))[PC];
  printf("PC  :     %d (%x)\n",x,x);
  
  x = (*(state->registers))[CPSR];
  printf("CPSR:     %d (%x)\n",x,x);
  printf("Non-zero memory:\n");
  for (unsigned int i = 0; i< 100; i+=4){
    x = (*(state->memory))[i/4];
    if (x){
      printf("0x%x: 0x%x\n",i,x);
    }
  }
}

int main(int argc, char **argv){
  // --------------------------- loader --------------------------------
  char *filename =argv[1];
  printf("opening :  %s\n", filename);
  //open the file
  FILE *in = fopen(filename,"rb");
  if (!in){
    printf("Failed to read file ");
    return EXIT_FAILURE;
  }
  
  //go to the end to find out how large the file is, then go back to the start
  fseek(in,0,SEEK_END);
  int size = (int) ftell(in);
  int n_lines = size/4;
  fseek(in,0,SEEK_SET);
  printf("file size of %d bytes, so %d instructions, each 4 bytes long \n",size,n_lines);
  
  unsigned int *heap_instr = (unsigned int *) calloc(n_lines+1,4);
  unsigned int *heap_reg = (unsigned int *) calloc(17,4);
  unsigned int *heap_mem = (unsigned int *) calloc(2048,4);
  assert(heap_instr && heap_reg && heap_mem);
  fread(heap_instr,4,n_lines,in);
  fclose(in);

  arm_state state;
  state.instructions = &heap_instr;
  state.registers = &heap_reg;
  state.memory = &heap_mem;
  /*
  for (int i = 0; i <= n_lines ;i++){
    unsigned int x = (*state.instructions)[i];
    printf("hex: %x bin :",x);
    binprint(x);
  }
  */
  
  //-------------------------------------------------------------------------

  unsigned int ignore_command = 0xf3a010ff;
  state.pipeline.fetched = ignore_command;
  state.pipeline.decoded = decode(ignore_command,&state);
  state.halted = 0;
  unsigned int a,b;
  a=b=ignore_command;
  // { some for loop initialising each register to 0 and all memory to 0, and also the pipeline
  /*
  while (0 &&!state.pipeline.fetched){
    unsigned int nextFetched = (*state.instructions)[state.registers[PC]/4];
    Instruction nextDecoded = decode(state.pipeline.fetched);
    int branched = execute(&state);//PC++;
    if (!branched){
      state.pipeline.fetched= nextFetched;
      state.pipeline.decoded = nextDecoded;
    }    
  }
  */
  do {
    printf("PC : %d, %x\n",(*state.registers)[PC],a);
    int branched = execute(&state);
    state.pipeline.decoded = decode(state.pipeline.fetched,&state);
    if (branched) state.pipeline.decoded =decode(ignore_command,&state);
    state.pipeline.fetched = (*state.instructions)[(*state.registers)[PC]/4];
    (*state.registers)[PC]+=4;
    a=b;
    b=state.pipeline.fetched;
  }while (!state.halted);
  printf("finished\n");
  printRegisters(&state);
  return EXIT_SUCCESS;
}
