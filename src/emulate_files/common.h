#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#define PC 15
#define CPSR 16
#define MAX_MEM 65536

//Instruction structures
typedef struct{
    int cond,
    I,
    Opcode,
    S,
    Rn,
    Rd,
    Op2;
    
}DataProc;

typedef struct{int cond,
A,
S,
Rd,
Rn,
Rs,
Rm;
}Mult;

typedef struct{
    int cond,
    I,
    P,
    U,
    L,
    Rn,
    Rd,
    Offset;
}SingleDT;

typedef struct{
    int cond,
    Offset;
}Branch;

//an enum for the instruction structure
enum instruction_type{ DATAPROC=1, MULT=2, SINGLEDT=3, BRANCH=4, IGNORE=5, HALT=6};

//A generic Instruction structure.
typedef struct{
    enum instruction_type type;
    union{DataProc dp;Mult m;SingleDT sdt;Branch b;};
}Instruction;

// A pipeline structure.
typedef struct {
  unsigned int fetched;
  Instruction decoded;
} pipeline;
// The main ARM structure, containing the state of the ARM.
typedef struct {
  unsigned int **instructions;
  unsigned int **registers;
  unsigned char *memory;
  pipeline pipeline;
  int halted;
} arm_state;
