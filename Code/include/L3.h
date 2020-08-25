/**
 * @file     L3.h
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    OSI layer 3: Network layer.
 *           This layer takes care of nodes network and packet routing
 */

#ifndef L3_H
#define L3_h

// Functions
void L3_init();

void L3_updateNode();
int L3_removeInactiveNodes();

void L3_printNodes();
int elapsedSeconds(uint32_t timestamp);

int L3_getActive(uint8_t destination);
int L3_getNextNode(uint8_t destination);
int L3_getHops(uint8_t destination);
int L3_getRssi(uint8_t destination);
int L3_getLastID(uint8_t destination);
char *L3_getNodeName(uint8_t destination);
int L3_getNodeNumber(char *name);

return_type L3_handlePacket(pack_struct packet);
return_type L3_handleAnnounce(pack_struct packet);
String L3_getStringNodeList();

#endif