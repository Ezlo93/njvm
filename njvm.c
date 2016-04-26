#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
#define IMMEDIATE(x) ((x) & 0x00FFFFFF) 
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i)) 


unsigned code[10] = {0};
unsigned int stack[8] = {0};
unsigned int programCounter = 0;
unsigned mask = (1<<24)-1;
unsigned int halt = 0;
unsigned int topOfStack = 0;
unsigned int currentInstruction = 0;
int input;
char c;

/*TEST Print Stack*/
void printStack(void){
	int a;

	printf("STACK\n");
	for(a=0; a < sizeof(stack);a++){
		if(stack[a] == 0){ break;}
		printf("%d:\t%d\n", a, stack[a]); 
	}
	printf("TOPOFSTACK=%d\n", topOfStack);
}

/*Executes the given insctruction*/
void exec(int instr){

	switch(instr >> 24){
		case PUSHC : stack[topOfStack] = instr & mask; 
					 topOfStack = topOfStack+1; 
					 break;
		case ADD : stack[topOfStack-2] = stack[topOfStack-2] + stack[topOfStack-1];
				   stack[topOfStack-1] = 0;
				   topOfStack = topOfStack-1;
				   break;
	    case SUB : stack[topOfStack-2] = stack[topOfStack-2] - stack[topOfStack-1];
				   stack[topOfStack-1] = 0;
				   topOfStack = topOfStack-1;
				   break;
	    case MUL : stack[topOfStack-2] = stack[topOfStack-2] * stack[topOfStack-1];
				   stack[topOfStack-1] = 0;
				   topOfStack = topOfStack-1;
				   break;
	    case DIV : if(stack[topOfStack-1] == 0){
					halt = 1;
					printf("Division by zero!\n");
				    break;
				   }
				   stack[topOfStack-2] = stack[topOfStack-2] / stack[topOfStack-1];
				   stack[topOfStack-1] = 0;
				   topOfStack = topOfStack-1;
				   break;
	    case MOD : stack[topOfStack-2] = stack[topOfStack-2] % stack[topOfStack-1];
				   stack[topOfStack-1] = 0;
				   topOfStack = topOfStack-1;
				   break; 	
				   			   		
		case RDINT : 
					 scanf("%d", &input); 
					 stack[topOfStack] = input;
					 topOfStack = topOfStack+1;
					 break;
		
		case WRINT : printf("%d",stack[topOfStack-1]); 
					 stack[topOfStack-1] = 0;
					 topOfStack = topOfStack-1;
					 break;
		
		case RDCHR : 		
					 stack[topOfStack] = getchar();
					 topOfStack = topOfStack+1;
					 break;
		
		case WRCHR : c = (char)stack[topOfStack-1];
					 printf("%c",c); 
					 stack[topOfStack-1] = 0;
					 topOfStack = topOfStack-1;
					 break;
		case HALT : halt = 1;
					break;
				   		   
		default: printf("Unknown Instruction '%d'!\n", instr >> 24); halt = 1; break;
	}
	
	/*printStack();*/
}

/* MAIN */
   int main(int argc, char *argv[]) {
     char version[] = "1.0";
     unsigned int i = 0;
     
     unsigned lastBits;
	 
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
	 printf("Ninja Virtual Machine Version %s\n", version);
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
		
		 
		code[0] = (PUSHC << 24) | IMMEDIATE(3);
		code[1] = (PUSHC << 24) | IMMEDIATE (15);
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
	 
	 /*?????
	 while ( ( retval = read ( fd, &buf, 4)) > 0) {
        if ( retval == 4) {
            for ( each = 0; each < 4; each++) {
                opcode[each] = buf[each];
                arg1[each] = buf[each + 4];
                arg2[each] = buf[each + 8];
                arg3[each] = buf[each + 12];
            }
            
        }
    }
	 */
	 
	 /*printf("Unknown argument '%s', check out --help!\n", argv[1]);
	 return 0;*/
	}
   }
     

    printf("Ninja Virtual Machine started\n");

/*Print the program before executing it */
	for(i = 0; i < sizeof(code); i++){
		if(code[i] == 0){printf("%d:\tHALT\n", i);break;}
		
		printf("%d:\t", i);
		
		lastBits = code[i] & mask;
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
		default: break;}

	}

/*Execute the loaded program*/
	
	while(!halt){
		currentInstruction = code[programCounter];
		programCounter = programCounter+1;
		exec(currentInstruction);
	}

     printf("Ninja Virtual Machine stopped\n");
     return 0;
   }

