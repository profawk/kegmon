/*
MIT License

Copyright (c) 2021-22 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#ifndef SRC_TEMP_HPP_
#define SRC_TEMP_HPP_

#include <DHT.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>

#include <main.hpp>
#include <utils.hpp>

class TempHumidity {
 private:
  // DHT22
  DHT* _temp = 0;

  // DS18B20
  OneWire* _oneWire = 0;
  DallasTemperature* _dallas = 0;

  // Generic
  float _lastTempC = NAN;
  float _lastHumidity = NAN;

  void dhtSetup();
  void dsSetup();
  void dhtRead();
  void dsRead();

 public:
  TempHumidity();
  TempHumidity(const TempHumidity&);
  TempHumidity& operator=(const TempHumidity&);
  void setup();
  void reset();
  void read();
  bool hasSensor() { return !isnan(_lastTempC); }
  float getLastTempC() { return _lastTempC; }
  float getLastTempF() {
    return isnan(_lastTempC) ? NAN : convertCtoF(_lastTempC);
  }
  float getLastHumidity() { return _lastHumidity; }
};

extern TempHumidity myTemp;

#endif  // SRC_TEMP_HPP_

// EOF
