#ifndef thermostat_h
#define thermostat_h

#include "Arduino.h"
#include "DallasTemperature.h"

class thermostat
{
	public:
		thermostat(uint8_t pin, DallasTemperature&, DeviceAddress&, uint8_t, float);
		void begin();
		void run();
		float get_temp();
		float get_actual_temp();
		void set_temp(float);
		bool get_status();
		void increase_temp(float);
		void decrease_temp(float);
	private:
		uint8_t _pin, _resolution;
		float _temp, _actual_temp, _range;
		bool _status;
		DallasTemperature _temp_sensors;
		DeviceAddress _probe_address;
};

#endif