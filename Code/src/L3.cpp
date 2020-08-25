/**
 * @file     L3.cpp
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    OSI layer 3: Network layer.
 *           This layer takes care of nodes network and packet routing
 */

// Include libraries
#include <Arduino.h>
#include "config.h"
#include "typedefs.h"
#include "L1.h"
#include "L2.h"
#include "L3.h"

// Exported variables
char node_name[16];

// Private variables
routing_table_struct routing_table[MAXNODES];

/**
 * @brief    Initializes the L3 layer
 * 
 */
void L3_init()
{
  memset(routing_table, 0, MAXNODES * sizeof(routing_table_struct));

  routing_table[NODENUMBER - 1].active = 1;
  if (NODENAMEOVERRIDEEN)
    strcpy(node_name, NODENAMEOVERRIDE);
  else
    sprintf(node_name, "Node %d", NODENUMBER);
  strcpy(routing_table[NODENUMBER - 1].name, node_name);

  L2_sendAnnounce();

  return;
}

/**
 * @brief    Sets node name and timestamp of this node
 * 
 */
void L3_updateNode()
{
  routing_table[NODENUMBER - 1].timestamp = millis();
  routing_table[NODENUMBER - 1].active = 1;
  strcpy(routing_table[NODENUMBER - 1].name, node_name);

  return;
}

/**
 * @brief    Searches and removes inactive nodes
 * 
 * @return   int number of removed nodes
 */
int L3_removeInactiveNodes()
{
  int ret = 0;

  for (int i = 0; i < MAXNODES; i++)
  {
    if (i != (NODENUMBER - 1) && routing_table[i].active)
    {
      if ((elapsedSeconds(routing_table[i].timestamp) / 60) >= INACTIVEMINS)
      {
        routing_table[i].active = 0;
        Serial.printf("Removed node %s from routing list\n\n", L3_getNodeName(i));

        ret++;
      }
    }
  }

  if (ret > 0)
    L3_printNodes();

  return ret;
}

/**
 * @brief    Prints current routing table
 * 
 */
void L3_printNodes()
{
  Serial.printf("--- Routing table ---\n\n");
  for (int i = 0; i < MAXNODES; i++)
  {
    if (routing_table[i].active)
    {
      Serial.printf("Name: %s\n", routing_table[i].name);
      Serial.printf("Destination: %d\n", i + 1);
      Serial.printf("Next hop: %d\n", routing_table[i].next_node);
      Serial.printf("Hops: %d\n", routing_table[i].hops);
      Serial.printf("ID: %d\n", routing_table[i].last_id);
      Serial.printf("RSSI: %d\n", routing_table[i].rssi);
      Serial.printf("Updated: %d seconds ago\n\n", elapsedSeconds(routing_table[i].timestamp));
    }
  }
  return;
}

/**
 * @brief    Returns elapsed seconds since timestamp
 * 
 * @param    timestamp: Timestamp to compare with current timestamp
 * @return   int seconds elapsed
 */
int elapsedSeconds(uint32_t timestamp)
{
  return (millis() - timestamp) / 1000;
}

/**
 * @brief    Returns if a node is active
 * 
 * @param    destination: Node to check
 * @return   int state
 */
int L3_getActive(uint8_t destination)
{
  return routing_table[destination - 1].active;
}

/**
 * @brief    Returns next node needed to reach destination node
 * 
 * @param    destination: Destination node
 * @return   int node number
 */
int L3_getNextNode(uint8_t destination)
{
  if (destination == BROADCASTADDR)
    return BROADCASTADDR;
  else if (routing_table[destination - 1].active)
    return routing_table[destination - 1].next_node;
  else
    return 0;
}

/**
 * @brief    Returns number of hops needed to reach destination node
 * 
 * @param    destination: Destination node
 * @return   int number of hops
 */
int L3_getHops(uint8_t destination)
{
  return routing_table[destination - 1].hops;
}

/**
 * @brief    Returns RSSI of destination node
 * 
 * @param    destination: Destination node
 * @return   int RSSI
 */
int L3_getRssi(uint8_t destination)
{
  return routing_table[destination - 1].rssi;
}

/**
 * @brief    Returns the name of the destination node given node number
 * 
 * @param    destination: Destination node
 * @return   char* node name
 */
char *L3_getNodeName(uint8_t destination)
{
  static char broadcast_string[10] = "Broadcast";
  if (destination == BROADCASTADDR)
    return broadcast_string;
  else
    return routing_table[destination - 1].name;
}

/**
 * @brief    Returns the number of the destination node given node name
 * 
 * @param    name: Pointer to the name to be searched
 * @return   int node number
 */
int L3_getNodeNumber(char *name)
{
  for (int i = 0; i < MAXNODES; i++)
  {
    if (routing_table[i].active)
    {
      if (strcmp(name, routing_table[i].name) == 0 && i != NODENUMBER - 1)
        return i + 1;
    }
  }
  if (strcmp(name, "Broadcast") == 0 || strcmp(name, "broadcast") == 0)
    return BROADCASTADDR;
  else
    return 0;
}

/**
 * @brief    Returns the last id associated to a node
 * 
 * @param    destination: Destination node
 * @return   int id
 */
int L3_getLastID(uint8_t destination)
{
  return routing_table[destination - 1].last_id;
}

/**
 * @brief    Handles routing table after a generic packet is received
 * 
 * @param    packet: Packet to be handled
 * @return   return_type status 
 */
return_type L3_handlePacket(pack_struct packet)
{
  int packet_rssi = packet.rssi;
  int packet_hops = TTL - packet.ttl;

  routing_table[packet.last_node - 1].timestamp = millis();
  routing_table[packet.last_node - 1].rssi = packet_rssi;

  if (packet.sender != NODENUMBER)
  {
    if (routing_table[packet.last_node - 1].active == 0)
    {
      routing_table[packet.last_node - 1].active = 1;
      routing_table[packet.last_node - 1].next_node = packet.last_node;
      routing_table[packet.last_node - 1].hops = 0;
      routing_table[packet.last_node - 1].last_id = 0;
      strcpy(routing_table[packet.last_node - 1].name, "Unknown");
    }

    if (packet_hops > 0)
    {
      routing_table[packet.sender - 1].timestamp = millis();
      if (routing_table[packet.sender - 1].active == 0)
      {
        routing_table[packet.sender - 1].active = 1;
        routing_table[packet.sender - 1].next_node = packet.last_node;
        routing_table[packet.sender - 1].hops = packet_hops;
        routing_table[packet.sender - 1].last_id = 0;
        strcpy(routing_table[packet.sender - 1].name, "Unknown");
      }
    }
    return ret_ok;
  }
  return ret_error;
}

/**
 * @brief    Handles routing table after an announce packet is received
 * 
 * @param    packet: Packet to be handled
 * @return   return_type status 
 */
return_type L3_handleAnnounce(pack_struct packet)
{
  int packet_rssi = packet.rssi;
  int packet_hops = TTL - packet.ttl;

  int current_hops = routing_table[packet.sender - 1].hops;
  int current_rssi = routing_table[packet.sender - 1].rssi;

  strncpy(routing_table[packet.sender - 1].name, ((payload_announce_struct *)packet.payload)->name_ptr, ((payload_announce_struct *)packet.payload)->name_size);
  routing_table[packet.sender - 1].name[((payload_announce_struct *)packet.payload)->name_size] = 0;

  if (packet.id == routing_table[packet.sender - 1].last_id)
  {
    if (packet_hops < current_hops || (packet_hops == current_hops && packet_rssi > current_rssi))
    {
      routing_table[packet.sender - 1].rssi = packet_rssi;
      routing_table[packet.sender - 1].next_node = packet.last_node;
      routing_table[packet.sender - 1].hops = packet_hops;
      return ret_routing_better;
    }
    else
      return ret_routing_worse;
  }
  else
  {
    routing_table[packet.sender - 1].rssi = packet_rssi;
    routing_table[packet.sender - 1].last_id = packet.id;
    routing_table[packet.sender - 1].next_node = packet.last_node;
    routing_table[packet.sender - 1].hops = packet_hops;
    return ret_routing_updated;
  }
}

/**
 * @brief    Creates a string containg active nodes (webserver)
 * 
 * @return   String active nodes
 */
String L3_getStringNodeList()
{
  String list = "";
  for (int i = 0; i < MAXNODES; i++)
  {
    if (routing_table[i].active && i != NODENUMBER - 1)
    {
      list += "<li><b>" + String(routing_table[i].name) + "</b>";
      if (routing_table[i].hops > 0)
      {
        list += " via " + String(L3_getNodeName(routing_table[i].next_node));
      }

      list += " | RSSI: " + String(routing_table[i].rssi) + " | Hops: " + String(routing_table[i].hops) +
              +" | " + String(elapsedSeconds(routing_table[i].timestamp)) + "s ago </li>";
    }
  }
  return list;
}