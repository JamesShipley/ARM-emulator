#include "utils.h"

//utils functions

bool isLabel(char *sentence){
  return strstr(sentence,":")!=NULL;
}

int getBits(unsigned int x, int point, int length){
  return (x>>point)%(1<<length);
}
unsigned int opcode_mapping(int index){
  //Symbol_table table = {{{"beq",0},{"bne",1},{"bge",10},{"blt",11},{"bgt",12},
  //                     {"ble",13},{"bal",14},{"b",14}}, 8};

  int mapping[17][2] = {{0,4},{1,2},{2,3},{3,0},{4,1},{5,12},{6,13},{7,8},{8,9},{9,10},
                       {14,0},{15,1},{16,10},{17,11},{18,12},{19,13},{20,14}};
  for(int i=0;i<17;i++){
      if (mapping[i][0] == index) return mapping[i][1];
  }
  perror("Could not find opcode mapping");
  exit(EXIT_FAILURE);
}
int lookup(sentence sent){
  strtok(sent.v," ");
  //printf("searching in lookup for %s\n",sent.v);
  char names[23][10] = {"add","sub","rsb","and","eor","orr",
		  "mov","tst","teq","cmp","mul","mla",
		  "ldr","str","beq","bne","bge","blt",
		  "bgt","ble","b","lsl","andeq"};
  for (int i=0; i<23;i++){
    //printf("comparing |%s| to |%s|",sent.v,names[i]);
    if (!strcmp(sent.v,names[i])){
      //printf("success, returning %d\n",i);
      return i;
    }
    //printf(", not the same\n");
  }
  return -1;
}
/*
void depack(Depacked_instr *instr, char *sentence){
  int n_args =0;
  char str[511];
  strcpy(str,sentence);
  const char s[] = " ,";
  char *token;
  token = strtok(str,s);
  token = strtok(NULL,s);
  while (token != NULL){
    strcpy(instr->args[n_args],token);
    token = strtok(NULL,s);
    n_args++;
  }
  instr->n_args=n_args;
  instr->index = lookup(sentence);
  }*/

void add(Symbol_table *table,sentence label, const int value){
  table->symbols[table->current_size].label = calloc(511,sizeof(char));
  strcpy(table->symbols[table->current_size].label,label.v);
  table->symbols[table->current_size].value =value;
  table->current_size++;
}

bool getValue(Symbol_table *table,sentence label_to_get, int* val_out){
  //printf("in getValue %d, searching for : %s\n",table->current_size,label_to_get.v);
  for (int i = 0; i < table->current_size; i++){
    //printf("comparing to (in table): %s\n",table->symbols[i].label);
    if (!strcmp(table->symbols[i].label, label_to_get.v)){
      *val_out = table->symbols[i].value;
      //printf("gotten value\n");
      return true;
    }
  }
  return false;
}

/*
Symbol_table *create_symbol_table(int size){
  Symbol_table *table = calloc(1,sizeof(Symbol_table));
  if (!table){
    perror("Error in allocating space for symbol table");
    exit(EXIT_FAILURE);
  }
  table->symbols = calloc(size, sizeof(Pair));
  table->current_size = 0;
  return table;
}*/

//initialises the symbol table, allocating enough space for the labels. [james]/utils
void initialise_symbol_table(Assembler *asmblr){
    asmblr->label_table.symbols = calloc(asmblr->total_n_labels+1,sizeof(Pair));
    asmblr->label_table.current_size = 0;
}

// a util for assemble_dp specifically
unsigned int str_to_operand2 (char *str, unsigned int *i){ //i bit is cleared if operand is a register
  char buffer[2];
  strncpy(buffer,str,2);
  if (!strcmp(buffer, "0x")){ //operand is given in hex
    return (unsigned int) strtol(str + 2, NULL, 16);
  }
  if (buffer[0] == 'r'){ //operand is given as register
    *i = 0;
    return (unsigned int) atoi(str + 1);
  }
  else{ //operand is given in decimal
    return (unsigned int) atoi(str);
  }
}
sentence cpy_sent(sentence s){ return s;}

void removeChar(char *str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}
