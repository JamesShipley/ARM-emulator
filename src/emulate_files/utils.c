#include "utils.h"

//decode functions

//gets a certain number of bits from a specific position in the 4 byte instruction.
int getBits(unsigned int x, int point, int length){
  return (x>>point)%(1<<length);
}
//prints the command in binary, 8 sets of 4 (legacy function).
void binprint(unsigned int x){
  for (int i=31; i>-1 ;i--){
    if (i%4 ==3 && i) printf(" ");
    printf("%d",getBits(x,i,1));
  }
  //printf("\n");
}

//modifies a specific bit within a specific position in the number to either 1 or 0.
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

//these are the 4 flag changing functions, as well as a function that doesnt do anything.
void update_nflag(unsigned int result, arm_state *state){
  int nbit = getBits(result,31,1);
  (*(state->registers))[CPSR] = modifyBit((*(state->registers))[CPSR],31,nbit);
}
void update_zflag(unsigned int result, arm_state *state){
  if (result == 0){
    (*(state->registers))[CPSR] = modifyBit((*(state->registers))[CPSR],30,1);
  }
  //(*(state->register))[CPSR] = modifyBit((*(state->register))[CPSR],30,!result);
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

unsigned int get_val_from_mem(arm_state *state, int address, int *error){
  if ((address> MAX_MEM - 4) || address < 0){
    printf("Error: Out of bounds memory access at address 0x%08x\n",address);
    return *error = 1;
  }
  unsigned int sum=0;
  for (int i=0;i<4;i++) sum += (state->memory[address+i])<<(i*8);
  return sum;
}
void store_val_to_mem(arm_state *state,unsigned int value,int address){
  if ((address>MAX_MEM - 4) || address < 0){
    printf("Error: Out of bounds memory access at address 0x%08x\n",address);
    return;
  }
  //printf("moving %x to index %d\n",value,address);
  for (int i=0;i<4;i++){
    unsigned char byte = getBits(value,i*8,8);
    //printf("byte %02x goes in address %d\n",byte,i+address);
    state->memory[address+i] = byte;
  }
}

void cpymem(arm_state *state,int n_instr){
  for (int i=0;i<n_instr;i++){
    unsigned int val =(*(state->instructions))[i];
    store_val_to_mem(state,val,i*4);
  }
}


//converts a number in big endian format to one in little endian format.
unsigned int big_to_little_endian(unsigned int x){
    unsigned int sum = 0;
    for (int i=0;i<32;i+=8) sum += getBits(x,i,8)<<(24-i);
    return sum;
}

void print_mem(arm_state* state){
  printf("Non-zero memory:");
  int error;
  for (int i=0;i<MAX_MEM;i+=4){
    if (get_val_from_mem(state,i, &error)){
      printf("\n0x%08x: 0x",i);
      for (int j=0;j<4;j++)printf("%02x",state->memory[i+j]);
    }
  }
  printf("\n");
}

//prints an integer with a specific number of spaces in front of it, used for formatting.
void print_with_buffer(int x){
  printf(":");
  int temp_x = x;
  if (temp_x<0) temp_x = -temp_x;
  else printf(" ");
  int counter = 0;
  while (temp_x>10){
    temp_x/=10;
    counter++;
  }
  int i=0;
  while (i++<(9-counter))printf(" ");
  //for (int i=0;i<(9-counter);i++)printf(" ");
  printf("%d",x);
}

  
//prints the values in each register and also in non-zero memory
/*
void printRegisters(arm_state *state,int n_lines){
  unsigned int x;
  printf("Registers:\n");
  for (int i =0; i<=12 ; i++){
    x = (*(state->registers))[i];
    printf("$%d ",i);
    if (i<10) printf(" ");
    print_with_buffer(x);
    printf(" (0x%08x)\n",x);
  }
  x = (*(state->registers))[PC];
  printf("PC  ");
  print_with_buffer(x);
  printf(" (0x%08x)\n",x);

  x = (*(state->registers))[CPSR];
  printf("CPSR");
  print_with_buffer(x);
  printf(" (0x%08x)\n",x);
  print_mem(state);
}

*/

void printRegisters(arm_state *state, int n_lines){
  unsigned int x;
  char reg_str[25];
  printf("Registers:\n");
  for (int i = 0; i <= 12; i++){
    x = (*(state->registers))[i];
    printf("$%d ", i);
    if (i < 10) {
      printf(" : ");
    }
    else{
      printf(": ");
    }

    sprintf(reg_str, "%d" , x);
    printf("%*s", 10, reg_str);
    printf(" (0x%08x)\n",x);
  }

  x = (*(state->registers))[PC];
  printf("PC  : ");
  sprintf(reg_str, "%d" , x);
  printf("%*s", 10, reg_str);
  printf(" (0x%08x)\n",x);

  x = (*(state->registers))[CPSR];
  printf("CPSR: ");
  sprintf(reg_str, "%d" , x);
  printf("%*s", 10, reg_str);
  printf(" (0x%08x)\n", x);

  print_mem(state);
}
