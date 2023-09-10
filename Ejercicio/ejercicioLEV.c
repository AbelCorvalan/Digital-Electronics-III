#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#define salida (1 << 22) //P0.22 salida

void configGPIO();
void configIE();
void configSystick();

int main(void) {

    configGPIO();
    configSystick();



    return 0 ;
}

void SysTick_Handler(void){
	static uint8_t clkDiv = 0;
	//static uint8_t counter = 0;
	static uint32_t señal = 0xFFB00; //<< 12);

	if(clkDiv==10){
		array= 0xFFB00;
		clkDiv=0;
	}
	else {
		array= (array >> 1);
		LPC_GPIO0->FIOPIN = (array & (0x003FF));
		clkDiv ++;
	}

	SysTick->CTRL &= SysTick->CTRL; //Clear flag
}

void configGPIO(){

	/* PINSEL */
	LPC_PINCON-> PINSEL1 |= ~(0x3 << 12);
	LPC_GPIO1 -> FIODIR  |= salida;

}

void configIE(){

	LPC_PINCON -> PINSEL4  |= (0x01 << 20);  //P2.10
	LPC_PINCON -> PINMODE4 |= (0x3  << 20);  //~~
	//LPC_GPIO1  -> FIODIR   |= (0x3  << 20);
	LPC_SC -> EXTMODE |= 0x1;
	LPC_SC -> EXTPOLAR |= 0x1;
	LPC_SC -> EXTINT   |= 0x1;  //Activar

	NVIC_SetPriority(EINT0_IRQn, 1);
	NVIC_EnableIRQ(EINT0_IRQn);

}

void configSystick(){

	SysTick -> LOAD= ((SystemCoreClock/100)-1);

	SysTick -> CTRL= (0x1 << 1)&(0x1 << 1)&(0x1 << 0);
	SysTick -> VAL= 0;

	NVIC_SetPriority(SysTick_IRQn, 0);
}
