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

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include "lpc17xx_uart.h"

void configUART();
void sendMessage();

int arreglo[8]= {0, 1, 1, 1, 1, 1, 1, 0};

int main(void) {

	configUART();

    while(1) {
    	sendMessage();
    }
    return 0;
}

void configUART(){

	LPC_PINCON-> PINSEL0 |=  (1 << 4); //P0.2 configured as TXD0 (Function 01 in pinsel 5:4).
	LPC_PINCON-> PINSEL0 &= ~(1 << 5);
	LPC_PINCON-> PINSEL0 |=  (1 << 6); //P0.3 configured as RXD0 (Function 01 in pinsel 6:7).
	LPC_PINCON-> PINSEL0 &= ~(1 << 7);

	LPC_SC-> PCONP |= (1 << 1);	       //PCONP UART0
	LPC_SC-> PCLKSEL0 |=  (1 << 6);	   //PCLK_UART0 01 -> PCLK without divisor.
	LPC_SC-> PCLKSEL0 &= ~(1 << 7);

	//Baudrate config (change times per second of signal)

	//UART0 Line Control Register.
	LPC_UART0-> LCR |=  (3 << 0); 	   //Word Length Select (8 bits).
	LPC_UART0-> LCR &= ~(1 << 2);      //Stop Bit Select (1 stop bit).
	LPC_UART0-> LCR &= ~(1 << 3);      //Parity Enable (Disable).
	LPC_UART0-> LCR &= ~(1 << 7);	   //Divisor Latch Access Bit (DLAB) is Enable (1).

	//UART0 Divisor Latch LSB register.
	LPC_UART0-> DLL = 12;
	LPC_UART0-> DLM = 0;

	//UART0 FIFO Control Register.
	LPC_UART0-> FCR |= (1 << 0);	   //
	LPC_UART0-> FCR |= (1 << 1);
	LPC_UART0-> FCR |= (1 << 2);

	LPC_UART0-> FDR = (14 << 4) | 2;

}

void sendMessage(){
	//Con alternativa de mandar char mediante arreglo.
	while(LPC_UART0-> LSR & 0x20){ //Line Status Register. Ask to register about bit 5 Transmitter Holding Register Empty
		for(int i=0; i < 7; i++){
			LPC_UART0-> THR = arreglo[i];
			//LPC_UART0-> THR = 'A';
		}
	}

}
