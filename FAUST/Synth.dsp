// SPACE TO BE RESONATOR INSTRUMENT v2.0
import("stdfaust.lib");

//-------------------------------------------CLOCKS
//-------------------------------------------FM-2OPs
//___________________VARIABLES
//fm synth 1
fmDur1 = hslider("fmDur1",0.3, 0.001, 30, 0.001);
fmTrig1 = en.ar(0, fmDur1, button("fmTrig1"));
fmFreq1 = hslider("fmFreq1",150,10,20000,0.01):si.smoo;
fmMm1 = hslider("fmMm1",0.5,0.0,100,0.0001):si.smoo;
fmDp1 = hslider("fmDp1",0.5,0.00,10,0.0001):si.smoo;
fmAM1 = hslider("fmAM1",0.1,0.01,10,0.01);
fmRM1 = hslider("fmRM1",0.3,0.01,10,0.01);
fmAC1 = hslider("fmAC1",0.1,0.01,10,0.01);
fmRC1 = hslider("fmRC1",0.3,0.01,10,0.01);
//fm synth 2
fmDur2 = hslider("fmDur2",0.3, 0.001, 30, 0.001);
fmTrig2 = en.ar(0, fmDur2, button("fmTrig2"));
fmFreq2 = hslider("fmFreq2",620,10,20000,0.01):si.smoo;
fmMm2 = hslider("fmMm2",0.5,0.0,100,0.0001):si.smoo;
fmDp2 = hslider("fmDp2",0.5,0.00,10,0.0001):si.smoo;
fmAM2 = hslider("fmAM2",0.1,0.01,10,0.01);
fmRM2 = hslider("fmRM2",0.3,0.01,10,0.01);
fmAC2 = hslider("fmAC2",0.1,0.01,10,0.01);
fmRC2 = hslider("fmRC2",0.3,0.01,10,0.01);
//fm synth 3
fmDur3 = hslider("fmDur3",0.3, 0.001, 30, 0.001);
fmTrig3 = en.ar(0, fmDur3, button("fmTrig3"));
fmFreq3 = hslider("fmFreq3",1200,10,20000,0.01):si.smoo;
fmMm3 = hslider("fmMm3",0.5,0.0,100,0.0001):si.smoo;
fmDp3 = hslider("fmDp3",0.5,0.00,10,0.0001):si.smoo;
fmAM3 = hslider("fmAM3",0.1,0.01,10,0.01);
fmRM3 = hslider("fmRM3",0.3,0.01,10,0.01);
fmAC3 = hslider("fmAC3",0.1,0.01,10,0.01);
fmRC3 = hslider("fmRC3",0.3,0.01,10,0.01);

//___________________DSP
//fm synth 1
fm2op1(freq,mMul,dMul,trig,am,rm,ac,rc) =  
    os.osc(
        freq+(  os.osc(freq*mMul)*freq*dMul*en.asr(am,1,rm,trig)  )
    )*en.asr(ac,1,rc, trig);
//fm synth 2
fm2op2(freq,mMul,dMul,trig,am,rm,ac,rc) =  
    os.osc(
        freq+(  os.osc(freq*mMul)*freq*dMul*en.asr(am,1,rm,trig)  )
    )*en.asr(ac,1,rc, trig);
//fm synth 3
fm2op3(freq,mMul,dMul,trig,am,rm,ac,rc) =  
    os.osc(
        freq+(  os.osc(freq*mMul)*freq*dMul*en.asr(am,1,rm,trig)  )
    )*en.asr(ac,1,rc, trig);
//fm group
fmSynths = (
    fm2op1(fmFreq1,fmMm1,fmDp1,fmTrig1, fmAM1,fmRM1,fmAC1,fmRC1)+
    fm2op2(fmFreq2,fmMm2,fmDp2,fmTrig2, fmAM2,fmRM2,fmAC2,fmRC2)+
    fm2op3(fmFreq3,fmMm3,fmDp3,fmTrig3, fmAM3,fmRM3,fmAC3,fmRC3)
):re.mono_freeverb(0.7,0.9,0.1,0.5)*(0.3);


// re.mono_freeverb(0.5,0.9,0.1,0.5)
//-------------------------------------------KICK/DRONEs
//___________________VARIABLES
//krone1
kdTrig1 = button("kdTrig1");
kdFreq1 = hslider("kdFreq1", 150, 10, 600, 1);
kdDelta1 = hslider("kdDelta1", -0.9,-0.99,5,0.001);
kdA1 = hslider("kdA1", 0.01, 0, 15, 0.01);
kdR1 = hslider("kdR1", 0.09, 0, 15, 0.01);

//krone2
kdTrig2 = button("kdTrig2");
kdFreq2 = hslider("kdFreq2", 300, 10, 600, 1);
kdDelta2 = hslider("kdDelta2", -0.9,-0.99,5,0.001);
kdA2 = hslider("kdA2", 2, 0, 15, 0.01);
kdR2 = hslider("kdR2", 2, 0, 15, 0.01);

//___________________DSP
//krone1
kdEnvT1 = en.ar(kdA1+kdR1, 0, kdTrig1);
kdRamp1 = kdFreq1+(kdFreq1*kdDelta1*kdEnvT1);
kdEnv1 = en.ar(kdA1, kdR1, kdTrig1);
krone1 = os.triangle(kdRamp1)*kdEnv1;

//krone2
kdEnvT2 = en.ar(kdA2+kdR2, 0, kdTrig2);
kdRamp2 = kdFreq2+(kdFreq2*kdDelta2*kdEnvT2);
kdEnv2 = en.ar(kdA2, kdR2, kdTrig2);
krone2 = os.triangle(kdRamp2)*kdEnv2;







//-------------------------------------------SWOOSH/PERC

//-------------------------------------------OUTPUT


//-------------------------------------------TESTING
// process = fmSynths:co.limiter_1176_R4_mono ;
// process = krone1+krone2:co.limiter_1176_R4_mono;