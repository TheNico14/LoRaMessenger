/**
 * @file     display.cpp
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    Display functions
 */

// Include libraries
#include <Arduino.h>
#include "config.h"
#include "typedefs.h"
#include "display.h"
#include "L3.h"
#include <SPI.h>
#include <U8x8lib.h>

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(I2CSCL, I2CSDA, LCDRESET);

// Imported variables
extern char node_name[16];
extern char wifi_ssid[20];

// Exported variables
uint32_t display_standby_timer = 0;
bool display_flag_screenOn = 0;

// Functions

/**
 * @brief    Initializes the display
 * 
 */
void display_init()
{
  u8x8.begin();
  u8x8.setFont(u8x8_font_artossans8_r);
  u8x8.setFlipMode(0);
}

/**
 * @brief    Turns off the display
 * 
 */
void display_turnOff()
{
  display_flag_screenOn = false;
  u8x8.setPowerSave(1);
}

/**
 * @brief    Prints startup information
 * 
 */
void display_printWelcome()
{
  char string[17];
  sprintf(string, "Node number: %-2d", NODENUMBER);

  u8x8.clear();
  u8x8.drawString(0, 0, "LoRaMessenger");
  u8x8.drawString(0, 3, string);
  if (WIFIENABLED)
  {
    u8x8.drawString(0, 5, "Wi-Fi hotspot:");
    u8x8.drawString(0, 6, wifi_ssid);
  }
  else
  {
    u8x8.drawString(0, 5, "Node name:");
    u8x8.drawString(0, 6, L3_getNodeName(NODENUMBER));
  }

  display_flag_screenOn = true;
  display_standby_timer = millis();
  u8x8.setPowerSave(0);
}

/**
 * @brief    Prints last message received
 * 
 * @param    message: Pointer to message to be printed
 * @param    sender_node: Sender node number
 */
void display_printLastMessage(char *message, uint8_t sender_node)
{
  char sender_name[16];
  char message_str[17];

  strcpy(sender_name, L3_getNodeName(sender_node));

  int message_length = strlen(message);

  if (message_length > 48)
    message_length = 48;

  u8x8.clear();
  u8x8.drawString(0, 0, "Message from:");
  u8x8.drawString(0, 1, sender_name);

  strncpy(message_str, message, 16);
  u8x8.drawString(0, 4, message_str);
  for (int i = 1; i < 3; i++)
  {
    if (message_length > (16 * i))
    {
      strncpy(message_str, message + (16 * i), 16);
      u8x8.drawString(0, 4 + i, message_str);
    }
  }

  display_flag_screenOn = true;
  display_standby_timer = millis();
  u8x8.setPowerSave(0);
}
