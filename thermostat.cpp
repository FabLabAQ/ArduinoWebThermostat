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


#include "thermostat.h"
#include "DallasTemperature.h"
#include "Arduino.h"
#include "EEPROM.h"
#include "TimeLib.h"

thermostat::thermostat(uint8_t pin, DallasTemperature *temp_sensors, uint8_t *probe_address, uint8_t eeprom_address)
{
	_pin = pin;
	_temp_sensors = temp_sensors;
	_probe_address = probe_address;
	_eeprom_address = eeprom_address;
	status = false;
}

void thermostat::begin()
{
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, HIGH);
	_temp_sensors->setResolution(_probe_address, _resolution);
	mode = EEPROM.read(_eeprom_address);
	for (uint8_t i=0; i<7; i++)
	{
		_temp[i] = EEPROM.read(_eeprom_address +1 +i);
	}
	for (uint8_t i=0; i<7; i++)
	{
		_on_hour[i] = EEPROM.read(_eeprom_address +8 +i);
	}
	for (uint8_t i=0; i<7; i++)
	{
		_off_hour[i] = EEPROM.read(_eeprom_address +15 +i);
	}
}

void thermostat::change_temp(uint8_t day, int8_t increase)
{
	_temp[day] += increase;
	EEPROM.write(_eeprom_address +1 +day, _temp[day]);
}

void thermostat::change_on_hour(uint8_t day, int8_t increase)
{
	_on_hour[day] += increase;
	if (_on_hour[day] > 24)
		_on_hour[day] = 0;
	else if (_on_hour[day] < 0)
		_on_hour[day] = 24;
	EEPROM.write(_eeprom_address +8 +day, _on_hour[day]);	
}

void thermostat::change_off_hour(uint8_t day, int8_t increase)
{
	_off_hour[day] += increase;
	if (_off_hour[day] > 24)
		_off_hour[day] = 0;
	else if (_off_hour[day] < 0)
		_off_hour[day] = 24;
	EEPROM.write(_eeprom_address +15 +day, _off_hour[day]);	
}

void thermostat::change_mode()
{
	mode++;
	if (mode > 2)
		mode = 0;
	EEPROM.write(_eeprom_address, mode);	
}

uint8_t thermostat::get_on_hour(uint8_t day)
{
	return _on_hour[day];
}

uint8_t thermostat::get_off_hour(uint8_t day)
{
	return _off_hour[day];
}

float thermostat::get_set_temp(uint8_t day)
{
	return _temp[day] / 4.0;
}

float thermostat::get_actual_temp()
{
	_temp_sensors->requestTemperaturesByAddress(_probe_address);
	return _temp_sensors->getTempC(_probe_address);
}

void thermostat::run()
{
	_actual_temp = get_actual_temp() * 4;
	_today = weekday();

	if (mode != 0)
	{
		if (hour() >= _on_hour[_today] && hour() < _off_hour[_today])
		{
			if(_actual_temp > (_temp[_today] + _range))
			{
				if (mode == 1)
					status = false;
				else
					status = true;
			}
			else if(_actual_temp < (_temp[_today] - _range))
			{
				if (mode == 1)
					status = true;
				else
					status = false;
			}
		}
		else if (hour() >= _off_hour[_today] || hour() < _on_hour[_today])
		{
			status = false;
		}
	}
	else
	{
		status = false;
	}

	digitalWrite(_pin, !status);		// status inverted because most relay boards work with a low = ON
}
