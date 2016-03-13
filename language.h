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
P(head) = "<html><head><body>";
P(tail) = "</body></html>";
P(form_start) = "<form action='/thermostat' method='POST'>";
P(form_end) = "</form>";
P(plus) = "+";
P(minus) = "-";
P(right) = ">";
P(left) = "<";

P(on_str) = " ON";
P(off_str) = " OFF";
P(double_zero) = ":00 ";

#if defined(ENGLISH)

// P(title_str) = "Arduino Web Thermostat ";
const unsigned char zone_names[4][7] PROGMEM = { "Zone 1", "Zone 2", "Zone 3", "Zone 4" };
const unsigned char weekday_names[7][4] PROGMEM = { "Sat", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri" };
//const unsigned char weekday_names_underlined[7][11] PROGMEM = { "<u>Sat</u>", "<u>Sun</u>", "<u>Mon</u>", "<u>Tue</u>", "<u>Wed</u>", "<u>Thu</u>", "<u>Fri</u>" };
const unsigned char mode_names[3][8] PROGMEM = { "Winter", "Summer", "Disable" };
//const unsigned char mode_names_underlined[3][15] PROGMEM = { "<u>Winter</u>", "<u>Summer</u>", "<u>Disable</u>" };
P(set_temp_str) = "set: ";
P(actual_temp_str) = "actual temp: ";
P(on_hour_str) = "ON hour: ";
P(off_hour_str) = "OFF hour: ";
P(mode_str) = "mode: ";
P(day_str) = "day: ";

#elif defined(ITALIAN)

// P(title_str) = "Termostato Web Arduino ";
const unsigned char zone_names[4][7] PROGMEM = { "Zona 1", "Zona 2", "Zona 3", "Zona 4" };
const unsigned char weekday_names[7][4] PROGMEM = { "Sab", "Dom", "Lun", "Mar", "Mer", "Gio", "Ven" };
const unsigned char weekday_names_underlined[7][11] PROGMEM = { "<u>Sab</u>", "<u>Dom</u>", "<u>Lun</u>", "<u>Mar</u>", "<u>Mer</u>", "<u>Gio</u>", "<u>Ven</u>" };
const unsigned char mode_names[3][8] PROGMEM = { "Inverno", "Estate", "Disatt." };
const unsigned char mode_names_underlined[3][15] PROGMEM = { "<u>Inverno</u>", "<u>Estate</u>", "<u>Disatt.</u>" };
P(set_temp_str) = "imposta: ";
P(actual_temp_str) = "temperatura: ";
P(on_hour_str) = "ora ON: ";
P(off_hour_str) = "ora OFF: ";
P(mode_str) = "modalità: ";
P(day_str) = "giorno: ";

#endif

P(degC) = "&deg;C";

#endif