#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

#define BLINK_DELAY_MS 300

void blink (char port)
{
    PORTB |= _BV(port);
    _delay_ms(BLINK_DELAY_MS);
    PORTB &= ~_BV(port);
    _delay_ms(BLINK_DELAY_MS);
}


void main (void)
{
    DDRB |= _BV(DDB2);
    DDRB |= _BV(DDB4);
    DDRB |= _BV(DDB5);
    DDRB |= _BV(DDB7);

    while (1) {
        switch (rand() % 3) {
        case 0:
            blink(PORTB2);
            break;

        case 1:
            blink(PORTB4);
            break;

        case 2:
            blink(PORTB5);
            break;
        }
    }
}