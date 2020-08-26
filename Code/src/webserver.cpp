/**
 * @file     webserver.cpp
 * @author   Nicholas Polledri
 * @version  1.0
 * @date     09-08-2020
 * 
 * @brief    Webserver functions
 */

// Include libraries
#include <Arduino.h>
#include "config.h"
#include "typedefs.h"
#include "webserver.h"
#include <WiFi.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include "L2.h"
#include "L3.h"
#include "message.h"

char wifi_ssid[20];

AsyncWebServer webServer(80);
DNSServer dnsServer;

// Imported variables
extern char node_name[16];

// Variables
char recipient[16] = "Broadcast";

// IP
IPAddress ap_local_IP(1, 1, 1, 1);
IPAddress ap_subnet(255, 255, 255, 0);

// Private Functions
String index_html();

// Functions

/**
 * @brief    Initializes webserver
 * 
 */
void webserver_init()
{
  sprintf(wifi_ssid, "%s %-2d", WIFISSID, NODENUMBER);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(wifi_ssid);
  delay(2000);
  WiFi.softAPConfig(ap_local_IP, ap_local_IP, ap_subnet);

  dnsServer.start(DNSPORT, "*", ap_local_IP);

  webServer.on("/", [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html());
  });
  webServer.on("/generate_204", [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html());
  });
  webServer.on("/captive-portal/api", [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html());
  });
  webServer.on("/rename", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebParameter *p = request->getParam(0);
    if (strlen(p->value().c_str()) > 0 && strlen(p->value().c_str()) < 15)
    {
      if (strcmp(p->value().c_str(), "Broadcast") != 0 && strcmp(p->value().c_str(), "broadcast") != 0)
      {
        strcpy(node_name, p->value().c_str());
        L3_updateNode();
        L2_sendAnnounce();
      }
    }
    request->redirect("/");
  });

  webServer.on("/send", HTTP_POST, [](AsyncWebServerRequest *request) {
    {
      AsyncWebParameter *p = request->getParam(0);

      if (strlen(p->value().c_str()) > 0 && strlen(p->value().c_str()) < 15)
      {
        strcpy(recipient, p->value().c_str());
      }
    }
    {
      AsyncWebParameter *p = request->getParam(1);
      if (strlen(p->value().c_str()) > 0 && strlen(p->value().c_str()) < 160)
      {
        L2_sendMessage(L3_getNodeNumber(recipient), const_cast<char *>(p->value().c_str()));
      }
    }
    request->redirect("/");
  });

  webServer.on("/refresh", [](AsyncWebServerRequest *request) {
    request->redirect("/");
  });

  webServer.begin();
  return;
}

/**
 * @brief    Processes dns requests
 * 
 */
void webserver_loop()
{
  dnsServer.processNextRequest();
}

/**
 * @brief    Creates a string containg the web page
 * 
 * @return   String 
 */
String index_html()
{
  String html = "<!DOCTYPE html><html>"
                "<head><title>LoRaMessenger</title>"
                "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
                "<style>article { background: #f2f2f2; padding: 1em; }"
                "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
                "div { padding: 0.5em; }"
                "h1 { margin: 0.5em 0 0 0; padding: 0; }"
                "input { border-radius: 0; }"
                "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
                "nav { background: #0061ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
                "nav b { display: block; font-size: 1.2em; margin-bottom: 0.5em; } "
                "textarea { width: 100%; }</style></head>"
                "<body><nav><b>LoRaMessenger</b></nav>"
                "<div> <form action=/rename method=post><br /><label>Node name</label>"
                "<textarea name=nodename rows=1>" +
                String(node_name) +
                "</textarea><br /><input type=submit value=Update></form>"
                "</div> <hr> <div><label>Online</label> <ul style=list-style: none;>" +
                L3_getStringNodeList() +
                "</ul> </div> <hr> <div><label>Messages</label> <ul style=list-style: none;>" +
                message_getStringMessageList() +
                "</ul> </div> <div> <form action=/refresh method=post><input type=submit value=Refresh></form> </div> <hr>"
                "<div> <form action=/send method=post><br /><label>Recipient</label><textarea name=message rows=1>" +
                String(recipient) +
                "</textarea><br /><label>Send new message</label>"
                "<textarea name=message></textarea><br /><input type=submit value=Send></form> </div> </html>";
  return html;
}