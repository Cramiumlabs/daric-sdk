#ifndef __TIMER_H__
#define __TIMER_H__
#include "nfc_type.h"

typedef struct {
    uint32_t time;     //current time
    uint32_t timeout;   //timer timeout
} tTIMER_CB;


void timer_wait_ms(uint32_t ms);
void timer_set_timeout(tTIMER_CB *t, uint32_t interval);
uint8_t timer_wait_timeout(tTIMER_CB *t);

#endif
