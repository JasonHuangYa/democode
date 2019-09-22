#include <stdio.h>

void method1()
{
    unsigned char a = 1;
    if((a&0x80)==0x80){
        printf("big \n");
    }else{
        printf("small \n");
    }
}

void method2()
{
    int a=1;
    char *p =(char*)&a;
    if(*p){
        printf("small\n");
    }else{
        printf("big \n");
    }
}

union Test{
    short a;
    char b;
};

void method3(){
    union Test test;
    test.a=1;
    if(test.b)
        printf("small\n");
    else
        printf("big\n");
}

void method4()
{

}

int main()
{
    method1();
    method2();
    method3();
    return 0;
}
