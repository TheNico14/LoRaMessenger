/**
 * @file     L1.cpp
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    OSI layer 1: Physical layer.
 *           This layer takes care of sending and receiving LoRa packets and 
 *           controls the anti-collision and duty cycle timings.
 */

#include <Arduino.h>
#include "config.h"
#include "typedefs.h"
#include "L1.h"
#include "L2.h"
#include "L3.h"
#include <SPI.h>
#include <LoRa.h>

// Exported variables
int L1_outBuffer_left = 0;
bool L1_flag_received = 0;

// Imported variables
extern uint8_t tx_dbm;
extern uint8_t spreading_factor;

// Private variables
static pack_struct outBuffer[L1BUFFER];
static int outBuffer_front = 0;
static int outBuffer_rear = 0;

uint32_t transmit_duration = 0;
uint32_t last_transmit_timestamp = 0;
uint32_t last_receive_timestamp = 0;
uint32_t anticollision_time;

// Private functions
void L1_onReceive(int packetSize);
return_type L1_packSend(pack_struct packet);
void L1_emptyBuffer();

// Functions

/**
 * @brief    Initializes the L1 layer
 * 
 */
void L1_init()
{
  anticollision_time = 500 * NODENUMBER; // WARNING: may need to be adusted if SF > 7 is used

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  LoRa.setTxPower(tx_dbm);

  if (LoRa.begin(LORABAND))
    Serial.println("LoRa module started correctly");
  else
  {
    Serial.println("Error starting LoRa module");
    exit(0);
  }
  LoRa.setSpreadingFactor(spreading_factor);
  LoRa.enableCrc();
  LoRa.onReceive(L1_onReceive);
  LoRa.receive();
}

/**
 * @brief    Adds a packet to the sending queue
 * 
 * @param    packet: Packet to be enqueued
 * @return   return_type status
 */
return_type L1_enqueue_outPacket(pack_struct packet)
{
  if (outBuffer_rear == L1BUFFER)
    return ret_buffer_full;

  else
  {
    outBuffer[outBuffer_rear].ttl = packet.ttl;
    outBuffer[outBuffer_rear].receiver = packet.receiver;
    outBuffer[outBuffer_rear].sender = packet.sender;
    outBuffer[outBuffer_rear].last_node = packet.last_node;
    outBuffer[outBuffer_rear].next_node = packet.next_node;
    outBuffer[outBuffer_rear].id = packet.id;
    outBuffer[outBuffer_rear].type = packet.type;
    outBuffer[outBuffer_rear].payload = packet.payload;
    outBuffer_rear++;
    L1_outBuffer_left++;
  }
  return ret_ok;
}

/**
 * @brief    Sends the first enqueued packet
 * 
 * @return   return_type status
 */
return_type L1_send_outPacket()
{
  if (outBuffer_front == outBuffer_rear)
    return ret_buffer_empty;

  else if ((millis() - last_transmit_timestamp) < (100 - LORADUTY) * transmit_duration)
    return ret_send_duty_error;

  else if ((millis() - last_receive_timestamp) < anticollision_time)
    return ret_send_anticollision_error;

  else
  {
    pack_struct packet = outBuffer[outBuffer_front];

    return_type ret = L1_packSend(packet);

    if (packet.type == payload_msg)
      free(((payload_message_struct *)packet.payload)->message_ptr);

    else if (packet.type == payload_ann)
      free(((payload_announce_struct *)packet.payload)->name_ptr);

    free(packet.payload);

    for (int i = 0; i < outBuffer_rear - 1; i++)
      outBuffer[i] = outBuffer[i + 1];

    outBuffer_rear--;
    L1_outBuffer_left--;
    return ret;
  }
}

/**
 * @brief    Creates a LoRa packet and sends it
 * 
 * @param    packet: Packet to be sent
 * @return   return_type status 
 */
return_type L1_packSend(pack_struct packet)
{
  int ret = LoRa.beginPacket();
  if (ret)
  {
    LoRa.write(NETID);
    LoRa.write(packet.ttl);
    LoRa.write(packet.receiver);
    LoRa.write(packet.sender);
    LoRa.write(packet.last_node);
    LoRa.write(packet.next_node);
    LoRa.write((uint8_t *)&(packet.id), 4);
    LoRa.write(packet.type);

    switch (packet.type)
    {
    case payload_msg:
    {
      payload_message_struct *payload_message = (payload_message_struct *)packet.payload;
      LoRa.write(payload_message->message_size);

      char message_temp[payload_message->message_size + 1];
      strcpy(message_temp, payload_message->message_ptr);
      LoRa.print(message_temp);
    }
    break;
    case payload_ack:
    {
      payload_acknowledgment_struct *payload_acknowledgment = (payload_acknowledgment_struct *)packet.payload;
      LoRa.write((uint8_t *)&(payload_acknowledgment->packet_id), 4);
    }
    break;
    case payload_ann:
    {
      payload_announce_struct *payload_announce = (payload_announce_struct *)packet.payload;
      LoRa.write(payload_announce->name_size);

      char *message_temp = (char *)malloc(payload_announce->name_size + 1);
      strcpy(message_temp, payload_announce->name_ptr);
      LoRa.print(message_temp);
    }
    }

    transmit_duration = millis();
    LoRa.endPacket();
    int current_millis = millis();
    transmit_duration = current_millis - transmit_duration;
    last_transmit_timestamp = current_millis;

    Serial.printf("--- Sent ");
    L1_printPacket(packet);

    LoRa.receive();
    return ret_ok;
  }

  LoRa.receive();
  return ret_error;
}

/**
 * @brief    Callback function after receiving LoRa packet
 * 
 * @param    packetSize: Packet size
 */
void L1_onReceive(int packetSize)
{
  L1_flag_received = 1;
  return;
}

/**
 * @brief    Receives a LoRa packet and calls the correct handler
 * 
 * @return   return_type status 
 */
return_type L1_receive()
{
  pack_struct packet;

  uint8_t netid = LoRa.read();
  if (netid != NETID)
  {
    L1_emptyBuffer();
    return ret_receive_netid_error;
  }

  packet.ttl = LoRa.read();
  if (packet.ttl == 0)
  {
    L1_emptyBuffer();
    return ret_ttl_error;
  }

  packet.receiver = LoRa.read();
  packet.sender = LoRa.read();
  packet.last_node = LoRa.read();
  packet.next_node = LoRa.read();

  if (packet.next_node != NODENUMBER && packet.next_node != BROADCASTADDR)
  {
    L1_emptyBuffer();
    return ret_receive_wrong_node;
  }

  LoRa.readBytes((uint8_t *)&packet.id, 4);
  packet.type = LoRa.read();
  packet.rssi = LoRa.packetRssi();

  L3_handlePacket(packet);

  switch (packet.type)
  {
  case payload_msg:
  {
    uint8_t message_size = LoRa.read();

    char *message = (char *)malloc(message_size + 1);
    int i;
    for (i = 0; i < message_size; i++)
    {
      *(message + i) = (char)LoRa.read();
    }
    *(message + i) = 0;

    packet.payload = L2_setPayloadMessage(message);

    free(message);

    L2_handleMessage(packet);
  }
  break;
  case payload_ack:
  {
    uint32_t message_crc;
    LoRa.readBytes((uint8_t *)&message_crc, 4);

    packet.payload = L2_setPayloadacknowledgment(message_crc);

    L2_handleacknowledgment(packet);
  }
  break;
  case payload_ann:
  {
    uint8_t name_size = LoRa.read();

    char *name = (char *)malloc(name_size + 1);
    int i;
    for (i = 0; i < name_size; i++)
    {
      *(name + i) = (char)LoRa.read();
    }
    *(name + i) = 0;

    packet.payload = L2_setPayloadAnnounce(name, name_size);

    L2_handleAnnounce(packet);

    free(name);
  }
  break;
  default:
    break;
  }

  last_receive_timestamp = millis();

  Serial.printf("--- Received ");
  L1_printPacket(packet);

  return ret_ok;
}

/**
 * @brief    Empties the LoRa input buffer
 * 
 */
void L1_emptyBuffer()
{
  if (LoRa.available())
  {
    while (LoRa.available())
      LoRa.read();
  }
  return;
}

/**
 * @brief    Prints packet's information
 * 
 * @param    packet: Packet to be printed
 */
void L1_printPacket(pack_struct packet)
{

  switch (packet.type)
  {
  case payload_msg:
    Serial.printf("message packet ---\n");
    break;

  case payload_ack:
    Serial.printf("acknowledgment packet: ---\n");
    break;

  case payload_ann:
    Serial.printf("announce packet ---\n");
    break;
  }

  Serial.printf("TTL: %d\n", packet.ttl);
  Serial.printf("Receiver: %d\n", packet.receiver);
  Serial.printf("Sender: %d\n", packet.sender);
  Serial.printf("Last node: %d\n", packet.last_node);
  Serial.printf("Next node: %d\n", packet.next_node);
  Serial.printf("id: %zu\n", packet.id);
  Serial.printf("RSSI: %d\n", packet.rssi);

  switch (packet.type)
  {
  case payload_msg:
  {
    payload_message_struct *payload_message = (payload_message_struct *)packet.payload;
    Serial.printf("Message: %s\n", payload_message->message_ptr);
  }
  break;
  case payload_ack:
  {
    payload_acknowledgment_struct *payload_acknowledgment = (payload_acknowledgment_struct *)packet.payload;
    Serial.printf("Packet id: %x\n", payload_acknowledgment->packet_id);
  }
  break;
  case payload_ann:
  {
    payload_announce_struct *payload_announce = (payload_announce_struct *)packet.payload;
    Serial.printf("Name: %s\n", payload_announce->name_ptr);
  }
  }
  Serial.printf("\n\n");
}