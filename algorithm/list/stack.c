#include <stdlib.h>
#include <stdio.h>

#define MAXSIZE  4 

typedef struct {
    int stack[MAXSIZE];
    int heapsize;

} mystack;

void print_stack(mystack *jstack)
{

  printf("stack item is: ");
  for(int i=0; i<jstack->heapsize; i++){
    printf("%d, ", jstack->stack[i]);
  }
  printf("\n");
}

int is_empty(mystack *jstack)
{
  return jstack->heapsize==0?1:0;
}

int is_full(mystack *jstack)
{
  return jstack->heapsize==MAXSIZE?1:0;
}

void push(mystack *jstack, int data)
{
  if(is_full(jstack)){
    return; 
  }
  (jstack)->stack[(jstack)->heapsize++] = data;; 
}

int pop(mystack *jstack)
{
  if(is_empty(jstack)){
    return -1;
  }
  return (jstack)->stack[--(jstack)->heapsize]; 
}


int main()
{
  mystack *s =  (mystack *) malloc(sizeof(mystack));
  printf("size:%lu \n", sizeof(mystack));
  push(s, 1);
  push(s, 2);
  push(s, 3);
  push(s, 4);
  push(s, 5);
  print_stack(s);
  printf("pop item:%u\n", pop(s));
  printf("pop item:%u\n", pop(s));
  print_stack(s);
  return 0;
}
