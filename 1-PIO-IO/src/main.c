/************************************************************************
* 5 semestre - Eng. da Computao - Insper
* Rafael Corsi - rafael.corsi@insper.edu.br
*
* Material:
*  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
*
* Objetivo:
*  - Demonstrar configuraçao do PIO
*
* Periféricos:
*  - PIO
*  - PMC
*
* Log:
*  - 10/2018: Criação
************************************************************************/

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LEDs
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

#define LED1_PIO      PIOA
#define LED1_PIO_ID   ID_PIOA
#define LED1_IDX      0
#define LED1_IDX_MASK (1 << LED1_IDX)

#define LED2_PIO      PIOC
#define LED2_PIO_ID   ID_PIOC
#define LED2_IDX      30
#define LED2_IDX_MASK (1 << LED2_IDX)

#define LED3_PIO      PIOB
#define LED3_PIO_ID   ID_PIOB
#define LED3_IDX      2
#define LED3_IDX_MASK (1 << LED3_IDX)

// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX      11
#define BUT_IDX_MASK (1 << BUT_IDX)

#define BUT1_PIO      PIOD
#define BUT1_PIO_ID   ID_PIOD
#define BUT1_IDX      28
#define BUT1_IDX_MASK (1 << BUT1_IDX)

#define BUT2_PIO      PIOC
#define BUT2_PIO_ID   ID_PIOC
#define BUT2_IDX      31
#define BUT2_IDX_MASK (1 << BUT2_IDX)

#define BUT3_PIO      PIOA
#define BUT3_PIO_ID   ID_PIOA
#define BUT3_IDX      19
#define BUT3_IDX_MASK (1 << BUT3_IDX)

/* Funcao principal chamada na inicalizacao do uC.                      */
int main(void)
{
	// Inicializa clock
	sysclk_init();

	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

	// Ativa PIOs
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(LED1_PIO_ID);
	pmc_enable_periph_clk(LED2_PIO_ID);
	pmc_enable_periph_clk(LED3_PIO_ID);
	
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);

	// Configura Pinos
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);
	
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_IDX_MASK, PIO_DEFAULT);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK, PIO_PULLUP);
	
	pio_configure(LED2_PIO, PIO_OUTPUT_0, LED2_IDX_MASK, PIO_DEFAULT);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK, PIO_PULLUP);

	pio_configure(LED3_PIO, PIO_OUTPUT_0, LED3_IDX_MASK, PIO_DEFAULT);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK, PIO_PULLUP);
	
	// SUPER LOOP
	while(1) {
		// Verifica valor do pino que o botão está conectado
		if(!pio_get(BUT_PIO, PIO_INPUT, BUT_IDX_MASK)) {
			// Pisca LED
			for (int i=0; i<10; i++) {
				pio_clear(LED_PIO, LED_IDX_MASK);  // Limpa o pino LED_PIO_PIN
				delay_ms(100);                         // delay
				pio_set(LED_PIO, LED_IDX_MASK);    // Ativa o pino LED_PIO_PIN
				delay_ms(100);                         // delay
			}
			} else  {
			// Ativa o pino LED_IDX (par apagar)
			pio_set(LED_PIO, LED_IDX_MASK);
		}
		if(!pio_get(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK)) {
			for (int i=0; i<10; i++) {
				pio_clear(LED1_PIO, LED1_IDX_MASK);  
				delay_ms(100);                         
				pio_set(LED1_PIO, LED1_IDX_MASK);  
				delay_ms(100);                     
			}
			} else  {
			pio_set(LED1_PIO, LED1_IDX_MASK);
		}
		if(!pio_get(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK)) {
			for (int i=0; i<10; i++) {
				pio_clear(LED2_PIO, LED2_IDX_MASK);  
				delay_ms(100);
				pio_set(LED2_PIO, LED2_IDX_MASK);
				delay_ms(100);                         
			}
			} else  {
			pio_set(LED2_PIO, LED2_IDX_MASK);
		}
		if(!pio_get(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK)) {
			// Pisca LED
			for (int i=0; i<10; i++) {
				pio_clear(LED3_PIO, LED3_IDX_MASK);  
				delay_ms(100);                        
				pio_set(LED3_PIO, LED3_IDX_MASK);    
				delay_ms(100);                        
			}
			} else  {
			pio_set(LED3_PIO, LED3_IDX_MASK);
		}
	}
	// Nunca devemos chegar aqui !
	return 0;
}