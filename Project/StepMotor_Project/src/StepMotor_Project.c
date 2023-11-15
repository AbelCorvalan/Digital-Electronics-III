#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"

void configPin(void);
void configGPIO(void);
void UpContinuosMov(void);
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
						     // 1 giro de 360° = 48 pasos = 12 interrupciones * 4 pasos;
uint16_t distPerRev = 5;     // Desplazamiento de la cortina [cm] en un giro del motor de 360°
uint16_t moveDist;
uint16_t steps = 48;		 // Cantidad de pasos por giro completo del motor (360°/7,5°)
uint16_t limInicial = 0;	 // Valor inicial del rango de movimiento de la cortina [cm]
uint16_t limFinal   = 150;	 // Valor final del rango de movimiento de la cortina [cm]
uint16_t posActual  = 0;	 // Posicion actual de la cortina comprendido entre limInicial y limFinal.

void delay(uint32_t);
//Sentido de giro.
uint8_t estado= 0; // 0 es sentido de giro a la derecha.

int main(void){

	configPin();
	configGPIO();
	//configTIMER0();
	//enableTIM0();

	while(1){
		accionar();
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
	GPIO_SetValue(PORT_ZERO, (1<<21)); //21 STEP
	delay(1000);
	GPIO_ClearValue(PORT_ZERO , (1<<21));
	delay(1000);

}

void delay(uint32_t times){

	for(uint32_t i=0; i<times; i++){
		for(uint32_t j=0; j<times; j++){

		}

	}
}

/*void UpContinuosMov(void){
	//  el movimiento es full, cambiar el nombre a la funcion.
	moveDist = revolution * distPerRev;
	if (posActual > (limInicial + moveDist)) {
		enableTIM0();
		SECUENCE = SECUENCE1;
		fullStepUp = 1;
	}
	return;
}*/

/*void enableTIM0(void){

	TIM_ResetCounter(LPC_TIM0);
	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
	return;
}*/

//void configTIMER0(void){

  /*time before interruption: t = (1/PCLK)*(PC + 1)*(TC + 1).
	 * Cuanto vale el CCLK?????????? Averiguar.
	 * t = (1/(CCLK/x_div))*(PR + 1)*(MRx + 1). Where MRx = 0. If t = 2s, and CCLK = 120MHz(supongo),
	 * So =>  PR = t*(CCLK/x_div) - 1    =>    PR = 2s*(120M/4) - 1 = 59999999
	 * El '-1' está mal ya que luego en la funcion le resta nuevamente '-1'.
	 * Luego,  PR = 60000000.*/

	/* Timer para temporizar la secuencia de control del motor*/
	/*TIM_TIMERCFG_Type TimerZERO;
	TIM_MATCHCFG_Type MatchConfig;
	//TIM_MATCHCFG_Type MatchZERO, MatchConfig;
	TimerZERO.PrescaleOption = TIM_PRESCALE_USVAL;						// Prescaler de 1ms
	TimerZERO.PrescaleValue = PRESCALE_VALUE_TIM0;

	for (int MatchX = 0; MatchX < 4; MatchX++) {						// Carga MR0, MR1, MR3 y MR4
		MatchConfig.MatchChannel = MatchX;
		MatchConfig.MatchValue = MATCH0_TIM0 + (MatchX * MATCH0_TIM0 ); //Load MRx with 5mseg, 10mseg, 15mseg and 20mseg.
		MatchConfig.IntOnMatch = ENABLE;
		MatchConfig.ResetOnMatch = DISABLE;
		MatchConfig.StopOnMatch = DISABLE;
		MatchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
		TIM_ConfigMatch(LPC_TIM0, &MatchConfig);
	}

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TimerZERO);
	NVIC_SetPriority(TIMER0_IRQn, 1);
	NVIC_EnableIRQ(TIMER0_IRQn);				// Habilita interrupcion en NVIC
	return;
}*/

/*void TIMER0_IRQHandler(void){

	//if(TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)){
		GPIO_SetValue(PORT_ZERO, (1<<15));
		if(estado == 0) {
			GPIO_ClearValue(PORT_ZERO , (1<<16));
			TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
			//estado= 1;
		} //else {
			//GPIO_SetValue(PORT_ZERO , (1<<16));
			//TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
			//estado= 0;
		//}
		//break;
	//}

	return;

}*/
