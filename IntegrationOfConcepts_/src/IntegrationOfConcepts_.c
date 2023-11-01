/*
===============================================================================
 Name        : IntegrationOfConcepts_.c
 Author      : Abel Nicolás Corvalán
 Version     : 1.0
 Copyright   :
 Description : Algoritmo de antirrebote de un pulsador: Escribir un programa en C
 	 	 	   que ante la interrupción por flanco de subida del pin P0.1,
 	 	 	   configurado como entrada digital con pull-down interno, se incremente
 	 	 	   un contador de un dígito, se deshabilite esta interrupción y se permita
 	 	 	   la interrupción por systick cada 20 milisegundos.
 	 	 	   En cada interrupción del systick se testeará una vez el pin P0.1.
 	 	 	   Solo para el caso de haber testeado 3 estados altos seguidos se sacará
 	 	 	   por los pines del puerto P2.0 al P2.7 el equivalente en ascii del valor del contador,
 	 	 	   se desactivará las interrupción por systick y se habilitará nuevamente la interrupción
 	 	 	   por P0.1. Por especificación de diseño se pide que los pines del puerto 2
 	 	 	   que no sean utilizados deben estar enmascarados por hardware.
 	 	 	   Considerar que el CPU se encuentra funcionando con el oscilador interno RC (4Mhz).
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>


void configPin(void);
void configGPIOINT(void);

void disableSysTick(void);
void mostrarAscii(void);

void configSysTick(void);

uint8_t contador= -1;

int main(void) {


	configPin();
	configGPIOINT();

    while(1) {

    }
    return 1;
}

void configPin(void){

	//(P0.1) Input configuration
	LPC_PINCON-> PINSEL0 &= ~(3 << 2);	//P0.1 as GPIO0.1
	LPC_PINCON-> PINMODE0 |= (3 << 2);	//configured as pull-down

	LPC_GPIO0-> FIODIR &= ~(1 << 1);	//configured as input

	//(P2.0 - P2.7) Output configuration
	LPC_PINCON-> PINSEL4 &= (0xFFFF);	//P2.0 - P2.7 as GPIO2.0 - GPIO2.7

	LPC_GPIO2-> FIODIR |= (0xFF);	//configured as output

}

void configGPIOINT(void){

	LPC_GPIOINT-> IO0IntEnR |= (1 << 1);	//Interrupt Enable Rising
	NVIC_EnableIRQ(EINT3_IRQn);				//Enable interrupt EINT3 share with GPIO interrupt.
	NVIC_SetPriority(EINT3_IRQn, 0);		//Set priority

}

void disableSysTick(void){

	SysTick-> CTRL &= ~3;	//Disable SysTick.

}

void mostrarAscii(void){

	LPC_GPIO2-> FIOPIN0 |= (LPC_GPIO2-> FIOMASK0 || contador);	//Write output and apply mask to PORT2 (output in P2.0 - P2.7 pins)
	LPC_GPIO2-> FIOMASK0 &= 0;		//Clear MASK

}

void configSysTick(void){

	SysTick-> CTRL |= 3;	//Enable SysTick counter and interruption
	SysTick-> CTRL &= ~(1 << 2);	//External CLK configuration (RC Oscillator)

}

void EINT3_IRQHandler(void){

	contador++;

	if(contador > 9){
		contador= 0;
	}

	LPC_GPIOINT-> IO0IntEnR &= ~(1 << 1);

	configSysTick();
}

void SysTick_Handler(void){

	if((LPC_GPIO0-> FIOPIN & 1) == 1){

		ones++;
		if(ones==3){
			mostrarAscii();
			disableSysTick();
			configGPIOINT();
		}

		if(ones > 3){
			ones= 0;
		}

	} else {

		ones= 0;

	}
}
