/********************************************************************************
 * Arduino Ethernet Thermostat													*
 * Copyright (C) 2015															*
 * Luca Anastasio <anastasio.lu@gmail.com>										*
 *																				*
 * This program is free software; you can redistribute it and/or modify			*
 * it under the terms of the GNU General Public License as published by			*
 * the Free Software Foundation; either version 3 of the License, or			*
 * (at your option) any later version.											*
 *																				*
 * This program is distributed in the hope that it will be useful,				*
 * but WITHOUT ANY WARRANTY; without even the implied warranty of				*
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the					*
 * GNU General Public License for more details.									*
 *																				*
 * You should have received a copy of the GNU General Public License			*
 * along with this program; if not, write to the Free Software					*
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA	02110-1301	USA *
 ********************************************************************************/
#ifndef language_h
#define language_h

#include "WebServer.h"

P(br) = "<br>";
P(tail) = "</body></html>";
P(form_start) = "<form action='/thermostat' method='POST'>";
P(form_end) = "</form>";
P(plus) = "+";
P(minus) = "-";
P(right) = ">";
P(left) = "<";
P(on_str) = ": ON";
P(off_str) = ": OFF";
P(double_zero) = ":00 ";
P(span_end) = "</span>";
P(head_1) = "<html><head><title>";
P(head_2) = "</title><meta http-equiv=\"refresh\" content=\"60\" ></head><body>";

#if defined(ENGLISH)

P(title) = "Arduino Web Thermostat";
const unsigned char zone_names[4][7] PROGMEM = { "Zone 1", "Zone 2", "Zone 3", "Zone 4" };
const unsigned char weekday_names[7][4] PROGMEM = { "Sat", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri" };
const unsigned char mode_names[3][8] PROGMEM = { "Disable", "Winter", "Summer" };
P(set_temp_str) = "Set: ";
P(actual_temp_str) = "Actual temp: ";
P(on_hour_str) = "ON hour: ";
P(off_hour_str) = "OFF hour: ";
P(mode_str) = "Mode: ";
P(day_str) = "Day: ";
P(heater_str) = "Heater";
P(cooler_str) = "Cooler";

#elif defined(ITALIAN)

P(title) = "Termostato Web Arduino";
const unsigned char zone_names[4][7] PROGMEM = { "Zona 1", "Zona 2", "Zona 3", "Zona 4" };
const unsigned char weekday_names[7][4] PROGMEM = { "Sab", "Dom", "Lun", "Mar", "Mer", "Gio", "Ven" };
const unsigned char mode_names[3][8] PROGMEM = { "Disatt.", "Inverno", "Estate" };
P(set_temp_str) = "Imposta: ";
P(actual_temp_str) = "Temperatura: ";
P(on_hour_str) = "Ora ON: ";
P(off_hour_str) = "Ora OFF: ";
P(mode_str) = "Modalità: ";
P(day_str) = "Giorno: ";
P(heater_str) = "Caldaia";
P(cooler_str) = "Condizionatore";

#endif

P(degC) = "&deg;C";

#endif