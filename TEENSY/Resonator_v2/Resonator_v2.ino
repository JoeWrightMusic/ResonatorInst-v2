//__________________________________________________________________LIBRARIES
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "HX711.h"
#include "Synth.h"

//__________________________________________________________________SET UP TEENSY AUDIO
Synth synth;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;
AudioConnection patchCord0(synth,0,out,0);
AudioConnection patchCord1(synth,0,out,1);


void setup() {
  AudioMemory(20);
  audioShield.enable();
  audioShield.volume(0.8);

  synth.setParamValue("fmGateT1",1);
  synth.setParamValue("fmGateT2",1);
  synth.setParamValue("fmGateT3",1);
  synth.setParamValue("kdGateT1",1);
  synth.setParamValue("kdGateT2",1);
}

void loop() {
  // put your main code here, to run repeatedly:

}
