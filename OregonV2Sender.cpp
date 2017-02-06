/*
 * connectingStuff, Oregon Scientific v2.1 Emitter
 * http://www.connectingstuff.net/blog/encodage-protocoles-oregon-scientific-sur-arduino/
 *
 * Copyright (C) 2013 olivier.lebrun@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
/*
 * OregonV2Sender library (C) 2017 tazounet
*/

#include "OregonV2Sender.h"

const unsigned long TIME = 512;
const unsigned long TWOTIME = TIME*2;
 
#define SEND_HIGH() _pin->hi();
#define SEND_LOW()  _pin->lo();
 
/**
 * \brief    Send logical "0" over RF
 * \details  azero bit be represented by an off-to-on transition
 * \         of the RF signal at the middle of a clock period.
 * \         Remenber, the Oregon v2.1 protocol add an inverted bit first 
 */
inline void OregonV2Sender::sendZero(void) 
{
  SEND_HIGH();
  delayMicroseconds(TIME);
  SEND_LOW();
  delayMicroseconds(TWOTIME);
  SEND_HIGH();
  delayMicroseconds(TIME);
}
 
/**
 * \brief    Send logical "1" over RF
 * \details  a one bit be represented by an on-to-off transition
 * \         of the RF signal at the middle of a clock period.
 * \         Remenber, the Oregon v2.1 protocol add an inverted bit first 
 */
inline void OregonV2Sender::sendOne(void) 
{
   SEND_LOW();
   delayMicroseconds(TIME);
   SEND_HIGH();
   delayMicroseconds(TWOTIME);
   SEND_LOW();
   delayMicroseconds(TIME);
}
 
/**
* Send a bits quarter (4 bits = MSB from 8 bits value) over RF
*
* @param data Source data to process and sent
*/
 
/**
 * \brief    Send a bits quarter (4 bits = MSB from 8 bits value) over RF
 * \param    data   Data to send
 */
inline void OregonV2Sender::sendQuarterMSB(const byte data) 
{
  (bitRead(data, 4)) ? sendOne() : sendZero();
  (bitRead(data, 5)) ? sendOne() : sendZero();
  (bitRead(data, 6)) ? sendOne() : sendZero();
  (bitRead(data, 7)) ? sendOne() : sendZero();
}
 
/**
 * \brief    Send a bits quarter (4 bits = LSB from 8 bits value) over RF
 * \param    data   Data to send
 */
inline void OregonV2Sender::sendQuarterLSB(const byte data) 
{
  (bitRead(data, 0)) ? sendOne() : sendZero();
  (bitRead(data, 1)) ? sendOne() : sendZero();
  (bitRead(data, 2)) ? sendOne() : sendZero();
  (bitRead(data, 3)) ? sendOne() : sendZero();
}
 
/******************************************************************/
/******************************************************************/
/******************************************************************/
 
/**
 * \brief    Send a buffer over RF
 * \param    data   Data to send
 * \param    size   size of data to send
 */
void OregonV2Sender::sendData(byte *data, byte size)
{
  for(byte i = 0; i < size; ++i)
  {
    sendQuarterLSB(data[i]);
    sendQuarterMSB(data[i]);
  }
}
 
/**
 * \brief    Send an Oregon message
 * \param    data   The Oregon message
 */
void OregonV2Sender::sendOregon(byte *data, byte size)
{
    sendPreamble();
    //sendSync();
    sendData(data, size);
    sendPostamble();
}
 
/**
 * \brief    Send preamble
 * \details  The preamble consists of 16 "1" bits
 */
inline void OregonV2Sender::sendPreamble(void)
{
  byte PREAMBLE[]={0xFF,0xFF};
  sendData(PREAMBLE, 2);
}
 
/**
 * \brief    Send postamble
 * \details  The postamble consists of 8 "0" bits
 */
inline void OregonV2Sender::sendPostamble(void)
{
	if (! _humidity)
	{
		sendQuarterLSB(0x00);
	}
	else
	{
		byte POSTAMBLE[]={0x00};
		sendData(POSTAMBLE, 1);  
	}
}
 
/**
 * \brief    Send sync nibble
 * \details  The sync is 0xA. It is not use in this version since the sync nibble
 * \         is include in the Oregon message to send.
 */
inline void OregonV2Sender::sendSync(void)
{
  sendQuarterLSB(0xA);
}
 
/******************************************************************/
/******************************************************************/
/******************************************************************/
 
/**
 * \brief    Set the sensor type
 * \param    data       Oregon message
 * \param    type       Sensor type
 */
inline void OregonV2Sender::setType(byte *data, byte* type) 
{
  data[0] = type[0];
  data[1] = type[1];
}
 
/**
 * \brief    Set the sensor channel
 * \param    data       Oregon message
 * \param    channel    Sensor channel (0x10, 0x20, 0x30)
 */
inline void OregonV2Sender::setChannel(byte *data, byte channel) 
{
    data[2] = channel;
}
 
/**
 * \brief    Set the sensor ID
 * \param    data       Oregon message
 * \param    ID         Sensor unique ID
 */
inline void OregonV2Sender::setId(byte *data, byte ID) 
{
  data[3] = ID;
}
 
/**
 * \brief    Set the sensor battery level
 * \param    data       Oregon message
 * \param    level      Battery level (0 = low, 1 = high)
 */
void OregonV2Sender::setBatteryLevel(byte *data, byte level)
{
  if(!level) data[4] = 0x0C;
  else data[4] = 0x00;
}
 
/**
 * \brief    Set the sensor temperature
 * \param    data       Oregon message
 * \param    temp       the temperature
 */
void OregonV2Sender::setTemperature(byte *data, float temp) 
{
  // Set temperature sign
  if(temp < 0)
  {
    data[6] = 0x08;
    temp *= -1;  
  }
  else
  {
    data[6] = 0x00;
  }
 
  // Determine decimal and float part
  int tempInt = (int)temp;
  int td = (int)(tempInt / 10);
  int tf = (int)round((float)((float)tempInt/10 - (float)td) * 10);
 
  int tempFloat =  (int)round((float)(temp - (float)tempInt) * 10);
 
  // Set temperature decimal part
  data[5] = (td << 4);
  data[5] |= tf;
 
  // Set temperature float part
  data[4] |= (tempFloat << 4);
}
 
/**
 * \brief    Set the sensor humidity
 * \param    data       Oregon message
 * \param    hum        the humidity
 */
void OregonV2Sender::setHumidity(byte* data, byte hum)
{
    data[7] = (hum/10);
    data[6] |= (hum - data[7]*10) << 4;
}
 
/**
 * \brief    Sum data for checksum
 * \param    count      number of bit to sum
 * \param    data       Oregon message
 */
int OregonV2Sender::Sum(byte count, const byte* data)
{
  int s = 0;
 
  for(byte i = 0; i<count;i++)
  {
    s += (data[i]&0xF0) >> 4;
    s += (data[i]&0xF);
  }
 
  if(int(count) != count)
    s += (data[count]&0xF0) >> 4;
 
  return s;
}
 
/**
 * \brief    Calculate checksum
 * \param    data       Oregon message
 */
void OregonV2Sender::calculateAndSetChecksum(byte* data)
{
	if (! _humidity)
	{
		int s = ((Sum(6, data) + (data[6]&0xF) - 0xa) & 0xff);
 
		data[6] |= (s&0x0F) << 4;
		data[7] = (s&0xF0) >> 4;
	}
	else
	{
		data[8] = ((Sum(8, data) - 0xa) & 0xFF);
	}
}
 
/******************************************************************/
/******************************************************************/
/****************0**************************************************/
 
OregonV2Sender::OregonV2Sender()
{
}

OregonV2Sender::OregonV2Sender(byte txPin, byte channel, byte sensorId, bool humidity)
{
	setup(txPin, channel, sensorId, humidity);
}

void OregonV2Sender::setup(byte txPin, byte channel, byte sensorId, bool humidity)
{
	setTxPin(txPin);
	setSensorIdAndChannel(sensorId, channel);
	setHumidity(humidity);
}

void OregonV2Sender::setTxPin(byte txPin)
{
	_pin = new OutputPin(txPin);
}

void OregonV2Sender::setSensorIdAndChannel(byte sensorId, byte channel)
{
	setChannel(_oregonMessageBuffer, channel);
	setId(_oregonMessageBuffer, sensorId);
}

void OregonV2Sender::setHumidity(bool humidity)
{
	_humidity = humidity;
	
	if (_humidity)
	{
		// Create the Oregon message for a temperature/humidity sensor (THGR2228N)
		byte ID[] = {0x1A,0x2D};
		setType(_oregonMessageBuffer, ID);
	}
	else
	{
		// Create the Oregon message for a temperature only sensor (TNHN132N)
		byte ID[] = {0xEA,0x4C};
		setType(_oregonMessageBuffer, ID);
	}
}

void OregonV2Sender::send(byte humidity, float temperature, bool batteryOk)
{
	setBatteryLevel(_oregonMessageBuffer, batteryOk); // 0 : low, 1 : high
	setTemperature(_oregonMessageBuffer, temperature);
	
	if (_humidity)
	{
		setHumidity(_oregonMessageBuffer, humidity);
	}
 
	// Calculate the checksum
	calculateAndSetChecksum(_oregonMessageBuffer);
 
	// Show the Oregon Message
	//for (byte i = 0; i < sizeof(_oregonMessageBuffer); ++i) {
	//	Serial.print(_oregonMessageBuffer[i] >> 4, HEX);
	//	Serial.print(_oregonMessageBuffer[i] & 0x0F, HEX);
	//}
 
	// Send the Message over RF
	sendOregon(_oregonMessageBuffer, _humidity ? 9 : 8);
	
	// Send a "pause"
	SEND_LOW();
	delayMicroseconds(TWOTIME*8);

	// Send a copie of the first message. The v2.1 protocol send the
	// message two time 
	sendOregon(_oregonMessageBuffer, _humidity ? 9 : 8);
 
	SEND_LOW();
}