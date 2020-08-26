/**
 * @file     message.cpp
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    Messages list functions
 */

// Include libraries
#include <Arduino.h>
#include "config.h"
#include "typedefs.h"
#include "L1.h"
#include "L2.h"
#include "L3.h"
#include "message.h"
#include "rom/crc.h"

// Private
message_struct message_list[KEEPNMESSAGES];
int write_index_msg = 0;

// Imported variables
extern uint8_t showmessages;

// Functions

/**
 * @brief    Initializes messages list
 * 
 */
void message_init()
{
  for (int i = 0; i < KEEPNMESSAGES; i++)
  {
    memset(&message_list[i], 0, sizeof(message_struct));
    message_list[i].message = NULL;
  }

  return;
}

/**
 * @brief    Saves a message into messages list
 * 
 * @param    receiver: Message receiver
 * @param    sender: Message sender
 * @param    message: Pointer to message to be saved
 * @param    id: Message id
 * @return   return_type status 
 */
return_type message_save(uint8_t receiver, uint8_t sender, char *message, uint32_t id)
{
  message_list[write_index_msg].sender = sender;
  message_list[write_index_msg].receiver = receiver;
  message_list[write_index_msg].id = id;
  message_list[write_index_msg].acks = 0;

  message_list[write_index_msg].message = (char *)realloc(message_list[write_index_msg].message, strlen(message) + 1);
  strcpy(message_list[write_index_msg].message, message);

  if (write_index_msg == KEEPNMESSAGES - 1)
    write_index_msg = 0;
  else
    write_index_msg++;
  return ret_ok;
}

/**
 * @brief    Saves an acknowledgment into messages list
 * 
 * @param    sender: Message sender
 * @param    id: Message id
 * @return   return_type status
 */
return_type message_saveAck(uint8_t sender, uint32_t id)
{
  return_type ret;
  for (int i = 0; i < KEEPNMESSAGES; i++)
  {
    ret = ret_message_not_found;
    bool already_saved = 0;

    if (message_list[i].receiver == sender || message_list[i].receiver == BROADCASTADDR)
    {
      if (message_list[i].id == id)
      {

        for (int j = 0; j < MAXNODES - 1; j++)
        {
          if (message_list[i].acks_nodes[j] == sender)
          {
            already_saved = 1;
          }
        }
        if (!already_saved)
        {
          message_list[i].acks_nodes[message_list[i].acks] = sender;
          message_list[i].acks++;
          ret = ret_ok;
        }
      }
    }
  }
  return ret;
}

/**
 * @brief    Returns node number from message acknowledgment list
 * 
 * @param    sender: Message sender
 * @param    id: Message id
 * @param    ack_number: acknowledgment list index
 * @return   uint8_t node number
 */
uint8_t message_getAckNode(uint8_t sender, uint32_t id, uint8_t ack_number)
{
  uint8_t ret = 0;
  for (int i = 0; i < KEEPNMESSAGES; i++)
  {
    if (message_list[i].receiver == sender && message_list[i].id == id)
    {
      ret = message_list[i].acks_nodes[ack_number];
    }
  }
  return ret;
}

/**
 * @brief    Returns number of acknowledgments for a given message
 * 
 * @param    sender: Message sender
 * @param    id: Message id
 * @return   uint8_t acknowledgments number
 */
uint8_t message_getAckNum(uint8_t sender, uint32_t id)
{
  for (int i = 0; i < KEEPNMESSAGES; i++)
  {
    if (message_list[i].receiver == sender && message_list[i].id == id)
    {
      return message_list[i].acks;
    }
  }
  return 0;
}

/**
 * @brief    Prints the last n messages
 * 
 * @param    number: Number of messages to print
 */
void message_printLastN(int number)
{
  int read_position = write_index_msg;

  if (number > KEEPNMESSAGES)
    number = KEEPNMESSAGES;

  if (read_position - number < 0)
    read_position = KEEPNMESSAGES + read_position - number;
  else
    read_position = read_position - number;

  Serial.printf("--- Last %d messages ---\n", number);
  for (int i = 0; i < number; i++)
  {

    if (read_position == KEEPNMESSAGES)
      read_position = 0;

    if (message_list[read_position].message != NULL)
    {
      char receiver_name[16];
      char sender_name[16];

      strcpy(receiver_name, L3_getNodeName(message_list[read_position].receiver));
      strcpy(sender_name, L3_getNodeName(message_list[read_position].sender));

      Serial.printf("%s->%s: %s\n", sender_name, receiver_name, message_list[read_position].message);
    }
    read_position++;
  }

  Serial.printf("\n");
  return;
}

/**
 * @brief    Checks if a message is already been saved
 * 
 * @param    sender: Message sender
 * @param    id: Message id
 * @return   int 
 */
int message_checkDuplicate(uint8_t sender, uint32_t id)
{
  for (int i = 0; i < KEEPNMESSAGES; i++)
  {
    if (message_list[i].sender == sender && message_list[i].id == id)
    {
      return ret_message_found;
    }
  }
  return ret_message_not_found;
}

/**
 * @brief    Creates a string containg the message list (webserver)
 * 
 * @return   String message list
 */
String message_getStringMessageList()
{
  int read_position = write_index_msg;
  int number = showmessages;

  String list = "";

  if (number > KEEPNMESSAGES)
    number = KEEPNMESSAGES;

  if (read_position - number < 0)
    read_position = KEEPNMESSAGES + read_position - number;
  else
    read_position = read_position - number;

  for (int i = 0; i < number; i++)
  {

    if (read_position == KEEPNMESSAGES)
      read_position = 0;

    if (message_list[read_position].message != NULL)
    {
      char receiver_name[16];
      char sender_name[16];

      strcpy(receiver_name, L3_getNodeName(message_list[read_position].receiver));
      strcpy(sender_name, L3_getNodeName(message_list[read_position].sender));

      list += "<li><b>" + String(sender_name) + " -> " + String(receiver_name) + ":</b> " + String(message_list[read_position].message);
      if (message_list[read_position].acks)
      {
        list += "<br>Received by:";
        for (int i = 0; i < message_list[read_position].acks; i++)
        {
          if (i > 0)
            list += ",";
          list += " " + String(L3_getNodeName(message_list[read_position].acks_nodes[i]));
        }
      }
    }
    list += "</li>";
    read_position++;
  }
  return list;
}