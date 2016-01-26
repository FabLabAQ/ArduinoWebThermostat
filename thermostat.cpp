#include "thermostat.h"
#include "DallasTemperature.h"
#include "Arduino.h"

thermostat::thermostat(uint8_t pin, DallasTemperature &temp_sensors, DeviceAddress &probe_address, uint8_t resolution, uint8_t range)
{
	_pin = pin;
	_temp_sensors = temp_sensors;
	_probe_address = probe_address;
	_resolution = resolution;
	_range = range;
	//_status = false;

	pinMode(_pin, OUTPUT);
	_temp_sensors.setResolution(_probe_address, _resolution);
}

void thermostat::set_temp(float temp)
{
	_temp = temp;
}

void thermostat::increase_temp(uint8_t increase)
{
	_increase = increase / 100;
	_temp += _increase;
}

void thermostat::decrease_temp(uint8_t decrease)
{
	_decrease = decrease / 100;
	_temp -= _decrease;
}

float thermostat::get_temp()
{
	return _temp;
}

float thermostat::get_actual_temp()
{
	_actual_temp = _temp_sensors.getTempC(_probe_address);
	return _actual_temp;
}

bool thermostat::get_status()
{
	return digitalRead(_pin);
}

void thermostat::run()
{
	_actual_temp = _temp_sensors.getTempC(_probe_address);
	if(_actual_temp > (_temp + _range))
		digitalWrite(_pin, LOW);
	else if(_actual_temp < (_temp - _range))
		digitalWrite(_pin, HIGH);
}