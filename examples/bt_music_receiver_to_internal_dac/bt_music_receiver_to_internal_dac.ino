/*
  Streaming data from Bluetooth to internal DAC of ESP32
  
  Copyright (C) 2020 Phil Schatzmann
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// ==> Example to use built in DAC of ESP32

#include "AudioTools.h"  // https://github.com/pschatzmann/arduino-audio-tools
#include "BluetoothA2DPSink.h" // https://github.com/pschatzmann/ESP32-A2DP

AnalogAudioStream dac;
BluetoothA2DPSink a2dp_sink(dac);

void setup() {
  Serial.begin(115200);

  dac.begin();

  a2dp_sink.set_i2s_config(i2s_config);  
  a2dp_sink.start("InternalDAC");  

}


void loop() {
}
