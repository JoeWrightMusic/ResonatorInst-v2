//__________________________________________________________________LIBRARIES
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h> 
#include <HX711.h>
#include "Synth.h"
//__________________________________________________________________DEBUG
#define DEBUG
//#define PRINTSENS
//__________________________________________________________________TEENSY AUDIO SETUP
/*create audio objects*/
Synth synth;
AudioOutputI2S        out;
AudioControlSGTL5000  audioShield;
AudioMixer4           mixerLeft;
AudioMixer4           mixerRight;
AudioPlaySdWav        playWav1;
/*route audio*/
/*Left Channel Mix: Sub and low FM only*/
AudioConnection       patchCord0(synth,1,mixerLeft,0);
/*Right Channel Mix: Upper synth and WAV only*/
AudioConnection       patchCord1(synth,0,mixerRight,0);
AudioConnection       patchCord2(playWav1,0,mixerRight,1);
/**/
AudioConnection       parchCord3(mixerLeft, 0, out, 0);
AudioConnection       parchCord4(mixerRight, 0, out, 1);
//__________________________________________________________________GYRO SETUP
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
float x=0;
float y=0;
float z=0;
float xSmooth[10];
float ySmooth[10];
float zSmooth[10];
float gyroCirc=0;
void setupGyro()
{
  /* 1.) Set the accelerometer range*/
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  /* 2.) Set the magnetometer sensitivity*/
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  /* 3.) Setup the gyroscope*/
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
}
//__________________________________________________________________LOAD CELL SETUP
  const int LOADCELL_DOUT_PIN = 3;
  const int LOADCELL_SCK_PIN = 4;
  int loadSmooth[10];
  int loadCirc = 0;
  HX711 scale;
//__________________________________________________________________WAV FILE SETUP
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
int wavPlaying = 0;
int wavBroken = 0;
//__________________________________________________________________VOLUME SETUP
const int VOL_PIN = 15;
const float SET_SUB_VOL=1;
const float SET_SYNTH_VOL=1;
const float SET_WAV_VOL=1;
float vol = 0.0;
float volSmooth[20];
int volCirc = 0;


//__________________________________________________________________START UP!
void setup() {
  //_______________________________Serial for Debugging
  #if defined(DEBUG) || defined(PRINTSENS)
  Serial.begin(115200);  
  #endif
  
  //_______________________________Start up Audio
  AudioMemory(20);
  audioShield.enable();
  audioShield.volume(0.8);
  mixerLeft.gain(0, SET_SUB_VOL*vol);
  mixerRight.gain(0, SET_SYNTH_VOL);
  mixerRight.gain(1, SET_WAV_VOL);
  
  //_______________________________Start up Synth
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  //_______________________________Start up Gyro
  #ifdef DEBUG//???????????????????
    if (!lsm.begin()){
      Serial.println("Oops ... unable to initialize the LSM9DS1.");
    }
    Serial.println("Found LSM9DS1 9DOF");
  #endif//?????????????????????????
  
  setupGyro();

  //________________________________Set up WAV Player
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  delay(10);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    wavBroken=1;
      
    #ifdef DEBUG//????????????????
    Serial.println("Unable to access the SD card");
    #endif//??????????????????????
  }
  //________________________________Set up Smoothing Arrays
  for(int i=0; i<20; i++){volSmooth[i]=0;};
  for(int i=0; i<10; i++){xSmooth[i]=0; ySmooth[i]=0;zSmooth[i]=0;loadSmooth[i]=0}; 
}

//__________________________________________________________________SENSOR READ FUNTIONS
//________________________________Read Load Cell
void readLoadCell(){
  /*if scale ready get loadcell value*/
    if (scale.is_ready()) {
      float reading = scale.read();
      
      #ifdef DEBUG//??????????????
      Serial.print(reading);Serial.print("\t");
      #endif//????????????????????
    }
    else{
      #ifdef DEBUG//??????????????
      Serial.print("---");Serial.print("\t");
      #endif//????????????????????
    }
}

//________________________________Read Gyro
void readGyro(){
  lsm.read(); 
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp); 
  
  #ifdef DEBUG//??????????????????
  Serial.print(g.gyro.x);Serial.print("\t");
  Serial.print(g.gyro.y);Serial.print("\t");
  Serial.print(g.gyro.z);Serial.print("\t");
  #endif
}

//________________________________Read Volume
void readVolume(){
  vol = analogRead(VOL_PIN);
  vol = constrain(vol,0,300);
  vol = map(vol,0,300,0.0,1.0);

  volSmooth[volCirc]=vol;
  volCirc=(volCirc+1)%20;

  float volTally=0;
  for(int i=0; i<20; i++){volTally = volTally+volSmooth[i];}
  vol = volTally/20;
  mixerLeft.gain(0, SET_SUB_VOL*vol);
  mixerRight.gain(0, SET_SYNTH_VOL*vol);
  mixerRight.gain(1, SET_WAV_VOL*vol);
  
  #ifdef DEBUG//??????????????????
  Serial.println(vol);
  #endif//????????????????????????
  
  };

//__________________________________________________________________PLAY AUDIO
void playAudio(const char *filename)
{
  #ifdef DEBUG//????????????????
  Serial.print("Playing file: ");
  Serial.println(filename);
  #endif 
  
  /*Start playing the file. This sketch continues to run while the file plays.*/
  playWav1.play(filename);
  /* A brief delay for the library read WAV info */
  delay(250);
  /* Wait for the file to finish playing. */
  if(playWav1.isPlaying()){
    while (playWav1.isPlaying()) {
      /*The rest of the code will execute here while the file plays*/
      readGyro();
      readLoadCell();
      readVolume();
      delay(10);
    }
  } else {
    wavBroken=1;
  }
}

//__________________________________________________________________MAIN LOOP
void loop() {
  /*play audio file*/
  if(wavBroken==0){
    playAudio("SDTEST1.WAV");  // filenames are always uppercase 8.3 format  
  } else {
    readGyro();
    readLoadCell();
    readVolume();
    delay(10);
  }
}





//__________________________________________________________________NOTES
/*
Load cell
operate across range
calc actual min/max from startup
use temporary min/max in use for range
keep running average of min max values to compensate for spikes

Gyro
use min/max movement to move a 'slider' accross an imaginary control-space
work from smoothed gyro values


Score
Run a score from the play position of the wav file


Wav File
Add extreme fade to start, possible with bigger gap before countdown. 
Remove tonal synth from mid section to leave room for mild peril
*/
