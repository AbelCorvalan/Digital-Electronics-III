#include "LPC17xx.h"

#define Entrada1 (1 << 22)
#define Salida   (1 << 25)

void configGPIO();
int comprobar();
void generador1();
void generador2();
void delay(uint32_t times);

int main(void) {
	configGPIO();
	while(1){
		if(comprobar()){
			generador1();
		} else {
			generador2();
		}
	}
	return 0;
}

void configGPIO(){

	//Configuro pines 22 y 25 para ser usados como GPIO.
	LPC_PINCON-> PINSEL3 &= ~(3 << 22);
	LPC_PINCON-> PINSEL3 &= ~(3 << 25);

	LPC_GPIO3 -> FIODIR  |= ~(Entrada1);
	LPC_GPIO3 -> FIODIR  |= Salida;

}

int comprobar(){
	int e= 0;
	e= LPC_GPIO3 -> FIOPIN &= (1 << 22);
	return e;
}

void generador1(){
	LPC_GPIO3 -> FIOSET |= Salida;
	delay(1000);
	LPC_GPIO3 -> FIOCLR |= ~Salida;
	delay(1000);
	LPC_GPIO3 -> FIOSET |= Salida;
	delay(1000);
	LPC_GPIO3 -> FIOCLR |= ~Salida;
	delay(1000);
	//2.10 pulsador
}

void generador2(){
	LPC_GPIO3 -> FIOSET |= Salida;
	delay(1000);
	LPC_GPIO3 -> FIOCLR |= ~Salida;
	delay(2000);
	LPC_GPIO3 -> FIOSET |= ~Salida;
	delay(1000);
}

void delay(uint32_t times) {
	for(uint32_t i=0; i<times; i++)
		for(uint32_t j=0; j<times; j++);
}
