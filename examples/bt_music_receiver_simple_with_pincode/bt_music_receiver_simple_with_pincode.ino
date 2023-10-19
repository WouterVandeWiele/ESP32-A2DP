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

// ==> Example A2DP Receiver which requires a confirmation - we use a Sensitive Touch pin

#include "AudioTools.h"  // https://github.com/pschatzmann/arduino-audio-tools
#include "BluetoothA2DPSink.h" // https://github.com/pschatzmann/ESP32-A2DP


const int BUTTON = 13; // Sensitive Touch 
const int BUTTON_PRESSED = 40; // touch limit

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

void setup() {
  Serial.begin(115200);

  // setup i2s output
  auto cfg = i2s.defaultConfig();
  cfg.pin_bck = 26;
  cfg.pin_ws = 25;
  cfg.pin_data = 22;
  i2s.begin(cfg);

  // setup a2dp
  a2dp_sink.activate_pin_code(true);
  a2dp_sink.start("ReceiverWithPin", false);  
}

void confirm() {
  a2dp_sink.confirm_pin_code();
}

void loop() {
 if (a2dp_sink.pin_code() != 0 && touchRead(BUTTON) < BUTTON_PRESSED) {
   a2dp_sink.debounce(confirm, 5000);
 }
}