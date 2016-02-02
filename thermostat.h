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

#ifndef thermostat_h
#define thermostat_h

#include "Arduino.h"
#include "DallasTemperature.h"

#ifndef THERMOSTAT_RESOLUTION
#define THERMOSTAT_RESOLUTION 9
#endif // #ifndef THERMOSTAT_RESOLUTION

#ifndef THERMOSTAT_RANGE
#define THERMOSTAT_RANGE 1
#endif // #ifndef THERMOSTAT_RANGE

class thermostat
{
	public:
		thermostat(uint8_t, DallasTemperature*, DeviceAddress, uint8_t);
		void begin();
		void run();
		float get_set_temp();
		float get_actual_temp();
//		void set_temp(uint8_t);
		bool get_status();
		void change_temp(int8_t);
	private:
		const uint8_t _resolution = THERMOSTAT_RESOLUTION, _range = THERMOSTAT_RANGE;
		uint8_t _pin, _eeprom_address, _temp, _actual_temp;
		bool _status;
		DallasTemperature *_temp_sensors;
		DeviceAddress _probe_address;
};

#endif