#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "hmi_msg.h"
#include "print_helper.h"
#include "../lib/hd44780_111/hd44780.h"


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
    fprintf_P(stderr, version_info);
    fprintf_P(stderr, avr_info);
}


static inline void init_uart0(void)
{
    simple_uart0_init();
    stdout = stdin = &simple_uart0_io;
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
        blink_leds();
        int input;
        fprintf_P(stdout, enter_input_msg);
        fscanf(stdin, "%s", &input);
        fprintf(stdout, "%s\n", &input);

        if (input > 57 || input < 48) {
            fprintf_P(stdout, not_valid_input_msg);
            display_msg(not_valid_input_LCD_msg);
        } else {
            PGM_P word = (PGM_P)pgm_read_word(&(lookup_list[input - 48]));
            fprintf_P(stdout, result_msg);
            fprintf_P(stdout, word);
            fprintf(stdout, ".\n");
            display_msg(word);
        }
    }
}
