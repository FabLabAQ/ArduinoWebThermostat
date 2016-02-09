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

thermostat::thermostat(uint8_t pin, DallasTemperature *temp_sensors, DeviceAddress probe_address, uint8_t eeprom_address)
{
	_pin = pin;
	_temp_sensors = temp_sensors;
	for (uint8_t i = 0; i < 8; ++i)
	{
		_probe_address[i] = probe_address[i];
	}
	_eeprom_address = eeprom_address;
	_status = false;
}

void thermostat::begin()
{
	pinMode(_pin, OUTPUT);
	_temp_sensors->setResolution(_probe_address, _resolution);
	_temp = EEPROM.read(_eeprom_address);
	_on_hour = EEPROM.read(_eeprom_address +1);
	_off_hour = EEPROM.read(_eeprom_address +2);
}

// void thermostat::set_temp(uint8_t temp)
// {
// 	_temp = temp;
// }

void thermostat::change_temp(int8_t increase)
{
	_temp += increase;
	EEPROM.write(_eeprom_address, _temp);
}

void thermostat::change_on_hour(int8_t increase)
{
	_on_hour += increase;
	EEPROM.write(_eeprom_address +1, _on_hour);	
}

void thermostat::change_off_hour(int8_t increase)
{
	_off_hour += increase;
	EEPROM.write(_eeprom_address +2, _off_hour);	
}

uint8_t thermostat::get_on_hour()
{
	return _on_hour;
}

uint8_t thermostat::get_off_hour()
{
	return _off_hour;
}

float thermostat::get_set_temp()
{
	return _temp / 10.0;
}

float thermostat::get_actual_temp()
{
	_temp_sensors->requestTemperaturesByAddress(_probe_address);
	return _temp_sensors->getTempC(_probe_address);
}

bool thermostat::get_status()
{
	return _status;
}

void thermostat::run()
{
	_actual_temp = get_actual_temp() * 10;

	if (hour() >= _on_hour && hour() < _off_hour)
	{
		if(_actual_temp > (_temp + _range))
		{
			_status = false;
		}
		else if(_actual_temp < (_temp - _range))
		{
			_status = true;
		}
	}
	else if (hour() >= _off_hour || hour() < _on_hour)
	{
		_status = false;
	}

	digitalWrite(_pin, _status);
}