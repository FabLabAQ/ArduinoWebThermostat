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
const uint8_t ZONE_01_PIN = 6;                         // pin connected to the relay
const uint8_t ZONE_02_PIN = 7;
const uint8_t ZONE_03_PIN = 8;
const uint8_t ZONE_04_PIN = 9;
const uint8_t THERM_RESOLUTION = 10;                   // 9=0.5°C, 10=0.25°C, 11=0.125°C, 12=1/16°C resolution of the temperature probes readings
const uint8_t THERM_RANGE = 25;                        // 0.25 °C, the range around the actual temperature in which the thermostat will be acting
const uint8_t THERM_INCREMENTS = 50;                   // 0.50 °C, the increment in set temperature when clicking on + or -

//----------- One Wire bus declaration and probes addresses ------------
#include "OneWire.h"
#include "DallasTemperature.h"
OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature *temp_sensors = new DallasTemperature(&oneWire);              // poiter to the temperature sensors object, 
DeviceAddress probe01 = { 0x28, 0xFF, 0x6A, 0xAD, 0xA1, 0x15, 0x03, 0x7C };     // to be passed in to the thermostat objects, requires OneWire object
DeviceAddress probe02 = { 0x28, 0xFF, 0xA1, 0xAA, 0x91, 0x15, 0x04, 0xF5 };     // use an address finder sketch to find each probe address
DeviceAddress probe03 = { 0x28, 0xFF, 0x6A, 0xAD, 0xA1, 0x15, 0x03, 0x7C };
DeviceAddress probe04 = { 0x28, 0xFF, 0xA1, 0xAA, 0x91, 0x15, 0x04, 0xF5 };

//--------- Web initialization ----------
#include "SPI.h"
#include "Ethernet.h"
#define WEBDUINO_AUTH_REALM "Arduino Ethernet Thermostat"                   // message shown at the web authentication prompt
#define WEBDUINO_FAVICON_DATA ""
#include "WebServer.h"
static uint8_t arduino_mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };      // set the ethernet shield MAC address here
static uint8_t arduino_ip[] = { 192, 168, 0, 127 };                         // set the static IP address here
#define WEBDUINO_PREFIX "/thermostat"                                       // the page will be available at: http://192.168.0.127/thermostat
WebServer webserver(WEBDUINO_PREFIX, 80);                                   // webserver object declaration with listening port (80)
#define WEBDUINO_CREDENTIALS "YWRtaW46YWRtaW4="                             // admin:admin username and password pair, encoded in base64
P(title_str) = "Arduino Web Thermostat";

//-------------------------- Thermostat objects declaration -------------------------     // thermostat class
#include "thermostat.h"                                                                   // parameters passed to the constructor:
thermostat *zone01 = new thermostat(ZONE_01_PIN, temp_sensors, probe01, THERM_RESOLUTION, THERM_RANGE);     // pin connected to the relay, temperature sensors object pointer, 
thermostat *zone02 = new thermostat(ZONE_02_PIN, temp_sensors, probe02, THERM_RESOLUTION, THERM_RANGE);     // probe address, desired reading resolution, activation temperature range.
thermostat *zone03 = new thermostat(ZONE_03_PIN, temp_sensors, probe03, THERM_RESOLUTION, THERM_RANGE);     // e.g. when measured temp goes under (set_temp - range) the thermostat will be activated,
thermostat *zone04 = new thermostat(ZONE_04_PIN, temp_sensors, probe04, THERM_RESOLUTION, THERM_RANGE);     // when measured temp goes over (set_temp + range) the thermostat will be deactivated
const uint8_t num_zone = 4;
thermostat *thermostats[num_zone] = { zone01, zone02, zone03, zone04 };

unsigned long prevMillis;       // variable to store time elapsed since the last time thermostats were updated

void print_thermostat_page()
{
  webserver.print("<html><head><title>");
  webserver.printP(title_str);
  webserver.print("</title><body>");
  webserver.printP(title_str);
  webserver.print("<br>");

  for (uint8_t i=0; i<num_zone; i++)
  {
      webserver.print("<br>Zone ");
      webserver.print(i);
      if(thermostats[i]->get_status())
        webserver.print(" ON");
      else webserver.print(" OFF");
      webserver.print("<br>actual temp: ");
      webserver.print(thermostats[i]->get_actual_temp());
      webserver.print("&deg;C<br><form action='/thermostat' method='POST'>set: <button name='");
      webserver.print(i);
      webserver.print("' value='-'>-</button> ");
      webserver.print(thermostats[i]->get_temp());
      webserver.print("&deg;C <button name='");
      webserver.print(i);
      webserver.print("' value='+'>+</button></form>");
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

      uint8_t i = name[0] - 48;

      if (!strcmp(value, "+"))
        {
          thermostats[i]->increase_temp(THERM_INCREMENTS);
        }
        else if (!strcmp(value, "-"))
        {
          thermostats[i]->decrease_temp(THERM_INCREMENTS);
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

void setup()
{
  for (uint8_t i=0; i<num_zone; i++)
    {
      thermostats[i]->begin();
      thermostats[i]->set_temp(20.00);
    }
  Ethernet.begin(arduino_mac, arduino_ip);
  webserver.setDefaultCommand(& thermostat_page_cmd);
  webserver.begin();
}

void loop()
{
  char connection_buffer[64];
  int buffer_lenght = 64;
  webserver.processConnection(connection_buffer, &buffer_lenght);
  if (millis() > prevMillis+1000)
  {
    for (uint8_t i=0; i<num_zone; i++)
    {
      thermostats[i]->run();
    }
    prevMillis = millis();
  }
}