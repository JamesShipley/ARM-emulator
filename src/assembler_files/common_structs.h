#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>


typedef struct{
    char *label;
    int value;
} Pair;

typedef struct {
    Pair *symbols;
    int current_size;
} Symbol_table;

typedef struct {
  FILE * in;
  FILE * out;
  int input_file_size;
  Symbol_table label_table;
  int total_n_labels;
  char **instructions;
  int total_n_lines;
  int curr_instr;
  int curr_index;
  unsigned int *assembled_instructions;
  unsigned int constants[10];
  int curr_num_constants;
  int assembled_i;
} Assembler;

typedef struct{
    int index;
    char args[5][511];
    int n_args;
} Depacked_instr;

typedef struct{char v[511];}sentence; 

