#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

typedef struct {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} calendar;

volatile char flag_rtc_alarm = 0;
volatile char rtc_time_flag = 0;

#define LED_PIO PIOC
#define LED_PIO_ID ID_PIOC
#define LED_IDX 8u
#define LED_IDX_MASK (1u << LED_IDX)

#define LED1_PIO PIOA
#define LED1_PIO_ID ID_PIOA
#define LED1_IDX 0
#define LED1_IDX_MASK (1 << LED1_IDX)

#define LED2_PIO PIOC
#define LED2_PIO_ID ID_PIOC
#define LED2_IDX 30
#define LED2_IDX_MASK (1 << LED2_IDX)

void pin_toggle(Pio *pio, uint32_t mask);
void io_init(void);

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);
void TC_init(Tc *TC, int ID_TC, int TC_CHANNEL, int freq);
void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type);

void pin_toggle(Pio *pio, uint32_t mask) {
	if (pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio, mask);
}

void io_init(void) {
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_1, LED_IDX_MASK, PIO_DEFAULT);

	pmc_enable_periph_clk(LED1_PIO_ID);
	pio_configure(LED1_PIO, PIO_OUTPUT_1, LED1_IDX_MASK, PIO_DEFAULT);

	pmc_enable_periph_clk(LED2_PIO_ID);
	pio_configure(LED2_PIO, PIO_OUTPUT_1, LED2_IDX_MASK, PIO_DEFAULT);

}

void TC0_Handler(void) {
	volatile uint32_t status = tc_get_status(TC0, 0);
	pin_toggle(LED_PIO, LED_IDX_MASK);
}

void TC3_Handler(void) {
	volatile uint32_t status = tc_get_status(TC1, 0);
	pin_toggle(LED2_PIO, LED2_IDX_MASK);
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

void RTT_Handler(void) {
	uint32_t ul_status;

	ul_status = rtt_get_status(RTT);

	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		RTT_init(4, 0, RTT_MR_RTTINCIEN);
	}

	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		pin_toggle(LED1_PIO, LED1_IDX_MASK);
	}
}

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int)(((float)32768) / freqPrescale);

	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);

	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT))
		;
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

void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);

	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		rtc_time_flag= 1;
	}

	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		flag_rtc_alarm = 1;
	}

	rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}

void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type) {
	pmc_enable_periph_clk(ID_RTC);

	rtc_set_hour_mode(rtc, 0);

	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.second);

	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 4);
	NVIC_EnableIRQ(id_rtc);

	rtc_enable_interrupt(rtc, irq_type);
}

int main(void) {
	sysclk_init();
	WDT->WDT_MR = WDT_MR_WDDIS;
	io_init();
	board_init();
	gfx_mono_ssd1306_init();
	char str[128];


	TC_init(TC0, ID_TC0, 0, 4);
	tc_start(TC0, 0);
	TC_init(TC1, ID_TC3, 0, 5);
	
	RTT_init(4, 16, RTT_MR_ALMIEN);

	calendar rtc_initial = {2022, 3, 18, 3, 11, 27, 0};
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_ALREN | RTC_IER_SECEN);
	uint32_t current_hour, current_min, current_sec;
	uint32_t current_year, current_month, current_day, current_week;
	rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
	rtc_get_date(RTC, &current_year, &current_month, &current_day, &current_week);
	
	rtc_set_date_alarm(RTC, 1, current_month, 1, current_day);
	rtc_set_time_alarm(RTC, 1, current_hour, 1, current_min, 1, current_sec + 5);
	
	while (1) {
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
		if (rtc_time_flag) {
			rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
			sprintf(str, "%d:%d:%d", current_hour, current_min, current_sec);
			gfx_mono_draw_string(str, 0,0, &sysfont);
			rtc_time_flag = 0;
		}
		
		if (flag_rtc_alarm) {
			tc_start(TC1, 0);
			flag_rtc_alarm = 0;
		}
	}
}