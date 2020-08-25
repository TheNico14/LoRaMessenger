# LoRaMessenger

LoRaMessenger is an off-grid chat between LoRa equipped ESP32 nodes with a simple wireless web interface.

<img src="https://github.com/TheNico14/LoRaMessenger/blob/master/Documentation/Images/interface.jpg?raw=true" width="456" height="489">

The ESP32 hosts a Wi-Fi network that will provide a simple chat-like interface that will display online nodes, received and sent messages, and some text boxes to set the node name, destination node, and message.

Since the LoRa communication protocol is used to send and receive messages, it is possible to set two or more modules to transmit and receive messages up to a distance of a few kilometers.

Most ESP32 LoRa modules, such as the TTGO LoRa32 or Heltec Wifi LoRa 32, are equipped with a display, so you can use them independently to receive messages like a pager.

<img src="https://github.com/TheNico14/LoRaMessenger/blob/master/Documentation/Images/welcome.jpg?raw=true" width="456" height="456">

A node can be easily installed inside a small box with a battery, thus creating a communication system that can communicate even in areas where there is no phone signal.

## Project details

After the ESP32 is started, a new Wi-Fi network called LoRaMessenger [number] is created.

When a device connects to the network, the web interface should open automatically, if nothing happens, the interface can be accessed by opening a web browser and navigating to the ESP32 IP, which is by default 1.1.1.1.

The web interface is now presented on your browser, the chat has the following features:

- At the top of the page, the node name can be entered so that the recipient knows who is writing. Press update to save and send the new name to all reachable nodes.
- The online section shows all available nodes detected, with some additional information such as the relay node that is being used by the receiving node if present, the receiving RSSI, the number of hops between relays, and the time elapsed since the last contact.
- The message section shows the last 5 (by default, user-settable) sent and received messages in chronological order.
The name of all the nodes that have received the message correctly is indicated under each message.
- And lastly, at the bottom of the page, there are two text boxes, the first one is used for setting the destination node name (found in the online section) or the string Broadcast for a broadcast to all nodes, and the second one to write the message.

Note that as of right now a page refresh is necessary to update the received messages and read receipts.

## Installation

This program can be easily installed by importing the project in platformio, updating the settings, and uploading it to the boards.

## Configuration

Into the includes folder, a configuration file called config.h is present. This file contains all the settings necessary for LoRaMessenger to function.

LoRa config:

- LORABAND: LoRa chip frequency. The frequency depends on your board and be sure to use only allowed frequencies in your country, [more info here](https://www.thethingsnetwork.org/wiki/LoRaWAN/Frequencies/By-Country).\
Possible values: 433E6, 866E6, 915E6.
- SPREADINGFACTOR: LoRa spreading factor. Be careful when using values higher than 7 because LoRaMessenger respects the transmission duty-cycle. High values greatly slow down the waiting time between transmissions and could affect the correct operation.
Possible values: 7 - 12.
- TXDBM: Transmission power of LoRa chip.\
Possible values: 1 - 20
- LORADUTY: Transmission duty-cycle. Be sure to use only allowed values in your country.
Possible values: 1 - 99.
- NETID: LoRaMessenger network id. This should permit to create multiple independent networks.\
Possible values: 0 - 255.

L1 config:

- L1BUFFER: Transmission packet queue. Increase if using big networks of nodes or using high spreading factors.
- TTL: Packet time to live. Sets the maximum number of hops that a packet can do before expiring.\
Possible values: 1 (only direct messages, no relaying), >1.
- BROADCASTADDR: Broadcast address number.

L3 config:

- NODENUMBER: Local node number. Each node needs a different node number! You can think this as the equivalent of an IP address for a regular network.\
Possible values: 1 - 255. Caution not to use the same address of BROADCASTADDR!
- MAXNODES: Max number of nodes expected in the network.
- ANNOUNCEMINS: Node presence announce and name update. This message is needed to inform all nodes of other nodes presence. The interval can be increased to prevent spam if using static nodes, high spreading factors or big networks.
- INACTIVEMINS: Inactivity time needed for a node to be considered offline. Caution to use at least 2-3 times the value of ANNOUNCEMINS or even bigger if poor reception.
- INACTIVESECONDSREMOVECHECK: Interval for checking removal of offline nodes.

Messages config:

- SHOWNMESSAGES: Number of messages to display on web interface.
- KEEPNMESSAGES: Number of messages to keep in memory.

Display config:

- DISPLAYSTBYSECS: Number of seconds after the display is switched off.

Network config:

- WIFIENABLED: Wi-Fi enabled. This can be used to reduce power used in case of deploying a node only for relaying of messages.
- NODENAMEOVERRIDEEN: Node name override enable. Used to override the default node name (Node [number]) to the string specified in NODENAMEOVERRIDE.
- NODENAMEOVERRIDE: Nome name override. This can be used for setting the node name without modifying it on the web interface. Useful when setting up a relay only node.
- WIFISSID: Wi-Fi network name prefix (LoRaMessenger [number])
- DNSPORT: DNS port.

Pinout:

- SCK: SPI clock.
- MISO: SPI MISO.
- MOSI: SPI MOSI.
- SS: SPI slave select.
- RST: LoRa chip reset.
- DI0: LoRa receive callback.
- I2CSCL: I2C clock.
- I2CSDA: I2C data.
- LCDRESET: LCD reset.

Pin definitions may need to be edited in case another board is used (Pin definitions are based on a TTGO LoRa32 V2).

# Notes
When a message is received, the text will be displayed instantly on the display but will not appear on the web interface until the page is refreshed.

# Future improvements
Other features that are planned for the future are:
- Message encryption, as of right one all messages are sent unencrypted.
- Automatic message refresh.

# License
MIT license.

- Based on Captive Portal by M. Ray Burnette 20150831
- Interface and message managing based on PopupChat by @tlack (https://github.com/tlack/popup-chat)
