// SPACE TO BE RESONATOR INSTRUMENT v2.0
import("stdfaust.lib");

//-------------------------------------------CLOCKS
bpm=hslider("bpm",120,1,400,1);
barHz=bpm/(60*6);
step=os.phasor(48,barHz):int+1;

//-------------------------------------------FM-2OPs
//___________________VARIABLES
//fm synth 1
fmNxtFreq1 = hslider("fmFreq1",150,10,20000,0.01):si.smoo;
fmMm1 = hslider("fmMm1",0.5,0.0,100,0.0001):si.smoo;
fmDp1 = hslider("fmDp1",0.5,0.00,10,0.0001):si.smoo;
fmAM1 = hslider("fmAM1",0.1,0.001,10,0.01);
fmRM1 = hslider("fmRM1",0.3,0.001,10,0.01);
fmAC1 = hslider("fmAC1",0.1,0.001,10,0.01);
fmRC1 = hslider("fmRC1",0.3,0.001,10,0.01);
fmTrigD1 = button("fmTrigD1");
fmGateT1 = checkbox("fmGateT1");
fmEucNo1 = hslider("fmEucNo1", 7, 0, 24, 1);
fmModWheel1 = hslider("fmModWheel1", 1, 0.1, 1.9, 0.001);

//fm synth 2
fmNxtFreq2 = hslider("fmFreq2",620,10,20000,0.01):si.smoo;
fmMm2 = hslider("fmMm2",0.5,0.0,100,0.0001):si.smoo;
fmDp2 = hslider("fmDp2",0.5,0.00,10,0.0001):si.smoo;
fmAM2 = hslider("fmAM2",0.1,0.001,10,0.01);
fmRM2 = hslider("fmRM2",0.3,0.001,10,0.01);
fmAC2 = hslider("fmAC2",0.1,0.001,10,0.01);
fmRC2 = hslider("fmRC2",0.3,0.001,10,0.01);
fmTrigD2 = button("fmTrigD2");
fmGateT2 = checkbox("fmGateT2");
fmEucNo2 = hslider("fmEucNo2", 9, 0, 24, 1);
fmModWheel2 = hslider("fmModWheel2", 1, 0.1, 1.9, 0.001);

//fm synth 3
fmNxtFreq3 = hslider("fmFreq3",1200,10,20000,0.01):si.smoo;
fmMm3 = hslider("fmMm3",0.5,0.0,100,0.0001):si.smoo;
fmDp3 = hslider("fmDp3",0.5,0.00,10,0.0001):si.smoo;
fmAM3 = hslider("fmAM3",0.1,0.001,10,0.01);
fmRM3 = hslider("fmRM3",0.3,0.001,10,0.01);
fmAC3 = hslider("fmAC3",0.1,0.001,10,0.01);
fmRC3 = hslider("fmRC3",0.3,0.001,10,0.01);
fmTrigD3 = button("fmTrigD3");
fmGateT3 = checkbox("fmGateT3");
fmEucNo3 = hslider("fmEucNo3", 16, 0, 24, 1);
fmModWheel3 = hslider("fmModWheel3", 1, 0.1, 1.9, 0.001);

//___________________DSP
//fm synth 1 
fmEuc1 = ((step*fmEucNo1) % 48) < fmEucNo1 : en.ar(0,0.001);
fmRawTrig1 = fmTrigD1+(fmEuc1*fmGateT1);
fmTrig1 = en.ar(0, fmAC1+fmRC1, fmRawTrig1);
fmFreq1 = fmNxtFreq1:ba.sAndH(fmRawTrig1==1);
fm2op1(freq,mMul,dMul,trig,am,rm,ac,rc) =  
    os.osc(
        freq+(  os.osc(freq*mMul)*freq*dMul*en.asr(am,1,rm,trig)  )
    )*en.asr(ac,1,rc, trig);
fmDpm1 = fmDp1*fmModWheel1;
//fm synth 2
fmEuc2 = step*fmEucNo2 % 48 < fmEucNo2:en.ar(0,0.001);
fmRawTrig2 = fmTrigD2+(fmEuc2*fmGateT2);
fmTrig2 = en.ar(0, fmAC2+fmRC2, fmRawTrig2);
fmFreq2 = fmNxtFreq2:ba.sAndH(fmRawTrig2==1);
fm2op2(freq,mMul,dMul,trig,am,rm,ac,rc) =  
    os.osc(
        freq+(  os.osc(freq*mMul)*freq*dMul*en.asr(am,1,rm,trig)  )
    )*en.asr(ac,1,rc, trig);
fmDpm2 = fmDp2*fmModWheel2;
//fm synth 3
fmEuc3 = step*fmEucNo3 % 48 < fmEucNo3:en.ar(0,0.001);
fmRawTrig3 = fmTrigD3+(fmEuc3*fmGateT3);
fmTrig3 = en.ar(0, fmAC3+fmRC3, fmRawTrig3);
fmFreq3 = fmNxtFreq3:ba.sAndH(fmRawTrig3==1);
fm2op3(freq,mMul,dMul,trig,am,rm,ac,rc) =  
    os.osc(
        freq+(  os.osc(freq*mMul)*freq*dMul*en.asr(am,1,rm,trig)  )
    )*en.asr(ac,1,rc, trig);
fmDpm3 = fmDp3*fmModWheel3;
//fm group
fmSynths = (
    fm2op1(fmFreq1,fmMm1,fmDpm1,fmTrig1, fmAM1,fmRM1,fmAC1,fmRC1)+
    fm2op2(fmFreq2,fmMm2,fmDpm2,fmTrig2, fmAM2,fmRM2,fmAC2,fmRC2)+
    fm2op3(fmFreq3,fmMm3,fmDpm3,fmTrig3, fmAM3,fmRM3,fmAC3,fmRC3)
)<:_*(0.5),_:re.mono_freeverb(0.6,0.9,0.1,0.9)*(0.4)+_;


// re.mono_freeverb(0.5,0.9,0.1,0.5)
//-------------------------------------------KICK/DRONEs
//___________________VARIABLES
//krone1
kdTrig1 = button("kdTrig1");
kdNxtFreq1 = hslider("kdNxtFreq1", 150, 10, 600, 1);
kdDelta1 = hslider("kdDelta1", -0.9,-0.99,5,0.001);
kdA1 = hslider("kdA1", 0.01, 0, 15, 0.01);
kdR1 = hslider("kdR1", 0.09, 0, 15, 0.01);
kdTrigD1 = button("kdTrigD1");
kdGateT1 = checkbox("kdGateT1");
kdEucNo1 = hslider("kdEucNo1", 16, 0, 24, 1);

//krone2
kdTrig2 = button("kdTrig2");
kdNxtFreq2 = hslider("kdNxtFreq2", 80, 10, 600, 1);
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
kdEnvT1 = en.ar(kdA1+kdR1, 0, kdRawTrig1);
kdFreq1 = kdNxtFreq1:ba.sAndH(kdRawTrig1);
kdRamp1 = kdFreq1+(kdFreq1*kdDelta1*kdEnvT1);
kdEnv1 = en.ar(kdA1, kdR1, kdRawTrig1);
krone1 = os.triangle(kdRamp1)*kdEnv1;

//krone2
kdEuc2 = ((step*kdEucNo2) % 48) < kdEucNo2 : en.ar(0,0.001);
kdRawTrig2 = kdTrig2+(kdEuc2*kdGateT2); 
kdEnvT2 = en.ar(kdA2+kdR2, 0, kdRawTrig2);
kdFreq2 = kdNxtFreq2:ba.sAndH(kdRawTrig2);
kdRamp2 = kdFreq2+(kdFreq2*kdDelta2*kdEnvT2);
kdEnv2 = en.ar(kdA2, kdR2, kdRawTrig2);
krone2 = os.triangle(kdRamp2)*kdEnv2;

//-------------------------------------------SWOOSH/PERC (add if time)
//___________________VARIABLES
//swerc1
// spFreq1 = hslider("spFreq1", 1500, 100, 15000, 1);
// spTrig1 = button("spTrig1");
// spDelta1 = hslider("spDelta1",-0.5,-0.99,5,0.001);
// spA1 = hslider("spA1", 0.01, 0, 15, 0.01);
// spR1 = hslider("spR1", 0.01, 0, 15, 0.01);

// //swerc2
// spFreq2 = hslider("spFreq2", 1500, 100, 15000, 1);
// spTrig2 = button("spTrig2");
// spDelta2 = hslider("spDelta2",-0.5,-0.99,5,0.001);
// spA2 = hslider("spA2", 0.01, 0, 15, 0.01);
// spR2 = hslider("spR2", 0.01, 0, 15, 0.01);

//___________________DSP
//swerc1
// spEnTrig1 = en.ar(spA1+spR1,0,spTrig1);
// spRamp1 = spFreq1+(spFreq1*spDelta1*spEnTrig1);
// spEnv1 = en.ar(spA1, spR1, spTrig1);
// swerc1 = no.noise:fi.resonlp(spRamp1,5,0.5)*spEnv1;

// //swerc2
// spEnTrig2 = en.ar(spA2+spR2,0,spTrig2);
// spRamp2 = spFreq2+(spFreq2*spDelta2*spEnTrig2);
// spEnv2 = en.ar(spA2, spR2, spTrig2);
// swerc2 = no.noise:fi.resonlp(spRamp2,5,0.5)*spEnv2;
 
//-------------------------------------------OUTPUT
high = fmSynths:co.compressor_mono(20,-15,0.01,0.02);
low = krone1+krone2+(high:fi.lowpass6e(150)*0.2):co.compressor_mono(20,-15,0.01,0.02);
process = high, low;

//-------------------------------------------TESTING
// process = fmEuc1;
// process = fmSynths:co.compressor_mono(20, -15, 0.01, 0.02);
// process = krone1+krone2:co.limiter_1176_R4_mono;
// process = kdFreq1;
// process = swerc1+swerc2:co.limiter_1176_R4_mono;