/**
 * @file     display.ch
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    Display functions
 */

#ifndef DISPLAY_H
#define DISPLAY_H

// Functions
void display_init();
void display_turnOff();
void display_printWelcome();
void display_printLastMessage(char *message, uint8_t sender_node);

#endif