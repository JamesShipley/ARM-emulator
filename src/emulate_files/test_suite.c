int file_compare(char *f_1,char *f_2){
  FILE * file_1 = fopen(f_1,"r");
  FILE * file_2 = fopen(f_2,"r");

  char sentence_1[1000];
  char sentence_2[1000];
  while (fgets(sentence_1,sizeof(sentence_1),file_1) &&fgets(sentence_2,sizeof(sentence_2),file_2)){
    if (strcmp(sentence_1,sentence_2)){
      perror("these two files are not the same!");
      return EXIT_FAILURE;
    }
  }
  printf("these two files are the same");
  return 


int main (int argc,char **argv){
  

  
