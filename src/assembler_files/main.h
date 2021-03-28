#include "assemble_main.h"

// void add(Assembler *asmblr, char *key, int value);
void load_instructions(Assembler *asmblr);
void first_pass(Assembler *asmblr);
void second_pass(Assembler *asmblr);
void write_to_bin_file(Assembler *asmblr);
int main(int argc, char **argv);
