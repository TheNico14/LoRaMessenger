# LoRaMessenger

LoRaMessenger is an off-grid chat between LoRa equipped ESP32 nodes with a simple wireless web interface.

<img src="https://github.com/TheNico14/LoRaMessenger/blob/master/Documentation/Images/interface.jpg?raw=true">

The ESP32 hosts a Wi-Fi network that will provide a simple chat-like interface that will display online nodes, received and sent messages, and some text boxes to set the node name, destination node, and message.

Since the LoRa communication protocol is used to send and receive messages, it is possible to set two or more modules to transmit and receive messages up to a distance of a few kilometers.

Most ESP32 LoRa modules, such as the TTGO LoRa32 or Heltec Wifi LoRa 32, are equipped with a display, so you can use them independently to receive messages like a pager.

<img src="https://github.com/TheNico14/LoRaMessenger/blob/master/Documentation/Images/welcome.jpg?raw=true">

A node can be easily installed inside a small box with a battery, thus creating a communication system that can communicate even in areas where there is no phone signal.

## Web interface

After the ESP32 is started, a new Wi-Fi network called LoRaMessenger [number] is created.

When a device connects to the network, the web interface should open automatically, if nothing happens, the interface can be accessed by opening a web browser and navigating to the ESP32 IP, which is by default 1.1.1.1.

On android, a wifi login page containing the web inteface is opened after connecting to the Wi-Fi network. This way, background 4g network connection is mantained even when using LoRaMessenger. A notification saying login to the network should stay into the notification panel and can be opened anytime to show the interface again.

The web interface is now presented on your browser, the chat has the following features:

- At the top of the page, the node name can be entered so that the recipient knows who is writing. After pressing update, the name is saved and sent to all reachable nodes.
- The online section shows all available nodes detected, with some additional information such as the relay node that is being used by the receiving node if present, the receiving RSSI, the number of hops between relays, and the time elapsed since the last contact.
- The message section shows the last 5 (by default, user-settable) sent and received messages in chronological order.
The name of all the nodes that have received the message correctly is indicated under each message.
- At the bottom of the page, there are two text boxes, the first one is used for setting the destination node and the second one to write the message.\
The destination field contains the Broadcast value by default. This way the message is sent to all available nodes. You can also write the name of a node exactly as reported in the online section to send the message only to a specific recipient.

Note that as of right now a page refresh is necessary to update the received messages and read receipts.

## LoRa protocol

LoRaMessenger uses a custom communication protocol, each packet sent consists of a header and a payload.

<img src="https://github.com/TheNico14/LoRaMessenger/blob/master/Documentation/Images/packet.jpg?raw=true">

The header provides the information needed for the network and packet routing to work properly, the parameters contained in the header are as follows:

- NETID: Network ID, specified in config.h. This allows the creation of multiple independent networks.
- TTL: Packet time to live, specified in config.h. This value is used to know how many hops a packet has done and is needed by the routing algorithm.
- RECEIVER: Receiver node number.
- SENDER: Sender node number.
- LAST NODE: Sender node number or last node that relayed the packet.
- NEXT NODE: Receiver node number or next node needed to relay the packet to the receiver node.
- ID: Packet ID, each packet sent from the same node has its unique 4 bytes long ID. This is needed to discard already received packets and for sending a received acknowledgment.
- PAYLOAD TYPE: Payload type, used for correctly interpreting the payload. Possible payloads types are: Message, Acknowledgment, and Announce.

Message payload:

- MESSAGE SIZE: Message size in bytes, needed for message reading.
- MESSAGE: Message content. This is sent in plain text for now!

Acknowledgment payload:

- RECEIVED PACKET ID: ID from received message packet. This is sent back to the sender to let him know that the packet has been received.

Announce payload:

- NAME SIZE: Node name size in bytes, needed for name reading.
- NODE NAME: Node name. This is displayed on every node web interface and can be written in the destination field to send a message to only a specific node.

## Packet relaying and routing

LoRaMessenger creates a network of nodes capable of forwarding messages to nodes not directly reachable by the sender.

To do this, each node utilizes an automatic routing table containing the destination nodes and the best route to reach them.
The table is updated through announcement packets that are sent periodically or upon name change by all nodes.

The current routing algorithm is very simple and prefers a lower number of hops, in the case of two routes with the same number of hops the one with the connection to the next strongest node is chosen.

## Installation

This program can be easily installed by importing the project in platformio, updating the settings, and uploading it to the boards.

## Configuration

Into the includes folder, a configuration file called config.h is present. This file contains all the settings necessary for LoRaMessenger to function.

LoRa config:

- LORABAND: LoRa chip frequency. The frequency depends on your board and local allowed frequencies, please be sure to use only allowed frequencies in your country, [more info here](https://www.thethingsnetwork.org/wiki/LoRaWAN/Frequencies/By-Country).\
Possible values: 433E6, 866E6, 915E6.
- SPREADINGFACTOR: LoRa spreading factor. Be careful when using values higher than 7 because LoRaMessenger respects the transmission duty-cycle. High values greatly slow down the waiting time between transmissions and could affect the correct operation, other adjustments like anticollision time may be needed.
Possible values: 7 - 12.
- TXDBM: Transmission power of LoRa chip.\
Possible values: 1 - 20
- LORADUTY: Transmission duty-cycle. Be sure to use only allowed values in your country.
Possible values: 1 - 99.
- NETID: LoRaMessenger network id. This allows the creation of multiple independent networks.\
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
- ANNOUNCEMINS: Node presence announce and name update. This message is needed to inform all nodes of the presence of all other nodes. The interval can be increased to prevent spam if using static nodes, high spreading factors, or big networks.
- INACTIVEMINS: Inactivity time needed for a node to be considered offline. Caution to use at least 2-3 times the value of ANNOUNCEMINS or even bigger if poor reception.
- INACTIVESECONDSREMOVECHECK: Interval for checking the removal of offline nodes.

Messages config:

- SHOWNMESSAGES: Number of messages to display on the web interface.
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

## Future improvements/fixes

Other features that are planned for the future are:

- Message encryption, as of right one all messages are sent unencrypted.
- Automatic message refresh.
- Testing and improvements of routing algorithm.

## License

MIT license.
