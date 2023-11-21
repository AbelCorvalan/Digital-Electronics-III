/*
 * Copyright 2022 NXP
 * NXP confidential.
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

/*******************************************INCLUDE******************************************************/
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"

#include <string.h>
//#include <stdio.h>
/*****************************************End of INCLUDE*************************************************/
/*------------------------------------------------ DEFINES ------------------------------------------------------------------*/
//GPIO
//#define PRESCALE_VALUE_TIM1  (uint32_t)		1000000

/*----------------------------------------------- END DEFINES ---------------------------------------------------------------*/
/*------------------------------------------------ VARIABLES ----------------------------------------------------------------*/
//GPIO
uint16_t stepsDone = 0;

/*---------------------------------------------- FIN DEFINE TIMER -----------------------------------------------------------*/
/*----------------------------------------------- FUNCTIONS -----------------------------------------------------------------*/
void configPin(void);
void configGPIO(void);
void delay(uint32_t);
/*---------------------------------------------- END FUNCTIONS --------------------------------------------------------------*/

int main(void) {

    configPin();
    configGPIO();
    while(1) {

    }
    return 1;
}

void configPin(){
	//GPIO
	PINSEL_CFG_Type Pin;
	Pin.Funcnum = PINSEL_FUNC_0;
	Pin.Portnum = PINSEL_PORT_2;
	Pin.Pinmode = PINSEL_PINMODE_PULLDOWN;
	//Pin.OpenDrain = PINSEL_PINMODE_NORMAL;
	//for(int i = 0; i <= 7; i++){
	//	Pin.Pinnum =  PINSEL_PIN_0 + i;
	//	PINSEL_ConfigPin(&Pin);
	//}
	Pin.Pinnum =  PINSEL_PIN_0;
	PINSEL_ConfigPin(&Pin);
	//LED Flag
	Pin.Portnum = PINSEL_PORT_0;
	Pin.Funcnum = PINSEL_FUNC_0;
	Pin.Pinnum = PINSEL_PIN_22;
	PINSEL_ConfigPin(&Pin);
}

void configGPIO(){

	//GPIO2 Interrupt P2.0
	GPIO_SetDir(2, 0x00000001, 0);		// P2.0 configured as Input
	GPIO_IntCmd(2, 0x00000001, 0);       // Habilita int. por GPIO por flanco de subida.
	GPIO_ClearInt(2, 0x00000001);			// Limpia banderas int. por GPIO.

	GPIO_SetDir(0, (1 << 22), 1);
	GPIO_SetValue(0, (1 << 22));

	// Configuracion del NVIC para prioridades e interrupciones
	NVIC_SetPriority(EINT3_IRQn, 0);					 	// More priority than ADC and TIMER.
	NVIC_EnableIRQ(EINT3_IRQn); 							// Interrupt enabled.
	return;

}

void EINT3_IRQHandler(){
	delay(6000);//antirebote
	//GPIO_ClearValue(1, (1 << 22));
	GPIO_ClearInt(2, 0x00000001);
	if(GPIO_GetIntStatus(2, 0, 0)==0){
		LPC_GPIO0-> FIOCLR |= (1 << 22);
		delay(1000);
		LPC_GPIO0-> FIOSET |= (1 << 22);
		delay(1000);
		LPC_GPIO0-> FIOCLR |= (1 << 22);
		delay(1000);
		LPC_GPIO0-> FIOSET |= (1 << 22);
		delay(1000);
	}
    GPIO_ClearInt(2, 0x00000001);
}

void delay(uint32_t times){

	for(uint32_t i=0; i < times; i++){
		for(uint32_t i=0; i < times; i++){
		}
	}

}

