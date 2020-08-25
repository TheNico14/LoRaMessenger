/**
 * @file     config.h
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    LoRaMessenger configuration.
 */

// LoRa config (check allowed parameters in your country!)
#define LORABAND 868E6    // LoRa frequency: 433E6, 866E6, 915E6
#define SPREADINGFACTOR 7 // Spreading factor
#define TXDBM 20          // TX power of the radio.

#define LORADUTY 1 // TX max duty cycle
#define NETID 121  // Network id

// L1 config (needs to be the same on each node!)
#define L1BUFFER 20       // Packet queue, increase if using high spreading factor
#define TTL 2             // Packet Time To Live (maximum number of hops)
#define BROADCASTADDR 255 // Broadcast address

// L3 config
#define NODENUMBER 1                  // Node number (1-n)
#define MAXNODES 10                   // Maximum nodes in network
#define ANNOUNCEMINS 1                // Availability announce interval (min)
#define INACTIVEMINS 3                // Inactivity time needed to consider a node offline (min)
#define INACTIVESECONDSREMOVECHECK 10 // Interval for checking inactive nodes (sec)

// Messages config
#define SHOWNMESSAGES 5  // Number of messages to display on web interface
#define KEEPNMESSAGES 20 // Number of messages to keep in memory

// Display config
#define DISPLAYSTBYSECS 10 // Display standby time (sec)

// Network config
#define WIFIENABLED 1            // Wi-Fi enabled
#define NODENAMEOVERRIDEEN 1     // Node name override enable (ex: relay without Wi-Fi)
#define NODENAMEOVERRIDE "Home"  // Node name override
#define WIFISSID "LoRaMessenger" // Wi-Fi prefix (ex: LoRaMessenger 1)
#define DNSPORT 53               // DNS port

// Pinout
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 23
#define DI0 26
#define I2CSCL 22
#define I2CSDA 21
#define LCDRESET 16