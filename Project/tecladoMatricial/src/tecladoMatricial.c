#include "lpc17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"

/********************************************DEFINE1*****************************************************/

#define ROWs	((4))			// four rows
#define COLs	((4))			// four columns

/******************************************END DEFINE1***************************************************/

/********************************************DEFINE******************************************************/
#define INPUT		  (uint8_t)		0
#define OUTPUT	      (uint8_t)		1
#define PINMOTOR	  (uint32_t)	(0x78000)		// Pines P0.15 a P0.18
#define KEYPAD_ROWS	  (uint16_t)    (0x0F)			// Pines P2.0  a P2.3
#define KEYPAD_COLUMS (uint16_t)    (0xF0)			// Pines P2.4  a P2.7
#define RISING_EDGE   (uint8_t)		0
#define FALLING_EDGE (uint8_t)		1
#define LIMPIOLETRA	 (char) '0'
#define TECLA_1 (char)		'1'
#define TECLA_2 (char)		'2'
#define TECLA_4 (char)		'4'
#define TECLA_5 (char)		'5'
#define TECLA_A	(char)			'A'
#define TECLA_B	(char)			'B'
#define TECLA_C	(char)			'C'
#define TECLA_D	(char)			'D'
#define TECLA_ASTERISK	(char)	'*'
#define TECLA_NUMERAL	(char)	'#'

int  ANTIREBOTE = 750000;
char keys[ROWs][COLs] = {{ '1','2','3','A' },
					     { '4','5','6','B' },
					     { '7','8','9','C' },
					     { '*','0','#','D' }};
char keyPush;
/****************************************FINAL DEFINE****************************************************/

void configPin(void);
void configGPIO(void);
void delay(uint32_t times);
void switchTeclas(void);

int main(){

	configPin();
	configGPIO();

	while(1){
		GPIO_SetValue(0, (1 << 22));
	}

	return 1;
}

void configPin(){

	PINSEL_CFG_Type Pin;
	Pin.Funcnum = PINSEL_FUNC_0;
	Pin.Portnum = PINSEL_PORT_2;
	Pin.Pinmode = PINSEL_PINMODE_PULLUP;
	Pin.OpenDrain = PINSEL_PINMODE_NORMAL;
	for(int i = 0; i <= 7; i++){
		Pin.Pinnum =  PINSEL_PIN_0 + i;
		PINSEL_ConfigPin(&Pin);
	}

	Pin.Funcnum = PINSEL_FUNC_0;
	Pin.Portnum = PINSEL_PORT_0;
	Pin.Pinnum = PINSEL_PIN_22;
	PINSEL_ConfigPin(&Pin);

}

void configGPIO(){

	// Configuracion GPIO de teclado matricial
	GPIO_SetDir(PINSEL_PORT_2, KEYPAD_COLUMS, INPUT);		// P0.4-7 para columnas del Keypad.
	GPIO_SetDir(PINSEL_PORT_2, KEYPAD_ROWS, OUTPUT);	 	// P0.0-3 para filas del Keypad.
	GPIO_ClearValue(PINSEL_PORT_2, KEYPAD_ROWS);			// Saca 0 por las filas del keypad.
	GPIO_IntCmd(PINSEL_PORT_2, KEYPAD_COLUMS, FALLING_EDGE);// Habilita int. por GPIO por flanco de bajada.
	GPIO_ClearInt(PINSEL_PORT_2, KEYPAD_COLUMS);			// Limpia banderas int. por GPIO.

	// Configuracion GPIO de LED indicador de Modo ADC
	GPIO_SetDir(0, (1 << 22), OUTPUT);
	GPIO_SetValue(0, (1 << 22));


	// Configuracion del NVIC para prioridades e interrupciones
	NVIC_SetPriority(EINT3_IRQn, 0);					 	// More priority than ADC and TIMER.
	NVIC_EnableIRQ(EINT3_IRQn);							// Interrupt enabled.
	return;

}

void EINT3_IRQHandler(){




	LPC_GPIO0-> FIOSET |= (1 << 22);
	delay(1000);
	LPC_GPIO0-> FIOCLR |= (1 << 22);
	delay(1000);
	LPC_GPIO0-> FIOSET |= (1 << 22);
	delay(1000);
	LPC_GPIO0-> FIOCLR |= (1 << 22);
	delay(1000);

	LPC_GPIOINT-> IO2IntClr |= (0xFF << 2);
	//GPIO_ClearInt(PINSEL_PORT_2, KEYPAD_COLUMS);
}

void delay(uint32_t times){

	for(uint32_t i=0; i < times; i++){
		for(uint32_t i=0; i < times; i++){
		}
	}

}
