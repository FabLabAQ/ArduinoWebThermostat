/******************************************************************************
 * Arduino Ethernet Thermostat                                                *
 * Copyright (C) 2015                                                         *
 * Luca Anastasio <anastasio.lu@gmail.com>                                    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 ******************************************************************************/

//----------- Pin and Constants Definitions -----------
//#define ENABLE_DEBUG
const uint8_t ONE_WIRE_BUS_PIN = 2;                    // pin connected to the temperature probes
const uint8_t ZONE_0_PIN = 6;                         // pin connected to the relay
const uint8_t ZONE_1_PIN = 7;
const uint8_t ZONE_2_PIN = 8;
const uint8_t ZONE_3_PIN = 9;
const uint8_t THERM_INCREMENTS = 5;                   // 0.50 °C, the increment in set temperature when clicking on + or -
const uint8_t eeprom_address_0 = 0;
const uint8_t eeprom_address_1 = 3;
const uint8_t eeprom_address_2 = 6;
const uint8_t eeprom_address_3 = 9;

//----------- One Wire bus declaration and probes addresses ------------
#include "OneWire.h"
#include "DallasTemperature.h"
OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature *temp_sensors = new DallasTemperature(&oneWire);              // poiter to the temperature sensors object, 
DeviceAddress probe_0 = { 0x28, 0xFF, 0x6A, 0xAD, 0xA1, 0x15, 0x03, 0x7C };     // to be passed in to the thermostat objects, requires OneWire object
DeviceAddress probe_1 = { 0x28, 0xFF, 0xA1, 0xAA, 0x91, 0x15, 0x04, 0xF5 };     // use an address finder sketch to find each probe address
DeviceAddress probe_2 = { 0x28, 0xFF, 0x6A, 0xAD, 0xA1, 0x15, 0x03, 0x7C };
DeviceAddress probe_3 = { 0x28, 0xFF, 0xA1, 0xAA, 0x91, 0x15, 0x04, 0xF5 };

//--------- Web initialization ----------
#include "TimeLib.h"
#include "SPI.h"
#include "Ethernet.h"
#include "EthernetUdp.h"
#define WEBDUINO_AUTH_REALM ""                   // message shown at the web authentication prompt
#define WEBDUINO_FAVICON_DATA ""
#include "WebServer.h"

static uint8_t arduino_mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };      // set the ethernet shield MAC address here
static uint8_t arduino_ip[] = { 192, 168, 0, 127 };                         // set the static IP address here
//static char ntp_server_address[] = { 62, 149, 204, 69 };                     // ntp.inrim.it
static char ntp_server_address[] = "ntp.ien.it";                        // ntp2.inrim.it
const int8_t time_zone = 1;
const uint8_t ntp_port = 123;
const int NTP_PACKET_SIZE = 48;
uint8_t ntp_packet_buffer[NTP_PACKET_SIZE];

EthernetUDP udp_conn;
#define WEBDUINO_PREFIX "/thermostat"                                       // the page will be available at: http://192.168.0.127/thermostat
WebServer webserver(WEBDUINO_PREFIX, 80);                                   // webserver object declaration with listening port (80)
#define WEBDUINO_CREDENTIALS "YWRtaW46YWRtaW4="                             // admin:admin username and password pair, encoded in base64
// P(title_str) = "Arduino Web Thermostat ";

//-------------------------- Thermostat objects declaration -------------------------     // thermostat class
#include "EEPROM.h"
#define THERMOSTAT_RESOLUTION 9                   // 9=0.5°C, 10=0.25°C, 11=0.125°C, 12=1/16°C resolution of the temperature probes readings, default to 9
#define THERMOSTAT_RANGE 2                        // 0.20 °C, the range around the actual temperature in which the thermostat will be acting, default to 1
#include "thermostat.h"                                                                   // parameters passed to the constructor:
thermostat *zone_0 = new thermostat(ZONE_0_PIN, temp_sensors, probe_0, eeprom_address_0);     // pin connected to the relay, temperature sensors object pointer, 
thermostat *zone_1 = new thermostat(ZONE_1_PIN, temp_sensors, probe_1, eeprom_address_1);     // probe address, desired reading resolution, activation temperature range.
thermostat *zone_2 = new thermostat(ZONE_2_PIN, temp_sensors, probe_2, eeprom_address_2);     // e.g. when measured temp goes under (set_temp - range) the thermostat will be activated,
thermostat *zone_3 = new thermostat(ZONE_3_PIN, temp_sensors, probe_3, eeprom_address_3);     // when measured temp goes over (set_temp + range) the thermostat will be deactivated
const uint8_t num_thermostats = 4;
thermostat *thermostats[num_thermostats] = { zone_0, zone_1, zone_2, zone_3 };

unsigned long prevMillis;       // variable to store time elapsed since the last time thermostats were updated

void print_thermostat_page()
{
  P(degC) = "&deg;C";
  P(br) = "<br>";

  webserver.print("<html><head><body>");
  // webserver.print("<html><head><title>");
  // webserver.printP(title_str);
  // webserver.print("</title><body>");
  // webserver.printP(title_str);
  webserver.print(hour());
  webserver.print(":");
  if (minute() < 10)
  {
    webserver.print("0");
  }
  webserver.print(minute());

  webserver.printP(br);
  webserver.printP(br);

  for (uint8_t i=0; i<num_thermostats; i++)
  {
      char name = i + '0';

      webserver.print("Zone ");
      webserver.print(i);
      if(thermostats[i]->get_status())
        webserver.print(" ON");
      else webserver.print(" OFF");
      
      webserver.printP(br);

      webserver.print("actual temp: ");
      webserver.print(thermostats[i]->get_actual_temp());
      webserver.printP(degC);

      webserver.printP(br);

      P(form_start) = "<form action='/thermostat' method='POST'>set: ";
      webserver.printP(form_start);

      webserver.print_button(name, "-", "-");
      webserver.print(thermostats[i]->get_set_temp());
      webserver.printP(degC);
      webserver.print_button(name, "+", "+");

      webserver.printP(br);

      webserver.print(" on hour: ");
      webserver.print_button(name, "0", "-");
      webserver.print(thermostats[i]->get_on_hour());
      webserver.print(":00 ");
      webserver.print_button(name, "1", "+");

      webserver.printP(br);

      webserver.print("off hour: ");
      webserver.print_button(name, "2", "-");
      webserver.print(thermostats[i]->get_off_hour());
      webserver.print(":00 ");
      webserver.print_button(name, "3", "+");

      webserver.print("</form>");
  }

  webserver.print("</body></html>");
}

//------------------------------ thermostat html page and POST handler routine ------------------------------------
void thermostat_page_cmd(WebServer &webserver, WebServer::ConnectionType type, char *, bool)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[2], value[2];

    do
    {
      repeat = webserver.readPOSTparam(name, 2, value, 2);

      uint8_t i = name[0] - '0';

      switch (value[0])
      {
          case ('+'): thermostats[i]->change_temp(THERM_INCREMENTS); break;
          case ('-'): thermostats[i]->change_temp(-THERM_INCREMENTS); break;
          case ('0'): thermostats[i]->change_on_hour(-1); break;
          case ('1'): thermostats[i]->change_on_hour(1); break;
          case ('2'): thermostats[i]->change_off_hour(-1); break;
          case ('3'): thermostats[i]->change_off_hour(1); break;
          default: break;
      }

    } while (repeat);

    webserver.httpSeeOther(WEBDUINO_PREFIX);

    return;
  }

  if (webserver.checkCredentials(WEBDUINO_CREDENTIALS))
  {
    webserver.httpSuccess();
    if (type != WebServer::HEAD)
    { 
      print_thermostat_page();
    }
  }
  else
  {
    webserver.httpUnauthorized();
  }
}

time_t getNTPtime()
{
  while (udp_conn.parsePacket() > 0) ; // discard any previously received packets
  sendNTPpacket(ntp_server_address);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp_conn.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      udp_conn.read(ntp_packet_buffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900; // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)ntp_packet_buffer[40] << 24;
      secsSince1900 |= (unsigned long)ntp_packet_buffer[41] << 16;
      secsSince1900 |= (unsigned long)ntp_packet_buffer[42] << 8;
      secsSince1900 |= (unsigned long)ntp_packet_buffer[43];
      return secsSince1900 - 2208988800UL + time_zone * SECS_PER_HOUR;
    }
  }
  return 0; // return 0 if unable to get the time
}

void sendNTPpacket(char* address)    // send an NTP request to the time server at the given address
{
  // set all bytes in the buffer to 0
  memset(ntp_packet_buffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  ntp_packet_buffer[0] = 0b11100011;   // LI, Version, Mode
  ntp_packet_buffer[1] = 0;     // Stratum, or type of clock
  ntp_packet_buffer[2] = 6;     // Polling Interval
  ntp_packet_buffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  ntp_packet_buffer[12]  = 49;
  ntp_packet_buffer[13]  = 0x4E;
  ntp_packet_buffer[14]  = 49;
  ntp_packet_buffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  udp_conn.beginPacket(address, ntp_port); //NTP requests are to port 123
  udp_conn.write(ntp_packet_buffer, NTP_PACKET_SIZE);
  udp_conn.endPacket();
}

void setup()
{
  Ethernet.begin(arduino_mac, arduino_ip);
  temp_sensors->begin();
  for (uint8_t i=0; i<num_thermostats; i++)
    {
      thermostats[i]->begin();
      // thermostats[i]->set_temp(200);
    }
  webserver.setDefaultCommand(& thermostat_page_cmd);
  webserver.begin();
  udp_conn.begin(ntp_port);
  while (getNTPtime() == 0)      // to prevent unwanted activation based on wrong time
  {
    delay(1000);
  }
  setSyncProvider(getNTPtime);
}

void loop()
{
  char connection_buffer[64];
  int buffer_lenght = 64;
  webserver.processConnection(connection_buffer, &buffer_lenght);
  
  if (millis() > prevMillis+1000)
  {
    for (uint8_t i=0; i<num_thermostats; i++)
    {
      thermostats[i]->run();
    }
    prevMillis = millis();
  }

}