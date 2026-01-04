#include <hardware/Timers/timer_common.h>
#include <avr/io.h>

static uint8_t timsk0_old = 0;
static uint8_t timsk1_old = 0;
static uint8_t timsk2_old = 0;

void timer_common_freeze() {
    timsk0_old = TIMSK0;
    timsk1_old = TIMSK1;
    timsk2_old = TIMSK2;

    TIMSK0 = 0;
    TIMSK1 = 0;
    TIMSK2 = 0;
}

void timer_common_resume() {
    TIMSK0 = timsk0_old;
    TIMSK1 = timsk1_old;
    TIMSK2 = timsk2_old;
}