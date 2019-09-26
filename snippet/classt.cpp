#include<iostream>
#include <assert.h>
#include"noncopyable.h"

using namespace std;

class Shape:noncopyable
{
    virtual int getarea()=0;

};

class Rectangle:Shape
{
    public:
        int length;
        int width;
    public:
        Rectangle(int l, int w){
            length = l;
            width = w;
        }
        Rectangle(){}
        virtual ~Rectangle(){}
    public:
        virtual void show(){
            cout<< "[Rectangle] length: "<<length << " ,width: "<< width <<endl;
        }
        virtual int getarea()
        {
            cout << "area: " << length*width << endl;
            return length*width;
        }
};

class Square:public Rectangle
{
    public:
        Square(int l, int w):Rectangle(l, w){
            assert(length==width);
        }
        ~ Square(){};
        virtual void show(){
            cout<< "[square] length: "<<length << " ,width: "<< width <<endl;
        }
};


int main()
{
    Rectangle *p = new Rectangle(4,2);
    p->show();
    p->getarea();

    Square square(2,2);
    square.show();
    square.getarea();

}
