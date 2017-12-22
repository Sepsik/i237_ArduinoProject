#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
//#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include "uart.h"
#include "hmi_msg.h"
#include "print_helper.h"
#include "../lib/hd44780_111/hd44780.h"
#include "../lib/andygock_avr-uart/uart.h"


#define BLINK_DELAY_MS 1000
#define COUNT_SECONDS
#define ASCII_PRINT


volatile uint32_t counter;

static inline void init_leds(void)
{
    DDRA |= _BV(DDA0) | _BV(DDA2);
    DDRB |= _BV(DDB7);
    PORTA &= ~(_BV(DDA0) | _BV(DDA2));
}


/* Init error console as stderr in UART1 and print user code info */
static inline void init_errcon(void)
{
    simple_uart1_init();
    stderr = &simple_uart1_out;
    fprintf_P(stderr, version_info);
    fprintf_P(stderr, avr_info);
}


static inline void init_uart0(void)
{
    simple_uart0_init();
    stdout = stdin = &simple_uart0_io;
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


static inline void blink_led(char port)
{
    PORTA ^= _BV(port);
    _delay_ms(BLINK_DELAY_MS);
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


void display_msg(PGM_P msg)
{
    lcd_clr(64, 16);
    lcd_goto(LCD_ROW_2_START);
    lcd_puts_P(msg);
}


void main(void)
{
    init_leds();
    init_errcon();
    init_uart0();
    init_sys_timer();
    simple_uart1_init();
    sei();
    lcd_init();
    lcd_home();
    unsigned char char_array[128];

    for (unsigned char i = 0; i < sizeof(char_array); i++) {
        char_array[i] = i;
    }

    fprintf_P(stdout, my_name);
    fprintf(stdout, "\n");
    print_ascii_tbl(stdout);
    print_for_human(stdout, char_array, sizeof(char_array));
    lcd_puts_P(my_name);

    while (1) {
        blink_led(PORTA0);
        heartbeat();
        // int input;
        // fprintf_P(stdout, enter_input_msg);
        // fscanf(stdin, "%s", &input);
        // fprintf(stdout, "%s\n", &input);
        // if (input > 57 || input < 48) {
        //     fprintf_P(stdout, not_valid_input_msg);
        //     display_msg(not_valid_input_LCD_msg);
        // } else {
        //     PGM_P word = (PGM_P)pgm_read_word(&(lookup_list[input - 48]));
        //     fprintf_P(stdout, result_msg);
        //     fprintf_P(stdout, word);
        //     fprintf(stdout, ".\n");
        //     display_msg(word);
        // }
    }
}


ISR(TIMER1_COMPA_vect)
{
    counter++;
}
