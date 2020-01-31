#include "systemtick.h"

volatile uint32_t system_millis = 0;

void systick_setup(void) {
	/* 1ms interrupt rate */
	systick_set_reload(168000);
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_counter_enable();
	systick_interrupt_enable();
}
