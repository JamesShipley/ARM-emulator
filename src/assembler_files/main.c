#include "main.h"

/* 
void add(Assembler *asmblr, char *key, int value){
    char *key_cpy = calloc(strlen(key)+1,sizeof(char));
    strcpy(key_cpy,key);
    (asmblr->label_table).symbols[(asmblr->label_table).current_size] = {key_cpy,value};
    (asmblr->label_table).current_size++;
} 
*/

//loads all of the instructions in the input file to an array of strings. [james]/loader
void load_instructions(Assembler *asmblr){
  //get the file size so we have an upper bound for the number of lines in the file.
  fseek(asmblr->in,0,SEEK_END);
  int size = ftell(asmblr->in);
  fseek(asmblr->in,0,SEEK_SET);

  //initialise the assembler struct variables/attributes.
  asmblr->instructions = (char **) calloc(size,sizeof(char *));
  asmblr->total_n_lines = 0;
  asmblr->total_n_labels = 0;

  //local variables for loading the instructions and etc.
  char sentence_buffer[511];
  int str_size;
  int curr_line = 0;


  //reads each line and stores it in the instruction array.
  while (fgets(sentence_buffer,sizeof(sentence_buffer),asmblr->in)){
      //removes any \n newline character from the end of the sentence.
      sentence_buffer[strcspn(sentence_buffer,"\n")]=0;
      //allocates space in instructions for the sentence, then copies the sentence into the allocated space.
      str_size = strlen(sentence_buffer) + 1;
      if (str_size == 1){
        break;
      }
      asmblr->instructions[curr_line] = (char *) calloc(str_size,sizeof(char));
      strcpy(asmblr->instructions[curr_line],sentence_buffer);
      if (isLabel(sentence_buffer)){
	  asmblr->total_n_labels++;
	  //printf("Label--");
      }
      //printf("line %d: code : %s\n",curr_line,asmblr->instructions[curr_line]);
      curr_line++;
  }
  //printf("total lines: %d, total labels : %d\n",curr_line,asmblr->total_n_labels);
  asmblr->total_n_lines = curr_line;
}

//completes the first pass on the code, populating the symbol table with labels. [james]/main
void first_pass(Assembler *asmblr){
  //printf("first pass: n_labels: %d, total_lines: %d\n",asmblr->total_n_labels,asmblr->total_n_lines);
  
  int n_labels_so_far=0;
  for (int i = 0 ; i < asmblr->total_n_lines; i++){
    //printf("----i:%d----\n",i);
    char temp[511];
    strcpy(temp,asmblr->instructions[i]);
    //printf("current line: %d, code : %s\n",i,temp);
    if (isLabel(temp)){
      // -- POSSIBLE SEGFAULT --
      sentence token;
      strcpy(token.v,strtok(temp,":"));
      //printf("adding label %s \n",token.v);
      add(&(asmblr->label_table),token,4*(i-n_labels_so_far));
      n_labels_so_far++;
      //printf("after labeled\n");
    }
    else{
      //printf("no label\n");
    }
  }
  printf("everything in the table:\n");
  for (int i=0;i<asmblr->label_table.current_size;i++){
    printf("label %d: %s\n",i,asmblr->label_table.symbols[i].label);
  }
  
}

//completes the second pass, resulting in an array of assembled instructions. [james]/main
void second_pass(Assembler *asmblr){
  asmblr->curr_num_constants = 0; 
  asmblr->assembled_i = asmblr->total_n_lines - asmblr->total_n_labels;
  //allocate space for the array
  sentence temp;
  asmblr->assembled_instructions = calloc(asmblr->assembled_i,sizeof(unsigned int));
  if (!asmblr->assembled_instructions){
    perror("calloc fail");
    exit(EXIT_FAILURE);
  }
  
  int n_labels_so_far=0;
  for (int i = 0; i < asmblr->total_n_lines; i++){
    asmblr->curr_instr= i-n_labels_so_far;  
    strcpy(temp.v,asmblr->instructions[i]);
    //printf("assembling instruction %d,code : %s\n",i,temp.v);
    if (isLabel(temp.v)) {
      n_labels_so_far++;
      continue;
    }
    //printf("not label\n");
    unsigned int k = assemble(temp,asmblr);
    //printf("saving %x to %d \n",k,i-n_labels_so_far);
    asmblr->assembled_instructions[i-n_labels_so_far]=k;
  }
}
void free_all(Assembler *asmblr){
  //close input and output files
  fclose(asmblr->in);
  fclose(asmblr->out);
  //free all assembled instructions
  free(asmblr->assembled_instructions);
  //printf("freed the assembled_instructions\n"); 
  //free the whole symbol table
  //printf("total labels: %d\n",asmblr->total_n_labels);
  for (int i=0;i<asmblr->total_n_labels;i++){
    //printf("freeing %s\n",asmblr->label_table.symbols[i].label);
    free(asmblr->label_table.symbols[i].label);
  }
  //printf("freed each label\n");
  free(asmblr->label_table.symbols);
}

//writes to the binary file. [james]\utils
void write_to_bin_file(Assembler *asmblr){
  fwrite(asmblr->assembled_instructions,sizeof(unsigned int),asmblr->assembled_i,asmblr->out);
  if (asmblr->curr_num_constants){
    printf("num constants: %d\n",asmblr->curr_num_constants);
    fwrite(asmblr->constants,sizeof(unsigned int),asmblr->curr_num_constants,asmblr->out);
  }
}

//main function, where everything is called. [james]
int main(int argc, char **argv){
    //start with creating an assembler struct to store all relevant arrays, files and variables.
    Assembler asmblr;

    //open the input and output files for reading and writing to.
    asmblr.in=fopen(argv[1],"r");
    asmblr.out=fopen(argv[2],"wb");

    //load the instructions:
    load_instructions(&asmblr);

    //initialise the symbol table
    initialise_symbol_table(&asmblr);

    //complete the first pass
    first_pass(&asmblr);
    //printf("after first pass-----\n");

    //complete the second pass, resulting in an array of assembled instructions.
    second_pass(&asmblr);

    //writes to the binary file.
    write_to_bin_file(&asmblr);
    free_all(&asmblr);
    return EXIT_SUCCESS;
}
