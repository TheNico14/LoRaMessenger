/**
 * @file     main.cpp
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    Main code of LoRaMessenger
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
#include "webserver.h"

// Imported variables
extern int L1_outBuffer_left;
extern bool L1_flag_received;
extern bool display_flag_screenOn;

extern uint32_t display_standby_timer;

// Global variables
uint32_t announce_mins = INACTIVEMINS * 60000;
uint32_t announce_timer = 0;
uint32_t announce_remove_seconds_check = INACTIVESECONDSREMOVECHECK * 1000;
uint32_t announce_remove_timer = 0;
uint32_t display_standby_secs = DISPLAYSTBYSECS * 1000;
uint8_t showmessages = SHOWNMESSAGES;
uint8_t tx_dbm = TXDBM;
uint8_t spreading_factor = SPREADINGFACTOR;

// Global Flags

/**
 * @brief    LoRaMessenger setup
 * 
 */
void setup()
{
  Serial.begin(115200);

  L1_init();

  L3_init();

  message_init();

  if (WIFIENABLED)
    webserver_init();

  display_init();
  display_printWelcome();
}

/**
 * @brief    LoRaMessenger main loop
 * 
 */
void loop()
{
  // Packet received
  if (L1_flag_received)
  {
    L1_flag_received = 0;
    L1_receive();
  }

  // Webserver
  if (WIFIENABLED)
    webserver_loop();

  // Packet ready to send
  if (L1_outBuffer_left)
    L1_send_outPacket();

  // Send announce to network
  if ((millis() - announce_timer) > announce_mins)
  {
    announce_timer = millis();
    L2_sendAnnounce();
  }

  // Check for inactive nodes
  if ((millis() - announce_remove_timer) > announce_remove_seconds_check)
  {
    announce_remove_timer = millis();
    L3_removeInactiveNodes();
  }

  // Turn off display
  if ((millis() - display_standby_timer) > display_standby_secs)
  {
    display_turnOff();
  }
}
