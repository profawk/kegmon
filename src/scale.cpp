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
#include <kegpush.hpp>
#include <perf.hpp>
#include <scale.hpp>

void Scale::setup(bool force) {
  if (!_scale[0] || force) {
    if (_scale[0]) delete _scale[0];

#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: Initializing scale [0], using offset %l." CR),
                myConfig.getScaleOffset(0));
#endif
    _scale[0] = new HX711();
    _scale[0]->begin(PIN_SCALE1_SDA, PIN_SCALE1_SCL);
    _scale[0]->set_offset(myConfig.getScaleOffset(UnitIndex::U1));

    if (_scale[0]->wait_ready_timeout(500)) {
      _scale[0]->get_units(1);
      // int32_t l = _scale[0]->get_units(1);
      // Log.notice(F("SCAL: Verified connection to scale [0] got %l." CR), l);
    } else {
      Log.error(F("SCAL: Scale [0] not responding, disabling interface." CR));
      delete _scale[0];
      _scale[0] = 0;
    }
  }

  if (!_scale[1] || force) {
    if (_scale[1]) delete _scale[1];

#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: Initializing scale [1], using offset %l." CR),
                myConfig.getScaleOffset(1));
#endif
    _scale[1] = new HX711();
    _scale[1]->begin(PIN_SCALE2_SDA, PIN_SCALE2_SCL);
    _scale[1]->set_offset(myConfig.getScaleOffset(UnitIndex::U2));

    if (_scale[1]->wait_ready_timeout(500)) {
      _scale[1]->get_units(1);
      // int32_t l = _scale[1]->get_units(1);
      // Log.notice(F("SCAL: Verified connection to scale [1] got %l." CR), l);
    } else {
      Log.error(F("SCAL: Scale [1] not responding, disabling interface." CR));
      delete _scale[1];
      _scale[1] = 0;
    }
  }

  setScaleFactor(UnitIndex::U1);
  setScaleFactor(UnitIndex::U2);
}

void Scale::setScaleFactor(UnitIndex idx) {
  if (!_scale[idx]) return;

  float fs = myConfig.getScaleFactor(idx);

  if (fs == 0.0) fs = 1.0;

  _scale[idx]->set_scale(
      fs);  // apply the saved scale factor so we get valid results
}
float Scale::read(UnitIndex idx) {
#if defined(DEBUG_LINK_SCALES)
  idx = UnitIndex::U1;
#endif

  if (myConfig.getScaleFactor(idx) == 0 ||
      myConfig.getScaleOffset(idx) == 0)  // Not initialized, just return zero
    return 0;

  if (!_scale[idx]) return 0;

  PERF_BEGIN("scale-read");
  float raw = _scale[idx]->get_units(myConfig.getScaleReadCount());
#if LOG_LEVEL == 6
  // Log.verbose(F("SCAL: Reading weight=%F, updating stats %s [%d]" CR), f,
  //            updateStats ? "true" : "false", idx);
#endif

  // If the value is higher/lower than 100 kb/lbs then the reading is proably
  // wrong, just ignore the reading
  if (raw > 100) {
    Log.error(
        F("SCAL: Ignoring value since it's higher than 100kg, %F [%d]." CR),
        raw, idx);
    PERF_END("scale-read");
    return NAN;
  }

  if (raw < -100) {
    Log.error(F("SCAL: Ignoring value since it's less than -100kg %F [%d]." CR),
              raw, idx);
    PERF_END("scale-read");
    return NAN;
  }
  PERF_END("scale-read");
  return raw;
}

void Scale::tare(UnitIndex idx) {
  if (!_scale[idx]) return;

#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: Set scale to zero, prepare for calibration [%d]." CR),
              idx);
#endif

  _scale[idx]->set_scale();  // set scale factor to 1
  _scale[idx]->tare(myConfig.getScaleReadCountCalibration());  // zero weight
  int32_t l = _scale[idx]->get_offset();
  Log.verbose(F("SCAL: New scale offset found %l [%d]." CR), l, idx);

  myConfig.setScaleOffset(idx, l);  // save the offset
  myConfig.saveFile();
}

int32_t Scale::readRaw(UnitIndex idx) {
#if defined(DEBUG_LINK_SCALES)
  idx = UnitIndex::U1;
#endif
  if (!_scale[idx]) return 0;
  PERF_BEGIN("scale-readraw");
  int32_t l = _scale[idx]->read_average(
      myConfig.getScaleReadCountCalibration());  // get the raw value without
                                                 // applying scaling factor
  PERF_END("scale-readraw");
  return l;
}

void Scale::findFactor(UnitIndex idx, float weight) {
  if (!_scale[idx]) return;

  float l = _scale[idx]->get_units(myConfig.getScaleReadCountCalibration());
  float f = l / weight;
#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: Detecting factor for weight %F, raw %l %F [%d]." CR),
              weight, l, f, idx);
#endif

  myConfig.setScaleFactor(idx, f);
  myConfig.saveFile();  // save the factor to file

  setScaleFactor(idx);  // apply the factor after it has been saved
  read(idx);
}

// EOF
