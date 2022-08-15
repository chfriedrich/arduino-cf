#ifndef internet_h
#define internet_h

/*------------------------------------------------------*/
/* CONFIGURATION */

// SELECT INTERFACE
// only define one of the following interfaces !
#define ETHERNET_W5500 1
//#define WIFI_NINA 1   // e.g. for Nano 33 iot
//#define WIFI_ESP32 1
//#define GSMGPRS 1


// -----------------------------------------------------
// FOR ETHERNET
// -----------------------------------------------------
// MAC address for W5500 ethernet module
// This is only used if you use the W5500 shield
// All other network modules have a unique MAC readable by software
// MAC1 : MAC2 : MAC3 : MAC4 : MAC5 : MAC6
#define MAC1 0x00
#define MAC2 0x08
#define MAC3 0xDC
#define MAC4 0x51
#define MAC5 0x39
#define MAC6 0x46


// -----------------------------------------------------
// FOR WIFI
// -----------------------------------------------------
// set wifi ssid and password
// for christoph
#define WIFI_SSID "iot"
#define WIFI_PASS "Muckendorf1"


// -----------------------------------------------------
// FOR GSM
// -----------------------------------------------------

#define PINCODE ""
#define GPRS_APN "webapn.at"

// -----------------------------------------------------
// GENERAL
// -----------------------------------------------------

// server hostname
#define HOSTNAME "chfr.000webhostapp.com"

// server scripts for communication
//#define SENSORSCRIPT "/writeJsonPost.php"
#define SENSORSCRIPT "/writeJsonGet.php"
#define CONTROLSCRIPT "/thermostatJson.php"

// timeout and max. size for http requests
#define HTTP_TIMEOUT_MS 5000
#define HTTP_MAX_CONTENT_SIZE 4096

// json buffer sizes
#define JSON_SENSOR_BUFFER_SIZE 2048 // must be quite large for higher port numbers
#define JSON_CONTROL_BUFFER_SIZE 2048

/*------------------------------------------------------*/

#include "Arduino.h"

void jsonAppendPort(String snr_send, int portnr, long type, long av, long mi, long ma, String textdata);
void jsonClearData();
int jsonSendData();
int jsonSendDataGET();
int internet_init();
void internet_printstatus();
//int askServer(byte controlPort, float *soll, int *modus, int *anw);
int askServerJson(String snr_send, byte nrOfPorts, byte controlPort[], float soll[], int modus[], int anw[]);
int askServerJsonGet(String snr_send, byte nrOfPorts, byte controlPort[], float soll[], int modus[], int anw[]);
void printMacAddress(byte mac[]);
String hex2string(byte hex);
int removeHeaders(char content[], int len);

#endif
