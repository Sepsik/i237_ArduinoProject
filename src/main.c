#define __AVR_ATmega328P__ 1
#include <avr/io.h>
#include <util/delay.h>

#define BLINK_DELAY_MS 1000

void blink (int port)
{
    PORTB |= _BV(port);
    _delay_ms(BLINK_DELAY_MS);
    PORTB &= ~_BV(port);
    _delay_ms(BLINK_DELAY_MS);
}

void main (void)
{
    DDRB |= _BV(DDB3);
    DDRB |= _BV(DDB4);
    DDRB |= _BV(DDB5);

    while (1) {
        blink(PORTB3);
        blink(PORTB4);
        blink(PORTB5);
    }
}


