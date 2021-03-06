// rainSensor.h

#ifndef _RAINSENSOR_h
#define _RAINSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
/*
http://www.netzmafia.de/skripten/hardware/RasPi/Projekt-Regensensor/index.html
// Sensoreingang
#define PIN 2
// Integrationszeit in Sekunden
#define LIMIT 60

int State, Sum;

void setup()
{
pinMode(PIN, INPUT);
Serial.begin(9600);
}

void loop()
{
State = digitalRead(PIN);
if (State == 1)
{ Sum = (Sum < LIMIT)? Sum + 1 : LIMIT; }
else
{ Sum = (Sum > 0)? Sum - 1 : 0; }
State = (Sum > 0);
Serial.println(State,DEC);
delay(1000);
}
*/

#include "Protothread.h"
#include "hardware.h"
#include "errorhandler.h"
#include "config.h"

class TrainSensor : public Protothread {
private:
	bool m_isRainingDefault, m_isRainingADC;
	byte m_count;

public:
	bool flagShowRainSensor;

	void setup() {
		m_isRainingDefault = false;
		m_isRainingADC = false;
		m_count = 0;
	}


	bool  Run() {
		PT_BEGIN();
		while (1) {
			PT_YIELD_INTERVAL();
			if (CONF_DISABLE_RAIN_SERVICE) {
				PT_EXIT();
			}

			if (CONF_RAINSENSOR_USE_ADC) {
				int value32 = aiPinRain.read_int32();

				if (value32 < CONF_RAINSENSOR_ADC_THRESHOLD) {
					m_count++;
					if (m_count > 100) { m_count = 100; } // limit count
					if (m_count > 5) { m_isRainingADC = true; }
				}
				else {
					m_count = 0;
					m_isRainingADC = false;
				}

				if (flagShowRainSensor) {
					errorHandler.setInfo(F("Is raining: %d adc: %d count: %d\r\n"), m_isRainingADC, value32, m_count);
				}
			}

			if (CONF_RAINSENSOR_USE_DEFAULT) {
				m_isRainingDefault = (diPinRain == LOW);
				if (flagShowRainSensor) {
					errorHandler.setInfo(F("Is raining: %d\r\n"), m_isRainingDefault);
				}
			}
		}
		PT_END();
	}

	bool isRaining() {
		return m_isRainingDefault || m_isRainingADC;
	}

	void showConfig() {
		errorHandler.setInfo(F("!03,Rain Sensor Config\r\n"));
		errorHandler.setInfo(F("!03,enabled: %d\r\n"), IsRunning());
		errorHandler.setInfo(F("!03,interval: %lu\r\n"), interval);
		errorHandler.setInfo(F("!03,is _isRainingDefault: %d\r\n"), m_isRainingDefault);
		errorHandler.setInfo(F("!03,is _isRainingADC: %d\r\n"), m_isRainingADC);
	}

};
#endif

