/**
 * @file     typedefs.h
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    Typedef definitions
 */

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <Arduino.h>

/**
 * @brief    Packet payload type
 * 
 */
typedef enum payload_type
{
  payload_msg = 0,
  payload_ack,
  payload_ann
} payload_type;

/**
 * @brief    Function return type
 * 
 */
typedef enum return_type
{
  ret_ok = 0,
  ret_error,
  ret_buffer_empty,
  ret_buffer_full,
  ret_send_duty_error,
  ret_send_anticollision_error,
  ret_send_error,
  ret_send_size_error,
  ret_receive_netid_error,
  ret_receive_wrong_node,
  ret_receive_duplicate,
  ret_ttl_error,
  ret_routing_worse,
  ret_routing_better,
  ret_routing_updated,
  ret_message_not_found,
  ret_message_found
} return_type;

/**
 * @brief    Packet structure
 * 
 */
typedef struct
{
  uint8_t ttl;
  uint8_t receiver;
  uint8_t sender;
  uint8_t last_node;
  uint8_t next_node;
  uint32_t id;
  uint8_t type;
  void *payload;
  int rssi;
} pack_struct;

/**
 * @brief    Message payload structure
 * 
 */
typedef struct
{
  uint8_t message_size;
  char *message_ptr;
} payload_message_struct;

/**
 * @brief    Acknowledgment payload structure
 * 
 */
typedef struct
{
  uint32_t packet_id;
} payload_acknowledgment_struct;

/**
 * @brief    Announce payload structure
 * 
 */
typedef struct
{
  uint8_t name_size;
  char *name_ptr;
} payload_announce_struct;

/**
 * @brief    Routing table structure
 * 
 */
typedef struct
{
  uint8_t active;
  uint8_t next_node;
  uint8_t hops;
  int8_t rssi;
  uint32_t last_id;
  char name[16];
  uint32_t timestamp;
} routing_table_struct;

/**
 * @brief    Message structure
 * 
 */
typedef struct
{
  uint8_t receiver;
  uint8_t sender;
  uint32_t id;
  char *message;
  uint8_t acks;
  uint8_t acks_nodes[MAXNODES - 1];
} message_struct;

#endif