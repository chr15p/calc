
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


Operator operators[] = {
	{0, 1, 1, "+", &add},
	{1, 1, 1, "-", &subtract},
	{2, 2, 1, "*", &multiply},
	{3, 2, 1, "/", &divide},
	{4, 2, 1, "%", &mod},
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

	//printf("evaluateStackframe called %x\n",*opstack);
	if(*opstack != NULL){
		//valueptr=(double*)popfromstack(&valuestack);
		value_one = (double*) popfromstack(valuestack);
		value_two = (double*) popfromstack(valuestack);
		operator = (int*) popfromstack(opstack);
		//printf("popped %f\n",*value_one);
		//printf("popped %f\n",*value_two);
		//printf("popped %s\n",operators[*operator].symbol);
		retval=operators[*operator].function(*value_one,*value_two);
		//printf("eval ");
		*valuestack=pushtostack(*valuestack, &retval,sizeof(double));
		//printf("valuestack = %x\n",*valuestack);
		return retval;
	}else{
		printf("opstack = %x\n",*opstack);
		return 0;
	}
}

#define peekvalue(val) (val ? *((double*)(val)->data): 0)
//#define peekop() (*((int*)opstack->data))
#define peekop(op) (op ? *((int*)(op)->data) : 0)

Stackframe* evalAndPush(Stackframe **opstack, Stackframe **valuestack, int value){
	int prevop;
	double retval;

	//printf("evalandpush = %x\n",*opstack);
	if(opstack != NULL){
		//while(opstack = popfromstack(opstack, (void*) &prevop)){
		//while(prevop = popfromstack(&opstack)){
		while(prevop = peekop(*opstack)){
		//while(prevop = *((int*)(*opstack)->data)){
			//printf("prevop %d\n",prevop);
			if(operators[prevop].precedence >= operators[value].precedence){
				retval = evaluateStackframe(opstack,valuestack);
			}else{
				//printf("eval1 ");
				*opstack = pushtostack(*opstack,&prevop,sizeof(int));
				break;
			}
		}
	}

	//printf("eval2 ");
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
	//char *input="1.0+2.3*(17+3)";
    char *curr;
	char *endptr;
	double value;
	int prevop;
	int operator;
	int i;
	int j;
	int len;
	int state=VALUE_STATE;
	Stackframe *valuestack = NULL;
	Stackframe *opstack = NULL;

	double *valueptr;
	int *opptr;

    if(argc==1){
      printf("usage: %s [expression]\n",argv[0]);
      exit(1);
    }
//printf("%s\n",argv[1]);

	for(j=1; j<argc; j++){
		curr=argv[j];

		while((*curr) != 0){
			curr = strip(curr);
			//printf("%s\n",curr);
			if(state==VALUE_STATE){
				value = strtod(curr,&endptr);	//try and read a number
				if(curr == endptr){	
					fprintf(stderr,"failed to read value\n", state);
					exit(1);
				}
				valuestack = pushtostack(valuestack,&value,sizeof(double));
				//valueptr=(double*)popfromstack(&valuestack);
				//printf("return: %f\n",*valueptr);
				//exit(0);
				curr = endptr;
				state=OP_STATE;
			}else if(state==OP_STATE){
				operator = -1;
				i = 0;
				while(operators[i].symbol != NULL){
					len = strlen(operators[i].symbol);
					if(strncmp(operators[i].symbol,curr,len) == 0){
						operator = operators[i].number;
						//printf("operator=%c %d\n",*endptr,operator);
						//opstack = pushtostack(opstack,&operator,sizeof(int));
						opstack = evalAndPush(&opstack, &valuestack, i);
						//opstack = pushtostack(opstack,&i,sizeof(int));
						//curr = endptr + len;
						curr+=len;
						break;
					}
					i++;
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
