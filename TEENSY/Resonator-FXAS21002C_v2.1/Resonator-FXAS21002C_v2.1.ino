/*
 * THIS VERSION USES THE FXOS8700+FXAS2100C for Gyro sensing
 */


//__________________________________________________________________LIBRARIES
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_FXAS21002C.h>
#include <Adafruit_Sensor.h> 
#include <HX711.h>
#include "Synth.h"
//__________________________________________________________________DEBUG

#define DEBUG
//#define PRINTSCORE

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
Adafruit_FXAS21002C fxas = Adafruit_FXAS21002C(0x0021002C);
float x=0;
float y=0;
float z=0;
float xSmooth[10];
float ySmooth[10];
float zSmooth[10];
int gyroCirc=0;
float gSlidX[2]={0,0};
float gSlidY[2]={0,0};
float gSlidZ[2]={0,0};


//__________________________________________________________________LOAD CELL SETUP
const int LOADCELL_SENSITIVITY=3.5;
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
const float SET_SYNTH_VOL=0.3;
const float SET_WAV_VOL=0.1;
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
    if (!fxas.begin()){
      Serial.println("Oops ... unable to initialize the FXAS21002C.");
    }
    Serial.println("Found FXAS21002C 9DOF");
  #endif//?????????????????????????
  
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
  sensors_event_t event;
  fxas.getEvent(&event);
  
  x=event.gyro.x;
  y=event.gyro.y;
  z=event.gyro.z;
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

  /*use gyros to move away from previous position*/
  if(abs(x)>0.1){gSlidX[0]=gSlidX[1]+x;};
  if(abs(y)>0.1){gSlidY[0]=gSlidY[1]+y;};
  if(abs(z)>0.1){gSlidZ[0]=gSlidZ[1]+z;};
  /*map arbitrary position values to a wrapped sin curve*/
  x= sin(gSlidX[0]/5);
  y= sin(gSlidY[0]/5);
  z= sin(gSlidZ[0]/5);
  /*record current state for the next loop*/
  gSlidX[1]=gSlidX[0];
  gSlidY[1]=gSlidY[0];
  gSlidZ[1]=gSlidZ[0];
  
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
 * 0 INTRO - Chill, mellow laminar chords - 0 - 52,000
 * 1 FOR ONE MOMENT ... - Space
 * 2 DESCENT - Sight of black hole: downward glisses   - 60,000 - 120000
 * 3 STRUGGLE - Drones/Action - 120000 - 370000
 * 4 JAWS OF DEFEAT - Rumble 370000-395000
 * 5 SOURCE STAR SAVES THE DAY - as intro - 395000+
 */
 //__________________________________________________________________STRUCTURE
//Get position in structure from Wav File & play score
void playScore(){
  pos = playWav1.positionMillis();
    
    if(pos<50000){//SECTION0 - Intro
      section = map(pos, 0, 50000, 0.0, 1.0);
      chillFM();
        
    };
    if(pos>50000 && pos<60000){//SECTION1 - One Moment
      section = map(pos, 50000, 60000, 0.0, 1.0);
      oneMoment();
    };
    if(pos>60000 && pos<70000){//SECTION2 - Descent
      section = map(pos, 60000, 70000, 0.0, 1.0);
      descent();
    };
    if(pos>70000 && pos<370000){//SECTION3 - Struggle
      section = map(pos, 70000, 370000, 0.0, 1.0);
      struggle();
    };
    if(pos>370000 && pos<395000){//SECTION4 - Jaws of Defeat
      section = map(pos, 370000, 395000, 0.0, 1.0);
      jod();
    };
    if(pos>395000){//SECTION5 - Saved
      section = map(pos, 395000, 450000, 0.0, 1.0);
      chillFM2();
    };
  
  #ifdef PRINTSCORE//?????????????
  Serial.print(pos);Serial.print("\t");
  Serial.println(section);
  #endif//????????????????????????
  };
 //__________________________________________________________________SECTIONS
 //________________________________0 - INTRO
 void chillFM(){
    int notes[] = {48, 50, 52, 55, 57, 60, 62, 64, 67, 69, 72, 74, 76, 79, 81};//, 84, 86, 88, 91, 93, 96, 98, 100, 103};
    int freq;
    int noteRange=15;
    
    synth.setParamValue("fmGateT1",1);/*Modulator Mult*/
    synth.setParamValue("fmGateT2",1);/*Modulator Mult*/
    synth.setParamValue("fmGateT3",1);/*Modulator Mult*/
    synth.setParamValue("fmVerb",1);
    synth.setParamValue("fmVol",map(constrain(section,0,0.25),0,0.3,0,0.2)*map(load,0,1,0.3,1));
    
    
    synth.setParamValue("bpm",1 + load*225);/*Modulator Mult*/
    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq1", freq);
    synth.setParamValue("fmMm1",2);/*Modulator Mult*/
    synth.setParamValue("fmDp1",1);/*Modulator Depth*/
    synth.setParamValue("fmAC1",3-(load*2.65));/*Carrier Attack*/
    synth.setParamValue("fmRC1",3-(load*2.65));/*Carrier Release*/
    synth.setParamValue("fmAM1",3-(load*2.65));/*Modulator Attack*/
    synth.setParamValue("fmRM1",3-(load*2.65));/*Modulator Release*/
    synth.setParamValue("fmEucNo1",11);
    float mod = map(x,-1,1,0.5,5);
    synth.setParamValue("fmModWheel1",mod);/*Mod Wheel*/

    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq2", freq);
    synth.setParamValue("fmMm2",2);/*Modulator Mult*/
    synth.setParamValue("fmDp2",1);/*Modulator Depth*/
    synth.setParamValue("fmAC2",3-(load*2.65));/*Carrier Attack*/
    synth.setParamValue("fmRC2",3-(load*2.65));/*Carrier Release*/
    synth.setParamValue("fmAM2",3-(load*2.65));/*Modulator Attack*/
    synth.setParamValue("fmRM2",3-(load*2.65));/*Modulator Release*/
    synth.setParamValue("fmEucNo2",3);
    mod = map(y,-1,1,0.5,5);
    synth.setParamValue("fmModWheel1",mod);/*Mod Wheel*/

    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq3", freq);
    synth.setParamValue("fmMm3",2);/*Modulator Mult*/
    synth.setParamValue("fmDp3",1);/*Modulator Depth*/
    synth.setParamValue("fmAC3",3-(load*2.65));/*Carrier Attack*/
    synth.setParamValue("fmRC3",3-(load*2.65));/*Carrier Release*/
    synth.setParamValue("fmAM3",3-(load*2.65));/*Modulator Attack*/
    synth.setParamValue("fmRM3",3-(load*2.65));/*Modulator Release*/
    synth.setParamValue("fmEucNo3",7);
    mod = map(z,-1,1,0.1,5);
    synth.setParamValue("fmModWheel1", mod);/*Mod Wheel*/
  };
  
 //________________________________1 - ONE MOMENT
  void oneMoment(){
    synth.setParamValue("fmVol",map(constrain(section,0,0.2),0,0.2,0.2,0)*map(load,0,1,0.3,1));
    };
    
 //________________________________2 - DESCENT
  void descent(){
    int notes[] = {48, 50, 52, 55, 57, 60, 62, 64, 67, 69, 72, 74, 76, 79, 81};//, 84, 86, 88, 91, 93, 96, 98, 100, 103};
    int freq;
    int noteRange=15;

    synth.setParamValue("fmVerb",1);
    synth.setParamValue("fmVol",map(constrain(section,0,0.65),0,0.65,0,0.3)*map(load,0,1,0.3,1));
    synth.setParamValue("kroneVol",map(constrain(section,0,1),0,1,0,0.7));

    
    synth.setParamValue("bpm",1 + load*225);/*Modulator Mult*/
    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq1", freq);
    synth.setParamValue("fmMm1",map(section, 0, 1, 2, 0.23));/*Modulator Mult*/
    synth.setParamValue("fmDp1",map(section, 0, 1, 2, 0.2));/*Modulator Depth*/
    synth.setParamValue("fmAC1",3-(load*2.65));/*Carrier Attack*/
    synth.setParamValue("fmRC1",3-(load*2.65));/*Carrier Release*/
    synth.setParamValue("fmAM1",3-(load*2.65));/*Modulator Attack*/
    synth.setParamValue("fmRM1",3-(load*2.65));/*Modulator Release*/
    synth.setParamValue("fmEucNo1",map(section,0,1,11,1));
    float mod = map(x,-1,1,0.5,10);
    synth.setParamValue("fmModWheel1",mod);/*Mod Wheel*/

    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq2", freq);
    synth.setParamValue("fmMm2",map(section,0,1,2,4.5));/*Modulator Mult*/
    synth.setParamValue("fmDp2",map(section,0,1,1,3.1));/*Modulator Depth*/
    synth.setParamValue("fmAC2",3-(load*2.65));/*Carrier Attack*/
    synth.setParamValue("fmRC2",3-(load*2.65));/*Carrier Release*/
    synth.setParamValue("fmAM2",3-(load*2.65));/*Modulator Attack*/
    synth.setParamValue("fmRM2",3-(load*2.65));/*Modulator Release*/
    synth.setParamValue("fmEucNo2",map(section,0,1,3,6));
    mod = map(y,-1,1,0.5,10);
    synth.setParamValue("fmModWheel1",mod);/*Mod Wheel*/

    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq3", freq);
    synth.setParamValue("fmMm3",map(section,0,1,2,2.3));/*Modulator Mult*/
    synth.setParamValue("fmDp3",map(section,0,1,1,0.5));/*Modulator Depth*/
    synth.setParamValue("fmAC3",3-(load*2.65));/*Carrier Attack*/
    synth.setParamValue("fmRC3",3-(load*2.65));/*Carrier Release*/
    synth.setParamValue("fmAM3",3-(load*2.65));/*Modulator Attack*/
    synth.setParamValue("fmRM3",3-(load*2.65));/*Modulator Release*/
    synth.setParamValue("fmEucNo3",map(section,0,1,7,12));
    mod = map(z,-1,1,0.1,10);
    synth.setParamValue("fmModWheel1", mod);/*Mod Wheel*/

    
    freq=random(10)+20;
    synth.setParamValue("kdGateT1", 1);
    synth.setParamValue("kdNxtFreq1",freq-random(3));
    synth.setParamValue("kdEucNo1",1);
    synth.setParamValue("kdA1",3+(random(1000)*0.001));
    synth.setParamValue("kdR1",5+(random(1000)*0.003));
    synth.setParamValue("kdDelta1",-0.12);

    
    synth.setParamValue("kdGateT2", 1);
    synth.setParamValue("kdNxtFreq2", freq+random(3));
    synth.setParamValue("kdEucNo2",1);
    synth.setParamValue("kdA2",3+(random(1000)*0.001));
    synth.setParamValue("kdR2",5+(random(1000)*0.003));
    synth.setParamValue("kdDelta2",-0.1);
    
    };

    

//________________________________3 STRUGGLE
int noteCyc=0;
void struggle(){

    int notes[] = {39,40,41, 45,46,47, 51,52,53, 57,58,59, 63,64,65};
    int freq;
    int noteRange=15;

    
    
    synth.setParamValue("fmGateT1",1);/*Modulator Mult*/
    synth.setParamValue("fmGateT2",1);/*Modulator Mult*/
    synth.setParamValue("fmGateT3",1);/*Modulator Mult*/
    synth.setParamValue("fmVerb",0.01);
    synth.setParamValue("fmVol", 0.3);
    synth.setParamValue("kroneVol", 1);
    synth.setParamValue("fm1vol", 0.8);
    synth.setParamValue("fm2vol", 0.6);
    synth.setParamValue("fm3vol", 1);
    synth.setParamValue("kd1vol", map(constrain(load,0.2,0.7),0.2,0.7,0,0.6));
    synth.setParamValue("kd2vol", map(constrain(load,0.75,1),0.75,1,0,1));

    
    synth.setParamValue("bpm",80 + map(constrain(load, 0.6,1),0.6,1,0,70));/*Modulator Mult*/
    freq = random(15);
    freq = notes[freq];
    synth.setParamValue("fmFreq1", freq);
    synth.setParamValue("fmMm1",0.23 + map(load,0,1,0,0.8));/*Modulator Mult*/
    synth.setParamValue("fmDp1",1 - (abs(z)+load*1));/*Modulator Depth*/
    synth.setParamValue("fmAC1",7-(load*4.55));/*Carrier Attack*/
    synth.setParamValue("fmRC1",7-(load*4.55));/*Carrier Release*/
    synth.setParamValue("fmAM1",0);/*Modulator Attack*/
    synth.setParamValue("fmRM1",10);/*Modulator Release*/
    synth.setParamValue("fmEucNo1",(random(100)>(100-(section*20))));
    float mod = map(y,-1,1,0.1,10);
    synth.setParamValue("fmModWheel1",mod);/*Mod Wheel*/

    int notes2[] = {51,52,53, 57,58,59, 63,64,65, 69,70,71, 75, 76, 77};
    freq = random(4);
    freq = freq+(load*15);
    freq = constrain(freq, 0,15);
    freq = notes2[freq];
    synth.setParamValue("fmFreq2", freq);
    synth.setParamValue("fmMm2",4.5);/*Modulator Mult*/
    synth.setParamValue("fmDp2",3.1);/*Modulator Depth*/
    synth.setParamValue("fmAC2",0.1);/*Carrier Attack*/
    synth.setParamValue("fmRC2",0.1-(section*0.1)+map(y,-1,1,0.01,1));/*Carrier Release*/
    synth.setParamValue("fmAM2",0.03);/*Modulator Attack*/
    synth.setParamValue("fmRM2",map(z,-1,1,0.01,1));/*Modulator Release*/
    synth.setParamValue("fmEucNo2", (random(100)>(50+(section*30)))*3*((x<-0.3)+3*(y>0.4)));
    mod = map(y,-1,1,0.5,2);
    synth.setParamValue("fmModWheel2",mod);/*Mod Wheel*/

    int notes3[] = {75,76,77, 71,72,73, 77,78,79, 83,84,85, 89,90,91};
    freq = 10+random(5*load);
    freq = notes3[freq];
    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq3", freq+12);
    synth.setParamValue("fmMm3",2.34531);/*Modulator Mult*/
    synth.setParamValue("fmDp3",0.5);/*Modulator Depth*/
    synth.setParamValue("fmAC3",0.01);/*Carrier Attack*/
    synth.setParamValue("fmRC3",0.2-(section*0.2)+0.1-(abs(x)*0.093));/*Carrier Release*/
    synth.setParamValue("fmAM3",0.01);/*Modulator Attack*/
    synth.setParamValue("fmRM3",0.1-(abs(x)*0.093));/*Modulator Release*/
    synth.setParamValue("fmEucNo3",12*(random(100)>(50+(section*30)))*((y<-0.2)+(x>0.4)));
    mod = map(x,-1,1,3,0.2);
    synth.setParamValue("fmModWheel3", mod);/*Mod Wheel*/

    int notes4[] = {33,40,36,31,33,40};
    int durs[] = {6,1,3};
    noteCyc=(noteCyc+1)%6;
    freq=notes4[noteCyc];
    
    synth.setParamValue("kdGateT1", 1);
    synth.setParamValue("kdNxtFreq1",freq);
    synth.setParamValue("kdEucNo1",1);
    synth.setParamValue("kdA1",durs[random(3)]);
    synth.setParamValue("kdR1",durs[random(3)]);
    synth.setParamValue("kdDelta1",0);


    synth.setParamValue("kdGateT2", 1);
    synth.setParamValue("kdNxtFreq2",33);
    synth.setParamValue("kdEucNo2",3);
    synth.setParamValue("kdA2",0.01);
    synth.setParamValue("kdR2",0.01+(0.1*load));
    synth.setParamValue("kdDelta2",-0.9);
    
    };
//________________________________4 JAWS OF DEFEAT
void jod(){
    int notes[] = {39,40,41, 45,46,47, 51,52,53, 57,58,59, 63,64,65};
    int freq;
    int noteRange=15;

    
    
    synth.setParamValue("fmGateT1",1);/*Modulator Mult*/
    synth.setParamValue("fmGateT2",1);/*Modulator Mult*/
    synth.setParamValue("fmGateT3",1);/*Modulator Mult*/
    synth.setParamValue("fmVerb",0.01);
    synth.setParamValue("fmVol", 0.3);
    synth.setParamValue("kroneVol", 1);
    synth.setParamValue("fm1vol", 0.8*map(constrain(section,0,0.2),0,0.2,1,0));
    synth.setParamValue("fm2vol", 0.6*map(constrain(section,0,0.2),0,0.2,1,0));
    synth.setParamValue("fm3vol", 1*map(constrain(section,0,0.2),0,0.2,1,0));
    synth.setParamValue("kd1vol", 0.8);
    synth.setParamValue("kd2vol", 0.8);

    
    synth.setParamValue("bpm",80 + map(constrain(load, 0.6,1),0.6,1,0,70));/*Modulator Mult*/
    freq = random(15);
    freq = notes[freq];
    synth.setParamValue("fmFreq1", freq);
    synth.setParamValue("fmMm1",0.23 + map(load,0,1,0,0.8));/*Modulator Mult*/
    synth.setParamValue("fmDp1",1 - (abs(z)+load*1));/*Modulator Depth*/
    synth.setParamValue("fmAC1",7-(load*4.55));/*Carrier Attack*/
    synth.setParamValue("fmRC1",7-(load*4.55));/*Carrier Release*/
    synth.setParamValue("fmAM1",0);/*Modulator Attack*/
    synth.setParamValue("fmRM1",10);/*Modulator Release*/
    synth.setParamValue("fmEucNo1",(random(100)>(87)));
    float mod = map(y,-1,1,0.1,10);
    synth.setParamValue("fmModWheel1",mod);/*Mod Wheel*/

    int notes2[] = {51,52,53, 57,58,59, 63,64,65, 69,70,71, 75, 76, 77};
    freq = random(4);
    freq = freq+(load*15);
    freq = constrain(freq, 0,15);
    freq = notes2[freq];
    synth.setParamValue("fmFreq2", freq);
    synth.setParamValue("fmMm2",4.5);/*Modulator Mult*/
    synth.setParamValue("fmDp2",3.1);/*Modulator Depth*/
    synth.setParamValue("fmAC2",0.1);/*Carrier Attack*/
    synth.setParamValue("fmRC2",map(y,-1,1,0.01,1));/*Carrier Release*/
    synth.setParamValue("fmAM2",0.03);/*Modulator Attack*/
    synth.setParamValue("fmRM2",map(z,-1,1,0.01,1));/*Modulator Release*/
    synth.setParamValue("fmEucNo2", (random(100)>(50))*3*((x<-0.3)+3*(y>0.4)));
    mod = map(y,-1,1,0.5,2);
    synth.setParamValue("fmModWheel2",mod);/*Mod Wheel*/

    int notes3[] = {75,76,77, 71,72,73, 77,78,79, 83,84,85, 89,90,91};
    freq = 10+random(5*load);
    freq = notes3[freq];
    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq3", freq+12);
    synth.setParamValue("fmMm3",2.34531);/*Modulator Mult*/
    synth.setParamValue("fmDp3",0.5);/*Modulator Depth*/
    synth.setParamValue("fmAC3",0.01);/*Carrier Attack*/
    synth.setParamValue("fmRC3",0.1-(abs(x)*0.093));/*Carrier Release*/
    synth.setParamValue("fmAM3",0.01);/*Modulator Attack*/
    synth.setParamValue("fmRM3",0.1-(abs(x)*0.093));/*Modulator Release*/
    synth.setParamValue("fmEucNo3",12*(random(100)>(50))*((y<-0.2)+(x>0.4)));
    mod = map(x,-1,1,3,0.2);
    synth.setParamValue("fmModWheel3", mod);/*Mod Wheel*/

  
    freq=random(10)+20;
    synth.setParamValue("kdGateT1", 1);
    synth.setParamValue("kdNxtFreq1",freq-random(3));
    synth.setParamValue("kdEucNo1",12);
    synth.setParamValue("kdA1",3+(random(1000)*0.001));
    synth.setParamValue("kdR1",3+(random(1000)*0.003));
    synth.setParamValue("kdDelta1",-0.12);

    
    synth.setParamValue("kdGateT2", 1);
    synth.setParamValue("kdNxtFreq2", freq+random(3));
    synth.setParamValue("kdEucNo2",12);
    synth.setParamValue("kdA2",3+(random(1000)*0.001));
    synth.setParamValue("kdR2",3+(random(1000)*0.003));
    synth.setParamValue("kdDelta2",-0.1);
  }

//_______________________________5 SAVED
  void chillFM2(){
    int notes[] = {48, 50, 52, 55, 57, 60, 62, 64, 67, 69, 72, 74, 76, 79, 81};//, 84, 86, 88, 91, 93, 96, 98, 100, 103};
    int freq;
    int noteRange=15;
    
    synth.setParamValue("fmGateT1",1);/*Modulator Mult*/
    synth.setParamValue("fmGateT2",1);/*Modulator Mult*/
    synth.setParamValue("fmGateT3",1);/*Modulator Mult*/
    synth.setParamValue("fmVerb",1);
    synth.setParamValue("fmVol",map(constrain(section,0,0.15),0,0.15,0,0.2)*map(load,0,1,0.3,1));
    synth.setParamValue("kroneVol",map(constrain(section,0,0.15),0,0.15,1,0));
    synth.setParamValue("fm1vol", 1);
    synth.setParamValue("fm2vol", 1);
    synth.setParamValue("fm3vol", 1);
    synth.setParamValue("kd1vol", 0.8);
    synth.setParamValue("kd2vol", 0.8);
    
    synth.setParamValue("bpm",1 + load*225);/*Modulator Mult*/
    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq1", freq);
    synth.setParamValue("fmMm1",2);/*Modulator Mult*/
    synth.setParamValue("fmDp1",1);/*Modulator Depth*/
    synth.setParamValue("fmAC1",3-(load*2.65));/*Carrier Attack*/
    synth.setParamValue("fmRC1",3-(load*2.65));/*Carrier Release*/
    synth.setParamValue("fmAM1",3-(load*2.65));/*Modulator Attack*/
    synth.setParamValue("fmRM1",3-(load*2.65));/*Modulator Release*/
    synth.setParamValue("fmEucNo1",11);
    float mod = map(x,-1,1,0.5,10);
    synth.setParamValue("fmModWheel1",mod);/*Mod Wheel*/

    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq2", freq);
    synth.setParamValue("fmMm2",2);/*Modulator Mult*/
    synth.setParamValue("fmDp2",1);/*Modulator Depth*/
    synth.setParamValue("fmAC2",3-(load*2.65));/*Carrier Attack*/
    synth.setParamValue("fmRC2",3-(load*2.65));/*Carrier Release*/
    synth.setParamValue("fmAM2",3-(load*2.65));/*Modulator Attack*/
    synth.setParamValue("fmRM2",3-(load*2.65));/*Modulator Release*/
    synth.setParamValue("fmEucNo2",3);
    mod = map(y,-1,1,0.5,10);
    synth.setParamValue("fmModWheel1",mod);/*Mod Wheel*/

    freq = random(noteRange);
    freq = notes[freq];
    synth.setParamValue("fmFreq3", freq);
    synth.setParamValue("fmMm3",2);/*Modulator Mult*/
    synth.setParamValue("fmDp3",1);/*Modulator Depth*/
    synth.setParamValue("fmAC3",3-(load*2.65));/*Carrier Attack*/
    synth.setParamValue("fmRC3",3-(load*2.65));/*Carrier Release*/
    synth.setParamValue("fmAM3",3-(load*2.65));/*Modulator Attack*/
    synth.setParamValue("fmRM3",3-(load*2.65));/*Modulator Release*/
    synth.setParamValue("fmEucNo3",7);
    mod = map(z,-1,1,0.1,10);
    synth.setParamValue("fmModWheel1", mod);/*Mod Wheel*/

    freq=random(10)+20;
    synth.setParamValue("kdGateT1", 1);
    synth.setParamValue("kdNxtFreq1",freq-random(3));
    synth.setParamValue("kdEucNo1",12);
    synth.setParamValue("kdA1",3+(random(1000)*0.001));
    synth.setParamValue("kdR1",3+(random(1000)*0.003));
    synth.setParamValue("kdDelta1",-0.12);

    
    synth.setParamValue("kdGateT2", 1);
    synth.setParamValue("kdNxtFreq2", freq+random(3));
    synth.setParamValue("kdEucNo2",12);
    synth.setParamValue("kdA2",3+(random(1000)*0.001));
    synth.setParamValue("kdR2",3+(random(1000)*0.003));
    synth.setParamValue("kdDelta2",-0.1);
  };

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
      playScore();
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
    section=0.8;
    struggle();
    delay(10);
  }
}
