
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define VALUE_STATE 0
#define OP_STATE 1

typedef struct _stackframe {
	void * data;
    struct _stackframe *prevframe;
} Stackframe;

typedef struct _operator {
	int number;
	int precedence;
	int symbol_len;
    char * symbol;
    double (*function)(double,double);
} Operator;


double add(double x, double y){
	//printf("add %f+%f=%f\n",x,y,x+y);
	return x+y;
}

double subtract(double x, double y){
	//printf("subtract %f-%f=%f\n",y,x,y-x);
	return y-x;
}

double multiply(double x, double y){
	//printf("multiply %f*%f=%f\n",y,x,y*x);
	return y*x;
}

double divide(double x, double y){
	//printf("divide %f/%f=%f\n",y,x,y/x);
	return y/x;
}

double mod(double x, double y){
	//printf("mod %f%%%f=%f\n",y,x,y%x);
	return (int)y%(int)x;
}

double power(double x,double y){
	printf("power %f^%f=%f\n",y,x,pow(y,x));
	return pow(y,x);
}

Operator operators[] = {
	{0, 1, 1, "+", &add},
	{1, 1, 1, "-", &subtract},
	{2, 2, 1, "*", &multiply},
	{3, 2, 1, "/", &divide},
	{4, 2, 1, "%", &mod},
	{5, 9, 1, "^", &power},
	{0, 0, 0, NULL,NULL}
};


Stackframe* pushtostack(Stackframe *frame, void* value,size_t size){

	Stackframe *newframe;
	void * newvalue;
	
	//printf("pushing %f\n",*((double *) value));
	newframe = (Stackframe *) malloc(sizeof(Stackframe));
	newvalue = malloc(size);
	newframe->data = memcpy(newvalue, value, size);
	if(frame != NULL){
		newframe->prevframe = frame;
	}else{
		newframe->prevframe = NULL;
	}
	//printf("pushtostack %f\n",*((double *) newframe->data));
	return newframe;
}




//Stackframe* popfromstack(Stackframe *frame, void **value){
void * popfromstack(Stackframe **frame){
	Stackframe *prevframe;
	void *value;
	
	if(*frame != NULL){
		value = (*frame)->data;
		prevframe = (*frame)->prevframe;
		free(*frame);
		*frame = prevframe;
		//printf("popfromstack %f\n",*(double*)value);
		return value;
	}else{
		return NULL;
	}
}


void* peekstack(Stackframe * frame){
	if(frame != NULL){
		return frame->data;
	}else{
		return NULL;
	}
}



double evaluateStackframe(Stackframe **opstack,Stackframe **valuestack){
	double *value_one;
	double *value_two;
	double retval;
	int *operator;

	if(*opstack != NULL){
		value_one = (double*) popfromstack(valuestack);
		value_two = (double*) popfromstack(valuestack);
		operator = (int*) popfromstack(opstack);
		retval=operators[*operator].function(*value_one,*value_two);
		*valuestack=pushtostack(*valuestack, &retval,sizeof(double));
		return retval;
	}else{
		return 0;
	}
}

#define peekvalue(val) (val ? *((double*)(val)->data): 0)
#define peekop(op) (op ? *((int*)(op)->data) : 0)

Stackframe* evalAndPush(Stackframe **opstack, Stackframe **valuestack, int value){
	int prevop;

	if(opstack != NULL){
		while((prevop = peekop(*opstack)) != 0){
			if(operators[prevop].precedence >= operators[value].precedence){
				evaluateStackframe(opstack,valuestack);
			}else{
				*opstack = pushtostack(*opstack,&prevop,sizeof(int));
				break;
			}
		}
	}

	*opstack = pushtostack(*opstack, &value,sizeof(int));
	return *opstack;
}


char * strip(char * string){
	while(*string==' '){
		string++;
	}
	return string;
}



int main(int argc, char* argv[]){
    char *curr;
	char *endptr;
	double value;
	int operator;
	int i;
	int j;
//	int len;
	int state=VALUE_STATE;
	Stackframe *valuestack = NULL;
	Stackframe *opstack = NULL;

    if(argc==1){
      printf("usage: %s [expression]\n",argv[0]);
      exit(1);
    }

	for(j=1; j<argc; j++){
		curr=argv[j];

		while((*curr) != 0){
			curr = strip(curr);
			if(state==VALUE_STATE){
				value = strtod(curr,&endptr);	//try and read a number
				if(curr == endptr){	
					fprintf(stderr,"failed to read value at %s\n",curr);
					exit(1);
				}
				valuestack = pushtostack(valuestack,&value,sizeof(double));
				curr = endptr;
				state=OP_STATE;
			}else if(state==OP_STATE){
				operator = -1;
				i = 0;
				while(operators[i].symbol != NULL){
					if(strncmp(operators[i].symbol,curr,operators[i].symbol_len) == 0){
						operator = operators[i].number;
						opstack = evalAndPush(&opstack, &valuestack, operator);
						curr+=operators[i].symbol_len;
						break;
					}
					i++;
				}
				if(operator == -1){
					fprintf(stderr,"invalid operator at %s\n",curr);
					exit(1);
				}
				state=VALUE_STATE;
			}else{
				fprintf(stderr,"unknown state %d\n", state);
				exit(1);
			}
		}
	}
	while((valuestack != NULL) && (opstack != NULL)){
		evaluateStackframe(&opstack,&valuestack);
	}

	printf("= %g\n",peekvalue(valuestack));

	/*
	while(valuestack != NULL){
		value = peekvalue();
		printf("valuestack=%f %x\n",value,valuestack);
		valuestack = popfromstack(valuestack, (void**)&valueptr);
		printf("value==%f\n",*valueptr);
	}

	while(opstack != NULL){
		operator = peekop();
		printf("opstack=%d %x\n",operator,opstack);
		opstack = popfromstack(opstack, (void**)&opptr);
		printf("value==%d\n",*opptr);
	}
	*/
    exit(0);
}
