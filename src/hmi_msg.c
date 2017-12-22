#include <avr/pgmspace.h>

const char version_info[] PROGMEM = "Version: " FW_VERSION " built on: "
                                    __DATE__ " " __TIME__ "\n";
const char avr_info[] PROGMEM = "avr-libc version: "
                                __AVR_LIBC_VERSION_STRING__" avr-gcc version: " __VERSION__ "\n";

const char my_name[] PROGMEM = "Anita Sepp";
const char not_valid_input_LCD_msg[] PROGMEM = "Not valid number";
const char enter_input_msg[] PROGMEM = "\nEnter number > ";
const char not_valid_input_msg[] PROGMEM =
    "Please enter number between 0 and 9!\n";
const char result_msg[] PROGMEM = "You entered number ";

const char zero[] PROGMEM = "zero";
const char one[] PROGMEM = "one";
const char two[] PROGMEM = "two";
const char three[] PROGMEM = "three";
const char four[] PROGMEM = "four";
const char five[] PROGMEM = "five";
const char six[] PROGMEM = "six";
const char seven[] PROGMEM = "seven";
const char eight[] PROGMEM = "eight";
const char nine[] PROGMEM = "nine";

PGM_P const lookup_list[] PROGMEM = {
    zero,
    one,
    two,
    three,
    four,
    five,
    six,
    seven,
    eight,
    nine
};
