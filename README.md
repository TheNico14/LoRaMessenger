# LoRaMessenger
LoRaMessenger is a simple offgrid chat between ESP32 nodes with a wireless web interface that utilizes the LoRa protocol to send messages between TTGO LoRa32, Heltec Wifi LoRa 32 or similar boards.

![Screenshot](https://github.com/TheNico14/LoRaMessenger/blob/master/Images/Interface.png)

The ESP32 hosts a WiFi network which will provide a simple chat-like interface showing the received messages and a text input box for sending a message. \
Since the LoRa communication protocol is used to send and receive messages, two or more modules can be set up to broadcast and receive messages up to a distance of 10 Kilometers. \
Most ESP32 LoRa modules such as the TTGO LoRa32 or Heltec Wifi LoRa 32 have a display, so you can use them standalone for receiving messages like a pager.

A node can be easily installed inside a small box with a battery, thus creating a communication system that can communicate even in areas where there is no phone signal.

![Screenshot](https://github.com/TheNico14/LoRaMessenger/blob/master/Images/Build.jpg)

# Project details
After the ESP is started, a new WiFi network called LoRa Node is created and should be visible, the default WiFi password is Password.\
When a device is connected to the network a captive portal should open, if nothing happens open a web browser and navigate to node.local or 10.10.10.1.

The web interface is now presented on your browser, the chat has the following features:
- At the beginning of the page you can enter your name so that the recipient knows who is writing. Press update to save the new name.
- The message section shows all sent and received messages in chronological order.
- The name of all devices that have received the message correctly is shown below the messages.
- And last but not least, at the bottom of the page there is a text box to send a new message.

Note that as of right now a page refresh is necessary to update the received messages and read receipts.

# Installation
This program requires the following libraries:
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP/) 
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer/)
- [TTGO OLED](https://github.com/CaptIgmu/Arduino/tree/master/esp32/TTGO_LoRa32_OLED/) (This may need to be replaced in case another board is used)

# Configuration
At the beginning of the program there are several \#define that allow you to change some operating parameters.

The most important ones are:
- LORANODE: Node number, set a different number (0-255) for each node.
- LORABAND: Band used by the LoRa chip, change this to the correct band available for your country!

Pin definitions may need to be edited in case another board is used (Pin definitions are based on a TTGO LoRa32 V2).

# Notes
When a message is received, the text will be displayed instantly on the display but will not appear on the web interface until the page is refreshed.

# Future improvements
Other features that are planned for the future are:
- Message encryption, as of right one all messages are sent unencrypted.
- Some sort of relaying to create a bigger network of nodes.
- Redesign of the web interface and automatic message refresh.

# License
MIT license.

- Based on Captive Portal by M. Ray Burnette 20150831
- Interface and message managing based on PopupChat by @tlack (https://github.com/tlack/popup-chat)
