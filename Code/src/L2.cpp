/**
 * @file     L2.cpp
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    OSI layer 2: Datalink layer.
 *           This layer handles received packets, relaying of packets and
 *           provides functions for sending announces, messages
 *           and acknowledgments.
 */

// Include libraries
#include <Arduino.h>
#include "config.h"
#include "typedefs.h"
#include "L1.h"
#include "L2.h"
#include "L3.h"
#include "message.h"
#include "display.h"

// Private functions
return_type L2_relayPacket(pack_struct packet);

// Imported variables
extern char node_name[16];

// Private variables

// Functions

/**
 * @brief    Handles a received message packet
 * 
 * @param    packet: Packet to be handled
 * @return   return_type status
 */
return_type L2_handleMessage(pack_struct packet)
{
  if (packet.sender != NODENUMBER && (packet.next_node == NODENUMBER || packet.next_node == BROADCASTADDR))
  {
    if (packet.receiver == NODENUMBER || packet.receiver == BROADCASTADDR)
    {
      if (packet.receiver == BROADCASTADDR && message_checkDuplicate(packet.sender, packet.id) == ret_message_found)
        return ret_receive_duplicate;

      message_save(NODENUMBER, packet.sender, ((payload_message_struct *)packet.payload)->message_ptr, packet.id);
      L2_sendacknowledgment(packet.sender, packet.id);

      message_printLastN(5);

      display_printLastMessage(((payload_message_struct *)packet.payload)->message_ptr, packet.sender);
    }

    if (packet.receiver != NODENUMBER && packet.ttl > 1)
    {
      L2_relayPacket(packet);
    }
  }
  return ret_ok;
}

/**
 * @brief    Handles a received acknowledgment packet
 * 
 * @param    packet: Packet to be handled
 * @return   return_type status
 */
return_type L2_handleacknowledgment(pack_struct packet)
{
  if (packet.sender != NODENUMBER && (packet.next_node == NODENUMBER || packet.next_node == BROADCASTADDR))
  {
    if (packet.receiver == NODENUMBER)
    {
      message_saveAck(packet.sender, ((payload_acknowledgment_struct *)packet.payload)->packet_id);
      int acks = message_getAckNum(packet.sender, ((payload_acknowledgment_struct *)packet.payload)->packet_id);
      int node_number;

      for (int i = 0; i < acks; i++)
      {
        node_number = message_getAckNode(packet.sender, ((payload_acknowledgment_struct *)packet.payload)->packet_id, i);
        Serial.printf("Message received by %s\n\n", L3_getNodeName(node_number));
      }
    }

    if (packet.receiver != NODENUMBER && packet.ttl > 1)
    {
      L2_relayPacket(packet);
    }
  }
  return ret_ok;
}

/**
 * @brief    Handles a received announce packet
 * 
 * @param    packet: Packet to be handled
 * @return   return_type status
 */
return_type L2_handleAnnounce(pack_struct packet)
{
  if (packet.receiver == BROADCASTADDR && packet.sender != NODENUMBER)
  {
    L3_handleAnnounce(packet);

    if (packet.ttl > 1)
    {
      L2_relayPacket(packet);
    }
    return ret_ok;
  }
  return ret_error;
}

/**
 * @brief    Relays a packet
 * 
 * @param    original_packet: Packet to be relayed
 * @return   return_type status
 */
return_type L2_relayPacket(pack_struct original_packet)
{
  if (original_packet.receiver == 0 || original_packet.receiver == NODENUMBER || (original_packet.receiver > MAXNODES && original_packet.receiver != BROADCASTADDR))
    return ret_send_error;

  if (original_packet.ttl == 0)
    return ret_ttl_error;

  pack_struct packet = original_packet;

  packet.ttl--;
  packet.last_node = NODENUMBER;
  packet.next_node = L3_getNextNode(original_packet.receiver);

  return L1_enqueue_outPacket(packet);
}

/**
 * @brief    Sends a message
 * 
 * @param    receiver: Receiver node
 * @param    message: Pointer to message to be sent
 * @return   return_type status
 */
return_type L2_sendMessage(uint8_t receiver, char *message)
{
  int message_size = strlen(message);

  if (message_size == 0 || message_size > 161)
    return ret_send_size_error;

  if (receiver == 0 || receiver == NODENUMBER || (receiver > MAXNODES && receiver != BROADCASTADDR))
    return ret_send_error;

  pack_struct packet;

  packet.ttl = TTL;
  packet.receiver = receiver;
  packet.sender = NODENUMBER;
  packet.last_node = NODENUMBER;
  packet.next_node = L3_getNextNode(receiver);
  packet.id = millis();
  packet.type = payload_msg;

  packet.payload = L2_setPayloadMessage(message);

  message_save(packet.receiver, NODENUMBER, ((payload_message_struct *)packet.payload)->message_ptr, packet.id);

  message_printLastN(5);

  return L1_enqueue_outPacket(packet);
}

/**
 * @brief    Sends an acknowledgment packet
 * 
 * @param    receiver: Receiver node
 * @param    packet_id: Packet id
 * @return   return_type status
 */
return_type L2_sendacknowledgment(uint8_t receiver, uint32_t packet_id)
{
  if (receiver == 0 || receiver == NODENUMBER || (receiver > MAXNODES && receiver != BROADCASTADDR))
    return ret_send_error;

  pack_struct packet;

  packet.ttl = TTL;
  packet.receiver = receiver;
  packet.sender = NODENUMBER;
  packet.last_node = NODENUMBER;
  packet.next_node = L3_getNextNode(receiver);
  packet.id = millis();
  packet.type = payload_ack;

  packet.payload = L2_setPayloadacknowledgment(packet_id);

  return L1_enqueue_outPacket(packet);
}

/**
 * @brief    Sends a network announce packet
 * 
 * @return   return_type status
 */
return_type L2_sendAnnounce()
{
  int name_size = strlen(node_name);

  if (name_size == 0 || name_size > 15)
    return ret_send_size_error;

  pack_struct packet;

  packet.ttl = TTL;
  packet.receiver = BROADCASTADDR;
  packet.sender = NODENUMBER;
  packet.last_node = NODENUMBER;
  packet.next_node = BROADCASTADDR;
  packet.id = millis();
  packet.type = payload_ann;

  packet.payload = L2_setPayloadAnnounce(node_name, name_size);

  return L1_enqueue_outPacket(packet);
}

/**
 * @brief    Sets packet payload as message
 * 
 * @param    message: Pointer to message to be sent
 * @return   void* payload pointer
 */
void *L2_setPayloadMessage(char *message)
{
  payload_message_struct *payload_message;
  payload_message = (payload_message_struct *)malloc(sizeof(payload_message_struct));
  int message_size = strlen(message);

  payload_message->message_size = message_size;
  payload_message->message_ptr = (char *)malloc(message_size + 1);
  strcpy(payload_message->message_ptr, message);

  return payload_message;
}

/**
 * @brief    Sets packet payload as acknowledgment
 * 
 * @param    packet_id: Pointer to message to be sent
 * @return   void* payload pointer
 */
void *L2_setPayloadacknowledgment(uint32_t packet_id)
{
  payload_acknowledgment_struct *payload_acknowledgment;
  payload_acknowledgment = (payload_acknowledgment_struct *)malloc(sizeof(payload_acknowledgment_struct));

  payload_acknowledgment->packet_id = packet_id;

  return payload_acknowledgment;
}

/**
 * @brief    Sets packet payload as network announce
 * 
 * @param    name: Pointer to node name
 * @param    name_size: Name length
 * @return   void* payload pointer
 */
void *L2_setPayloadAnnounce(char *name, uint8_t name_size)
{
  payload_announce_struct *payload_announce;
  payload_announce = (payload_announce_struct *)malloc(sizeof(payload_announce_struct));

  payload_announce->name_size = name_size;
  payload_announce->name_ptr = (char *)malloc(name_size + 1);
  strcpy(payload_announce->name_ptr, name);

  return payload_announce;
}