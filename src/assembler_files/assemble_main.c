#include "assemble_main.h"

int contains(sentence sent,const char *val){
    return (strstr(sent.v,val)!=NULL);
}

unsigned int assemble_dp(sentence sent,Assembler *asmblr){
  //printf("assemble_dp: %s \n",sent.v); 
  unsigned int cond = 14;
  unsigned int i = 1;
  unsigned int opcode;
  unsigned int s = 0;
  unsigned int rn = 0;
  unsigned int rd = 0;
  unsigned int operand2;


  char *register2;
  char *value;

  strtok(sent.v, " ,#");
  opcode = opcode_mapping(asmblr->curr_index);
  char *register1 = strtok(NULL, " ,#");
  
  if (opcode <= 4 || opcode == 12){
    rd = atoi(register1 + 1);
    register2 = strtok(NULL, " ,#");
    rn = atoi(register2 + 1);
    value = strtok(NULL, " ,#");
    operand2 = str_to_operand2(value, &i);
  }

  else if(opcode == 13){
    rd = atoi(register1 + 1);
    value = strtok(NULL, " ,#");
    operand2 = str_to_operand2(value, &i);
  }

  else{
    s = 1;
    rn = atoi(register1 + 1);
    value = strtok(NULL, " ,#");
    operand2 = str_to_operand2(value, &i);
  }

  /*printf("cond : %u \n", cond);
  printf("i: %u\n", i);
  printf("opcode : %u \n", opcode);
  printf("s : %u \n", s);
  printf("rn : %u \n", rn);
  printf("rd : %u \n", rd);
  printf("operand2 : %u \n", operand2);
  */
  return (cond << 28) + (i << 25) + (opcode << 21) + (s << 20) + (rn << 16)
         + (rd << 12) + (operand2);
}

unsigned int assemble_special(sentence sent,Assembler *asmblr){
  //printf("assemble special \n");
  //andeq
  if (asmblr->curr_index == 22) return 0;
  //lsl
  strtok(sent.v," ,");
  char *Rn = strtok(NULL, " ,");
  char *expr = strtok(NULL," ,");
  //create a mov sentence

  sentence new_sentence;
  char str[] = "mov %s, %s, lsl %s";
  sprintf(new_sentence.v,str,Rn,Rn,expr);
  //printf("final sentence: %s\n",new_sentence.v);
  asmblr->instructions[asmblr->curr_instr] = new_sentence.v;
  return assemble(new_sentence,asmblr);
}

unsigned int assemble_m(sentence sent,Assembler *asmblr){
  //printf("assemble mult: %s\n",sent.v);
  unsigned int cond = 14;
  unsigned int a = 0;
  unsigned int s = 0;
  unsigned int rd;
  unsigned int rn = 0;
  unsigned int rs;
  unsigned int rm;


  char *register4;
  
  
  char *multiply = strtok(sent.v,  " ,");
  char *register1 = strtok(NULL, " ,");
  char *register2 = strtok(NULL, " ,");
  char *register3 = strtok(NULL, " ,");

  rd = atoi(register1 + 1);
  rm = atoi(register2 + 1);
  rs = atoi(register3 + 1);

  if (!strcmp(multiply, "mla")){
    a = 1;
    register4 = strtok(NULL, " ,");
    rn = atoi(register4 + 1);
  }

  return (cond << 28) + (a << 21) + (s << 20) + (rd << 16) + (rn << 12)
         + (rs << 8) + (9 << 4) + rm;
}
unsigned int assemble_b (sentence sent,Assembler *asmblr){
  //printf("in branch: %s\n",sent.v);
  unsigned int cond;
  int offset;
  
  cond = opcode_mapping(asmblr->curr_index);
  strtok(sent.v," ");
  sentence expression;
  strcpy(expression.v,strtok(NULL, " "));
  
  if (!getValue(&(asmblr->label_table), expression, &offset)){ //target
    printf("value not returned to assemble_b\n");
  }

  if (offset < asmblr->curr_instr * 4){ //target address is less than the current address
    offset = offset - ((asmblr->curr_instr)*4 + 8);
  }

  else{
    offset = offset - ((asmblr->curr_instr)*4 + 8);
  }
  printf("offset is %d\n",offset);
  unsigned int q = offset;
  q= q &((1<<26) -1);
  q = q<<2;
  if (offset<0){
    offset = q | 0x000000F;
  }
  else offset = (int) q;
  //printf("Offset is : %d, %x \n", offset, offset);
  offset = offset  & ((1 << 24) - 1);
  //printf("Offset changes to : %d, %x \n", offset, offset);
  return (cond << 28) + (10 << 24) + offset;
}
/*
unsigned int big_hex(char *value){
  char *without_x,part_a[51],part_b[51];
  strcpy(without_x,value+2);
  strcpy(part_a,without_x);
  part_a[4]=0;
  strcpy(part_b,without_x+4);
  printf("without_x:%s, part_1:%s,part2:%s\n",without_x,part_a,part_b);
  int str_size = strlen(without_x);
  return 5;
}*/

unsigned int ldr_const(sentence sent,Assembler *asmblr){
  unsigned int cond=14,i=0,p=1,u=1,l=1,Rd,Rn=15,Offset,constant;
  char *reg_1;
  sentence sentcpy;
  strcpy(sentcpy.v,sent.v);
  //ldr
  strtok(sent.v, " ");
  //negative
  if (contains(sent,"-")){
    u=0;
    reg_1 = strtok(NULL,",=-");
  }
  else reg_1 = strtok(NULL,",=");
  char *value = strtok(NULL,"=");
  Rd = atoi(reg_1+1); 
  //hex or denary
  if (contains(sentcpy,"x")){
    constant = strtol(value,NULL,16);
  }
  else{
    constant = strtol(value,NULL,10);
  }
  //mov
  if (constant<256){
    sentence new_sentence;
    char str[] = "mov %s, #%s";
    sprintf(new_sentence.v,str,reg_1,value);
    return assemble(new_sentence,asmblr);
  }
  //proper constant
  asmblr->constants[asmblr->curr_num_constants]=constant;
  unsigned int mem_address = (asmblr->assembled_i + asmblr->curr_num_constants) * 4;
  //printf("saving %x to address %d\n",constant,mem_address);
  Offset = (mem_address - asmblr->curr_instr*4) - 8;
  //printf("curr_instr is %d so offset is %d\n",asmblr->curr_instr,Offset);
  asmblr->curr_num_constants++;
  unsigned int result =(cond<<28)+(1<<26)+ (i << 25) + (p << 24) + (u << 23) + (l << 20);
  return result + (Rn<<16) +(Rd<<12) +Offset;
}

unsigned int ldr_reg(sentence sent,Assembler *asmblr){
  unsigned int cond=14,i=0,p=1,u=1,l=1,Rd,Rn,Offset=0,start_bit=12;
  char str_Rd[2] ={sent.v[5],0};
  char str_Rn[2] ={sent.v[9],0};
  Rd = atoi(str_Rd);
  Rn = atoi(str_Rn);
  printf("Rd: %d, Rn: %d\n",Rd,Rn);
  if (contains(sent,"#")){
    if (sent.v[start_bit]=='-'){
      u=0;
      start_bit++;
    }
    if (contains(sent,"x")){
      Offset = strtol(sent.v+start_bit,NULL,16);
    }
    else Offset = strtol(sent.v+start_bit,NULL,10);
  }
      
  unsigned int result =(cond<<28)+(1<<26)+ (i << 25) + (p << 24) + (u << 23) + (l << 20);
  return result + (Rn<<16) +(Rd<<12)+Offset;
}

unsigned int assemble_str(sentence sent, Assembler *asmblr){
  unsigned int cond=14,i=0,p=1,u=1,l=0,Rd,Rn,Offset=0,str_size=strlen(sent.v)-1;
  char *str_Rn,*sent_cpy=sent.v, str_Rd[2] ={sent.v[5],0};
  Rd = atoi(str_Rd);
  
  if (sent.v[str_size]==']'){
    //pre indexing
    if (contains(sent,"#")){
      strtok_r(sent_cpy,"[",&sent_cpy);
      str_Rn = strtok_r(sent_cpy,",",&sent_cpy);
      Rn = atoi(str_Rn+1);
      char *val = strtok_r(sent_cpy," #",&sent_cpy);
      //for (int i=0;(val[i]==']')&&(1+(val[i]=0));i++){};
      val[strlen(val)-1]=0;
      printf("sent:%s, Rn:%s, val:%s\n",sent_cpy,str_Rn,val);
      Offset=atoi(val);
    }
    else{
      printf("optional pre index shift\n");
    }
  }
  else{
    //post indexing
    p=0;
    if (contains(sent,"#")){
      strtok_r(sent_cpy,"[",&sent_cpy);
      str_Rn = strtok_r(sent_cpy,",",&sent_cpy);
      Rn = atoi(str_Rn+1);
      while (*sent_cpy!='#')sent_cpy++;
      sent_cpy++;
      Offset= atoi(sent_cpy);
    }
    else{
      printf("optional post index shift");
    }  
  }
  unsigned int result =(cond<<28)+(1<<26)+ (i << 25) + (p << 24) + (u << 23) + (l << 20);
  return result + (Rn<<16) +(Rd<<12)+Offset;
}

unsigned int assemble_sdt(sentence sent, Assembler *asmblr){
  if (contains(sent,"ldr")){
    //ldr
    //printf("checkpoint 1 \n");
    
    if (contains(sent, "[")){
      //reg case
      //printf("check point 2.a 2\n");
      return ldr_reg(sent, asmblr);
    }
    else {
      //const case
      //printf("checkpoint 2.b \n");
      unsigned int a =ldr_const(sent,asmblr);
      return a;
    }
  }
  //str
  printf("str functions\n");
  return assemble_str(sent,asmblr);
}

unsigned int assemble_sdt_legacy(sentence sent,Assembler *asmblr) {
  //printf("assemble sdt: %s\n",sent.v);
  unsigned int cond = 14;
  unsigned int i = 0;
  unsigned int p = 0;
  unsigned int u = 0;
  unsigned int l = 0;
  unsigned int rn = 0;
  unsigned int result = 0;
  unsigned int num_constants = asmblr->curr_num_constants;
  char *mnemonic = strtok(sent.v, " ,#");
  char *rd = strtok(NULL, " ,#");
  

  
  if (!strcmp(mnemonic,"ldr")){
    if (strstr(sent.v,"[")==NULL){
      char *address = strtok(NULL, " ,#");
      if (address[1] != '-'){
	u = 1;
      }

      l = 1;
      unsigned int constant;

      if (*address == '=') {
	if (strstr(address,"x")!=NULL){
	  constant = strtol(address+1,NULL,16);
	}
	else{
	  constant=strtol(address+1,NULL,10);
	}  
	if (constant > 255){
	  asmblr->constants[num_constants] = constant;
	  unsigned int mem_address = (asmblr->assembled_i + asmblr->curr_num_constants) * 4;
	  p = 1;
	  rn = 15;
	  result = mem_address - asmblr->curr_instr - 8; // could be + 8
	  num_constants++;
	  return 4;
	}
	else {
	  sentence new_sentence;
	  char str[] = "mov %s, #%s";
	  sprintf(new_sentence.v,str,rd,address+1);
	  //printf("final sentence: %s\n",new_sentence.v);
	  return assemble(new_sentence,asmblr);
	}
      }
      else {
	//ldr r0, [r2];
      }
    }
    //--------------register
    else{
      printf("store instruction\n");

    }
  }
  
  char *fourth = strtok(NULL, " ,");

  if (fourth) {
    if (strchr(fourth, ']')) {
      p = 1;
    }
    result += atoi(fourth);
  }
  result += (cond << 28) + (1 << 26) + (i << 25) + (p << 24) + (u << 23) 
	+ (l << 20) + (rn << 16) + (atoi(rd) << 12);
  return result;
}


unsigned int assemble(sentence sent, Assembler *asmblr){
  //printf("in assemble, sentence %s\n",sent.v);
  //printf("sentence address : %p\n",(void*)&sent.v);
  int index = lookup(sent);
  if (index==-1)printf("lookup failed\n");
  //else printf("lookup succeded, sent is now %s\n",sent.v);
  asmblr->curr_index=index;
  unsigned int (*assemble_instr)(sentence,Assembler*)=assemble_special;

  if (index<=9) assemble_instr = assemble_dp;
  else if (index<=11) assemble_instr = assemble_m;
  else if (index<=13) assemble_instr = assemble_sdt;
  else if (index<=20) assemble_instr = assemble_b;

  return assemble_instr(sent,asmblr);
}
