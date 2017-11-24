#include <stdio.h>
#define __ASSERT_USE_STDERR
#include <assert.h>
#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"

#define BLINK_DELAY_MS 300


static inline void init_leds(void)
{
    DDRB |= _BV(DDB0);
    DDRB |= _BV(DDB1);
    DDRB |= _BV(DDB2);
    DDRB |= _BV(DDB5);
    DDRB |= _BV(DDB7);
}


/* Init error console as stderr in UART1 and print user code info */
static inline void init_errcon(void)
{
    simple_uart1_init();
    stderr = &simple_uart1_out;
    fprintf(stderr, "Version: %s built on: %s %s\n",
            FW_VERSION, __DATE__, __TIME__);
    fprintf(stderr, "avr-libc version: %s avr-gcc version: %s\n",
            __AVR_LIBC_VERSION_STRING__, __VERSION__);
}


static inline void blink_led(char port)
{
    PORTB |= _BV(port);
    _delay_ms(BLINK_DELAY_MS);
    PORTB &= ~_BV(port);
    _delay_ms(BLINK_DELAY_MS);
}


static inline void blink_leds(void)
{
    switch (rand() % 3) {
    case 0:
        blink_led(PORTB0);
        break;

    case 1:
        blink_led(PORTB1);
        break;

    case 2:
        blink_led(PORTB2);
        break;
    }
}


void main (void)
{
    init_leds();
    init_errcon();

    while (1) {
        blink_leds();
    }
}
