/**
 * @file     L1.h
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    OSI layer 1: Physical layer.
 *           This layer takes care of sending and receiving LoRa packets and 
 *           controls the anti-collision and duty cycle timings.
 */

#ifndef L1_H
#define L1_H

#include "typedefs.h"

// Functions
void L1_init();
return_type L1_enqueue_outPacket(pack_struct packet);
return_type L1_send_outPacket();
return_type L1_receive();

void L1_printPacket(pack_struct packet);

#endif