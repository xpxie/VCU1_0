#include "stack.h" 
#include <stdio.h>

void Stack_Init(Stack *S)
{
    S->size = 0;
}

double Stack_Top(Stack *S)
{
    if (S->size == 0) {
        printf("Error: stack empty\n");
        return -1;
    } 
    return S->tempMeas[S->size-1];
}

void Stack_Push(Stack *S, double d, byte addr)
{
    if (S->size < STACK_MAX){
        S->tempMeas[S->size++] = d;
				S->address[S->size++] = addr;
    }else{
        printf("Error: stack full \n");
		}
}

void Stack_Pop(Stack *S)
{
    if (S->size == 0)
        printf("Error: stack empty \n");
    else
        S->size--;
}

double Stack_Avg (Stack *S){
	double sum = 0;
  for(int i=0; i < S->size; i++){
      sum += S->tempMeas[i];
  }	
	return (sum/S->size); 
}
