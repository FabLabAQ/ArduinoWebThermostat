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
const uint8_t ONE_WIRE_BUS_PIN = 2;
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
DallasTemperature *temp_sensors = new DallasTemperature(&oneWire);              // poiter to the temperature sensors manager, to be passed in to the thermostat objects, requires OneWire object
DeviceAddress probe01 = { 0x28, 0xFF, 0x6A, 0xAD, 0xA1, 0x15, 0x03, 0x7C };
DeviceAddress probe02 = { 0x28, 0xFF, 0xA1, 0xAA, 0x91, 0x15, 0x04, 0xF5 };
DeviceAddress probe03 = { 0x28, 0xFF, 0x6A, 0xAD, 0xA1, 0x15, 0x03, 0x7C };
DeviceAddress probe04 = { 0x28, 0xFF, 0xA1, 0xAA, 0x91, 0x15, 0x04, 0xF5 };

//--------- Web initialization ----------
#include "SPI.h"
#include "Ethernet.h"
#define WEBDUINO_AUTH_REALM "Arduino Ethernet Thermostat"                   // message shown at the web authentication prompt
#include "WebServer.h"
static uint8_t arduino_mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };      // set the ethernet shield MAC address here
static uint8_t arduino_ip[] = { 192, 168, 0, 127 };                         // set the static IP address here
#define WEBDUINO_PREFIX "/thermostat"                                       // the page will be available at: http://192.168.0.127/thermostat
WebServer webserver(WEBDUINO_PREFIX, 80);                                   // webserver object declaration with listening port (80)
#define WEBDUINO_CREDENTIALS "YWRtaW46YWRtaW4="                             // admin:admin username and password pair, encoded in base64

//-------------------------- Thermostat objects declaration -------------------------
#include "thermostat.h"                                                                   // thermostat class
thermostat zone01(ZONE_01_PIN, temp_sensors, probe01, THERM_RESOLUTION, THERM_RANGE);     // parameters passed to the constructor: 
thermostat zone02(ZONE_02_PIN, temp_sensors, probe02, THERM_RESOLUTION, THERM_RANGE);     // pin connected to the relay, temperature sensors object pointer, probe address, desired reading resolution, activation temperature range,
thermostat zone03(ZONE_03_PIN, temp_sensors, probe03, THERM_RESOLUTION, THERM_RANGE);     // e.g. when measured temp goes under (set_temp - range) the thermostat will be activated,
thermostat zone04(ZONE_04_PIN, temp_sensors, probe04, THERM_RESOLUTION, THERM_RANGE);     // when measured temp goes over (set_temp + range) the thermostat will be deactivated

unsigned long prevMillis;       // variable to store time elapsed since the last time thermostats were updated

//------------------------------ thermostat html page and POST handler routine ------------------------------------
void thermostat_page(WebServer &webserver, WebServer::ConnectionType type, char *, bool)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[2], value[2];
    do
    {
      repeat = webserver.readPOSTparam(name, 2, value, 2);
      if (!strcmp(name, "1"))
      {
        if (!strcmp(value, "+"))
        {
          zone01.increase_temp(THERM_INCREMENTS);
          #ifdef ENABLE_DEBUG
          Serial.println("zone01 increase_temp");
          #endif
        }
        else if (!strcmp(value, "-"))
        {
          zone01.decrease_temp(THERM_INCREMENTS);
          #ifdef ENABLE_DEBUG
          Serial.println("zone01 decrease_temp");
          #endif
        }
      }
      else if (!strcmp(name, "2"))
      {
        if (!strcmp(value, "+"))
        {
          zone02.increase_temp(THERM_INCREMENTS);
          #ifdef ENABLE_DEBUG
          Serial.println("zone02 increase_temp");
          #endif
        }
        else if (!strcmp(value, "-"))
        {
          zone02.decrease_temp(THERM_INCREMENTS);
          #ifdef ENABLE_DEBUG
          Serial.println("zone02 decrease_temp");
          #endif
        }
      }
      else if (!strcmp(name, "3"))
      {
        if (!strcmp(value, "+"))
        {
          zone03.increase_temp(THERM_INCREMENTS);
          #ifdef ENABLE_DEBUG
          Serial.println("zone03 increase_temp");
          #endif
        }
        else if (!strcmp(value, "-"))
        {
          zone03.decrease_temp(THERM_INCREMENTS);
          #ifdef ENABLE_DEBUG
          Serial.println("zone03 decrease_temp");
          #endif
        }
      }
      else if (!strcmp(name, "4"))
      {
        if (!strcmp(value, "+"))
        {
          zone04.increase_temp(THERM_INCREMENTS);
          #ifdef ENABLE_DEBUG
          Serial.println("zone04 increase_temp");
          #endif
        }
        else if (!strcmp(value, "-"))
        {
          zone04.decrease_temp(THERM_INCREMENTS);
          #ifdef ENABLE_DEBUG
          Serial.println("zone04 decrease_temp");
          #endif
        }
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
      webserver.print("<html><head><title>Arduino Web Thermostat</title><body><span style=\"font-family: verdana; font-size: 16px; color: #000000;\"><b>Arduino Web Thermostat</b><br><br><b>Zone 01 ");
      if(zone01.get_status())
        webserver.print("ON");
      else webserver.print("OFF");
      webserver.print("</b><br>actual temp: <b>");
      webserver.print(zone01.get_actual_temp());
      webserver.print("&deg;C</b><br><form action='/thermostat' method='POST'>set: <button name='1' value='-'><b>-</b></button> ");
      webserver.print(zone01.get_temp());
      webserver.print("&deg;C <button name='1' value='+'><b>+</b></button></form>");
      webserver.print("<br><b>Zone 02 ");
      if(zone02.get_status())
        webserver.print("ON");
      else webserver.print("OFF");
      webserver.print("</b><br>actual temp: <b>");
      webserver.print(zone02.get_actual_temp());
      webserver.print("&deg;C</b><br><form action='/thermostat' method='POST'>set: <button name='2' value='-'><b>-</b></button> ");
      webserver.print(zone02.get_temp());
      webserver.print("&deg;C <button name='2' value='+'><b>+</b></button></form>");
      webserver.print("<br><b>Zone 03 ");
      if(zone03.get_status())
        webserver.print("ON");
      else webserver.print("OFF");
      webserver.print("</b><br>actual temp: <b>");
      webserver.print(zone03.get_actual_temp());
      webserver.print("&deg;C</b><br><form action='/thermostat' method='POST'>set: <button name='3' value='-'><b>-</b></button> ");
      webserver.print(zone03.get_temp());
      webserver.print("&deg;C <button name='3' value='+'><b>+</b></button></form>");
      webserver.print("<br><b>Zone 04 ");
      if(zone04.get_status())
        webserver.print("ON");
      else webserver.print("OFF");
      webserver.print("</b><br>actual temp: <b>");
      webserver.print(zone04.get_actual_temp());
      webserver.print("&deg;C</b><br><form action='/thermostat' method='POST'>set: <button name='4' value='-'><b>-</b></button> ");
      webserver.print(zone04.get_temp());
      webserver.print("&deg;C <button name='4' value='+'><b>+</b></button></form></span></body></html>");

    }
  }
  else
  {
    webserver.httpUnauthorized();
  }
}

void setup()
{
  #ifdef ENABLE_DEBUG
  Serial.begin(9600);
  Serial.println("Starting");
  #endif
  // temp_sensors.begin();
  // #ifdef ENABLE_DEBUG
  // Serial.println("temp_sensors initialization");
  // #endif
  zone01.begin();
  zone02.begin();
  zone03.begin();
  zone04.begin();
  #ifdef ENABLE_DEBUG
  Serial.println("zone01 initialization");
  #endif
  zone01.set_temp(20.00);
  zone02.set_temp(20.00);
  zone03.set_temp(20.00);
  zone04.set_temp(20.00);
  #ifdef ENABLE_DEBUG
  Serial.println("zone01 set_temp");
  #endif
  Ethernet.begin(arduino_mac, arduino_ip);
  #ifdef ENABLE_DEBUG
  Serial.println("Ethernet initialization");
  #endif
  webserver.setDefaultCommand(&thermostat_page);
  #ifdef ENABLE_DEBUG
  Serial.println("WebServer setDefaultCommand");
  #endif
  webserver.begin();
  #ifdef ENABLE_DEBUG
  Serial.println("WebServer begin");
  #endif
}

void loop()
{
  char connection_buffer[64];
  int buffer_lenght = 64;
  webserver.processConnection(connection_buffer, &buffer_lenght);
  if (millis() > prevMillis+1000)
  {
    zone01.run();
    zone02.run();
    zone03.run();
    zone04.run();
    prevMillis = millis();
    #ifdef ENABLE_DEBUG
    Serial.println("zone01 run");
    #endif
  }
}