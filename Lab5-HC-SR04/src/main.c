#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);

#define TRIG_PIO PIOA
#define TRIG_PIO_ID ID_PIOA
#define TRIG_IDX 4
#define TRIG_IDX_MASK (1 << TRIG_IDX)

#define ECHO_PIO PIOA
#define ECHO_PIO_ID ID_PIOA
#define ECHO_IDX 24
#define ECHO_IDX_MASK (1 << ECHO_IDX)

#define FREQ 1.0 / (2*0.000058)

volatile char flag_echo_rise;
volatile char flag_echo_fall;
volatile char flag_trig;

void echo_callback() {
    if (!pio_get(ECHO_PIO, PIO_INPUT, ECHO_IDX_MASK)) {
        flag_echo_fall = 1;
        flag_echo_rise = 0;
    } else if (pio_get(ECHO_PIO, PIO_INPUT, ECHO_IDX_MASK)) {
        flag_echo_rise = 1;
        flag_echo_fall = 0;
    }
}

void trig_pulse() {
    pio_set(TRIG_PIO, TRIG_IDX_MASK);
    delay_us(10);
    pio_clear(TRIG_PIO, TRIG_IDX_MASK);
}

void init(void) {
    pmc_enable_periph_clk(ECHO_PIO_ID);
    pio_configure(ECHO_PIO, PIO_INPUT, ECHO_IDX_MASK, PIO_DEFAULT);
    pio_handler_set(ECHO_PIO_ID,
                    ECHO_PIO_ID,
                    ECHO_IDX_MASK,
                    PIO_IT_EDGE,
                    echo_callback);
    pio_enable_interrupt(ECHO_PIO, ECHO_IDX_MASK);
    pio_get_interrupt_status(ECHO_PIO);
    NVIC_EnableIRQ(ECHO_PIO_ID);
    NVIC_SetPriority(ECHO_PIO_ID, 4);

    pmc_enable_periph_clk(TRIG_PIO_ID);
    pio_configure(TRIG_PIO, PIO_OUTPUT_1, TRIG_IDX_MASK, PIO_DEFAULT);
}

void RTT_Handler(void) {
    uint32_t ul_status;
    ul_status = rtt_get_status(RTT);
    if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
    }
    if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
    }
}

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {
    uint16_t pllPreScale = (int)(((float)32768) / freqPrescale);
    rtt_sel_source(RTT, false);
    rtt_init(RTT, pllPreScale);
    if (rttIRQSource & RTT_MR_ALMIEN) {
        uint32_t ul_previous_time;
        ul_previous_time = rtt_read_timer_value(RTT);
        while (ul_previous_time == rtt_read_timer_value(RTT));
        rtt_write_alarm_time(RTT, IrqNPulses + ul_previous_time);
    }
    NVIC_DisableIRQ(RTT_IRQn);
    NVIC_ClearPendingIRQ(RTT_IRQn);
    NVIC_SetPriority(RTT_IRQn, 4);
    NVIC_EnableIRQ(RTT_IRQn);
    if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
        rtt_enable_interrupt(RTT, rttIRQSource);
    else
        rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
}

void TC0_Handler(void) {
	volatile uint32_t status = tc_get_status(TC0, 0);
    flag_trig = 1;
}

void TC_init(Tc *TC, int ID_TC, int TC_CHANNEL, int freq) {
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	pmc_enable_periph_clk(ID_TC);

	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	NVIC_SetPriority(ID_TC, 4);
	NVIC_EnableIRQ((IRQn_Type)ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}


int main(void) {
    double temp = 0.0;
    char str[128];
    WDT->WDT_MR = WDT_MR_WDDIS;

    board_init();
    sysclk_init();
    delay_init();
    gfx_mono_ssd1306_init();
    init();
    
	TC_init(TC0, ID_TC0, 0,(int) 1/0.011764);
	tc_start(TC0, 0);

    while (1) {
        if (flag_trig) {
            trig_pulse();
            flag_trig = 0;
        }
		
        if (flag_echo_rise) {
            RTT_init(FREQ, 0, 0);
            flag_echo_rise = 0;
        }
        if (flag_echo_fall){
            int tempo = rtt_read_timer_value(RTT);
			gfx_mono_draw_string("             ", 0, 0, &sysfont);
            sprintf(str, "%.2lf cm", tempo*0.000058*34000);
            gfx_mono_draw_string(str, 0, 0, &sysfont);
			flag_echo_fall = 0;
        }
    }
}
