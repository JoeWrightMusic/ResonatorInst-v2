// SPACE TO BE RESONATOR INSTRUMENT v2.0
import("stdfaust.lib");

//-------------------------------------------CLOCKS
//-------------------------------------------FM-2OPs
//___________________VARIABLES
fmDur1 = hslider("fmDur1",0.3, 0.001, 30, 0.001);
fmTrig1 = en.ar(0, fmDur1, button("fmTrig1"));
fmFreq1 = hslider("fmFreq1",200,10,20000,0.01):si.smoo;
fmMm1 = hslider("fmMm1",0.5,0.0,100,0.0001):si.smoo;
fmDp1 = hslider("fmDp1",0.5,0.00,10,0.0001):si.smoo;
fmAM1 = hslider("fmAM1",0.1,0.01,10,0.01);
fmRM1 = hslider("fmRM1",0.3,0.01,10,0.01);
fmAC1 = hslider("fmAC1",0.1,0.01,10,0.01);
fmRC1 = hslider("fmRC1",0.3,0.01,10,0.01);

//___________________DSP
fm2op1(freq,mMul,dMul,trig,am,rm,ac,rc) =  
os.osc(
        freq+(os.osc(freq*mMul)*freq*dMul*en.asr(am,1,rm,trig))
)*en.asr(ac,1,rc, trig);

process = fm2op1(fmFreq1,fmMm1,fmDp1,fmTrig1, fmAM1,fmRM1,fmAC1,fmRC1);
// re.mono_freeverb(0.5,0.9,0.1,0.5)
//-------------------------------------------KICK/DRONEs
//-------------------------------------------SWOOSH/PERC
//-------------------------------------------VERB/FILT
//-------------------------------------------OUTPUT