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

//----------- Pin Definitions -----------
//#define ENABLE_DEBUG
#define ONE_WIRE_BUS_PIN 2
#define ZONE_01_PIN 9
#define THERM_RESOLUTION 10       // 10: 0.25 °C, 9: 0.5 °C
const float THERM_RANGE = 0.50;            // 0.50 °C
const float THERM_INCREMENTS = 0.25;       // 0.25 °C

//----------- One Wire bus initialization and probes addresses ------------
#include "OneWire.h"
#include "DallasTemperature.h"
OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature temp_sensors(&oneWire);
DeviceAddress Probe01 = { 0x28, 0xFF, 0x6A, 0xAD, 0xA1, 0x15, 0x03, 0x7C };

//--------- Web initialization ----------
#include "SPI.h"
#include "Ethernet.h"
#define WEBDUINO_AUTH_REALM "Arduino Ethernet Thermostat"
#include "WebServer.h"
static uint8_t arduino_mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static uint8_t arduino_ip[] = { 192, 168, 0, 127 };
#define WEBDUINO_PREFIX "/thermostat"
WebServer webserver(WEBDUINO_PREFIX, 80);
#define WEBDUINO_CREDENTIALS "YWRtaW46YWRtaW4="

#include "thermostat.h"
thermostat zone01(ZONE_01_PIN, temp_sensors, Probe01, THERM_RESOLUTION, THERM_RANGE);

unsigned long prevMillis;

void thermostat_page(WebServer &webserver, WebServer::ConnectionType type, char *, bool)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[11], value[2];
    do
    {
      repeat = webserver.readPOSTparam(name, 11, value, 2);
      if (strcmp(name, "thermostat") == 0)
      {
        if (strcmp(value, "+") == 0)
        {
          zone01.increase_temp(THERM_INCREMENTS);
          Serial.println("increase_temp");
        }
        else if (strcmp(value, "-") == 0)
        {
          zone01.decrease_temp(THERM_INCREMENTS);
          Serial.println("decrease_temp");
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
      webserver.print("&deg;C</b><br><form action='/thermostat' method='POST'>set: <button name='thermostat' value='-'><b>-</b></button> ");
      webserver.print(zone01.get_temp());
      webserver.print("&deg;C <button name='thermostat' value='+'><b>+</b></button></form></span></body></html>");
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
  temp_sensors.begin();
  #ifdef ENABLE_DEBUG
  Serial.println("temp_sensors initialization");
  #endif
  zone01.begin();
  #ifdef ENABLE_DEBUG
  Serial.println("zone01 initialization");
  #endif
  zone01.set_temp(20.50);
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
    prevMillis = millis();
    #ifdef ENABLE_DEBUG
    Serial.println("zone01 run");
    #endif
  }
}