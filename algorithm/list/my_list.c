#include <stdlib.h>
#include <stdio.h>
#include "list.h"

void list_insert(jlink *list, int idx, int data)
{

  if(idx==0){
    return list_insert_head(list, data);
  }else if(idx==-1){
    return list_insert_tail(list, data);
  }

  jlink p = (jlink)(malloc(sizeof(struct List)));
  p->data = data;
  p->next = NULL;

  jlink cur = *list;
  int cnt = 0;
  while(cur->next){
    cnt ++;
    if(cnt==idx){
      jlink temp = cur->next;
      cur->next = p;
      p->next = temp;
      break;
    }
    cur = cur->next;
  }
  if(idx>cnt){
    return list_insert_tail(list, data);
  }


}

void list_insert_head(jlink *list, int data)
{
  jlink p = (jlink)(malloc(sizeof(struct List)));
  p->data = data;
  p->next = NULL;
  if(!*list){
    printf("NULL\n");
    *list = p;	
    p->next=NULL;
  }else{
    p->next = *list;
    *list = p;
  }
  return;

}

void list_insert_tail(jlink *list, int data)
{
  jlink p = (jlink)(malloc(sizeof(struct List)));
  p->data = data;
  p->next = NULL;

  if(!*list){
    *list = p;
    return;
  }
  jlink cur = *list;
  while(cur->next){
    cur = cur->next;
  }
  cur->next = p;
  return;

}

void print_list(jlink list)
{
  if(list==NULL){
    return ;
  }
  jlink p = list;
  printf("list is " );
  while(p!=NULL){
    printf("%d, ", p->data);
    p = p->next;
  }
  printf("\n");
  return ;
}


void list_reserve(jlink *list)
{
  jlink pre, cur, next;
  pre = *list;
  cur = pre->next;
  next = cur->next;
  pre->next = NULL;
  int cnt = 0;
  while(1){
    cnt ++;
    cur->next = pre;
    pre = cur;
    cur = next;
    next = next->next;
    if(next==NULL){
      cur->next = pre;
      break;
    }
  }
  *list = cur;
  return; 
}


int main()
{

  jlink llink = NULL; 
  int data[]={1,2,3,4,5,6,7};
  int size=sizeof(data)/sizeof(int);
  for(int i=0;i<size;i++){
    list_insert_tail(&llink, data[i]);
  }

  //list_insert_head(&llink, 999);
  //list_insert_head(&llink, 998);
  //list_insert_head(&llink, 997);
  list_insert(&llink, -1,9999);
  list_insert(&llink, 0, -9999);
  list_insert(&llink, 100, 100);

  print_list(llink);
  list_reserve(&llink);
  print_list(llink);

}
