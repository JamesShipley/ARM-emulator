#include "decode.h"
//decodes an instruction (unsigned int), returning an Instruction struct
//because instruction contains only integers and 1 enum, we can pass it through by value without performance issues.
Instruction decode(unsigned int x, arm_state* state){

  //printf("current instruction: %08x, ",x);
  //binprint(x);

  int Cond = getBits(x,28,4);
  //the partial specifier can be used to tell whether an instruction is a singleDT or Branch
  int partial_specifier = getBits(x,26,2);
  Instruction ret;
  
  if (partial_specifier ==1){//if its a singleDT instruction
    //printf(", its a singleDT\n");
    SingleDT instr = {
		      Cond,
		      getBits(x,25,1),
		      getBits(x,24,1),
		      getBits(x,23,1),
		      getBits(x,20,1),
		      getBits(x,16,4),
		      getBits(x,12,4),
		      getBits(x,0,12)
    };
    ret.type =SINGLEDT;
    ret.sdt = instr;
    return ret;
  }

  else if (partial_specifier ==2){//if its a Branch instruction
    //printf(", its a branch\n");
    Branch instr = {
		    Cond,
		    getBits(x,0,24)};
    ret.type = BRANCH;
    ret.b = instr;
    return ret;
  }

  else if (getBits(x,22,6)==0 && getBits(x,4,4)==9){//if its a multiply instruction
    //might unintentionally think a dp instr is a mult instr.
    //printf(", its a mult \n");
    Mult instr = {
		  Cond,
		  getBits(x,21,1),
		  getBits(x,20,1),
		  getBits(x,16,4),
		  getBits(x,12,4),
		  getBits(x,8,4),
		  getBits(x,0,4)
    };
    
    ret.type =MULT;
    ret.m = instr;
    return ret;
  }
  else{ //otherwise its a data processing instruction.
    //if (x==0xf3a010ff) printf(", its an IGNORE\n");
    //else if (x==0) printf(", its a HALT\n");
    //else printf(", its a DP\n");
    DataProc instr = {
		      Cond,
		      getBits(x,25,1),
		      getBits(x,21,4),
		      getBits(x,20,1),
		      getBits(x,16,4),
		      getBits(x,12,4),
		      getBits(x,0,12)
    };
    ret.type = x == 0 ? HALT : DATAPROC;
    ret.dp =  instr;
  }

  return ret;
}
