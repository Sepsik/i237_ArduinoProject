#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include "hmi_msg.h"
#include "print_helper.h"
#include "cli_microrl.h"
#include "../lib/hd44780_111/hd44780.h"
#include "../lib/andygock_avr-uart/uart.h"
#include "../lib/helius_microrl/microrl.h"

#define UART_BAUD           9600
#define UART_STATUS_MASK    0x00FF
#define BLINK_DELAY_MS 1000
#define COUNT_SECONDS
#define ASCII_PRINT

volatile uint32_t counter;

microrl_t rl;
microrl_t *prl = &rl;

static inline void init_leds(void)
{
    DDRA |= _BV(DDA0) | _BV(DDA2);
    DDRB |= _BV(DDB7);
    PORTA &= ~(_BV(DDA0) | _BV(DDA2));
}


static inline void init_con_uarts(void)
{
    uart0_init(UART_BAUD_SELECT(UART_BAUD, F_CPU));
    uart1_init(UART_BAUD_SELECT(UART_BAUD, F_CPU));
    uart0_puts_p(my_name);
    uart0_puts_p(PSTR("\r\n"));
    uart0_puts_p(enter_input_msg);
    uart0_puts_p(PSTR("\r\n"));
    uart1_puts_p(version_info);
    uart1_puts_p(avr_info);
    microrl_init(prl, uart0_puts);
    microrl_set_execute_callback(prl, cli_execute);
}


static inline void init_sys_timer(void)
{
    counter = 0;
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B |= _BV(WGM12);
    TCCR1B |= _BV(CS12);
    OCR1A = 62549;
    TIMSK1 |= _BV(OCIE1A);
}


static inline void heartbeat(void)
{
    static uint32_t prev_time;
    uint32_t counter_cpy = 0;
    char ascii_buf[11] = {0x00};
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        counter_cpy = counter;
    }

    if (prev_time != counter_cpy) {
        ltoa(counter_cpy, ascii_buf, 10);
        uart1_puts_p(PSTR("Uptime: "));
        uart1_puts(ascii_buf);
        uart1_puts_p(PSTR(" s.\r\n"));
        PORTA ^= _BV(PORTA2);
        prev_time = counter_cpy;
    }
}


void main(void)
{
    lcd_init();
    lcd_home();
    lcd_puts_P(my_name);
    sei();
    init_leds();
    init_con_uarts();
    init_sys_timer();

    while (1) {
        heartbeat();
        microrl_insert_char(prl, (uart0_getc() & UART_STATUS_MASK));
    }
}


ISR(TIMER1_COMPA_vect)
{
    counter++;
}
