#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*Instruction codes*/
#define HALT 0
#define PUSHC 1
#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
#define MOD 6
#define RDINT 7
#define WRINT 8
#define RDCHR 9
#define WRCHR 10
#define PUSHG 11
#define POPG 12
#define ASF 13
#define RSF 14
#define PUSHL 15
#define POPL 16
#define IMMEDIATE(x) ((x) & 0x00FFFFFF) 
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i)) 

int version = 2;

unsigned int stack[100] = {0};
unsigned int topOfStack = 0;
unsigned int programCounter = 0;
unsigned int currentInstruction = 0;
unsigned int halt = 0;
int *variables;
int input;
char njvm[4];
unsigned mask = (1<<24)-1;

/*TEST Print Stack*/
void printStack(void){
	int a;

	printf("STACK\n");
	for(a=0; a < sizeof(stack);a++){
		if(stack[a] == 0){ break;}
		printf("%d:\t%d\n", a, stack[a]); 
	}
	printf("VAR\n");
	for(a=0; a < sizeof(variables);a++){
		printf("%d:\t%d\n", a, variables[a]);
	}
}

/*Print int as bits*/
void printBits(unsigned int num)
{
   int bit;
   for(bit=0;bit<(sizeof(unsigned int) * 8); bit++)
   {
      printf("%i", num & 0x01);
      num = num >> 1;
   }
   printf("\n");
}

/*Stack push pop*/
void push(int c){
	if(topOfStack > sizeof(stack)){
		halt = 1;
		printf("Stackoverflow!");
	}
	stack[topOfStack] = c;
	topOfStack += 1;
}

int pop(){
	topOfStack -= 1;
	return stack[topOfStack];
}

/*Executes the given insctruction*/
void exec(int instr){
    int tmp1, tmp2;
	switch(instr >> 24){
		case PUSHC : push(instr & mask);
					 break;
		case ADD : push(pop()+pop());
				   break;
	    case SUB : tmp1 = pop(); tmp2 = pop();
				   push(tmp2-tmp1);
				   break;
	    case MUL : push(pop()*pop());
				   break;
	    case DIV : if(stack[topOfStack-1] == 0){
					halt = 1;
					printf("Division by zero!\n");
				    break;
				   }
				   tmp1 = pop(); tmp2 = pop();
				   push(tmp2/tmp1);
				   break;
	    case MOD : tmp1 = pop(); tmp2 = pop();
				   push(tmp2 % tmp1);
				   break; 	
				   			   		
		case RDINT : 
					 scanf("%d", &input); 
					 push(input);
					 break;
		
		case WRINT : printf("%d",pop()); 
					 break;
		
		case RDCHR : 		
					 push(getchar());
					 break;
		
		case WRCHR : printf("%c",(char)(pop())); 
					 break;
		case PUSHG : push(variables[instr&mask]);
					 break;
		case POPG : variables[instr&mask] = pop();
					break;
		case HALT : halt = 1;
					break;
				   		   
		default: printf("Unknown Instruction '%d'!\n", instr >> 24); halt = 1; break;
	}
	
	/*printStack();*/
}

/* MAIN */
   int main(int argc, char *argv[]) {
     unsigned int i = 0;
     int *code;
     
     unsigned lastBits;
     int header[2] = {0};
	 
/*Arguments*/

/*Print all arguments*/
     /*int argCounter=0;

     if(argc > 1){
	  printf("Called with: ");
	  for(argCounter=1; argCounter<argc; argCounter++){
	   printf("%s ", argv[argCounter]);
	   }
	   printf("\n");
     }
     */


/*React to command line arguments*/
   if(argc > 1){
     
	if(strcmp(argv[1], "--version") == 0){
	 printf("Ninja Virtual Machine Version %d\n", version);
	 return 0;
	}else if(strcmp(argv[1], "--help") == 0){
	 printf("--version\tshow version\n");
	 printf("--help\t\tshow this help\n");
	 printf("--prog1\t\tStart program 1\n");
	 printf("--prog2\t\tStart program 2\n");
	 printf("--prog3\t\tStart program 3\n");
	 return 0;
/*Load one of the three possible programs*/
	}else if(strcmp(argv[1], "--prog1") == 0){
		
		code = malloc(50*sizeof(int));
		variables = malloc(1*sizeof(int));
		code[0] = (PUSHC << 24) | IMMEDIATE(3);
		code[1] = (PUSHC << 24) | IMMEDIATE (4);
		code[2] =  (ADD<<24);
		code[3]	= (PUSHC << 24) | IMMEDIATE(10);
		code[4] =  (PUSHC << 24) | IMMEDIATE(6);
		code[5] = 	 (SUB<<24);
		code[6] = 	 (MUL<<24);
		code[7] = 	 (WRINT<<24);
		code[8] =	 (PUSHC<<24) | IMMEDIATE(10);
	    code[9] = (WRCHR<<24);
		code[10]  =	 (HALT<<0);
			 
	}else if (strcmp(argv[1], "--prog2") == 0){
		code = malloc(50*sizeof(int));
		variables = malloc(1*sizeof(int));
		code[0] = (PUSHC<<24) | IMMEDIATE(2);
		code[1] =	(RDINT<<24);
		code[2] = 	(MUL<<24);
		code[3] = 	(PUSHC<<24)|IMMEDIATE(3);
		code[4] =	(ADD<<24);
		code[5] =	(WRINT<<24);
		code[6] =	(PUSHC<<24)|IMMEDIATE(10);
		code[7] =	(WRCHR<<24);
		code[8] =	(HALT<<24);
			
	}else if(strcmp(argv[1], "--prog3") == 0){
		 code = malloc(50*sizeof(int));
		 variables = malloc(1*sizeof(int));
		 code[0] = (RDCHR<<24);
		 code[1] = (WRINT<<24);
		 code[2] = (PUSHC<<24)|IMMEDIATE(10);
		 code[3] = (WRCHR<<24);
		 code[4] = (HALT<<24);
		 
	}else{
		
		
	 /* try to open the given file */
	 FILE *file;
	 if(! (file = fopen(argv[1], "r"))){
		printf("File \"%s\" doesn't exist or can't be opened!", argv[1]);
		return 0;
	 }	
	
	
	 /* Read header[0] identifier, header[1] version of VM*/
	 fread(header,sizeof(int), 4, file);
     
	 njvm[3] = (header[0] >> 24);
	 njvm[2] = (header[0] >> 16);
	 njvm[1] = (header[0] >> 8);
	 njvm[0] = header[0];	 	 	 
	 
	 /*Verify header of the file*/
	 if(strcmp(njvm, "NJBF") != 0){
		 printf("File is not a Ninja file!\n");
		 return 0;
	 }
	 
	 /*Check file version*/
	 if(header[1] != version){
		 printf("File not supported: NJVM version: %d, file version: %d\n", header[1], version);
		 return 0;
	 }
	 
	 /*Tests passed, read number of instructions/variables*/ 
	 code = malloc(header[2] * sizeof(int));
	 variables = malloc(header[3] * sizeof(int));
	 for(i=0; i < sizeof(variables); i++){
		 variables[i] = 0;
	 }
	 /*Copy instructions in code array*/
	 fread(code, sizeof(int), header[2]* sizeof(int), file);
	 
	 if(fclose(file)){
		 printf("File couldn't be closed!\n");
		 return 0;
	 }
	}
   }
     

    printf("Ninja Virtual Machine started\n");

/*Print the program before executing it */
	for(i = 0; i < sizeof(code); i++){
		/*printBits(code[i]);*/
		printf("%03d:\t", i);
		if(code[i] == 0){printf("HALT\n");break;}
		
		lastBits = code[i]& mask;
		switch(code[i]>>24){
			case PUSHC: printf("PUSHC\t%d\n", lastBits);break;
			case ADD: printf("ADD\n"); break;
			case DIV: printf("DIV\n"); break;
			case HALT : printf("HALT\n"); break;
			case MOD : printf("MOD\n"); break;
			case MUL : printf("MUL\n"); break;
			case RDCHR : printf("RDCHR\n"); break;
			case RDINT : printf("RDINT\n"); break;
			case SUB : printf("SUB\n"); break;
			case WRCHR : printf("WRCHR\n"); break;
			case WRINT : printf ("WRINT\n"); break;
			case PUSHG : printf ("PUSHG\t%d\n", lastBits);break;
			case POPG : printf ("POPG\t%d\n", lastBits);break;
			case ASF : printf("ASF\t%d\n", lastBits);break;
			case RSF : printf("RSF\n"); break;
			case PUSHL : printf("PUSHL\t%d\n", lastBits); break;
			case POPL : printf("POPL\t%d\n",lastBits); break;
		default: printf("Unknown Instruction Code %d!\n", code[i]>>24);break;}

	}

/*Execute the loaded program*/
	
	while(!halt){
		currentInstruction = code[programCounter];
		programCounter = programCounter+1;
		exec(currentInstruction);
	}

	/* free(code);
     free(variables);*/
     printf("Ninja Virtual Machine stopped\n");
     return 0;
   }

