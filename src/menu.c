#include "stm32h7xx.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "usart.h"
#include "menu.h"
#include "can.h"

char* Menu_Commands_Text[MENU_NUM_ITEMS] = {
    "", // strtol interprets the empty string as a 0
    "Set ARBID",
    "Show ARBIDS",
    "Set buadrate",
    "Choose Attack",
};

static void handle_command();
static void setArbids();
static void showArbids();
static void setBaudrate();
static void chooseAttack();

void display_menu()
{
    write_string("? - Help\r\n");
    for(int i = 1; i < MENU_NUM_ITEMS; i++)
    {
        printf("%d - %s\r\n", i, Menu_Commands_Text[i]);
    }
    write_string("\r\nCANT>");
}

void process_menu()
{
    /* Wait to receive entire command */
    if((rx_counter > 0) && (rx_buffer[rx_counter - 1] == '\r'))
    {
        rx_buffer[rx_counter - 1] = '\0';
        if(rx_buffer[0] == '?')
        {
            /* Disable the interrupt, reset the rx_counter */
            CLEAR_BIT(USART3->CR1, USART_CR1_RXNEIE);
            rx_counter = 0;
            SET_BIT(USART3->CR1, USART_CR1_RXNEIE);
            display_menu();
        }
        else
            handle_command();
    }
}

static void handle_command()
{
    char *endptr;
    long int command_num = strtol((char *)rx_buffer, &endptr, 0);

    /* Disable the interrupt, reset the rx_counter */
    CLEAR_BIT(USART3->CR1, USART_CR1_RXNEIE);
    rx_counter = 0;
    SET_BIT(USART3->CR1, USART_CR1_RXNEIE);

    switch(command_num)
    {
        case MENU_UNUSED:
            break;
        case MENU_SET_ARBID:
            setArbids();
            break;
        case MENU_SHOW_ARBID:
            showArbids();
            break;
        case MENU_SET_BAUD:
            setBaudrate();
            break;
        case MENU_CHOOSE_ATTACK:
            chooseAttack();
            break;
        default:
            write_string("No such command\r\n");
            break;
        
    }
    write_string("CANT>");
}

/**
 * Sets the arbids to act on
 */
static void setArbids(void)
{
    write_string("Enter arbid: 0x");

    /* Wait for enter key to be pressed */
    while((rx_counter == 0) || (rx_buffer[rx_counter - 1] != '\r'));
    rx_buffer[rx_counter - 1] = '\0';

    /* Get the entered value and convert to integer */
    attack_arbid = strtol((char*)rx_buffer, NULL, 16);
    printf("Attacking 0x%lx\r\n", attack_arbid);

    /* Disable the interrupt, reset the rx_counter */
    CLEAR_BIT(USART3->CR1, USART_CR1_RXNEIE);
    rx_counter = 0;
    SET_BIT(USART3->CR1, USART_CR1_RXNEIE);
}

/**
 * Shows the arbids to act on
 */
static void showArbids(void)
{
    printf("Currently attacking 0x%lx\r\n", attack_arbid);
}

/**
 * Sets the CAN baud rate
 */
static void setBaudrate(void)
{
    long int baud;

    write_string("Enter baudrate in BPS: ");

    /* Wait for enter key to be pressed */
    while((rx_counter == 0) || (rx_buffer[rx_counter - 1] != '\r'));
    rx_buffer[rx_counter - 1] = '\0';

    /* Get the entered value and convert to integer */
    baud = strtol((char*)rx_buffer, NULL, 0);

    /* Disable the interrupt, reset the rx_counter */
    CLEAR_BIT(USART3->CR1, USART_CR1_RXNEIE);
    rx_counter = 0;
    SET_BIT(USART3->CR1, USART_CR1_RXNEIE);

    /* Start the CAN sync */
    setCanBaudrate(baud);
    printf("Baud rate: %ld BPS\r\n", baud);
    can_sync();
}

/**
 * Choose the attack to run
 */
static void chooseAttack(void)
{
    printf("Unimplemented\r\n");
}

