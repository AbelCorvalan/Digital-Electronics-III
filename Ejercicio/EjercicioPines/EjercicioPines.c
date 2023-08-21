/*
================================================================================
Name     :  I/O pins configuration.
Author   :  Corvalán, Abel Nicolás.
Date     :  21 Oct 2023
================================================================================
*/

#include <LPC17xx.H>

int main(){
    int i;         //Defino una variable i.
    PINSEL0= 0;
    FIO0DIR= 0XFFFFFFFF;

    while(1){
        FIO0SET= 0xFFFFFFFF;
        for(i=0; i<100000; i++);
        FIO0CLR= 0XFFFFFFFF;
        for(i=0; i<100000; i++);
    }
}