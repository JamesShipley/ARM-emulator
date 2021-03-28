#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct Pair{
  char *label;
  int value;
} Pair;

typedef struct Symbol_table {
  Pair *symbols;
  int size;
} Symbol_table;

void add(Symbol_table *table, const char *label, const int value){
  Pair new_pair = {label, value};
  table->symbols[table->size] = new_pair;
  table->size++;
}

bool getValue(Symbol_table *table, const char *label, int* val_out){
  for (int i = 0; i < table->size; i++){
    if (!strcmp(table->symbols[i].label, label)){
      *val_out = table->symbols[i].value;
      return true;
    }
  }
  return false;
}

Symbol_table *create_symbol_table(int size){
  Symbol_table *table = calloc(1,sizeof(Symbol_table));
  if (!table){
    perror("Error in allocating space for symbol table");
    exit(EXIT_FAILURE);
  }
  table->symbols = calloc(size, sizeof(Pair));
  table->size = 0;
  return table;
}

int main(int argc, char **argv) {
  return EXIT_SUCCESS;
}
