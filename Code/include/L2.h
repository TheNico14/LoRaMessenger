/**
 * @file     L2.h
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    OSI layer 2: Datalink layer.
 *           This layer handles received packets, relaying of packets and
 *           provides functions for sending announces, messages
 *           and acknowledgements.
 */

#ifndef L2_H
#define L2_h

#include "typedefs.h"

// Functions
return_type L2_handleMessage(pack_struct packet);
return_type L2_handleAcknowledgement(pack_struct packet);
return_type L2_handleAnnounce(pack_struct packet);

return_type L2_sendMessage(uint8_t receiver, char *message);
return_type L2_sendAcknowledgement(uint8_t receiver, uint32_t packet_id);
return_type L2_sendAnnounce();

void *L2_setPayloadMessage(char *message);
void *L2_setPayloadAcknowledgement(uint32_t packet_id);
void *L2_setPayloadAnnounce(char *name, uint8_t name_size);

#endif