#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "print_helper.h"
#include "rfid_helper.h"
#include "../lib/andygock_avr-uart/uart.h"
#include "../lib/matejx_avr_lib/mfrc522.h"
#include "../lib/hd44780_111/hd44780.h"
#include <avr/wdt.h>

#define DOOR_ACCESS_DENIED "Access denied"
#define LED_RED PORTA0

extern uint32_t counter;

typedef unsigned int id_t;

typedef struct card {
    char *name;
    uint8_t *uid;
    int size;
} card_t;

typedef struct card_list {
    id_t card_id;
    card_t *card;
    struct card_list *next;
} card_list_t;

typedef enum {
    door_opening,
    door_open,
    door_closing,
    door_closed
} door_state_t;

typedef enum {
    display_name,
    display_access_denied,
    display_clear,
    clear_name,
    display_no_update,
} display_state_t;


card_list_t *list_head_ptr;
card_t *card_ptr;
char print_buf[256] = {0x00};
char lcd_buf[16] = {0x00};
char *display_name_str;
uint8_t *saved_uid;
door_state_t door_state = door_closed;
display_state_t display_state = display_no_update;
uint32_t door_closing_time_helper, repeating_card_time_helper,
         msg_display_time_helper, name_display_time_helper;


id_t get_id(void)
{
    static id_t card_u_id = 0;
    return card_u_id++;
}

typedef struct {
    byte        size;
    byte        uidByte[10];
    byte        sak;
    byte        bufferATQA[10];
} current_uid;


card_list_t *create_list(card_t *card)
{
    card_list_t *list_head_ptr = malloc(sizeof(card_list_t));

    if (list_head_ptr == NULL) {
        uart0_puts_p(PSTR("Memory operation failed\r\n"));
        exit(1);
    }

    list_head_ptr->card_id = get_id();
    list_head_ptr->card = card;
    list_head_ptr->next = NULL;
    return list_head_ptr;
}


card_t *create_card(const char *name, const char *input_uid, const int length)
{
    card_ptr = malloc(sizeof(card_t));

    if (card_ptr == NULL) {
        uart0_puts_p(PSTR("Memory operation failed\r\n"));
        exit(1);
    }

    uint8_t *bytes = malloc((length / 2) * sizeof(uint8_t));

    if (bytes == NULL) {
        uart0_puts_p(PSTR("Memory operation failed\r\n"));
        free(bytes);
        exit(1);
    }

    tallymarker_hextobin(input_uid, bytes, length);
    card_list_t *current = list_head_ptr;

    while (current != NULL) {
        if (memcmp(current->card->uid, bytes, current->card->size) == 0) {
            uart0_puts_p(PSTR("This card is already in use.\r\n"));
            return 0;
        }

        current = current->next;
    }

    card_ptr->name = malloc(strlen(name) + 1);

    if (card_ptr->name == NULL) {
        uart0_puts_p(PSTR("Memory operation failed\r\n"));
        free(card_ptr);
        exit(1);
    }

    strcpy(card_ptr->name, name);
    card_ptr->size = length / 2;
    card_ptr->uid = bytes;
    uart0_puts_p(PSTR("Card added.\r\nCard UID: "));
    print_bytes(card_ptr->uid, card_ptr->size);
    uart0_puts_p(PSTR("  Card UID size: "));
    sprintf_P(print_buf, PSTR("%i"), card_ptr->size);
    uart0_puts(print_buf);
    uart0_puts_p(PSTR("\r\n"));
    uart0_puts_p(PSTR("Card holder name: "));
    uart0_puts(card_ptr->name);
    uart0_puts_p(PSTR("\r\n"));
    return card_ptr;
}


void push_card(card_list_t *list_head_ptr, card_t *card)
{
    card_list_t *current = list_head_ptr;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = malloc(sizeof(card_list_t));

    if (current->next == NULL) {
        printf("Memory operation failed\n");
        exit(1);
    }

    current->next->card_id = get_id();
    current->next->card = card;
    current->next->next = NULL;
}

void remove_card(const char *input_uid, card_list_t **list_head_ptr)
{
    card_list_t *current = *list_head_ptr;
    card_list_t *prev = NULL;
    uint8_t *bytes = malloc((strlen(input_uid) / 2) * sizeof(uint8_t));
    tallymarker_hextobin(input_uid, bytes, strlen(input_uid));

    while (current != NULL) {
        if (memcmp(current->card->uid, bytes, current->card->size) == 0) {
            if (prev == NULL) {
                *list_head_ptr = current->next;
            } else {
                prev->next = current->next;
            }

            uart0_puts_p(PSTR("Card with UID: "));
            uart0_puts(input_uid);
            uart0_puts_p(PSTR(" is removed."));
            uart0_puts_p(PSTR("\r\n"));
            free(current->card->name);
            free(current->card->uid);
            free(current);
            return;
        }

        prev = current;
        current = current->next;
    }

    uart0_puts_p(PSTR("Card with UID: "));
    uart0_puts(input_uid);
    uart0_puts_p(PSTR(" is not in list."));
}


void rfid_card_read(const char *const *argv)
{
    (void) argv;
    Uid uid;
    byte bufferSize = sizeof(uid.bufferATQA);;
    uart0_puts_p(PSTR("\r\n"));

    if (PICC_WakeupA(uid.bufferATQA, &bufferSize) != STATUS_OK) {
        if (PICC_IsNewCardPresent()) {
            uart0_puts_p(PSTR("Card selected!\r\n"));
            PICC_ReadCardSerial(&uid);
            uart0_puts_p(PSTR("Card type: "));
            uart0_puts(PICC_GetTypeName(PICC_GetType(uid.sak)));
            uart0_puts_p(PSTR("\r\n"));
            uart0_puts_p(PSTR("Card UID: "));
            print_bytes(uid.uidByte, uid.size);
            uart0_puts_p(PSTR("  Card UID size: "));
            print_bytes(&uid.size, 1);
            uart0_puts_p(PSTR("\r\n"));
        } else {
            uart0_puts_p((PSTR("Unable to select card.\r\n")));
        }
    }
}


void rfid_card_add(const char *const *argv)
{
    (void) argv;
    int input_uid_length = strlen(argv[2]);

    if (input_uid_length > 20) {
        uart0_puts_p(PSTR("Card has to be max 10 bytes.\r\n"));
        return;
    }

    card_ptr = create_card(argv[1], argv[2], input_uid_length);

    if (list_head_ptr == 0 && card_ptr != 0) {
        list_head_ptr = create_list(card_ptr);
    } else if (card_ptr != 0) {
        push_card(list_head_ptr, card_ptr);
    } else {
        uart0_puts_p(PSTR("Adding card failed.\r\n"));
    }
}

void rfid_card_remove(const char *const *argv)
{
    if (list_head_ptr == 0) {
        uart0_puts_p(PSTR("There are no cards in the list."));
        uart0_puts_p(PSTR("\r\n"));
    } else {
        remove_card(argv[1], &list_head_ptr);
    }
}


void rfid_card_print_list(const char *const *argv)
{
    (void) argv;
    card_list_t *current = list_head_ptr;

    if (current == NULL) {
        uart0_puts_p(PSTR("There are no cards in the list."));
        uart0_puts_p(PSTR("\r\n"));
    } else {
        while (current != NULL) {
            uart0_puts_p(PSTR("Card No. "));
            sprintf_P(print_buf, PSTR("%i"), current->card_id);
            uart0_puts(print_buf);
            uart0_puts_p(PSTR("\r\n"));
            uart0_puts_p(PSTR("Card UID: "));
            print_bytes(current->card->uid, current->card->size);
            uart0_puts_p(PSTR("  Card UID size: "));
            sprintf_P(print_buf, PSTR("%i"), current->card->size);
            uart0_puts(print_buf);
            uart0_puts_p(PSTR("\r\n"));
            uart0_puts_p(PSTR("Card holder name: "));
            uart0_puts(current->card->name);
            uart0_puts_p(PSTR("\r\n\r\n"));
            current = current->next;
        }
    }
}


char *get_cardholder_name(Uid uid)
{
    card_list_t *current = list_head_ptr;

    while (current != NULL) {
        if (memcmp(uid.uidByte, current->card->uid, uid.size) == 0) {
            return current->card->name;
        }

        current = current->next;
    }

    return NULL;
}


bool is_repeating_card(Uid uid)
{
    return saved_uid && memcmp(uid.uidByte, saved_uid, uid.size) == 0;
}


void remember_uid (Uid uid)
{
    saved_uid = realloc(saved_uid, sizeof(uid.uidByte));
    memcpy(saved_uid, uid.uidByte, sizeof(uid.uidByte));
    repeating_card_time_helper = counter;
}


void rfid_process_card(void)
{
    Uid uid;

    if (PICC_IsNewCardPresent()) {
        PICC_ReadCardSerial(&uid);

        if (!is_repeating_card(uid)) {
            remember_uid(uid);

            if (get_cardholder_name(uid) != NULL) {
                display_name_str = get_cardholder_name(uid);
                door_state = door_opening;
                display_state = display_name;
            } else {
                door_state = door_closing;
                display_state = display_access_denied;
            }
        }
    }

    if (repeating_card_time_helper && (counter - repeating_card_time_helper >= 3)) {
        free(saved_uid);
        saved_uid = NULL;
        repeating_card_time_helper = 0;
    }

    switch (door_state) {
    case door_opening:
        PORTA ^= _BV(LED_RED);
        door_closing_time_helper = counter;
        door_state = door_open;
        break;

    case door_open:
        if (counter - door_closing_time_helper >= 2) {
            door_state = door_closing;
        }

        break;

    case door_closing:
        door_state = door_closed;
        break;

    case door_closed:
        PORTA &= ~_BV(LED_RED);
        break;
    }

    switch (display_state) {
    case display_name:
        lcd_clr(LCD_ROW_2_START, LCD_VISIBLE_COLS);
        lcd_goto(LCD_ROW_2_START);

        if (display_name_str != NULL) {
            name_display_time_helper = counter;
            strncpy(lcd_buf, display_name_str, LCD_VISIBLE_COLS);
            lcd_puts(lcd_buf);
        } else {
            msg_display_time_helper = counter;
            lcd_puts_P(PSTR("Name read error"));
        }

        display_state = clear_name;
        display_state = display_clear;
        break;

    case display_access_denied:
        msg_display_time_helper = counter;
        lcd_clr(LCD_ROW_2_START, LCD_VISIBLE_COLS);
        lcd_goto(LCD_ROW_2_START);
        lcd_puts_P(PSTR(DOOR_ACCESS_DENIED));
        display_state = display_clear;
        break;

    case display_clear:
        if (msg_display_time_helper && (counter - msg_display_time_helper >= 5)) {
            lcd_clr(LCD_ROW_2_START, LCD_VISIBLE_COLS);
            display_state = display_no_update;
            msg_display_time_helper = 0;
        }

        break;

    case clear_name:
        while (counter - name_display_time_helper < 3) {
        }

        lcd_clr(LCD_ROW_2_START, LCD_VISIBLE_COLS);
        display_state = display_no_update;
        name_display_time_helper = 0;
        break;

    case display_no_update:
        break;
    }
}
