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

#include "configuration.h"
#include "language.h"
#include "SPI.h"
#include "Ethernet.h"
#include "OneWire.h"

int8_t thermostat_day[num_thermostats];

void print_thermostat_page()
{
	webserver.printP(head);

	uint8_t weekday_today = weekday();
	webserver.printP(weekday_names[weekday_today]);
	webserver.print(" ");

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
		webserver.printP(form_start);

		webserver.printP(zone_names[i]);
		if(thermostats[i]->status)
		webserver.printP(on_str);
		else webserver.printP(off_str);

		webserver.printP(br);

		webserver.printP(mode_str);
		char btn_val = '0';
		webserver.print_button(name, btn_val, mode_names[thermostats[i]->mode]); btn_val++;

		webserver.printP(br);

		webserver.printP(actual_temp_str);
		webserver.print(thermostats[i]->get_actual_temp());
		webserver.printP(degC);

		webserver.printP(br);

		webserver.printP(day_str);
		webserver.print_button(name, btn_val, left); btn_val++;
		webserver.printP(weekday_names[thermostat_day[i]]);
		webserver.print_button(name, btn_val, right); btn_val++;

		webserver.printP(br);

		webserver.printP(set_temp_str);
		webserver.print_button(name, btn_val, minus); btn_val++;
		webserver.print(thermostats[i]->get_set_temp(thermostat_day[i]));
		webserver.printP(degC);
		webserver.print_button(name, btn_val, plus); btn_val++;

		webserver.printP(br);

		webserver.printP(on_hour_str);
		webserver.print_button(name, btn_val, minus); btn_val++;
		webserver.print(thermostats[i]->get_on_hour(thermostat_day[i]));
		webserver.printP(double_zero);
		webserver.print_button(name, btn_val, plus); btn_val++;

		webserver.printP(br);

		webserver.printP(off_hour_str);
		webserver.print_button(name, btn_val, minus); btn_val++;
		webserver.print(thermostats[i]->get_off_hour(thermostat_day[i]));
		webserver.printP(double_zero);
		webserver.print_button(name, btn_val, plus);

		webserver.printP(form_end);
	}
	
	webserver.printP(tail);
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

			uint8_t i = name[0] -'0', k = value[0] -'0';

				switch (k)
				{
					case (0): thermostats[i]->change_mode(); break;
					case (1):
						thermostat_day[i]--;
						if (thermostat_day[i] < 0)
							thermostat_day[i] = 6;
						break;
					case (2):
						thermostat_day[i]++;
						if (thermostat_day[i] > 6)
							thermostat_day[i] = 0;
						break;
					case (3): thermostats[i]->change_temp(thermostat_day[i], -THERM_INCREMENTS); break;
					case (4): thermostats[i]->change_temp(thermostat_day[i], THERM_INCREMENTS); break;
					case (5): thermostats[i]->change_on_hour(thermostat_day[i], -1); break;
					case (6): thermostats[i]->change_on_hour(thermostat_day[i], 1); break;
					case (7): thermostats[i]->change_off_hour(thermostat_day[i], -1); break;
					case (8): thermostats[i]->change_off_hour(thermostat_day[i], 1); break;
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
			udp_conn.read(ntp_packet_buffer, NTP_PACKET_SIZE);	// read packet into the buffer
			unsigned long secsSince1900; // convert four bytes starting at location 40 to a long integer
			secsSince1900 =	(unsigned long)ntp_packet_buffer[40] << 24;
			secsSince1900 |= (unsigned long)ntp_packet_buffer[41] << 16;
			secsSince1900 |= (unsigned long)ntp_packet_buffer[42] << 8;
			secsSince1900 |= (unsigned long)ntp_packet_buffer[43];
			return secsSince1900 - 2208988800UL + time_zone * SECS_PER_HOUR;
		}
	}
	return 0; // return 0 if unable to get the time
}

void sendNTPpacket(char* address)		// send an NTP request to the time server at the given address
{
	// set all bytes in the buffer to 0
	memset(ntp_packet_buffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	ntp_packet_buffer[0] = 0b11100011;	 // LI, Version, Mode
	ntp_packet_buffer[1] = 0;		 // Stratum, or type of clock
	ntp_packet_buffer[2] = 6;		 // Polling Interval
	ntp_packet_buffer[3] = 0xEC;	// Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	ntp_packet_buffer[12]	= 49;
	ntp_packet_buffer[13]	= 0x4E;
	ntp_packet_buffer[14]	= 49;
	ntp_packet_buffer[15]	= 52;
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
	webserver.setDefaultCommand(& thermostat_page_cmd);
	webserver.begin();
	udp_conn.begin(ntp_port);
	while (getNTPtime() == 0)			// to prevent unwanted activation based on wrong time
	{
		delay(1000);
	}
	setSyncProvider(getNTPtime);
	for (uint8_t i=0; i<num_thermostats; i++)
	{
		thermostats[i]->begin();
	}
}

void loop()
{
	static unsigned long prevMillis_1, prevMillis_2, prevMillis_3;			 // variable to store time elapsed since the last time thermostats were updated
	char connection_buffer[64];
	int buffer_lenght = 64;
	webserver.processConnection(connection_buffer, &buffer_lenght);
	
	if (millis() > prevMillis_1 +CHECK_INTERVAL)
	{
		for (uint8_t i=0; i<num_thermostats; i++)
		{
			thermostats[i]->run();
		}
		prevMillis_1 = millis();
	}

	if (millis() > prevMillis_2 +WEEKDAY_RESET_INTERVAL)
	{
		uint8_t today = weekday();
		for (uint8_t i=0; i<num_thermostats; i++)
		{
			thermostat_day[i]=today;
		}
		prevMillis_2 = millis();
	}

	if (millis() > prevMillis_3 +HOUR_RESET_INTERVAL)
	{
		setSyncProvider(getNTPtime);
		prevMillis_3 = millis();
	}

}