/* njvm.c Executes ninja binary files
 * 
 * 
 * Nicolai Sehrt
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bigint.h>

#define TRUE 1
#define FALSE 0

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
#define EQ 17
#define NE 18
#define LT 19
#define LE 20
#define GT 21
#define GE 22
#define JMP 23
#define BRF 24
#define BRT 25
#define CALL 26
#define RET 27
#define DROP 28
#define PUSHR 29
#define POPR 30
#define DUP 31
#define IMMEDIATE(x) ((x) & 0x00FFFFFF) 
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i)) 

/*typedef*/


typedef struct{
	int isObjectRef;
	union{
		ObjRef* objref;
		ObjRef bigint;
	}u;
} StackSlot;



/*
 * Objref interger = malloc(sizeof unsigned int + sizeof int)
 * integer -> size = sizeof(int);
 * *(int*)(integer->data) = 5;
 * 
 */

/* global variables */

int version = 6;

StackSlot* stack[10000];
int stackSize = 10000;
int stackPointer = 0, framePointer = 0;
int programCounter = 0;

StackSlot* *variables;
int glVarSize=0;

int returnAddress = 0;
StackSlot *returnValue;

int currentInstruction = 0;
int halt = 0;

int debug = 0;
int debugNextStep = 0;
char debugInput[20];

int input;
char njvm[4];
unsigned mask = (1<<24)-1;

ObjRef oRef;

/*library funtions*/

ObjRef newPrimObject(int dataSize){
	oRef = malloc(sizeof(unsigned int) + dataSize);
	oRef->size = dataSize;
	
	return oRef;
}

void fatalError(char *msg){
	printf("%s", msg);
	exit(0);
}

/*Print Stack*/
void printStack(void){
	int a;
	printf("STACK\n");
	printf("Stackpointer: %d\n", stackPointer);
	
	for(a=0; a < stackPointer;a++){
		if((*stack[a]).isObjectRef == FALSE){
			bip.op1 = (*stack[a]).u.bigint;
			printf("%d:\t", a);
			bigPrint(stdout);
			printf("\n");
		}else{
			printf("%d:\t%p\n", a, (void *)(*stack[a]).u.objref);
		}
	}
}

/*Print all global Variables*/
void printGlobalVariables(void){
	int a;
	
	if(glVarSize > 0){
		printf("GLOBAL VARIABLES\n");
		
		for(a=0;a<glVarSize;a++){
			bip.op1 = (*variables[a]).u.bigint;
			printf("%03d: \n", a);
			bigPrint(stdout);
			printf("\n");
		}
	}else{
		printf("NO GLOBAL VARIABLES\n");
	}
}

 
/*Stack push pop*/
void push(StackSlot *c){
	if(stackPointer > stackSize){
		halt = 1;
		printf("Stackoverflow! %d", stackPointer);
	}else{
	
		stack[stackPointer] = c;
		stackPointer += 1;
	}
}

StackSlot* pop(){
	if(stackPointer == 0){
		halt = 1;
		printf("Out of range! -1");
	}else{
		stackPointer -= 1;
		return stack[stackPointer];
	}
	return NULL;
}

/*Prints the given instruction*/

void printInstruction(int c){
	int lastBits = SIGN_EXTEND(c & mask);
		switch(c>>24){
			case PUSHC: printf("PUSHC\t%d\n", lastBits);break;
			case ADD: printf("ADD\n"); break;
			case DIV: printf("DIV\n"); break;
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
			case EQ : printf("EQ\n"); break;
			case NE : printf("NE\n"); break;
			case LT : printf("LT\n"); break;
			case LE : printf("LE\n"); break;
			case GT : printf("GT\n"); break;
			case GE : printf("GE\n"); break;
			case JMP : printf("JMP\t%d\n", lastBits); break;
			case BRF : printf("BRF\t%d\n", lastBits); break;
			case BRT : printf("BRT\t%d\n", lastBits); break;
			case CALL : printf("CALL\t%d\n", lastBits); break;
			case RET : printf("RET\n"); break;
			case DROP : printf("DROP\t%d\n", lastBits); break;
			case PUSHR : printf("PUSHR\n"); break;
			case POPR : printf("POPR\n"); break;
			case DUP : printf("DUP\n"); break;
			case HALT : printf("HALT\n"); break;
		default: printf("Unknown Instruction Code %d!\n", c>>24);break;}
	
	
}

/*Executes the given insctruction*/

void exec(int instr){
	int i;
    int lastBits=SIGN_EXTEND(instr&mask);
    StackSlot *tmp;
    
	switch(instr >> 24){
		case PUSHC : tmp = malloc(sizeof(StackSlot));
					 bigFromInt(lastBits);
					 (*tmp).isObjectRef = FALSE;
					 (*tmp).u.bigint = bip.res;
					 push(tmp);
					 break;
		case ADD : tmp = malloc(sizeof(StackSlot));
					bip.op1 = (*pop()).u.bigint;
					bip.op2 = (*pop()).u.bigint;
					bigAdd();
					(*tmp).isObjectRef = FALSE;
					(*tmp).u.bigint = bip.res;
				   push(tmp);
				   break;
	    case SUB : tmp = malloc(sizeof(StackSlot));
					bip.op2 = (*pop()).u.bigint;
					bip.op1 = (*pop()).u.bigint;
					bigSub();
					(*tmp).isObjectRef = FALSE;
					(*tmp).u.bigint = bip.res;
				   push(tmp);
				   break;
	    case MUL : tmp = malloc(sizeof(StackSlot));
					bip.op1 = (*pop()).u.bigint;
					bip.op2 = (*pop()).u.bigint;
					bigMul();
					(*tmp).isObjectRef = FALSE;
					(*tmp).u.bigint = bip.res;
				   push(tmp);
				   break;
	    case DIV : tmp = malloc(sizeof(StackSlot));
					bip.op2 = (*pop()).u.bigint;
					bip.op1 = (*pop()).u.bigint;
					bigDiv();
					(*tmp).isObjectRef = FALSE;
					(*tmp).u.bigint = bip.res;
				   push(tmp);
				   break;
	    case MOD : tmp = malloc(sizeof(StackSlot));
					bip.op2 = (*pop()).u.bigint;
					bip.op1 = (*pop()).u.bigint;
					bigDiv();
					(*tmp).isObjectRef = FALSE;
					(*tmp).u.bigint = bip.rem;
				   push(tmp);
				   break;	
				   			   		
		case RDINT : scanf("%d", &input);
					 bigFromInt(input);
					 tmp = malloc(sizeof(StackSlot));
					 (*tmp).isObjectRef = FALSE;
					 (*tmp).u.bigint = bip.res;
					 push(tmp);
					 break;
		
		case WRINT : bip.op1 = (*pop()).u.bigint;
					 bigPrint(stdout);
					 break;
		
		case RDCHR : tmp = malloc(sizeof(StackSlot));
					 bigFromInt(getchar());
					 (*tmp).isObjectRef = FALSE;
					 (*tmp).u.bigint = bip.res;
					 push(tmp);
					 break;
		
		case WRCHR : bip.op1 = (*pop()).u.bigint;
					 printf("%c",(char)(bigToInt())); 
					 break;
		case PUSHG : push(variables[lastBits]);
					 break;
		case POPG : variables[lastBits] = pop();
					break;
		case ASF  : tmp = malloc(sizeof(StackSlot));
				    bigFromInt(framePointer);
					(*tmp).isObjectRef = FALSE;
					(*tmp).u.bigint = bip.res;
					push(tmp);
					framePointer = stackPointer;
					stackPointer += lastBits;
					break;
		case RSF :  stackPointer = framePointer;
					bip.op1 = (*pop()).u.bigint;
					framePointer = bigToInt();
					break;
		case PUSHL : push(stack[framePointer + lastBits]);
					break;
		case POPL : stack[framePointer+lastBits] = pop();
					break;
		case EQ : 	bip.op1 = (*pop()).u.bigint;
					bip.op2 = (*pop()).u.bigint;
					if(bigCmp() == 0){
						bigFromInt(1);
						tmp = malloc(sizeof(StackSlot));
						(*tmp).isObjectRef = FALSE;
						(*tmp).u.bigint = bip.res;
						push(tmp);
					}else{
						bigFromInt(0);
						tmp = malloc(sizeof(StackSlot));
						(*tmp).isObjectRef = FALSE;
						(*tmp).u.bigint = bip.res;
						push(tmp);
					}
					
					break;
		case NE : 	bip.op1 = (*pop()).u.bigint;
					bip.op2 = (*pop()).u.bigint;
					if(bigCmp() == 0){
						bigFromInt(0);
						tmp = malloc(sizeof(StackSlot));
						(*tmp).isObjectRef = FALSE;
						(*tmp).u.bigint = bip.res;
						push(tmp);
					}else{
						bigFromInt(1);
						tmp = malloc(sizeof(StackSlot));
						(*tmp).isObjectRef = FALSE;
						(*tmp).u.bigint = bip.res;
						push(tmp);
					}
					
					break;
		case LT : 	bip.op2 = (*pop()).u.bigint;
					bip.op1 = (*pop()).u.bigint;
					if(bigCmp() < 0){
						bigFromInt(1);
						tmp = malloc(sizeof(StackSlot));
						(*tmp).isObjectRef = FALSE;
						(*tmp).u.bigint = bip.res;
						push(tmp);
					}else{
						bigFromInt(0);
						tmp = malloc(sizeof(StackSlot));
						(*tmp).isObjectRef = FALSE;
						(*tmp).u.bigint = bip.res;
						push(tmp);
					}
					
					break;			
		case LE : 	bip.op2 = (*pop()).u.bigint;
					bip.op1 = (*pop()).u.bigint;
					if(bigCmp() <= 0){
						bigFromInt(1);
						tmp = malloc(sizeof(StackSlot));
						(*tmp).isObjectRef = FALSE;
						(*tmp).u.bigint = bip.res;
						push(tmp);
					}else{
						bigFromInt(0);
						tmp = malloc(sizeof(StackSlot));
						(*tmp).isObjectRef = FALSE;
						(*tmp).u.bigint = bip.res;
						push(tmp);
					}
					
					break;
		case GT : 	bip.op2 = (*pop()).u.bigint;
					bip.op1 = (*pop()).u.bigint;
					if(bigCmp() > 0){
						bigFromInt(1);
						tmp = malloc(sizeof(StackSlot));
						(*tmp).isObjectRef = FALSE;
						(*tmp).u.bigint = bip.res;
						push(tmp);
					}else{
						bigFromInt(0);
						tmp = malloc(sizeof(StackSlot));
						(*tmp).isObjectRef = FALSE;
						(*tmp).u.bigint = bip.res;
						push(tmp);
					}
					
					break;
		case GE :	bip.op2 = (*pop()).u.bigint;
					bip.op1 = (*pop()).u.bigint;
					if(bigCmp() >= 0){
						bigFromInt(1);
						tmp = malloc(sizeof(StackSlot));
						(*tmp).isObjectRef = FALSE;
						(*tmp).u.bigint = bip.res;
						push(tmp);
					}else{
						bigFromInt(0);
						tmp = malloc(sizeof(StackSlot));
						(*tmp).isObjectRef = FALSE;
						(*tmp).u.bigint = bip.res;
						push(tmp);
					}
					
					break;
		case JMP :  programCounter = lastBits;
					break;
		case BRF :  bip.op1 = (*pop()).u.bigint;
					bigFromInt(0);
					bip.op2 = bip.res;
					if(bigCmp() == 0){
						programCounter = lastBits;
					}
					break;
		case BRT :  bip.op1 = (*pop()).u.bigint;
					bigFromInt(1);
					bip.op2 = bip.res;
					if(bigCmp() == 0){
						programCounter = lastBits;
					}
					break;
		case CALL : tmp = malloc(sizeof(StackSlot));
				    bigFromInt(programCounter);
					(*tmp).isObjectRef = FALSE;
					(*tmp).u.bigint = bip.res;
					push(tmp);
					programCounter = lastBits;
					break;
		case RET :  bip.op1 = (*pop()).u.bigint;
					programCounter = bigToInt();
					break;
		case DROP : for(i = 0; i < lastBits; i++){
						pop();
					}
					break;
		case PUSHR : push(returnValue);
					 break;
		case POPR :  returnValue = pop();
					 break;
		case DUP :  tmp = pop();
					push(tmp); push(tmp);
					break;
		case HALT : halt = 1;
					break;
				   		   
		default: printf("Unknown Instruction '%d'!\n", instr >> 24); halt = 1; break;
	}
	
}

/* MAIN */
   int main(int argc, char *argv[]) {
     unsigned int i = 0;
     int *code;
     int codeSize=11;
     
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
   if(argc == 1){
	   printf("ERROR: No file specified!\n--help for help\n");
	   return 0;
   }

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
	 printf("file name\t\tExecute the file\n");
	 printf("file name --debug\tExecute the file with debugger\n");
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
	 if(header[1] > version){
		 printf("File not supported: NJVM version: %d, file version: %d\n", version, header[1]);
		 return 0;
	 }
	 
	 /*Tests passed, read number of instructions/variables*/ 
	 codeSize = header[2];
	 glVarSize = header[3];
	 
	 code = malloc(codeSize * sizeof(int));
	 variables = malloc(glVarSize * sizeof(StackSlot*));
	 
	 /*Copy instructions in code array*/
	 fread(code, sizeof(int), header[2]* sizeof(int), file);
	 
	 if(fclose(file)){
		 printf("File couldn't be closed!\n");
		 return 0;
	 }
	}
   }
     
     if(argc > 2){
	  if( !strcmp(argv[2], "--debug") || !strcmp(argv[2], "--d")){
		  debug = 1;
	  }
     }

    printf("Ninja Virtual Machine started\n");

/*Print the program before executing it if debug enabled*/
if(debug == 1){ 
    printf("File has %d instructions\n", codeSize);
    
	for(i = 0; i < codeSize; i++){
		
		printf("%03d:\t", i);
		printInstruction(code[i]);

	}
	printf("\n");
}

/*Execute the loaded program*/
	if(debug == 1){
		printf(">>Debugger instructions: next, run, pstack, pgv, break, inspect, exit\n");
	}
	
	while(!halt){
		
		/*Debugger*/
		if(debug==1){
			if(debugNextStep < 0){
			  if( (debugNextStep*-1) == programCounter ){
				  debugNextStep = 0;
			  }	
			}else{
			debugNextStep = 0;
			}
			while(debugNextStep == 0){
				printf("%03d:\t", programCounter);
				printInstruction(code[programCounter]);
				printf(">>");
				scanf("%s", debugInput);
				if(strcmp(debugInput, "exit") == 0){
					exit(0);
				}else if(strcmp(debugInput, "next") == 0){
					debugNextStep = 1;
				}else if(strcmp(debugInput, "run") == 0){
					debug = 0;
					debugNextStep = 1;
				}else if(strcmp(debugInput, "pstack") == 0){
					printStack();
				}else if(strcmp(debugInput, "pgv") == 0){
					printGlobalVariables();
				}else if(strcmp(debugInput, "inspect") == 0){
					/*inspect*/
					
				}else if(strcmp(debugInput, "break") == 0){
					printf("Breakpoint at line: ");
					scanf("%d", &debugNextStep);
					debugNextStep *= -1;
				}
			}
		}
		
		currentInstruction = code[programCounter];
		programCounter = programCounter+1;
		exec(currentInstruction);
	}
     
     printf("Ninja Virtual Machine stopped\n");
     return 0;
   }

