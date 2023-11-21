/*----------------------------------------------- INCLUDES ------------------------------------------------------------------*/
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_adc.h"
/*---------------------------------------------- END INCLUDES ---------------------------------------------------------------*/
/*------------------------------------------------ DEFINES ------------------------------------------------------------------*/
//ADC Converter//
#define PRESCALE_VALUE_TIM1  (uint32_t)		1000000
#define MATCH0_TIM1 		 (uint32_t)		2
#define REFERENCE_VALUE_LDR	 (uint32_t)		2055

//TIMER0 y MATCH0//
//Direction (DIR) and Step (STEP) pins.
#define STEP 	(1<<3)		//Step pin.
#define DIR     (1<<21)		//Direction pin.


//Prescaler and Match to Timer0 Values.
#define PRESCALE_VALUE_TIM0  (uint32_t)		2000	  //TIMER0 Prescaler Value in [ms]. (Prescaler configured to 2000ms)
#define MATCH0_TIM0		 	 (uint32_t)		5		  //Match0 Value (MATCH0_TIM0=1/10). Determines the parameter velocity
#define PORT_ZERO            (uint32_t)     0		  //PORT0 define.




#define fullTurn       (uint16_t) 200	//Steps numbers: (360°/1.8°)= 200 steps
#define fullEnrollment (uint16_t) 800
/*----------------------------------------------- END DEFINES ---------------------------------------------------------------*/
/*------------------------------------------------ VARIABLES ----------------------------------------------------------------*/
//Motor Variables
uint16_t stepsDone = 0;

uint8_t  SECUENCE = 0;		 // Variable que determina sentido de avance (up/down) de la cortina.
uint8_t  fullStepUp = 0;

uint16_t revolution  = 1;	 // Variable que representa la cantidad de giros completos del motor.
							 // Toma un valor entre 1 y 5
						     // 1 giro de 360° = 48 pasos = 12 interrupciones * 4 pasos;
uint16_t distPerRev = 5;     // Desplazamiento de la cortina [cm] en un giro del motor de 360°
///uint16_t moveDist;
//uint16_t steps = 48;		 // Cantidad de pasos por giro completo del motor (360°/7,5°)
uint16_t limInicial = 0;	 // Valor inicial del rango de movimiento de la cortina [cm]
uint16_t limFinal   = 150;	 // Valor final del rango de movimiento de la cortina [cm]
//uint16_t posActual  = 0;	 // Posicion actual de la cortina comprendido entre limInicial y limFinal.


//ADC Converter Variables//
//volatile uint16_t ADC0Value = 0; 	//Variable auxiliar para observar el valor del registro de captura.
//uint16_t ADC0Value = 0;
volatile float ADC0Value = 0;
//Steps
uint16_t lapsC= 600;	//600
float volt=0;
//volatile float volt1= 0;
/*---------------------------------------------- FIN DEFINE TIMER -----------------------------------------------------------*/
/*----------------------------------------------- FUNCTIONS -----------------------------------------------------------------*/
void configPin(void);

void configGPIO(void);		//*
void configTIMER0(void);
void enableTIM0(void);

void configTIMER1(void);
void configADC(void);
void ADC_LDR(void);
//void ADC_IRQHandler(void);

void disableTIM1(void);
void enableTIM0(void);
void delay(uint32_t);

/*---------------------------------------------- END FUNCTIONS --------------------------------------------------------------*/

int main(void) {

	configPin();
	configGPIO();
	configADC();
	configTIMER1();
	//configTIMER1();
	ADC_LDR();

    while(1) {
    }
    return 0 ;
}

/* Función que habilita/deshabilita conversion ADC */
void ADC_LDR(void){

	int static activateADC = 0;
	// LimpioLetraPresionada(keyPush);

	switch (activateADC) {
	case 1:
		//GPIO_ClearValue(PORT_ZERO, (1 << 2));
		TIM_Cmd(LPC_TIM1, DISABLE);
		ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, DISABLE);
		NVIC_DisableIRQ(ADC_IRQn);
		activateADC = 0;
		break;
	default:
		//GPIO_SetValue(PORT_ZERO, (1 << 2));
		TIM_ResetCounter(LPC_TIM1);
		TIM_Cmd(LPC_TIM1, ENABLE);
		ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
		NVIC_EnableIRQ(ADC_IRQn);
		activateADC = 1;
		break;
	}
	return;
}


void ADC_IRQHandler(void){

	// 	Codigo extraido de clase con adaptacion a este proyecto.

	//float volt= 0;

	ADC0Value = (ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0) & 0xFFF); 	 //12 bit Mask to extract result.
	volt = ((float)ADC0Value / 4095.0) * 3.3;
	lapsC= fullEnrollment*(volt/3.3);
	// La idea es aprovechar la luz del dia.
	if (0 <= volt && volt <  0.825) {
		//Discrete_Up();							// Deberia subir toda la cortina
	}
	else if(0.825 <= volt && volt < 1.65){
		//Discrete_Up();							// Deberia subir parcialmente la cortina
	}
	else if (1.65 <= volt && volt < 2.475){
		//Discrete_Down();						// Deberia bajar parcialmente la cortina
	}
	else if(2.475 <= volt && volt <= 3.3){
		//Discrete_Down();						// Deberia bajar toda la cortina.
	}
	//volatile uint16_t ADC0Value = 0; //Variable auxiliar para observar el valor del registro de captura.
	//float volt = 0;

	//ADC0Value = (ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0) & 0xFFF);
	//(volt = (ADC0Value / 4096) * 3.3;
	if (ADC0Value > REFERENCE_VALUE_LDR) {
		//Discrete_Down();
	}
	else if (ADC0Value < REFERENCE_VALUE_LDR) {
		//Discrete_Up();
	}
	configTIMER0();
	enableTIM0();
	//Desactivo TIM1
	TIM_DeInit(LPC_TIM1);
	//NVIC_SetPriority(TIMER0_IRQn, 1);
	NVIC_DisableIRQ(TIMER1_IRQn);
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, DISABLE);
	NVIC_DisableIRQ(ADC_IRQn);

	//enableTIM0();

	return;
}

void enableTIM0(void){

	TIM_ResetCounter(LPC_TIM0);
	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
	return;
}

void configPin(void){

	PINSEL_CFG_Type Pin;

	Pin.Funcnum = PINSEL_FUNC_1;						// P0.23 como ADC Channel 0.
	Pin.Portnum = PINSEL_PORT_0;
	Pin.Pinnum = PINSEL_PIN_23;
	Pin.Pinmode = PINSEL_PINMODE_TRISTATE;
	Pin.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&Pin);
	return;

	//Stepper Motor pins.
		//PINSEL_CFG_Type Pin;
		Pin.Funcnum = PINSEL_FUNC_0;
		Pin.Portnum = PINSEL_PORT_0;
		//Pin.Pinmode = PINSEL_PINMODE_PULLUP;
		Pin.OpenDrain = PINSEL_PINMODE_NORMAL;
		Pin.Pinnum =  PINSEL_PIN_21;
		Pin.Pinnum =  PINSEL_PIN_3;
		PINSEL_ConfigPin(&Pin);
}

void configGPIO(){
	//Stepper Motor GPIO pins configuration
	GPIO_SetDir(PINSEL_PORT_0, STEP, 1);		// Motor Step (STEP) - P0.21 (GPIO0).
	GPIO_SetDir(PINSEL_PORT_0, DIR, 1);		    // Motor Direction (DIR) - P0.3 (GPIO0.3).
	GPIO_ClearValue(PINSEL_PORT_0, STEP);
	GPIO_ClearValue(PINSEL_PORT_0, DIR);

}

/* Función que configura el ADC para dispararse mediante TIMER1
 * Realiza un conversion aproximadamente cada 10[s] */
void configADC(void){

	ADC_Init(LPC_ADC, 200000);							// Conversion a 200kHz
	LPC_ADC->ADCR &= ~(1<<16);							// No burst
	ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT10);			// Conversion en MR0 del TIM1
	ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_RISING);	// en flanco de bajada.
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);		// Habilita interrupcion por ADC.
	NVIC_SetPriority(ADC_IRQn, 4);
	NVIC_EnableIRQ(ADC_IRQn);							// Interrupcion del NVIC para el ADC habilitada.
	return;
}

/* Rutina de interrupcion del TIMER1
 * Genera la base de tiempo para contar 10 segundos y disparar conversion de ADC*/
void configTIMER1(void){

	TIM_TIMERCFG_Type TimerONE;
	TIM_MATCHCFG_Type MatchZERO;
	TimerONE.PrescaleOption = TIM_PRESCALE_USVAL;
	TimerONE.PrescaleValue = PRESCALE_VALUE_TIM1;		 // Prescaler de 1s

	MatchZERO.MatchChannel = 0;
	MatchZERO.MatchValue = MATCH0_TIM1;					 // Match para disparar ADC cada 10 segundos
	MatchZERO.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
	MatchZERO.IntOnMatch = DISABLE;
	MatchZERO.StopOnMatch = DISABLE;
	MatchZERO.ResetOnMatch = ENABLE;

	TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &TimerONE);
	TIM_ConfigMatch(LPC_TIM1, &MatchZERO);
	return;
}

void configTIMER0(void){

  /*time before interruption: t = (1/PCLK)*(PC + 1)*(TC + 1).
	 * Cuanto vale el CCLK?????????? Averiguar.
	 * t = (1/(CCLK/x_div))*(PR + 1)*(MRx + 1). Where MRx = 0. If t = 2s, and CCLK = 120MHz(supongo),
	 * So =>  PR = t*(CCLK/x_div) - 1    =>    PR = 2s*(120M/4) - 1 = 59999999
	 * El '-1' está mal ya que luego en la funcion le resta nuevamente '-1'.
	 * Luego,  PR = 60000000.*/

	/* Timer para temporizar la secuencia de control del motor*/
	TIM_TIMERCFG_Type TimerZERO;
	TIM_MATCHCFG_Type MatchConfig;
	//TIM_MATCHCFG_Type MatchZERO, MatchConfig;
	TimerZERO.PrescaleOption = TIM_PRESCALE_USVAL;						// Prescaler de 1ms
	TimerZERO.PrescaleValue = PRESCALE_VALUE_TIM0;

	//for (int MatchX = 0; MatchX < 4; MatchX++) {						// Carga MR0, MR1, MR3 y MR4
		MatchConfig.MatchChannel = 0;
		MatchConfig.MatchValue = MATCH0_TIM0; //+ (MatchX * MATCH0_TIM0 ); //Load MRx with 5mseg, 10mseg, 15mseg and 20mseg.
		MatchConfig.IntOnMatch = ENABLE;
		MatchConfig.ResetOnMatch = ENABLE;
		MatchConfig.StopOnMatch = DISABLE;
		MatchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
		TIM_ConfigMatch(LPC_TIM0, &MatchConfig);
	//}

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TimerZERO);
	NVIC_SetPriority(TIMER0_IRQn, 1);
	NVIC_EnableIRQ(TIMER0_IRQn);				// Habilita interrupcion en NVIC
	return;
}

void TIMER0_IRQHandler(void){

	//fullTurn.
	//if(stepsDone == lapsC){
	if(stepsDone == lapsC){
		stepsDone=0;
		TIM_DeInit(LPC_TIM0);
		//NVIC_SetPriority(TIMER0_IRQn, 1);
		NVIC_DisableIRQ(TIMER0_IRQn);
	} else {
		if(TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)){
			GPIO_ClearValue(PORT_ZERO, DIR);
			GPIO_SetValue(PORT_ZERO, STEP);
			TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
		}
		GPIO_ClearValue(PORT_ZERO, STEP);
		stepsDone++;
	}

	return;

}

void delay(uint32_t times){

	for(uint32_t i=0; i < times; i++){
		for(uint32_t i=0; i < times; i++){
		}
	}

}
