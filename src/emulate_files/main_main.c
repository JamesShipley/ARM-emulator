#include "main_main.h"


int main(int argc, char **argv){
  // --------------------------- loader --------------------------------
  //the filename is the first non-trivial argument
  char *filename =argv[1];
  
  //open the file
  FILE *in = fopen(filename,"rb");
  if (!in){
    perror("Failed to read file ");
    return EXIT_FAILURE;
  }

  //go to the end to find out how large the file is, then go back to the start
  fseek(in,0,SEEK_END);
  int size = (int) ftell(in);
  int n_lines = size/4;
  fseek(in,0,SEEK_SET);
  
  
  //allocate memory for registers, instructions and arm_memory
  unsigned int *heap_instr = (unsigned int *) calloc(n_lines+1,4);
  unsigned int *heap_reg = (unsigned int *) calloc(17,4);
  //assert that no callocs failed
  assert(heap_instr && heap_reg);
  
  //read all of the instructions into the instructions array.
  fread(heap_instr,4,n_lines,in);
  fclose(in);
  //-------------------------------------------------------------------------
  
  //create the ARM state struct and initialise.
  arm_state state;
  state.instructions = &heap_instr;
  state.registers = &heap_reg;
  state.memory = (unsigned char *) calloc(MAX_MEM,1);
  assert(state.memory);
  unsigned int ignore_command = 0xf3a010ff;
  state.pipeline.fetched = ignore_command;
  state.pipeline.decoded = decode(ignore_command,&state);
  state.halted = 0;
  int a,b;
  int branched = 0;
  a=b=ignore_command;
  cpymem(&state,n_lines);
    //this is the main loop for the emulator
  do {  
    //execute
    //printf("PC %d, instr: %x\n",(*state.registers)[PC], a);
    branched = execute(&state);
    if (state.halted) break;
    //decode
    state.pipeline.decoded = decode(state.pipeline.fetched,&state);
    //if branched, flush pipeline
    if (branched) state.pipeline.decoded =decode(ignore_command,&state);
    //fetch next instruction
    state.pipeline.fetched = (*state.instructions)[(*state.registers)[PC]/4];
    //increment PC
    (*state.registers)[PC]+=4;
    a=b;
    b=state.pipeline.fetched;
  }while (!state.halted);
  
  
  printRegisters(&state,n_lines);
  return EXIT_SUCCESS;
}
