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

// ==> Example A2DP Receiver which uses the A2DP I2S output to an AudioKit board

#include "AudioTools.h"  // https://github.com/pschatzmann/arduino-audio-tools
#include "AudioLibs/AudioKit.h" // https://github.com/pschatzmann/arduino-audiokit
#include "BluetoothA2DPSink.h" // https://github.com/pschatzmann/ESP32-A2DP

AudioKitStream kit;
BluetoothA2DPSink a2dp_sink(kit);

void setup() {
  Serial.begin(115200);
  kit.begin();
  kit.setVolume(1.0); // max volume

  a2dp_sink.start("AudioKit");  
  //a2dp_sink.set_volume(255); // max volume

}


void loop() {
  delay(1000); // do nothing
}