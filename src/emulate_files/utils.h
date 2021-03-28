#include "common.h"

//decode functions

//gets a certain number of bits from the command
int getBits(unsigned int x, int point, int length);

//prints the command in binary, 8 sets of 4
void binprint(unsigned int x);

unsigned int modifyBit(unsigned int number, int position,int binary);
unsigned int shiftRegister(arm_state *state);

unsigned int extend(int immediate);

void update_nflag(unsigned int result, arm_state *state);
void update_zflag(unsigned int result, arm_state *state);
void update_cflag_add(unsigned int first, unsigned int second, arm_state *state);
void update_cflag_sub(unsigned int first, unsigned int second, arm_state *state);
void no_update(unsigned int a, unsigned int b, arm_state *state);

unsigned int big_to_little_endian(unsigned int x);

void printRegisters(arm_state *state,int n_lines);

