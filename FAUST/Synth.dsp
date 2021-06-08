// SPACE TO BE RESONATOR INSTRUMENT v2.0
import("stdfaust.lib");

//-------------------------------------------CLOCKS
bpm=hslider("bpm",120,1,400,1);
barHz=bpm/(60*6);
step=os.phasor(48,barHz):int+1;

//-------------------------------------------UTILS
//___________________SELF BLOCKING ENV
envAR(att,rel,gate) = (_:1,_>0:- <: en.ar((_*gate,att:ba.sAndH),(_*gate,rel:ba.sAndH),(_*gate))) ~ _;

//-------------------------------------------FM-2OPs
//___________________VARIABLES
fmVol = hslider("fmVol",0.5,0,1,0.001):si.smoo;
fmVerb = hslider("fmVerb", 0.5,0,1,0.001):si.smoo;
//fm synth 1
fm1vol=hslider("fm1vol",1,0,1,0.001);
fmNxtFreq1 = hslider("fmFreq1",69,0,127,0.01):ba.midikey2hz;
fmMm1 = hslider("fmMm1",0.5,0.0,100,0.0001):si.smoo;
fmDp1 = hslider("fmDp1",0.5,0.00,10,0.0001):si.smoo;
fmAM1 = hslider("fmAM1",0.1,0.001,10,0.01);
fmRM1 = hslider("fmRM1",0.3,0.001,10,0.01);
fmAC1 = hslider("fmAC1",0.1,0.001,10,0.01);
fmRC1 = hslider("fmRC1",0.3,0.001,10,0.01);
fmTrigD1 = button("fmTrigD1");
fmGateT1 = checkbox("fmGateT1");
fmEucNo1 = hslider("fmEucNo1", 7, 0, 24, 1);
fmModWheel1 = hslider("fmModWheel1", 1, 0.1, 100, 0.001):si.smoo;

//fm synth 2
fm2vol=hslider("fm2vol",1,0,1,0.001);
fmNxtFreq2 = hslider("fmFreq2",62,0,127,0.01):ba.midikey2hz;
fmMm2 = hslider("fmMm2",0.5,0.0,100,0.0001):si.smoo;
fmDp2 = hslider("fmDp2",0.5,0.00,10,0.0001):si.smoo;
fmAM2 = hslider("fmAM2",0.1,0.001,10,0.01);
fmRM2 = hslider("fmRM2",0.3,0.001,10,0.01);
fmAC2 = hslider("fmAC2",0.1,0.001,10,0.01);
fmRC2 = hslider("fmRC2",0.3,0.001,10,0.01);
fmTrigD2 = button("fmTrigD2");
fmGateT2 = checkbox("fmGateT2");
fmEucNo2 = hslider("fmEucNo2", 9, 0, 24, 1);
fmModWheel2 = hslider("fmModWheel2", 1, 0.1, 100, 0.001):si.smoo;

//fm synth 3
fm3vol=hslider("fm3vol",1,0,1,0.001);
fmNxtFreq3 = hslider("fmFreq3",52,0,127,0.01):ba.midikey2hz;
fmMm3 = hslider("fmMm3",0.5,0.0,100,0.0001):si.smoo;
fmDp3 = hslider("fmDp3",0.5,0.00,10,0.0001):si.smoo;
fmAM3 = hslider("fmAM3",0.1,0.001,10,0.01);
fmRM3 = hslider("fmRM3",0.3,0.001,10,0.01);
fmAC3 = hslider("fmAC3",0.1,0.001,10,0.01);
fmRC3 = hslider("fmRC3",0.3,0.001,10,0.01);
fmTrigD3 = button("fmTrigD3");
fmGateT3 = checkbox("fmGateT3");
fmEucNo3 = hslider("fmEucNo3", 16, 0, 24, 1);
fmModWheel3 = hslider("fmModWheel3", 1, 0.1, 100, 0.001):si.smoo;

//___________________DSP

//fm synth 1 
fmEuc1 = ((step*fmEucNo1) % 48) < fmEucNo1 : en.ar(0,0.001);
fmRawTrig1 = fmTrigD1+(fmEuc1*fmGateT1);
fmTrig1 = envAR(0, fmAC1+fmRC1, fmRawTrig1):en.ar(0,0.001);
fmFreq1 = fmNxtFreq1:ba.sAndH(fmTrig1);
fmac1 = fmAC1:ba.sAndH(fmTrig1);
fmrc1 = fmRC1:ba.sAndH(fmTrig1);
fmam1 = fmAM1:ba.sAndH(fmTrig1);
fmrm1 = fmRM1:ba.sAndH(fmTrig1);
fm2op1(freq,mMul,dMul,trig,am,rm,ac,rc) =  
    os.osc(
        freq+(  os.osc(freq*mMul)*freq*dMul*en.ar(am,rm,trig)  )
    )*envAR(ac,rc, trig)*fm1vol;
fmDpm1 = fmDp1*fmModWheel1;
//fm synth 2
fmEuc2 = ((step*fmEucNo2) % 48) < fmEucNo2 : en.ar(0,0.001);
fmRawTrig2 = fmTrigD2+(fmEuc2*fmGateT2);
fmTrig2 = envAR(0, fmAC2+fmRC2, fmRawTrig2):en.ar(0,0.001);
fmFreq2 = fmNxtFreq2:ba.sAndH(fmTrig2);
fmac2 = fmAC2:ba.sAndH(fmTrig2);
fmrc2 = fmRC2:ba.sAndH(fmTrig2);
fmam2 = fmAM2:ba.sAndH(fmTrig2);
fmrm2 = fmRM2:ba.sAndH(fmTrig2);
fm2op2(freq,mMul,dMul,trig,am,rm,ac,rc) =  
    os.osc(
        freq+(  os.osc(freq*mMul)*freq*dMul*en.ar(am,rm,trig)  )
    )*envAR(ac,rc, trig)*fm2vol;
fmDpm2 = fmDp2*fmModWheel2;
//fm synth 3
fmEuc3 = ((step*fmEucNo3) % 48) < fmEucNo3 : en.ar(0,0.001);
fmRawTrig3 = fmTrigD3+(fmEuc3*fmGateT3);
fmTrig3 = envAR(0, fmAC3+fmRC3, fmRawTrig3):en.ar(0,0.001);
fmFreq3 = fmNxtFreq3:ba.sAndH(fmTrig3);
fmac3 = fmAC3:ba.sAndH(fmTrig3);
fmrc3 = fmRC3:ba.sAndH(fmTrig3);
fmam3 = fmAM3:ba.sAndH(fmTrig3);
fmrm3 = fmRM3:ba.sAndH(fmTrig3);
fm2op3(freq,mMul,dMul,trig,am,rm,ac,rc) =  
    os.osc(
        freq+(  os.osc(freq*mMul)*freq*dMul*en.ar(am,rm,trig)  )
    )*envAR(ac,rc, trig)*fm3vol;
fmDpm3 = fmDp3*fmModWheel3;
//fm group
dry = 1-(fmVerb);
fmSynths = (
    fm2op1(fmFreq1,fmMm1,fmDpm1,fmTrig1, fmam1,fmrm1,fmac1,fmrc1)+
    fm2op2(fmFreq2,fmMm2,fmDpm2,fmTrig2, fmam2,fmrm2,fmac2,fmrc2)+
    fm2op3(fmFreq3,fmMm3,fmDpm3,fmTrig3, fmam3,fmrm3,fmac3,fmrc3)
)<: (_*dry),(_*fmVerb) : _,(_:re.mono_freeverb(0.88,0.2,0.8,0)) :+;


// re.mono_freeverb(0.5,0.9,0.1,0.5)
//-------------------------------------------KICK/DRONEs
//___________________VARIABLES
kroneVol = hslider("kroneVol",0.5,0,1,0.001);
//krone1
kd1vol=hslider("kd1vol",1,0,1,0.001);
kdTrig1 = button("kdTrig1");
kdNxtFreq1 = hslider("kdNxtFreq1", 30, 0, 127, 1):ba.midikey2hz;
kdDelta1 = hslider("kdDelta1", -0.9,-0.99,5,0.001);
kdA1 = hslider("kdA1", 0.01, 0, 15, 0.01);
kdR1 = hslider("kdR1", 0.09, 0, 15, 0.01);
kdTrigD1 = button("kdTrigD1");
kdGateT1 = checkbox("kdGateT1");
kdEucNo1 = hslider("kdEucNo1", 16, 0, 24, 1);

//krone2
kd2vol=hslider("kd2vol",1,0,1,0.001);
kdTrig2 = button("kdTrig2");
kdNxtFreq2 = hslider("kdNxtFreq2", 34, 0, 127, 1):ba.midikey2hz;
kdDelta2 = hslider("kdDelta2", -0.1,-0.99,5,0.001);
kdA2 = hslider("kdA2", 2, 0, 15, 0.01);
kdR2 = hslider("kdR2", 2, 0, 15, 0.01);
kdTrigD2 = button("kdTrigD2");
kdGateT2 = checkbox("kdGateT2");
kdEucNo2 = hslider("kdEucNo2", 1, 0, 24, 1);

//___________________DSP
//krone1
kdEuc1 = ((step*kdEucNo1) % 48) < kdEucNo1 : en.ar(0,0.001);
kdRawTrig1 = kdTrig1+(kdEuc1*kdGateT1); 
kdEnvT1 = envAR(kdA1+kdR1, 0, kdRawTrig1);
kdGate1 = 1-en.ar(0,0.001,kdEnvT1);
kdFreq1 = kdNxtFreq1:ba.sAndH(kdGate1);
kddelta1 = kdDelta1:ba.sAndH(kdGate1);
kdRamp1 = kdFreq1+(kdFreq1*kddelta1*kdEnvT1);
kdEnv1 = envAR(kdA1, kdR1, kdRawTrig1);
krone1 = os.triangle(kdRamp1)*kdEnv1*kd1vol;

//krone2
kdEuc2 = ((step*kdEucNo2) % 48) < kdEucNo2 : en.ar(0,0.001);
kdRawTrig2 = kdTrig2+(kdEuc2*kdGateT2); 
kdEnvT2 = envAR(kdA2+kdR2, 0, kdRawTrig2);
kdGate2 = 1-en.ar(0,0.001,kdEnvT2);
kdFreq2 = kdNxtFreq2:ba.sAndH(kdGate2);
kddelta2 = kdDelta2:ba.sAndH(kdGate2);
kdRamp2 = kdFreq2+(kdFreq2*kddelta2*kdEnvT2);
kdEnv2 = envAR(kdA2, kdR2, kdRawTrig2);
krone2 = os.triangle(kdRamp2)*kdEnv2*kd2vol;


 
//-------------------------------------------OUTPUT
high = fmSynths:co.compressor_mono(20,-15,0.01,0.02)*fmVol;
low = krone1+krone2+(high:fi.lowpass6e(150)*0.2):co.compressor_mono(20,-15,0.01,0.02)*kroneVol;
process = high, low;

//-------------------------------------------TESTING
// process = fmEuc1;
// process = fmSynths:co.compressor_mono(20, -15, 0.01, 0.02);
// process = krone1+krone2:co.limiter_1176_R4_mono;
// process = kdFreq1;
// process = swerc1+swerc2:co.limiter_1176_R4_mono;s