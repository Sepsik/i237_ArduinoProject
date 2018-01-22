#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "print_helper.h"
#include "rfid_helper.h"
#include "../lib/andygock_avr-uart/uart.h"
#include "../lib/matejx_avr_lib/mfrc522.h"


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

card_list_t *list_head_ptr;
card_t *card_ptr;
char print_buf[256] = {0x00};


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

void remove_card(const char *input_uid, card_list_t **list_head_ptr) {
    card_list_t *current = *list_head_ptr;
    card_list_t *prev = NULL;

    uint8_t *bytes = malloc((strlen(input_uid)/2) * sizeof(uint8_t));
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
    }
    else {
        remove_card(argv[1], &list_head_ptr);
    }
}


void rfid_card_print_list(const char *const *argv)
{
    (void) argv;
    card_list_t *current = list_head_ptr;

    if (current == 0) {
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
