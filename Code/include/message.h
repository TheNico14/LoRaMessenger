/**
 * @file     message.h
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    Messages list functions
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include "typedefs.h"

// Functions
void message_init();

return_type message_save(uint8_t receiver, uint8_t sender, char *message, uint32_t id);
return_type message_saveAck(uint8_t sender, uint32_t id);

uint8_t message_getAckNode(uint8_t sender, uint32_t id, uint8_t ack_number);
uint8_t message_getAckNum(uint8_t sender, uint32_t id);

void message_printLastN(int number);

int message_checkDuplicate(uint8_t sender, uint32_t id);
String message_getStringMessageList();
#endif