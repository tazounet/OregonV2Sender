/*
 * OregonV2Sender library (C) 2017 tazounet
*/

#include "Arduino.h"
#include "fastpin.h"

class OregonV2Sender {
	
public:
	OregonV2Sender();
	OregonV2Sender(byte txpin, byte channel, byte sensorId, bool humidity = false);
	void setup(byte txpin, byte channel, byte sensorId, bool humidity = false);
	void setTxPin(byte txPin);
	void setHumidity(bool humidity);
	void setSensorIdAndChannel(byte sensorId, byte channel);
	void send(byte humidity, float temperature, bool batteryOk);

private:
	void sendZero(void);
	void sendOne(void);
	void sendQuarterMSB(const byte data);
	void sendQuarterLSB(const byte data);
	void sendData(byte *data, byte size);
	void sendOregon(byte *data, byte size);
	void sendPreamble(void);
	void sendPostamble(void);
	void sendSync(void);
	void setType(byte *data, byte* type);
	void setChannel(byte *data, byte channel);
	void setId(byte *data, byte ID);
	void setBatteryLevel(byte *data, byte level);
	void setTemperature(byte *data, float temp);
	void setHumidity(byte* data, byte hum);
	int Sum(byte count, const byte* data);
	void calculateAndSetChecksum(byte* data);

	bool _humidity;
	// Buffer for Oregon message
	// THN132N => 8
	// THGR2228N => 9
	byte _oregonMessageBuffer[9];
	OutputPin* _pin;

};