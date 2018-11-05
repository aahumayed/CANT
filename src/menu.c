#include "stm32h7xx.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "usart.h"
#include "menu.h"
#include "can.h"
static volatile uint8_t chosenAttack = 0;

char* Menu_Commands_Text[MENU_NUM_ITEMS] = {
    "", // strtol interprets the empty string as a 0
    "Set buadrate",
    "Choose Attack",
    "Show Current Attack",
    "Stop Firewall"
};

char* Attack_Commands_Text[ATTACK_NUM_ITEMS] = {
    "",
    "All - prevents all attacks listed below",
    "Bus Denial - prevents constant transmission of arbid 0x0 or dominant state",
    "ECU & Arbitration Denial - prevents transmission after the arbitration phases ended",
    "Frame Denial - prevents transmission of arbids belonging to other ECUs"
};

static void handle_command();
static void setBaudrate();
static void chooseAttack();
static void showCurrentAttack();
static void stopFirewall();

void display_menu()
{
    write_string("\r\n? - Help\r\n");
    for(int i = 1; i < MENU_NUM_ITEMS; i++)
    {
        printf("%d - %s\r\n", i, Menu_Commands_Text[i]);
    }
    write_string("\r\nCANFW>");
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
        case MENU_SET_BAUD:
            setBaudrate();
            break;
        case MENU_CHOOSE_ATTACK:
            chooseAttack();
            break;
        case MENU_SHOW_CURRENT_ATTACK:
            showCurrentAttack();
            break;
        case MENU_STOP_FIREWALL:
            remove_attack();
            break;
        default:
            write_string("No such command\r\n");
            break;

    }
    write_string("\r\nCANFW>");
}

/**
 * Sets the arbids to act on
 */
static void setArbids(void)
{
    write_string("Enter arbid: 0x");
    attack_arbid = read_hex();
    printf("Attacking 0x%lx\r\n", attack_arbid);
}
/**
 * Sets the CAN baud rate
 */
static void setBaudrate(void)
{
    long int baud;

    write_string("Enter baudrate in BPS: ");
    baud = read_int();

    /* Start the CAN sync */
    setCanBaudrate(baud);
    printf("Baud rate: %ld BPS\r\n", baud);
    can1_sync();//main bus --> attacker
    can_sync();//forward traffic attacker --> main bus
    //install_firewall();
}
/**
 * Choose the attack to prevent
 */
static void chooseAttack(void)
{
    long int command_num;
    remove_attack();
    for(int i = 1; i < ATTACK_NUM_ITEMS; i++)
    {
        printf("%d - %s\r\n", i, Attack_Commands_Text[i]);
    }
    write_string("\r\nCANFW ATTACK>");

    command_num = read_int();
    switch(command_num)
    {
        case ATTACK_UNUSED:
            break;
        case ATTACK_ALL:
            chosenAttack=ATTACK_ALL;
            install_attack_all();
            write_string("Installing All attacks rules\r\n");
            break;
        case ATTACK_BUS_DENIAL:
            chosenAttack=ATTACK_BUS_DENIAL;
            install_bus_denial();
            write_string("Installing the Bus Denial rules\r\n");
            break;
        case ATTACK_ECU_AND_ARBITRATION_DENIAL:
            chosenAttack = ATTACK_ECU_AND_ARBITRATION_DENIAL;
            install_ecu_and_arbitration_denial();
            write_string("Installing the ECU and Arbitration Denial rules\r\n");
            break;
        case ATTACK_FRAME_DENIAL:
            install_frame_denial();
            write_string("Installing the Frame Denial rules\r\n");
            break;
        default:
            write_string("No such Attack\r\n");
            break;
    }
}
/**
 * Shows the arbids to act on
 */
static void showCurrentAttack(void)
{
    printf("Currently preventing %s\r\n", Attack_Commands_Text[chosenAttack]);
}
