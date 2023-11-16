#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"

void configPin(void);
void configGPIO(void);
void OneStep(void);
void enableTIM0(void);
void configTIMER0(void);
void accionar();

//Secuencias
#define SECUENCE1 (uint8_t) 1
#define SECUENCE2 (uint8_t) 2

/********************************************* DEFINE TIMER *************************************************************************************/
//Valores de Prescaler y Match para Timer0
#define PRESCALE_VALUE_TIM0  (uint32_t)		1000
#define MATCH0_TIM0		 	 (uint32_t)		5		  // VALOR QUE FUNCIONO -> MATCH0_TIM0=5
#define PORT_ZERO            (uint32_t)     0
/*********************************************** FIN DEFINE TIMER *******************************************************************************/


uint8_t  SECUENCE = 0;		 // Variable que determina sentido de avance (up/down) de la cortina.
uint8_t  fullStepUp = 0;


uint16_t revolution  = 1;	 // Variable que representa la cantidad de giros completos del motor.
							 // Toma un valor entre 1 y 5
						     // 1 giro de 360° = 200 pasos = 12 interrupciones * 4 pasos;
uint16_t distPerRev = 5;     // Desplazamiento de la cortina [cm] en un giro del motor de 360°
uint16_t moveDist;
uint16_t steps = 200;		 // Cantidad de pasos por giro completo del motor (360°/1,8°)
uint16_t limInicial = 0;	 // Valor inicial del rango de movimiento de la cortina [cm]
uint16_t limFinal   = 150;	 // Valor final del rango de movimiento de la cortina [cm]
uint16_t posActual  = 0;	 // Posicion actual de la cortina comprendido entre limInicial y limFinal.

void delay(uint32_t);
int count= 0;
//Sentido de giro.
uint8_t estado= 0; // 0 es sentido de giro a la derecha.

int main(void){

	configPin();
	configGPIO();
	configTIMER0();
	//enableTIM0();

	while(1){
		//accionar();
	}

	return 1;
}

void configPin(){

	PINSEL_CFG_Type Pin;
	Pin.Funcnum = PINSEL_FUNC_0;
	Pin.Portnum = PINSEL_PORT_0;
	//Pin.Pinmode = PINSEL_PINMODE_PULLUP;
	Pin.OpenDrain = PINSEL_PINMODE_NORMAL;
	Pin.Pinnum =  PINSEL_PIN_21;
	Pin.Pinnum =  PINSEL_PIN_3;
	PINSEL_ConfigPin(&Pin);

}

void configGPIO(){

	GPIO_SetDir(PINSEL_PORT_0, (1<<3), 1); 		 	// GPIO0 - Motor Dir. OUTPUT
	GPIO_SetDir(PINSEL_PORT_0, (1<<21), 1);
	GPIO_ClearValue(PINSEL_PORT_0, (1<<3));
	GPIO_ClearValue(PINSEL_PORT_0, (1<<21));

}

void accionar(){

	GPIO_ClearValue(PORT_ZERO , (1<<3)); //3 DIR
	GPIO_ClearValue(PORT_ZERO, (1<<21)); //21 STEP
	GPIO_SetValue(PORT_ZERO, (1<<21)); //21 STEP
	//delay(500);
	//GPIO_ClearValue(PORT_ZERO , (1<<21));
	//delay(500);

}

void delay(uint32_t times){

	for(uint32_t i=0; i<times; i++){
		for(uint32_t j=0; j<times; j++){

		}

	}
}

void OneStep(void){
	//  el movimiento es full, cambiar el nombre a la funcion.
	//moveDist = revolution * distPerRev;
	//if (posActual > (limInicial + moveDist)) {

		//count++;

		//SECUENCE = SECUENCE1;
		//accionar();
		//fullStepUp = 1;
	//}
	return;
}

void enableTIM0(void){
	TIM_ResetCounter(LPC_TIM0);
	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
	return;
}

void configTIMER0(void){

	LPC_PINCON-> PINSEL3 |= (3 << 24);   //P1.28 as MAT0.0

	LPC_SC-> PCONP  |= (1 << 1);
	LPC_SC-> PCLKSEL0 |= (1 << 2);	//PCLK = cclk

	LPC_TIM0-> PR = 0;
	LPC_TIM0-> MR0 = 1;

	LPC_TIM0->MCR = 2;         // Timer0 reset on Match0
	LPC_TIM0->EMR |= (3 << 4); // MAT0.0 toggle mode

	LPC_TIM0->IR |= 0x3F;      // Clear all interrupt flag
	LPC_TIM0->TCR = 3;         // Enable and Reset
	LPC_TIM0->TCR &= ~2;

	NVIC_SetPriority(TIMER0_IRQn, 1);
	NVIC_EnableIRQ(TIMER0_IRQn);				// Habilita interrupcion en NVIC
	return;
}

void TIMER0_IRQHandler(void){
	//Giro completo.
	if(count < 200){
    	OneStep();
		count++;
	} else {
		count= 300;
	}

	LPC_TIM0->IR |= 1 << 0;
	return;

}

