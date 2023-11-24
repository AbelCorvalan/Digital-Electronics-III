/*
 ===============================================================================
 Name        : Trabajo Final Electrónica Digital III.c
 Authors     :	Corvalán, Abel
 	 	Mangin, Matías
 Version     : 1.0
 Description :	Cortina Automática
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

/*******************************************INCLUDE******************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_uart.h"

/*****************************************End of INCLUDE*************************************************/

/********************************************DEFINE******************************************************/
#define ADC_CH_ZERO   (uint8_t) 0
#define CHANNEL_0	  (uint8_t) 0
#define INPUT 		  (uint8_t) 0
#define OUTPUT 		  (uint8_t) 1
#define DMA_SIZE			 1		// cant de datos a transferir.

//Valores de Prescaler y Match para Timer1
#define PRESCALE_VALUE_TIM1  (uint32_t)		1000000 //  TIMER1 config cada 1seg.
#define MATCH0_TIM1 		 (uint32_t)		4 //5

//TIMER0 y MATCH0.
#define STEP 	(1<<3)
#define DIR     (1<<21)

//Valores de Prescaler y Match para Timer0.
#define PRESCALE_VALUE_TIM0  (uint32_t)		5000	  // TIMER0 Prescaler Value in [us]. (Prescaler configured to 2000ms)
#define MATCH0_TIM0		 	 (uint32_t)		1		  // Match0 Value (MATCH0_TIM0=1).
#define PORT_ZERO            (uint32_t)     0

//Secuencias
#define SECUENCE1 (uint8_t)  1
#define SECUENCE2 (uint8_t)  2

#define fullTurn       (uint16_t) 200	//Steps numbers: (360°/1.8°)= 200 steps
#define fullEnrollment (uint16_t) 800	//800
/****************************************END of DEFINE***************************************************/

/************************************DECLARED FUNCTIONS**************************************************/
void configPIN(void);
void configGPIO(void);
void enableTIM0(void);
void configTIMER0(void);
void configTimerOne(void);
void configADC(void);
void ADCStart(void);
void ADC_IRQHandler(void);
void UART2_IRQHandler(void);
void configDMAList(void);
void configDMA(void);
void configUART2(void);
//void processData(uint16_t);
void processData(void);
void send_data(void);

/*********************************END of DECLARED FUNCTIONS**********************************************/

/****************************************GLOBAL VARIABLE*************************************************/
uint16_t ADC0ValueAux = 0;
uint8_t activateADC = 0;
uint8_t sendVoltData[10] = { '0' };

//Motor Variables
uint16_t stepsDone = 0;


uint8_t state = 0;

volatile float ADC0Value1 = 0;
volatile uint16_t ADC0Value2 = 0;
uint16_t lapsC = 0;	//600
float volt = 0;


uint8_t statusDMA=0;
uint8_t completo= 0;

//UART
uint8_t instruct;
uint8_t uartState=1;
uint8_t muestra=1;
uint8_t seteo= 0;


uint8_t valor_int8;

char valor_char;
uint8_t valor;
uint8_t valorAux;
uint8_t valorAuxA1;
char valorAuxA1Array[16];

/*************************************END of GLOBAL VARIABLE*********************************************/

/********************************************STRUCTURE***************************************************/
GPDMA_LLI_Type DMA_LLI_Struct;
GPDMA_Channel_CFG_Type GPDMACfg; 	//Estructura de configuracion del DMA
/*****************************************END of STRUCTURE***********************************************/

/***********************************************MAIN*****************************************************/
int main() {

	configPIN();
	configGPIO();
	configTIMER0();
	configTimerOne();
	configADC();
	configDMA();
	configUART2();
	GPDMA_ChannelCmd(CHANNEL_0, ENABLE); 				   // Enable DMA channel 0
	ADCStart();

	while (1) {
		if(muestra){
			UART_Send(LPC_UART2, "Ingrese un sentido de giro para la cortina \n\r", sizeof("Ingrese un sentido de giro para la cortina \n\r"), BLOCKING);
			UART_Send(LPC_UART2, "Opcion 0: enrolla la cortina \n\r", sizeof("Opcion 0: enrolla la cortina \n\r"), BLOCKING);
			UART_Send(LPC_UART2, "Opcion 1: desenrolla la cortina \n\r", sizeof("Opcion 1: desenrolla la cortina \n\r"), BLOCKING);
			UART_Send(LPC_UART2, "Opcion 3: frena el motor \n\r", sizeof("Opcion 3: frena el motor \n\r"), BLOCKING);
			UART_Send(LPC_UART2, "Opcion 4: enciende el motor \n\r\n\r", sizeof("Opcion 4: enciende el motor \n\r\n\r"), BLOCKING);
			muestra=0;
		}
	}

	return 0;

}

void send_data(void) {


	valor_char= (char)ADC0Value2 & (0xFF);
	valor_int8= (ADC0Value2 & (0xFF));
	valor= (unsigned int) valor_int8;
	valorAux= (-160*valor) + 255;
	valorAuxA1= (100*valorAux)/380; //280
	uitoa(valorAuxA1, valorAuxA1Array, 10);


	char caracter[16];


	////Funciona bien
	UART_Send(LPC_UART2, "Nivel de luz es de: ", sizeof("Nivel de luz es de: "), BLOCKING);
	UART_Send(LPC_UART2, valorAuxA1Array, sizeof(valorAuxA1Array), BLOCKING);;
	UART_Send(LPC_UART2, " %.\n\r\n\r", sizeof(" %.\n\r\n\r"), BLOCKING);

}

void configGPIO() {
	//Stepper Motor GPIO pins configuration
	GPIO_SetDir(PINSEL_PORT_0, STEP, 1);// Motor Step (STEP) - P0.21 (GPIO0).
	GPIO_SetDir(PINSEL_PORT_0, DIR, 1);	// Motor Direction (DIR) - P0.3 (GPIO0.3).
	GPIO_ClearValue(PINSEL_PORT_0, STEP);
	GPIO_ClearValue(PINSEL_PORT_0, DIR);

}

/* Función que habilita/deshabilita conversion ADC */
void ADCStart(void) {

	switch (activateADC) {
	case 1:										// Se desactiva el ADC.
		TIM_Cmd(LPC_TIM1, DISABLE);
		ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, DISABLE);
		NVIC_DisableIRQ(ADC_IRQn);
		activateADC = 0;
		break;
	default:									// Se activa el ADC.
		TIM_ResetCounter(LPC_TIM1);
		TIM_Cmd(LPC_TIM1, ENABLE);
		ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
		NVIC_EnableIRQ(ADC_IRQn);
		activateADC = 1;

		break;
	}
	return;
}


void ADC_IRQHandler(void) {

	ADC0Value1 = (ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0) & 0xFFF); //12 bit Mask to extract result.
	volt = ((float) ADC0Value1 / 4095.0) * 3.3;

	ADC0Value2 = (ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0) & 0xFFF); //Para mostrar el valor en porcentaje mediante UART

	activateADC = 1;
	ADCStart();

	// La idea es aprovechar la luz del dia.
if(seteo==0){
	if (0 <= volt && volt < 0.825 ) {
		lapsC = fullEnrollment;					// Deberia subir toda la cortina
	        state = 0;
	} else if (0.825 <= volt && volt < 1.65 ) {
		lapsC = fullEnrollment;			// Deberia desenrollar la cortina
		state = 1;
	}
} else {
	seteo=0;
}

	send_data();
	enableTIM0();
	return;
}


/*****************************************FUNCTION SETTINGS**********************************************/

/*
 * 	Configuracion de pines.
 */
void configPIN(void) {

	PINSEL_CFG_Type PinCfg;

	PinCfg.Portnum = PINSEL_PORT_0;	// Configuracion de los pines para el ADC.
	PinCfg.Funcnum = PINSEL_FUNC_1;				// AD0.0 function.
	PinCfg.Pinnum = PINSEL_PIN_23;				// P0.23
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;// Pin has a tristate resistor enabled.
	PINSEL_ConfigPin(&PinCfg);

	//Configuracion pin de Tx y Rx
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	//Stepper Motor pins.
	PINSEL_CFG_Type Pin;
	Pin.Funcnum = PINSEL_FUNC_0;
	Pin.Portnum = PINSEL_PORT_0;
	//Pin.Pinmode = PINSEL_PINMODE_PULLUP;
	Pin.OpenDrain = PINSEL_PINMODE_NORMAL;
	Pin.Pinnum = PINSEL_PIN_21;
	Pin.Pinnum = PINSEL_PIN_3;
	PINSEL_ConfigPin(&Pin);


	return;
}

/*
 *
 */
void configTimerOne() {

	TIM_TIMERCFG_Type TimerONE;
	TIM_MATCHCFG_Type MatchZERO;

	TimerONE.PrescaleOption = TIM_PRESCALE_USVAL;
	TimerONE.PrescaleValue = PRESCALE_VALUE_TIM1;		 // Prescaler de 1s
	MatchZERO.MatchChannel = 0;
	MatchZERO.MatchValue = MATCH0_TIM1;	// Match para disparar ADC cada 10 segundos
	MatchZERO.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE; // Toggle de la salida de MAT1.0 (automatico)
	MatchZERO.IntOnMatch = DISABLE;	// Interrupciones y Stop on match desactivados
	MatchZERO.StopOnMatch = DISABLE;
	MatchZERO.ResetOnMatch = ENABLE;				// Reset on Match activado

	TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &TimerONE);// Inicializacion del TIMER1 con pclk=cclk/4
	TIM_ConfigMatch(LPC_TIM1, &MatchZERO);

	return;
}

/* Función que configura el ADC para dispararse mediante TIMER1
 * Realiza una conversion aproximadamente cada 10[s]
 */
void configADC() {

	ADC_Init(LPC_ADC, 200000);	// Enable ADC and put rate ADC conversion rate.
	LPC_ADC->ADCR &= ~(1 << 16);						// No burst
	ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT10);	// Conversion en MR0 del TIM1
	ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_RISING);	// en flanco de subida.
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);// Habilita interrupcion por ADC.
	NVIC_SetPriority(ADC_IRQn, 1);// Menor prioridad que TIMER0 e Int. por GPIO.

	return;
}

/*
 * Configuracion de la lista del DMA. Se debe hacer antes de habilitar el canal.
 */
void configDMA(void) {

	//Prepare DMA linked list item structure
	GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, CHANNEL_0); // Se limpia inerrupcion por si las hubiera.

	DMA_LLI_Struct.SrcAddr = (uint32_t) &sendVoltData;	// &Data source address
	DMA_LLI_Struct.DstAddr = (uint32_t) &(LPC_UART2->THR); // Destination: UART0. THR: Transmit holding register
	DMA_LLI_Struct.NextLLI = (uint32_t) &DMA_LLI_Struct; // Next LLI is the same. Not want to deseable DMA channel.
	DMA_LLI_Struct.Control = DMA_SIZE | (0 << 12) | (0 << 15) // Size 1; SBsize: 8bits; DBsize: 8bits;
			| (0 << 18) | (0 << 21) | (0 << 26)	// Swidth: 8bits; Dwidht: 8bits; source incremented.
			| (0 << 27) | (1 << 31);			// Destination address is not incremented.

	GPDMA_Init();								   	// Inicializa el modulo DMA

	GPDMACfg.ChannelNum = 0; 					 		   // Canal 0
	GPDMACfg.SrcMemAddr = (uint32_t) &sendVoltData; 	   //&
	GPDMACfg.DstMemAddr = 0; // Es 0 ya que el destino es de M2P. Luego, no se usa.
	GPDMACfg.TransferSize = DMA_SIZE;			// Tamaño de la transferencia
	GPDMACfg.TransferWidth = 0; 	// No usado ya que no es una transf. M2M.
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;	// Tipo de transferencia = Memory 2 Peripheral
	GPDMACfg.SrcConn = 0;	// La fuente es memoria => no connection. No se usa
	GPDMACfg.DstConn = GPDMA_CONN_UART2_Tx;      // Destino : conexión al UART2
	GPDMACfg.DMALLI = (uint32_t) &DMA_LLI_Struct;	 // Lista de enlace del	DMA
	GPDMA_Setup(&GPDMACfg);

	return;
}



/*
 * Funcion de configuracion de UART2. Se configura para transmitir datos.
 */
void configUART2(void) {

	UART_CFG_Type UARTConfigStruct;
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	//configuracion por defecto:
	UART_ConfigStructInit(&UARTConfigStruct);
	//inicializa periferico
	UART_Init(LPC_UART2, &UARTConfigStruct);
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	//Inicializa FIFO
	UART_FIFOConfig(LPC_UART2, &UARTFIFOConfigStruct);
	//Habilita transmision
	UART_TxCmd(LPC_UART2, ENABLE);


	LPC_UART2-> FCR |= (1<<3);	//UART with DMA ocnfiguration

    UART_IntConfig(LPC_UART2, UART_INTCFG_RBR, ENABLE);
    UART_IntConfig(LPC_UART2, UART_INTCFG_RLS, ENABLE);
	NVIC_EnableIRQ(UART2_IRQn);

	return;
}

void enableTIM0(void) {

	TIM_ResetCounter(LPC_TIM0);
	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
	return;
}

void configTIMER0(void) {

	/* Timer para temporizar la secuencia de control del motor*/
	TIM_TIMERCFG_Type TimerZERO;
	TIM_MATCHCFG_Type MatchConfig;

	TimerZERO.PrescaleOption = TIM_PRESCALE_USVAL;			// Prescaler en microsegundos
	TimerZERO.PrescaleValue = PRESCALE_VALUE_TIM0;			// 5000useg.

	MatchConfig.MatchChannel = 0;
	MatchConfig.MatchValue = MATCH0_TIM0;
	MatchConfig.IntOnMatch = ENABLE;
	MatchConfig.ResetOnMatch = ENABLE;
	MatchConfig.StopOnMatch = DISABLE;
	MatchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	TIM_ConfigMatch(LPC_TIM0, &MatchConfig);

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TimerZERO);
	NVIC_SetPriority(TIMER0_IRQn, 2);
	return;
}

void TIMER0_IRQHandler(void) {

	if (state == 0) {
		if (stepsDone == lapsC) {
			stepsDone = 0;
			NVIC_DisableIRQ(TIMER0_IRQn);
			activateADC = 0;
			ADCStart();
			TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);

		} else {
			if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)) {
				GPIO_ClearValue(PORT_ZERO, DIR);
				GPIO_SetValue(PORT_ZERO, STEP);
				TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
			}
			GPIO_ClearValue(PORT_ZERO, STEP);
			stepsDone++;
		}
	} else if(state == 1){

		if (stepsDone == lapsC) {
			stepsDone = 0;
			NVIC_DisableIRQ(TIMER0_IRQn);
			activateADC = 0;
			ADCStart();
			TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);

		} else {
			if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)) {
				GPIO_SetValue(PORT_ZERO, DIR);
				GPIO_SetValue(PORT_ZERO, STEP);
				TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
			}
			GPIO_ClearValue(PORT_ZERO, STEP);
			stepsDone++;
		}

	}

	return;

}

void UART2_IRQHandler(void) {

    // Comprobar si la interrupción es por recepción de datos
    if (UART_GetIntId(LPC_UART2) & UART_IIR_INTID_RDA) {
    	seteo=1;
    	instruct = UART_ReceiveByte(LPC_UART2); // Leer el dato recibido
        if(uartState){
        	if(instruct == '0'){
        		UART_Send(LPC_UART2, "Opcion ingresada: 0. Enrolla la cortina \n\r", sizeof("Opcion ingresada: 0. Enrolla la cortina \n\r"), BLOCKING);
        		state=0;
        		stepsDone=0;
        		lapsC= 800;
        		activateADC = 0;
        		ADCStart();
        		muestra=1;
        	}
        	if(instruct == '1'){
          		UART_Send(LPC_UART2, "Opcion ingresada: 1. Desenrolla la cortina \n\r", sizeof("Opcion ingresada: 1. Desenrolla la cortina \n\r"), BLOCKING);
          		state=1;
          		lapsC= 800;
          		stepsDone=0;
          		activateADC = 0;
          		ADCStart();
          		muestra=1;
        	}

        	if(instruct == '3'){
        	    UART_Send(LPC_UART2, "Opcion ingresada: 3. Frena el motor \n\r", sizeof("Opcion ingresada: 3. Frena el motor \n\r"), BLOCKING);
        	    state=0;
        	    activateADC = 1;
        	    ADCStart();
        	    muestra=1;
        	 }

        	if(instruct == '4'){
        	     UART_Send(LPC_UART2, "Opcion ingresada: 4. Enciende el motor \n\r", sizeof("Opcion ingresada: 4. Enciende el motor \n\r"), BLOCKING);
        	     state=0;
        	     activateADC = 0;
        	     ADCStart();
        	     muestra=1;
        	}

        	if(instruct != '1' && instruct != '0' && instruct == '3' && instruct == '4'){
        		UART_Send(LPC_UART2, "Opcion no valida, intente nuevamente \n\r", sizeof("Opcion no valida, intente nuevamente \n\r"), BLOCKING);
        		muestra=1;
        	}
        }

    }
}
