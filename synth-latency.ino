/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define MIDDLE_C 60
#define VELOCITY 99
#define DEFAULT_VELOCITY 64
#define MIDI_CHANNEL 0

#define AUDIO_PIN 22
#define TIMEOUT_US 300000
#define AUDIO_THRESH 64 // based on a scale from 0-4095

#define LCD_WIDTH 20
#define LCD_LINES 4
#define LCD_ADDRESS 0x27

int cur_sample = 0;

// connect the I2C controlled LCD to SCL0 and SDA0 (pins 19 and 18 on Teensy LC)
LiquidCrystal_I2C lcd(LCD_ADDRESS,LCD_WIDTH,LCD_LINES);

elapsedMicros time_us;

void setup() {
  analogReadRes(12);
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  lcd.print("Samples: 0");
}


void loop() {
  lcd.setCursor(0,1);
  lcd.print("Last:               ");
  usbMIDI.sendNoteOn(MIDDLE_C, VELOCITY, MIDI_CHANNEL);
  long res = measure();
  if(res < 0) {
    lcd.setCursor(6,1);
    lcd.print("Timed out");
  } else {
    cur_sample++;
    lcd.setCursor(9,0);
    lcd.print(cur_sample);
    lcd.setCursor(9,1);
    lcd.print(res);
    update_stats(res);

  }
  delay(100);
  usbMIDI.sendNoteOff(MIDDLE_C, DEFAULT_VELOCITY, MIDI_CHANNEL);
  delay(200);
}

inline long measure() {
  unsigned long tSent, tCancel, tRec;

  tSent = time_us;
  tCancel = time_us + TIMEOUT_US;
  while(analogRead(AUDIO_PIN) < AUDIO_THRESH) {
    if(time_us > tCancel) {
      return -1;
    }
  }
  tRec = time_us;
  return tRec - tSent;
}

float M2 = 0;
float var, mean = 0;

void update_stats(long x) {
  // Algorithm from: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
  // TODO: Consider using Kahan summation algorithm to avoid error accumulation
  float delta = (float) x - mean;
  mean += delta / cur_sample;
  M2 += delta * (x - mean);
  if(cur_sample < 2) return;
  var = M2 / cur_sample;

  lcd.setCursor(0,2);
  lcd.print("Mean:               ");
  lcd.setCursor(9,2);
  lcd.print(mean);
  lcd.setCursor(0,3);
  lcd.print("Std Dev:            ");
  lcd.setCursor(9,3);
  lcd.print(sqrt(var));
}

