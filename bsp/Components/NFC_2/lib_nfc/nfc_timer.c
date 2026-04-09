#include "nfc_timer.h"
#include "port.h"

void timer_wait_ms(uint32_t ms)
{
	port_timer_delay_ms(ms);
}

void timer_set_timeout(tTIMER_CB *t, uint32_t timeout)
{
	t->timeout = timeout;
	t->time	   = 0;
}

uint8_t timer_wait_timeout(tTIMER_CB *t)
{
	if (t->time >= t->timeout)
	{ //定时时间到
		return 1;
	}
	else
	{ //定时未到
		// debug_io(20);
		timer_wait_ms(5);
		t->time += 5;
		return 0;
	}
}
