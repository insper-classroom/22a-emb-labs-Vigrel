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

#define BUT1_PIO PIOD
#define BUT1_PIO_ID ID_PIOD
#define BUT1_PIO_IDX 28
#define BUT1_IDX_MASK (1 << BUT1_PIO_IDX)

#define LED_PIO PIOC
#define LED_PIO_ID ID_PIOC
#define LED_IDX 8
#define LED_IDX_MASK (1u << LED_IDX)

void pisca_led() {
    pio_clear(LED_PIO, LED_IDX_MASK);
    delay_ms(1000);
    pio_set(LED_PIO, LED_IDX_MASK);
    delay_ms(1000);
}

#define FREQ 1.0 / (2*0.000058)

volatile char flag_echo_rise;
volatile char flag_echo_fall;
volatile char flag_butt;
volatile char flag_draw;

void echo_callback() {
    if (!pio_get(ECHO_PIO, PIO_INPUT, ECHO_IDX_MASK)) {
        flag_echo_fall = 1;
        flag_echo_rise = 0;
    } else if (pio_get(ECHO_PIO, PIO_INPUT, ECHO_IDX_MASK)) {
        flag_echo_rise = 1;
        flag_echo_fall = 0;
    }
}

void but_callback() {
        flag_butt = 1;
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

    pmc_enable_periph_clk(BUT1_PIO_ID);
    pio_configure(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
    pio_set_debounce_filter(BUT1_PIO, BUT1_IDX_MASK, 60);
    pio_handler_set(BUT1_PIO,
                    BUT1_PIO_ID,
                    BUT1_IDX_MASK,
                    PIO_IT_RISE_EDGE,
                    but_callback);
    pio_enable_interrupt(BUT1_PIO, BUT1_IDX_MASK);
    pio_get_interrupt_status(BUT1_PIO);
    NVIC_EnableIRQ(BUT1_PIO_ID);
    NVIC_SetPriority(BUT1_PIO_ID, 4);

    pmc_enable_periph_clk(TRIG_PIO_ID);
    pio_configure(TRIG_PIO, PIO_OUTPUT_1, TRIG_IDX_MASK, PIO_DEFAULT);

    pmc_enable_periph_clk(LED_PIO_ID);
    pio_configure(LED_PIO, PIO_OUTPUT_1, LED_IDX_MASK, PIO_DEFAULT);
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


int main(void) {
    double temp = 0.0;
    char str[128];
    WDT->WDT_MR = WDT_MR_WDDIS;

    board_init();
    sysclk_init();
    delay_init();
    gfx_mono_ssd1306_init();
    init();
    
    while (1) {
        if (flag_butt) {
            flag_butt = 0;
            pisca_led();
            trig_pulse();
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
