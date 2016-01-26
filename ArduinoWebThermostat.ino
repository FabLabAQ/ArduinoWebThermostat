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
#define ONE_WIRE_BUS_PIN 2
#define ZONE_01_PIN 13
#define THERM_RESOLUTION 10       // 10: 0.25 °C, 9: 0.5 °C
#define THERM_RANGE 50            // 0.50 °C
#define THERM_INCREMENTS 25       // 0.25 °C

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
static uint8_t arduino_ip[] = { 192, 168, 0, 208 };
#define WEBDUINO_PREFIX "/thermostat"
WebServer webserver(WEBDUINO_PREFIX, 80);
#define WEBDUINO_CREDENTIALS "YWRtaW46YWRtaW4="

#include "thermostat.h"
thermostat zone01(ZONE_01_PIN, temp_sensors, Probe01, THERM_RESOLUTION, THERM_RANGE);

void thermostat_page(WebServer &webserver, WebServer::ConnectionType type, char *, bool)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[10], value[1];
    do
    {
      repeat = webserver.readPOSTparam(name, 16, value, 16);
      if (strcmp(name, "thermostat") == 0)
      {
        if (!strcmp(value, "+"))
          zone01.increase_temp(THERM_INCREMENTS);
        else if (!strcmp(value, "-"))
          zone01.decrease_temp(THERM_INCREMENTS);
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
      webserver.print("&deg;C</b><br><form action='/thermostat' method='POST'>set: <button name='zone01' value='-'><b>-</b></button> ");
      webserver.print(zone01.get_temp());
      webserver.print("&deg;C <button name='zone01' value='+'><b>+</b></button></form></span></body></html>");
    }
  }
  else
  {
    webserver.httpUnauthorized();
  }
}

void setup()
{
  temp_sensors.begin();
  zone01.set_temp(20.00);
  Ethernet.begin(arduino_mac, arduino_ip);
  webserver.setDefaultCommand(&thermostat_page);
  webserver.begin();
}

void loop()
{
  char connection_buffer[64];
  int buffer_lenght = 64;
  webserver.processConnection(connection_buffer, &buffer_lenght);
  zone01.run();
}