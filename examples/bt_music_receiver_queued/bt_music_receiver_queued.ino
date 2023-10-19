/*
  Streaming Music from Bluetooth
  
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

// ==> Example A2DP Receiver which uses I2S to an external DAC. The I2S output is managed via a separate Queue which might resolve issues mainly using the volume control on some IOS devices

#include "AudioTools.h"  // https://github.com/pschatzmann/arduino-audio-tools
#include "BluetoothA2DPSinkQueued.h" // https://github.com/pschatzmann/ESP32-A2DP

I2SStream i2s;
BluetoothA2DPSinkQueued a2dp_sink(i2s);

void setup() {
  Serial.begin(115200);

  // setup i2s output
  auto cfg = i2s.defaultConfig();
  cfg.pin_bck = 26;
  cfg.pin_ws = 25;
  cfg.pin_data = 22;
  i2s.begin(cfg);

  // start a2dp
  a2dp_sink.start("MyMusicQueued");  
}


void loop() {
  delay(1000); // do nothing
}
