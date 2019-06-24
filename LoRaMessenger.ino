// Include libraries
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h> 
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <SPI.h>
#include <LoRa.h>
#include <U8x8lib.h>

// Config
#define LORANODE    1
#define INDEXTITLE  "LoRaMessenger"
#define WIFIPREFIX  "LoRa Node"
#define DNS         "node"
#define LORABAND    915E6 // 915E6 for US, 868E6 for EU

// Pinout
#define SCK         5
#define MISO        19
#define MOSI        27
#define SS          18
#define RST         14 
#define DI0         26
#define I2C_SCL     22
#define I2C_SDA     21
#define LCD_RESET   16

// Init System Settings
#define MSGLENGTH   200
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 22, /* data=*/ 21, /* reset=*/ 16);
IPAddress ip(10,10,10,1);
IPAddress subnet(255,255,255,0);
const byte DNS_PORT = 53;
String allMsgs="<li>Welcome to LoRaMessenger</li>";
String node_name = String(String(WIFIPREFIX)+" "+String(LORANODE));
DNSServer dnsServer;
AsyncWebServer webServer(80);

// Message structure
typedef struct Data {
  byte msg_from;
  String msg;
  int rssi;
} Data;

void setup() {
 
  // Start serial interface
  Serial.begin(115200); 

  // Setup webserver
  setup_server();

  // LoRa setup
  setup_lora();
  
  // Display setup
  setup_LCD();
  updateScreen(node_name, "No messages yet", 0);
}

void loop() { 

  // Handle DNS
  dnsServer.processNextRequest(); 
}

void LoRa_send(String message) {

  // Message cleanup
  message.replace("<","&lt;");
  message.replace(">","&gt;");
  message.substring(0, MSGLENGTH);

  // Message send
  if(message != ""){
    Serial.println("Sending message: "+message);
    allMsgs = allMsgs+"<li>"+"<b>Me:</b> "+message+"</li>";
   
    LoRa.beginPacket();
    LoRa.write(LORANODE);
    LoRa.print(message);

    if (LoRa.endPacket()) Serial.println("Message successfully sent");
    else Serial.println("Error sending message");
    
    String display_message = String("Me: "+message);
    updateScreen(node_name, display_message, 0);
  }
  LoRa.receive();
}

void LoRa_receive(int packetSize) {

  Data message;
  
  message.msg_from = LoRa.read();
  while (LoRa.available()) {
    message.msg += (char)LoRa.read();
  }
  message.rssi = LoRa.packetRssi();

  String display_message = String("Node "+String(message.msg_from)+": "+message.msg);

  Serial.println("Received message from: Node "+String(message.msg_from));
  Serial.println("Message: "+message.msg);
  Serial.println("RSSI: "+String(message.rssi));

  updateScreen(node_name, display_message, message.rssi);  
  allMsgs = allMsgs+"<li>"+"<b>Node "+String(message.msg_from)+": </b>"+message.msg+"</li>";
}

void setup_server() {
  
  // WiFi setup
  WiFi.mode(WIFI_AP);
  WiFi.softAP(node_name.c_str());
  delay(200);
  WiFi.softAPConfig(ip, ip, subnet);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", ip);
  
  Serial.println("WiFi AP started with name: "+String(node_name));

  // Webserver setup
  webServer.on("/restart", [](AsyncWebServerRequest *request) {
    request->send(200,"text/plain", "Restarting...");
    Serial.println("Restarting...");
    ESP.restart(); 
  });
  webServer.on("/", [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index());
  });
  webServer.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebParameter* p = request->getParam(0);
    String message = String(p->value().c_str());  
    if(message != ""){
      LoRa_send(message);    
    }
    request->redirect("/");
  }); 
  webServer.begin();
  
  if (MDNS.begin(DNS)) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("Webserver started");
  }
  else Serial.println("Error setting up local DNS");
}

void setup_lora() {
  
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  LoRa.setTxPower(20);
  if (LoRa.begin(LORABAND)) Serial.println("LoRa module started");
  else Serial.println("Error starting LoRa module");
  LoRa.setSpreadingFactor(9);
  LoRa.enableCrc();
  LoRa.onReceive(LoRa_receive);
  LoRa.receive();
}

void setup_LCD() {

  u8x8.begin();
  u8x8.setFont(u8x8_font_artossans8_r);
  u8x8.setFlipMode(1);

}

void updateScreen(String index, String message, int rssi){

  u8x8.clear();
  u8x8.drawString(0,0,index.c_str());
  u8x8.drawString(0,2,"Last message:");
  u8x8.drawString(0,3,(message.substring(0,15)).c_str());
  u8x8.drawString(0,4,(message.substring(15,30)).c_str());
  u8x8.drawString(0,5,(message.substring(30,45)).c_str());
  if(rssi != 0) u8x8.drawString(0,6,(String("RSSI: "+String(rssi))).c_str());
}

String index() {
  return header()+"<div></div><div><label>Messages:</label>"+"<ul style=list-style: none;>"+allMsgs+
    "</ul>"+"</div><div><form action=/submit method=post id=usrform><label>Post new message:</label><br/>"+
    "<textarea name=message form=usrform></textarea><br/><input type=submit value=Send></form>";
}

String header(void) {
  String a = String(INDEXTITLE);
  String CSS = "article { background: #f2f2f2; padding: 1em; }" 
    "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
    "div { padding: 0.5em; }"
    "h1 { margin: 0.5em 0 0 0; padding: 0; }"
    "input { border-radius: 0; }"
    "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
    "nav { background: #0061ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
    "nav b { display: block; font-size: 1.2em; margin-bottom: 0.5em; } "
    "textarea { width: 100%; }";
  String h = "<!DOCTYPE html><html>"
    "<head><title>"+a+" - "+node_name+"</title>"
    "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
    "<style>"+CSS+"</style></head>"
    "<body><nav><b>"+a+" - "+node_name+"</b></nav>";
  return h;
}
