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
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#endif

#include <stdio.h>

//#include <cr_section_macros.h>

void configPin(void);
void configTIMER0(void);
void configDAC(void);
void configDMA(void);

int main(void) {

	configPin();
	configTIMER0();
	configDAC();

	uint16_t valores[2048];

	for(uint16_t i=0; i < 1025; i++){
		valores.add[i];
	}

	for(uint16_t j=1024; j=0; j--){
		for(uint16_t k= 1025; k < 2049; k++){
			valores.add[k]= j;
		}

	}

    while(1) {
    	GPDMA_ChannelCmd(0, ENABLE);
    }

    return 0 ;
}

void configPin(){

	LPC_PINCON-> PINSEL1  |= (2 << 20); //P0.26 configured like AOUT.
	LPC_PINCON-> PINMODE1 |= (2 << 20); //neither pull down nor pull up.
	LPC_PINCON-> PINMODE_OD0 &= ~(1 << 20); //Hasn't configured open drain option.

}

void configTimer0(){

	LPC_SC-> PCONP |= (1 << 1);
	LPC_SC-> PCLKSEL0 |= (1 << 2);

	LPC_PINCON-> PINSEL3 |= (3 << 24);

	LPC_TIM0-> PR = 0;
	LPC_TIM0-> EMR &= ~(3 << 4);
	LPC_TIM0-> MR0 = 100000000;
	LPC_TIM0-> MCR |= 3;
	LPC_TIM0-> TCR |= 3;
	LPC_TIM0-> TCR &= ~(2);

	NVIC_EnableIRQ(TIMER0_IRQn);

}

void configDAC(){

	DAC_CONVERTER_CFG_Type dacCfg;

	dacCfg.CNT_ENA= SET;
	dacCfg.DMA_ENA= SET;

	DAC_Init(LPC_DAC);

	uint32_t tmp;

	tmp= (100*1000000);
	DAC_SetDMATimeOut(LPC_DAC, tmp);

	DAC_ConfigDAConverterControl(LPC_DAC, &dacCfg);

}

void configDMA(){

	GPDMA_LLI_Type LLI1;
	LLI1.SrcAddr= (uint16_t) &valores;
	LLI1.DstAddr= (uint16_t) LPC_DAC-> DACR;
	LLI1.NextLLI= (uint16_t) &LLI1;
	LLI1.Control= 2048 | (1 << 18) | (1 << 21) | (1 << 26);

	GPDMA_Init();

	GPDMA_Channel_CFG_Type GPDMACfg;
	GPDMACfg.ChannelNum = 0;
	GPDMACfg.SrcMemAddr = (uint16_t)valores;
	GPDMACfg.DstMemAddr = 0;
	GPDMACfg.TransferSize = 2048;
	GPDMACfg.TransferWidth = 0;
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
	GPDMACfg.SrcConn = 0;
	GPDMACfg.DstConn = GPDMA_CONN_DAC;
	GPDMACfg.DMALLI = (uint16_t)&LLI1;
	GPDMA_Setup(&GPDMACfg);

}
