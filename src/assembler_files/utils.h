#include "common_structs.h"

bool isLabel(char *sentence);
int getBits(unsigned int x, int point, int length);
unsigned int opcode_mapping(int index);
int lookup(sentence sent);
//void depack(Depacked_instr *instr, char *sentence);

void add(Symbol_table *table,sentence label, const int value);
bool getValue (Symbol_table *table,sentence label_to_get, int *val_out);
Symbol_table *create_symbol_table(int size);

void initialise_symbol_table(Assembler *asmblr);

unsigned int str_to_operand2 (char *str, unsigned int *i);


