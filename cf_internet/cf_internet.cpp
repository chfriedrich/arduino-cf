// enable debug output over Serial.print
#define DEBUG 2

#include "debugUtils.h"
#include "internet.h"
#include <ArduinoJson.h>

// For Network connection
#ifdef WIFI_NINA
  #include <SPI.h>
  #include <WiFiNINA.h>
  WiFiClient client;
  const char* ssid = WIFI_SSID;
  const char* pass = WIFI_PASS;
  int status = WL_IDLE_STATUS;
#endif

#ifdef ETHERNET_W5500
  #include <SPI.h>
  #include <Ethernet.h>
  EthernetClient client;
  uint8_t mac[6] = { MAC6, MAC5, MAC4, MAC3, MAC2, MAC1 };
#endif

#ifdef GSMGPRS
  #include <MKRGSM.h>
  GSMClient client;
  GPRS gprs;
  GSM gsmAccess;
  GSMModem modemTest;
#endif


// For HTTP calls
const char* host = HOSTNAME;
const char* sensorScript = SENSORSCRIPT;
const char* controlScript = CONTROLSCRIPT;
String snr = "";

// For Json encoding and decoding
DynamicJsonDocument doc_sensorrequest(JSON_SENSOR_BUFFER_SIZE);


//######################################################################################################

int internet_init()
{
  #ifdef WIFI_NINA
    // check for the WiFi module:
    if (WiFi.status() == WL_NO_MODULE) {
      DEBUG2_PRINTLN("Communication with WiFi module failed!");
      // don't continue
      while (true);
    }
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    {
      DEBUG2_PRINTLN("Please upgrade the firmware");
    }
    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED)
    {
      DEBUG2_PRINT("Attempting to connect to WPA SSID: ");
      DEBUG2_PRINTLN(ssid);
      // Connect to WPA/WPA2 network:
      status = WiFi.begin(ssid, pass);
      // wait 5 seconds for connection:
      delay(5000);
    }
    byte mac[6];
    WiFi.macAddress(mac);
    snr = hex2string(mac[2])+hex2string(mac[1])+hex2string(mac[0]);
    // you're connected now, so print out the data:
    DEBUG2_PRINT("You're connected to the network");
    internet_printstatus(); 
  #endif

  #ifdef ETHERNET_W5500
    Ethernet.init(5);
    // start the Ethernet connection:
    DEBUG2_PRINTLN("Initialize Ethernet with DHCP:");
    if (Ethernet.begin(mac) == 0)
    {
      DEBUG2_PRINTLN("Failed to configure Ethernet using DHCP");
    }
    else
    {
      DEBUG2_PRINT("DHCP success");
    }
    // give the Ethernet shield a second to initialize:
    delay(1000);
    snr = hex2string(mac[2])+hex2string(mac[1])+hex2string(mac[0]);
    DEBUG2_PRINT("You're connected to the network");
    internet_printstatus(); 
  #endif

  #ifdef GSMGPRS
    DEBUG2_PRINTLN("Initialize GSM Connection.");
    // connection state
    bool connected = false;
  
    // After starting the modem with GSM.begin()
    // attach the shield to the GPRS network with the APN, login and password
    while (!connected)
    {
      if ((gsmAccess.begin(PINCODE) == GSM_READY) && (gprs.attachGPRS(GPRS_APN, "", "") == GPRS_READY))
      {
        connected = true;
      }
      else
      {
        DEBUG2_PRINTLN("Not connected");
        delay(1000);
      }
    }
    DEBUG2_PRINT("Modem IMEI: ");
    String imei = modemTest.getIMEI();
    imei.replace("\n", "");
    if (imei != NULL)
    {
      DEBUG2_PRINTLN(imei);
      snr = imei.substring(0,6);
      DEBUG2_PRINTLN("SNR: " + snr);
    }
    
  #endif

  return 1;
}

//######################################################################################################

void internet_printstatus()
{
  #ifdef WIFI_NINA
    DEBUG2_PRINTLN("Connected to Wifi");
    DEBUG2_PRINTLN("");
    DEBUG2_PRINTLN("");
    
    // print the SSID of the network you're attached to:
    DEBUG2_PRINT("SSID: ");
    DEBUG2_PRINTLN(WiFi.SSID());
  
    // print the MAC address of the router you're attached to:
    byte bssid[6];
    WiFi.BSSID(bssid);
    DEBUG2_PRINT("Router BSSID: ");
    printMacAddress(bssid);
  
    // print the received signal strength:
    long rssi = WiFi.RSSI();
    DEBUG2_PRINT("WiFi signal strength (RSSI):");
    DEBUG2_PRINTLN(rssi);
  
    // print the encryption type:
    byte encryption = WiFi.encryptionType();
    DEBUG2_PRINT("Encryption Type:");
    DEBUG2_PRINTHEX(encryption);
    DEBUG2_PRINTLN();

    DEBUG2_PRINTLN("");
    DEBUG2_PRINTLN("Local Data");
    // print your board's IP address:
    IPAddress ip = WiFi.localIP();
    DEBUG2_PRINT("IP Address: ");
    DEBUG2_PRINTLN(ip);
    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    DEBUG2_PRINT("MAC Address: ");
    printMacAddress(mac);
    DEBUG2_PRINT("snr: ");
    DEBUG2_PRINTLN(snr);
  #endif

  #ifdef ETHERNET_W5500
    DEBUG2_PRINTLN("Connected to Ethernet");
    DEBUG2_PRINT("IP address: ");
    DEBUG2_PRINTLN(Ethernet.localIP());
    DEBUG2_PRINTLN("snr: "+ String(snr));
  #endif
}

//######################################################################################################

String get_wifi_snr()
{
  #ifdef WIFI_NINA
    byte mac[6];  
    WiFi.macAddress(mac);
    //snr = mac.substring(9, 11) + mac.substring(12, 14) + mac.substring(15, 17);
  
    return String(mac[2])+String(mac[1])+String(mac[0]);
  #endif
  return "";
}

//######################################################################################################

void jsonAppendPort(String snr_send, int portnr, long type, long av, long mi, long ma, String textdata)
{
  if(snr_send == "") snr_send=snr;
  
  JsonObject newObject = doc_sensorrequest.createNestedObject();
  newObject["snr"] = snr_send;
  newObject["port"] = portnr;
  newObject["type"] = type;
  newObject["avg"] = av;
  newObject["min"] = mi;
  newObject["max"] = ma;
  newObject["textdata"] = textdata;
  newObject["factor"] = 1000;  
}

//######################################################################################################

void jsonClearData()
{
  JsonArray arr = doc_sensorrequest.to<JsonArray>();
}

//######################################################################################################

int jsonSendDataGET()
{
  int write_successful_count = 0;
  
  DEBUG2_PRINTLN("sending sensor data to server: ");
  serializeJsonPretty(doc_sensorrequest, Serial);
  DEBUG2_PRINTLN();
  
  DEBUG2_PRINT("connecting to ");
  DEBUG2_PRINTLN(String(host));

  const int httpPort = 80;
  int connect_return = client.connect(host, httpPort);
  switch(connect_return)
  {
    case  1: DEBUG2_PRINTLN("success");                               break;
    case -1: DEBUG2_PRINTLN("connection failed (timed out)");         return -1;
    case -2: DEBUG2_PRINTLN("connection failed (invalid server)");    return -1;
    case -3: DEBUG2_PRINTLN("connection failed (truncated)");         return -1;
    case -4: DEBUG2_PRINTLN("connection failed (invalid response)");  return -1;
  }

  // We now create a URI for the request
  String req = "GET " + String(sensorScript);
  req += "?json=";
  serializeJson(doc_sensorrequest, req);
  req += " HTTP/1.1\r\n";
  req += "Host: " + String(host) + "\r\n";
  req += "Connection: close\r\n\r\n";

  // This will send the request to the server
  client.print(req);

  DEBUG2_PRINTLN("Sent Request:");
  DEBUG2_PRINTLN(req);
   
  unsigned long timeout = millis();
  while (client.available() == 0)
  {
      if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          return -1;
      }
  }

  // Read all the lines of the reply from server and print them to Serial
  
  while(client.available()) {
      String line = client.readStringUntil('\r');
      Serial.println(line);
  }
  return 1;

  DEBUG2_PRINTLN("received answere");
  DEBUG2_PRINTLN("closing connection");
  client.stop();
  
  delay(100);

  char content[HTTP_MAX_CONTENT_SIZE];
  size_t length = client.readBytes(content, HTTP_MAX_CONTENT_SIZE);
  content[length] = 0;

  DEBUG2_PRINTLN("");
  DEBUG2_PRINTLN("closing connection");
  client.stop();

  // remove characters before { and after }
  removeHeaders(content, HTTP_MAX_CONTENT_SIZE);
  
  DEBUG2_PRINTLN("answere from server, without headers:");
  DEBUG2_PRINTLN("---");
  DEBUG2_PRINTLN(content);
  DEBUG2_PRINTLN("---");
    
  // decode json
  DynamicJsonDocument doc_sensoranswere(JSON_SENSOR_BUFFER_SIZE);
  DeserializationError error = deserializeJson(doc_sensoranswere, content);
  // Test if parsing succeeds.
  if (error)
  {
    DEBUG2_PRINT("deserializeJson() failed: ");
    DEBUG2_PRINTLN(error.c_str());
    return -1;
  }
  DEBUG2_PRINT("deserializeJson() success:");
  serializeJsonPretty(doc_sensoranswere, Serial);
  DEBUG2_PRINTLN("");
  DEBUG2_PRINTLN("doc size: " + String(doc_sensoranswere.size()));
  for(int i=0; i<doc_sensoranswere.size(); i++)
  {
    if(doc_sensoranswere[i]["write_successful"].as<long>() == (long)1)
      write_successful_count++;
  }
  DEBUG2_PRINTLN("write_successful_count = " + String(write_successful_count));
  
  return write_successful_count;
}

//######################################################################################################

int jsonSendData()
{
  int write_successful_count = 0;
  
  DEBUG2_PRINTLN("sending sensor data to server: ");
  serializeJsonPretty(doc_sensorrequest, Serial);
  DEBUG2_PRINTLN();
  
  DEBUG2_PRINT("connecting to ");
  DEBUG2_PRINTLN(host);

  const int httpPort = 80;
  int connect_return = client.connect(host, httpPort);
  switch(connect_return)
  {
    case  1: DEBUG2_PRINTLN("success");                               break;
    case -1: DEBUG2_PRINTLN("connection failed (timed out)");         return -1;
    case -2: DEBUG2_PRINTLN("connection failed (invalid server)");    return -1;
    case -3: DEBUG2_PRINTLN("connection failed (truncated)");         return -1;
    case -4: DEBUG2_PRINTLN("connection failed (invalid response)");  return -1;
  }

  client.print("POST ");
  client.print(sensorScript);
  client.println(" HTTP/1.1");
  client.print("Host:  ");
  client.println(host);
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: close");
  client.println("Content-type: application/json");
  client.print("Content-Length: ");
  client.println(measureJson(doc_sensorrequest));
  client.println();
  serializeJson(doc_sensorrequest, client);

  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > HTTP_TIMEOUT_MS)
    {
      DEBUG2_PRINTLN(">>> Client Timeout !");
      client.stop();
      return -1;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  
  delay(100);

  char content[HTTP_MAX_CONTENT_SIZE];
  size_t length = client.readBytes(content, HTTP_MAX_CONTENT_SIZE);
  content[length] = 0;

  DEBUG2_PRINTLN("");
  DEBUG2_PRINTLN("closing connection");
  client.stop();

  // remove characters before { and after }
  removeHeaders(content, HTTP_MAX_CONTENT_SIZE);
  
  DEBUG2_PRINTLN("answere from server, without headers:");
  DEBUG2_PRINTLN("---");
  DEBUG2_PRINTLN(content);
  DEBUG2_PRINTLN("---");
    
  // decode json
  DynamicJsonDocument doc_sensoranswere(JSON_SENSOR_BUFFER_SIZE);
  DeserializationError error = deserializeJson(doc_sensoranswere, content);
  // Test if parsing succeeds.
  if (error)
  {
    DEBUG2_PRINT("deserializeJson() failed: ");
    DEBUG2_PRINTLN(error.c_str());
    return -1;
  }
  DEBUG2_PRINT("deserializeJson() success:");
  serializeJsonPretty(doc_sensoranswere, Serial);
  DEBUG2_PRINTLN("");
  DEBUG2_PRINTLN("doc size: " + String(doc_sensoranswere.size()));
  for(int i=0; i<doc_sensoranswere.size(); i++)
  {
    if(doc_sensoranswere[i]["write_successful"].as<long>() == (long)1)
      write_successful_count++;
  }
  DEBUG2_PRINTLN("write_successful_count = " + String(write_successful_count));
  
  return write_successful_count;
}

//######################################################################################################

void printMacAddress(byte mac[])
{
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      DEBUG2_PRINT("0");
    }
    DEBUG2_PRINTHEX(mac[i]);
    if (i > 0) {
      DEBUG2_PRINT(":");
    }
  }
  DEBUG2_PRINTLN();
}

//######################################################################################################

String hex2string(byte hex)
{
  String s = "";
  switch (hex / 16)
  {
    case 0: s += "0"; break;
    case 1: s += "1"; break;
    case 2: s += "2"; break;
    case 3: s += "3"; break;
    case 4: s += "4"; break;
    case 5: s += "5"; break;
    case 6: s += "6"; break;
    case 7: s += "7"; break;
    case 8: s += "8"; break;
    case 9: s += "9"; break;
    case 10: s += "A"; break;
    case 11: s += "B"; break;
    case 12: s += "C"; break;
    case 13: s += "D"; break;
    case 14: s += "E"; break;
    case 15: s += "F"; break;
    default: break;
  }
  switch (hex % 16)
  {
    case 0: s += "0"; break;
    case 1: s += "1"; break;
    case 2: s += "2"; break;
    case 3: s += "3"; break;
    case 4: s += "4"; break;
    case 5: s += "5"; break;
    case 6: s += "6"; break;
    case 7: s += "7"; break;
    case 8: s += "8"; break;
    case 9: s += "9"; break;
    case 10: s += "A"; break;
    case 11: s += "B"; break;
    case 12: s += "C"; break;
    case 13: s += "D"; break;
    case 14: s += "E"; break;
    case 15: s += "F"; break;
    default: break;
  }
  return s;
}

//######################################################################################################

int askServerJsonGet(String snr_send, byte nrOfPorts, byte controlPort[], float soll[], int modus[], int anw[])
{
  //DEBUG2_PRINTLN("creating json data");
  DynamicJsonDocument doc_controlrequest(JSON_CONTROL_BUFFER_SIZE);

  if(snr_send == "") snr_send=snr;

  for(int i=0; i<nrOfPorts; i++)
  {
    JsonObject newObject = doc_controlrequest.createNestedObject();
    newObject["snr"] = snr_send;
    newObject["port"] = controlPort[i];
  }
  
  DEBUG2_PRINTLN("asking server for configuration:");
  
#ifdef DEBUG
#if DEBUG ==2
  serializeJsonPretty(doc_controlrequest, Serial);
  DEBUG2_PRINTLN();
#endif
#endif
  
  //DEBUG2_PRINT("connecting to ");
  //DEBUG2_PRINTLN(host);

  const int httpPort = 80;
  if (!client.connect(host, httpPort))
  {
    DEBUG2_PRINTLN("connection failed");
    return -1;
  }

  client.print("POST ");
  client.print(controlScript);
  client.println(" HTTP/1.1");
  client.print("Host:  ");
  client.println(host);
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: close");
  client.println("Content-type: application/json");
  client.print("Content-Length: ");
  client.println(measureJson(doc_controlrequest));
  client.println();
  serializeJson(doc_controlrequest, client);

  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > HTTP_TIMEOUT_MS)
    {
      DEBUG2_PRINTLN(">>> Client Timeout !");
      client.stop();
      return -1;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  
  delay(100);

  char content[HTTP_MAX_CONTENT_SIZE];
  size_t length = client.readBytes(content, HTTP_MAX_CONTENT_SIZE);
  content[length] = 0;

  DEBUG2_PRINTLN("");
  DEBUG2_PRINTLN("closing connection");
  client.stop();

  // remove characters before [ and after ]
  removeHeaders(content, HTTP_MAX_CONTENT_SIZE);
  
  DEBUG2_PRINTLN("answere from server, without headers:");
  DEBUG2_PRINTLN("---");
  DEBUG2_PRINTLN(content);
  DEBUG2_PRINTLN("---");
    
  // decode json
  DynamicJsonDocument doc_controlanswere(JSON_CONTROL_BUFFER_SIZE);
  DeserializationError error = deserializeJson(doc_controlanswere, content);
  // Test if parsing succeeds.
  if (error)
  {
    DEBUG2_PRINT("deserializeJson() failed: ");
    DEBUG2_PRINTLN(error.c_str());
    return -1;
  }
  DEBUG2_PRINT("deserializeJson() success: ");

#ifdef DEBUG
#if DEBUG ==2
  serializeJsonPretty(doc_controlanswere, Serial);
#endif
#endif

  DEBUG2_PRINTLN("");
  //DEBUG2_PRINTLN("doc size: " + String(doc_controlanswere.size()));
  
  for(int i=0; i<doc_controlanswere.size(); i++)
  {
    
    if (String(doc_controlanswere[i]["mode"].as<char*>()) == "auto")     modus[i] = 2;
    else if (String(doc_controlanswere[i]["mode"].as<char*>()) == "ein")  modus[i] = 1;
    else if (String(doc_controlanswere[i]["mode"].as<char*>()) == "aus") modus[i] = 0;  
    
    soll[i] = doc_controlanswere[i]["wert"].as<long>();
    anw[i] = doc_controlanswere[i]["anwesend"].as<long>();
  }
  return 1;
}


//######################################################################################################

int askServerJson(String snr_send, byte nrOfPorts, byte controlPort[], float soll[], int modus[], int anw[])
{
  //DEBUG2_PRINTLN("creating json data");
  DynamicJsonDocument doc_controlrequest(JSON_CONTROL_BUFFER_SIZE);

  if(snr_send == "") snr_send=snr;

  for(int i=0; i<nrOfPorts; i++)
  {
    JsonObject newObject = doc_controlrequest.createNestedObject();
    newObject["snr"] = snr_send;
    newObject["port"] = controlPort[i];
  }
  
  DEBUG2_PRINTLN("asking server for configuration:");
  
#ifdef DEBUG
#if DEBUG ==2
  serializeJsonPretty(doc_controlrequest, Serial);
  DEBUG2_PRINTLN();
#endif
#endif
  
  //DEBUG2_PRINT("connecting to ");
  //DEBUG2_PRINTLN(host);

  const int httpPort = 80;
  if (!client.connect(host, httpPort))
  {
    DEBUG2_PRINTLN("connection failed");
    return -1;
  }

  client.print("POST ");
  client.print(controlScript);
  client.println(" HTTP/1.1");
  client.print("Host:  ");
  client.println(host);
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: close");
  client.println("Content-type: application/json");
  client.print("Content-Length: ");
  client.println(measureJson(doc_controlrequest));
  client.println();
  serializeJson(doc_controlrequest, client);

  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > HTTP_TIMEOUT_MS)
    {
      DEBUG2_PRINTLN(">>> Client Timeout !");
      client.stop();
      return -1;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  
  delay(100);

  char content[HTTP_MAX_CONTENT_SIZE];
  size_t length = client.readBytes(content, HTTP_MAX_CONTENT_SIZE);
  content[length] = 0;

  DEBUG2_PRINTLN("");
  DEBUG2_PRINTLN("closing connection");
  client.stop();

  // remove characters before [ and after ]
  removeHeaders(content, HTTP_MAX_CONTENT_SIZE);
  
  DEBUG2_PRINTLN("answere from server, without headers:");
  DEBUG2_PRINTLN("---");
  DEBUG2_PRINTLN(content);
  DEBUG2_PRINTLN("---");
    
  // decode json
  DynamicJsonDocument doc_controlanswere(JSON_CONTROL_BUFFER_SIZE);
  DeserializationError error = deserializeJson(doc_controlanswere, content);
  // Test if parsing succeeds.
  if (error)
  {
    DEBUG2_PRINT("deserializeJson() failed: ");
    DEBUG2_PRINTLN(error.c_str());
    return -1;
  }
  DEBUG2_PRINT("deserializeJson() success: ");

#ifdef DEBUG
#if DEBUG ==2
  serializeJsonPretty(doc_controlanswere, Serial);
#endif
#endif

  DEBUG2_PRINTLN("");
  //DEBUG2_PRINTLN("doc size: " + String(doc_controlanswere.size()));
  
  for(int i=0; i<doc_controlanswere.size(); i++)
  {
    
    if (String(doc_controlanswere[i]["mode"].as<char*>()) == "auto")     modus[i] = 2;
    else if (String(doc_controlanswere[i]["mode"].as<char*>()) == "ein")  modus[i] = 1;
    else if (String(doc_controlanswere[i]["mode"].as<char*>()) == "aus") modus[i] = 0;  
    
    soll[i] = doc_controlanswere[i]["wert"].as<long>();
    anw[i] = doc_controlanswere[i]["anwesend"].as<long>();
  }
  return 1;
}

//######################################################################################################

int removeHeaders(char content[], int len)
{
  int s=-1; 
  int e=-1;
     
  for(int i=0; i<len; i++)
  {
    if(content[i]=='[')
    {
      s=i;
      break;
    }
  }
  for(int i=s; i<len; i++)
  { 
    if(content[i]==']')
    {
      e=i;
      break;
    }
  }

  if(s<0 || e<0)
    return 0;
  else
  {
    for(int i=0; i<=(e-s); i++)
    {
      content[i] = content[s+i];
    }
    content[e-s+1] = 0;
    return 1;
  }
}
