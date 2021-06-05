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

//#define DEBUG
#define PRINTSCORE

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
int gyroCirc=0;

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
const int LOADCELL_SENSITIVITY=1;
const int LOADCELL_DOUT_PIN = 3;
const int LOADCELL_SCK_PIN = 4;
float load=0;
float loadSmooth[2];
int loadCirc = 0;
float lcMin=0;
float lcMax=0;
float lcRange=300;
int threshCount=0;
int thresh=3;
HX711 scale;

//__________________________________________________________________WAV FILE SETUP
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
int wavPlaying = 0;
int wavBroken = 0;
float pos=0;
float section=0;

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
  #if defined(DEBUG) || defined(PRINTSCORE)
  Serial.begin(115200);  
  #endif
  
  //_______________________________Start up Audio
  AudioMemory(20);
  audioShield.enable();
  audioShield.volume(0.8);
  mixerLeft.gain(0, SET_SUB_VOL*vol);
  mixerRight.gain(0, SET_SYNTH_VOL);
  mixerRight.gain(1, SET_WAV_VOL);
  
  //_______________________________Start up Load Cell
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  while(scale.is_ready() != true){delay(0.1);};
  load = scale.read()*0.0001*LOADCELL_SENSITIVITY;
  lcMin=load;
  lcMax=load+lcRange;
  

  #ifdef DEBUG//???????????????????
  Serial.print("loadcell started\tInitial Min:\t");
  Serial.println(lcMin);
  #endif//?????????????????????????
  
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
  for(int i=0; i<10; i++){xSmooth[i]=0; ySmooth[i]=0;zSmooth[i]=0;};
  for(int i=0; i<2; i++){loadSmooth[i]=0;}; 
}

//__________________________________________________________________SENSOR READ FUNTIONS
//________________________________Dynamically Adjust Load Cell Range
void adjustRange(int curRd){
  if(curRd<(lcMin-5)){
      threshCount--;
      if(threshCount<(thresh*-1)){
        lcMin=lcMin-1;
        lcMax=lcMin+lcRange;
      }
    }
  else if(curRd>(lcMax+5)){
      threshCount++;
      if(threshCount>thresh){
        lcMax=lcMax+1;
        lcMin=lcMax-lcRange;
      }
  }
  else{
      threshCount=0;
  }
}
//________________________________Read Load Cell
void readLoadCell(){
    /*if scale ready get loadcell value*/
    if (scale.is_ready()) {
      load = scale.read()*0.0001*LOADCELL_SENSITIVITY;
      /*store last 10 readings and average
      to smooth out readings*/
      loadCirc=(loadCirc+1)%2;
      float loadTally=0;
      loadSmooth[loadCirc]=load;
      for(int i=0; i<2; i++){loadTally=loadTally+loadSmooth[i];};
      load = loadTally/2;
      /* adjust range and scale to 0-1 */
      adjustRange(load);
      load = map(load, lcMin, lcMax, 0.0, 1.0);
      load = constrain(load, 0.0, 1.0);
      
      #ifdef DEBUG//??????????????
      Serial.print(load);Serial.print("\t");
      #endif//????????????????????
    }
    else{
      #ifdef DEBUG//??????????????
      Serial.print(load);Serial.print("\t");
      #endif//????????????????????
    }
}
//________________________________Read Gyro
void readGyro(){
  /*get gyro readings*/
  lsm.read(); 
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp); 
  x=g.gyro.x;
  y=g.gyro.y;
  z=g.gyro.z;
  /*store last 10 readings and average
    to smooth out readings*/
  gyroCirc=(gyroCirc+1)%10;
  float gyroTallyX=0;
  float gyroTallyY=0;
  float gyroTallyZ=0;
  xSmooth[gyroCirc]=x;
  ySmooth[gyroCirc]=y;
  zSmooth[gyroCirc]=z;
  for(int i=0; i<10; i++){
    gyroTallyX=gyroTallyX+xSmooth[i];
    gyroTallyY=gyroTallyY+ySmooth[i];
    gyroTallyZ=gyroTallyZ+zSmooth[i];
  };
  x=gyroTallyX/10;
  y=gyroTallyY/10;
  z=gyroTallyZ/10;
  
  #ifdef DEBUG//??????????????????
  Serial.print(x);Serial.print("\t");
  Serial.print(y);Serial.print("\t");
  Serial.print(z);Serial.print("\t");
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




  
//==================================================================SCORE
/*SECTIONS
 * 0 INTRO - Chill, mellow laminar chords - 0 - 60,000
 * 1 DESCENT - Sight of black hole: downward glisses   - 60,000 - 120000
 * 2 STRUGGLE - Drones/Action - 120000 - 370000
 * 3 JAWS OF DEFEAT - Rumble 370000-395000
 * 4 SOURCE STAR SAVES THE DAY - as intro - 395000+
 */
 //__________________________________________________________________STRUCTURE
//Get position in structure from Wav File
void locate(){
  pos = playWav1.positionMillis();

    if(pos<60000){//SECTION0 - Intro
      section = map(pos, 0, 60000, 0.0, 1.0);
    };
    if(pos>60000 && pos<120000){//SECTION1 - Descent
      section = map(pos, 60000, 120000, 1.0, 2.0);
    };
    if(pos>120000 && pos<370000){//SECTION2 - Struggle
      section = map(pos, 120000, 370000, 2.0, 3.0);
    };
    if(pos>370000 && pos<395000){//SECTION3 - Jaws of Defeat
      section = map(pos, 370000, 395000, 3.0, 4.0);
    };
    if(pos>395000){//SECTION4 - Saved
      section = map(pos, 395000, 450000, 4.0, 5.0);
    };
  
  #ifdef PRINTSCORE//?????????????
  Serial.print(pos);Serial.print("\t");
  Serial.println(section);
  #endif//????????????????????????
  };
 //__________________________________________________________________SECTIONS
 void chillFM(){
    int notes[] = {48, 50, 52, 55, 57, 60, 62, 64, 67, 69, 72, 74, 76, 79, 81};//, 84, 86, 88, 91, 93, 96, 98, 100, 103};
    int freq;
    int noteRange=15;
    
    synth.setParamValue("fmGateT1",1);/*Modulator Mult*/
    synth.setParamValue("fmGateT2",1);/*Modulator Mult*/
    synth.setParamValue("fmGateT3",1);/*Modulator Mult*/
    synth.setParamValue("fmVerb",0.6);
    synth.setParamValue("fmVol",map(constrain(section,0,0.3),0,0.3,0,0.45));
    
    
    synth.setParamValue("bpm",1 + load*5);/*Modulator Mult*/
    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq1", freq);
  
    synth.setParamValue("fmMm1",2);/*Modulator Mult*/
    synth.setParamValue("fmDp1",1);/*Modulator Depth*/
    synth.setParamValue("fmAC1",3-(load*1.5));/*Carrier Attack*/
    synth.setParamValue("fmRC1",3-(load*1.5));/*Carrier Release*/
    synth.setParamValue("fmAM1",1);/*Modulator Attack*/
    synth.setParamValue("fmRM1",1.5);/*Modulator Release*/
    float mod = map(constrain(x,-2,2),-2,2,0.5,10);
    Serial.println(map(constrain(x,-2,2),-2,2,0.5,10));
    synth.setParamValue("fmModWheel1",mod);/*Mod Wheel*/

    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq2", freq);

    synth.setParamValue("fmMm2",2);/*Modulator Mult*/
    synth.setParamValue("fmDp2",1);/*Modulator Depth*/
    synth.setParamValue("fmAC2",3-(load*1.5));/*Carrier Attack*/
    synth.setParamValue("fmRC2",3-(load*1.5));/*Carrier Release*/
    synth.setParamValue("fmAM2",1);/*Modulator Attack*/
    synth.setParamValue("fmRM2",1.5);/*Modulator Release*/
    mod = map(constrain(y,-2,2),-2,2,0.5,10);
    synth.setParamValue("fmModWheel1",mod);/*Mod Wheel*/

    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq3", freq);
    
    synth.setParamValue("fmMm3",2);/*Modulator Mult*/
    synth.setParamValue("fmDp3",1);/*Modulator Depth*/
    synth.setParamValue("fmAC3",3-(load*1.5));/*Carrier Attack*/
    synth.setParamValue("fmRC3",3-(load*1.5));/*Carrier Release*/
    synth.setParamValue("fmAM3",1);/*Modulator Attack*/
    synth.setParamValue("fmRM3",1.5);/*Modulator Release*/
    mod = map(constrain(z,-2,2),-2,2,0.1,10);
    synth.setParamValue("fmModWheel1", mod);/*Mod Wheel*/
  };
  
//  void descent(){
//    int notes[] = {48, 50, 52, 55, 57, 60, 62, 64, 67, 69, 72, 74, 76, 79, 81};//, 84, 86, 88, 91, 93, 96, 98, 100, 103};
//    int freq;
//    int noteRange=15;
//    float sect = section - 1;
//    
//    synth.setParamValue("fmGateT1",1);/*Modulator Mult*/
//    synth.setParamValue("fmGateT2",1);/*Modulator Mult*/
//    synth.setParamValue("fmGateT3",1);/*Modulator Mult*/
//    synth.setParamValue("fmVerb",0.6);
//    synth.setParamValue("fmVol",map(constrain(sect,0,0.3),0,0.3,0.45,0.6));
//    
//    
//    synth.setParamValue("bpm",1 + load*5);/*Modulator Mult*/
//    freq = random(noteRange);
//    freq = notes[freq];
//    synth.setParamValue("fmFreq1", freq);
//  
//    synth.setParamValue("fmMm1",2+(sect*4));/*Modulator Mult*/
//    synth.setParamValue("fmDp1",1);/*Modulator Depth*/
//    synth.setParamValue("fmAC1",3-(load*1.5));/*Carrier Attack*/
//    synth.setParamValue("fmRC1",3-(load*1.5));/*Carrier Release*/
//    synth.setParamValue("fmAM1",1);/*Modulator Attack*/
//    synth.setParamValue("fmRM1",1.5);/*Modulator Release*/
//    float mod = map(constrain(x,-2,2),-2,2,0.5,10);
//    Serial.println(map(constrain(x,-2,2),-2,2,0.5,10));
//    synth.setParamValue("fmModWheel1",mod);/*Mod Wheel*/
//
//    freq = random(noteRange);
//    freq = notes[freq];
//    synth.setParamValue("fmFreq2", freq);
//
//    synth.setParamValue("fmMm2",2+(sect*5.8));/*Modulator Mult*/
//    synth.setParamValue("fmDp2",1);/*Modulator Depth*/
//    synth.setParamValue("fmAC2",3-(load*1.5));/*Carrier Attack*/
//    synth.setParamValue("fmRC2",3-(load*1.5));/*Carrier Release*/
//    synth.setParamValue("fmAM2",1);/*Modulator Attack*/
//    synth.setParamValue("fmRM2",1.5);/*Modulator Release*/
//    mod = map(constrain(y,-2,2),-2,2,0.5,10);
//    synth.setParamValue("fmModWheel1",mod);/*Mod Wheel*/
//
//    freq = random(noteRange);
//    freq = notes[freq];
//    synth.setParamValue("fmFreq3", freq);
//    
//    synth.setParamValue("fmMm3",2+(sect*-1.3));/*Modulator Mult*/
//    synth.setParamValue("fmDp3",1);/*Modulator Depth*/
//    synth.setParamValue("fmAC3",3-(load*1.5));/*Carrier Attack*/
//    synth.setParamValue("fmRC3",3-(load*1.5));/*Carrier Release*/
//    synth.setParamValue("fmAM3",1);/*Modulator Attack*/
//    synth.setParamValue("fmRM3",1.5);/*Modulator Release*/
//    mod = map(constrain(z,-2,2),-2,2,0.1,10);
//    synth.setParamValue("fmModWheel1", mod);/*Mod Wheel*/
//  };
 
 //__________________________________________________________________

  
//==================================================================





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
      locate();
      chillFM();
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
