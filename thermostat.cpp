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

thermostat::thermostat(uint8_t pin, DallasTemperature &temp_sensors, DeviceAddress &probe_address, uint8_t resolution, float range)
{
	_pin = pin;
	_temp_sensors = temp_sensors;
	for (uint8_t i=0; i<8; i++)
 		_probe_address[i] = probe_address[i];
	_resolution = resolution;
	_range = range;
	_status = false;
}

void thermostat::begin()
{
	pinMode(_pin, OUTPUT);
	_temp_sensors.setResolution(_probe_address, _resolution);
}

void thermostat::set_temp(float temp)
{
	_temp = temp;
}

void thermostat::increase_temp(float increase)
{
	_temp += increase;
}

void thermostat::decrease_temp(float decrease)
{
	_temp -= decrease;
}

float thermostat::get_temp()
{
	return _temp;
}

float thermostat::get_actual_temp()
{
	_temp_sensors.requestTemperatures();
	_actual_temp = _temp_sensors.getTempC(_probe_address);
	return _actual_temp;
}

bool thermostat::get_status()
{
	return _status;
}

void thermostat::run()
{
	thermostat::get_actual_temp();
	if(_actual_temp > (_temp + _range))
	{
		digitalWrite(_pin, LOW);
		_status = false;
	}
	else if(_actual_temp < (_temp - _range))
	{
		digitalWrite(_pin, HIGH);
		_status = true;
	}
}