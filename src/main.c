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
    fprintf(stderr, "Version: %s built on: %s %s\n",
            FW_VERSION, __DATE__, __TIME__);
    fprintf(stderr, "avr-libc version: %s avr-gcc version: %s\n",
            __AVR_LIBC_VERSION_STRING__, __VERSION__);
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

void display_msg(char* msg)
{
    lcd_clr(64, 16);
    lcd_goto(LCD_ROW_2_START);
    lcd_puts(msg);
}


void main (void)
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

    fprintf(stdout, "%s\n", my_name);
    print_ascii_tbl(stdout);
    print_for_human(stdout, char_array, sizeof(char_array));
    lcd_puts(my_name);

    while (1) {
        blink_leds();
        int input;
        fprintf(stdout, "\nEnter number > ");
        fscanf(stdin, "%s", &input);
        fprintf(stdout, "%s\n", &input);

        if (input > 57 || input < 48) {
            char* msg = "Please enter number between 0 and 9!";
            fprintf(stdout, "%s\n", msg);
            display_msg(not_valid_input_msg);
        } else {
            char* word = lookup_list[input - 48];
            fprintf(stdout, "You entered number %s.\n", word);
            display_msg(word);
        }
    }
}
