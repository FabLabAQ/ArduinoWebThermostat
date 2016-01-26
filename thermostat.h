#ifndef thermostat_h
#define thermostat_h

#include "Arduino.h"
#include "DallasTemperature.h"

class thermostat
{
	public:
		thermostat(uint8_t pin, DallasTemperature&, DeviceAddress&, uint8_t, uint8_t);
		void run();
		float get_temp();
		float get_actual_temp();
		void set_temp(float);
		bool get_status();
		void increase_temp(uint8_t);
		void decrease_temp(uint8_t);
	private:
		uint8_t _pin, _resolution, _range;
		float _temp, _actual_temp, _increase, _decrease;
		//bool _status;
		DallasTemperature _temp_sensors;
		DeviceAddress _probe_address;
};

#endif