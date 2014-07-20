// **************************************************************
//     Filename: wavTrigger.cpp
// Date Created: 2/23/2014
//
//     Comments: Robertsonics WAV Trigger serial control library
//
// Programmers: Jamie Robertson, jamie@robertsonics.com
//
// **************************************************************

#include "wavTrigger.h"


// **************************************************************
void wavTrigger::start(void) {

  WTSerial.begin(57600);
}

// **************************************************************
void wavTrigger::masterGain(int gain) {

byte txbuf[8];
unsigned short vol;

  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_MASTER_VOLUME;
  vol = (unsigned short)gain;
  txbuf[4] = (byte)vol;
  txbuf[5] = (byte)(vol >> 8);
  txbuf[6] = 0x55;
  WTSerial.write(txbuf, 7);
}

// **************************************************************
void wavTrigger::trackPlaySolo(int trk) {
  
  trackControl(trk, TRK_PLAY_SOLO);
}

// **************************************************************
void wavTrigger::trackPlayPoly(int trk) {
  
  trackControl(trk, TRK_PLAY_POLY);
}

// **************************************************************
void wavTrigger::trackLoad(int trk) {
  
  trackControl(trk, TRK_LOAD);
}

// **************************************************************
void wavTrigger::trackStop(int trk) {

  trackControl(trk, TRK_STOP);
}

// **************************************************************
void wavTrigger::trackPause(int trk) {

  trackControl(trk, TRK_PAUSE);
}

// **************************************************************
void wavTrigger::trackResume(int trk) {

  trackControl(trk, TRK_RESUME);
}

// **************************************************************
void wavTrigger::trackLoop(int trk, bool enable) {
 
  if (enable)
    trackControl(trk, TRK_LOOP_ON);
  else
    trackControl(trk, TRK_LOOP_OFF);
}

// **************************************************************
void wavTrigger::trackControl(int trk, int code) {
  
byte txbuf[8];

  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x08;
  txbuf[3] = CMD_TRACK_CONTROL;
  txbuf[4] = (byte)code;
  txbuf[5] = (byte)trk;
  txbuf[6] = (byte)(trk >> 8);
  txbuf[7] = 0x55;
  WTSerial.write(txbuf, 8);
}

// **************************************************************
void wavTrigger::stopAllTracks(void) {

byte txbuf[5];

  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_STOP_ALL;
  txbuf[4] = 0x55;
  WTSerial.write(txbuf, 5);
}

// **************************************************************
void wavTrigger::resumeAllInSync(void) {

byte txbuf[5];

  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_RESUME_ALL_SYNC;
  txbuf[4] = 0x55;
  WTSerial.write(txbuf, 5);
}

// **************************************************************
void wavTrigger::trackGain(int trk, int gain) {

byte txbuf[9];
unsigned short vol;

  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x09;
  txbuf[3] = CMD_TRACK_VOLUME;
  txbuf[4] = (byte)trk;
  txbuf[5] = (byte)(trk >> 8);
  vol = (unsigned short)gain;
  txbuf[6] = (byte)vol;
  txbuf[7] = (byte)(vol >> 8);
  txbuf[8] = 0x55;
  WTSerial.write(txbuf, 9);
}

// **************************************************************
void wavTrigger::trackFade(int trk, int gain, int time, bool stopFlag) {

byte txbuf[12];
unsigned short vol;

  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x0c;
  txbuf[3] = CMD_TRACK_FADE;
  txbuf[4] = (byte)trk;
  txbuf[5] = (byte)(trk >> 8);
  vol = (unsigned short)gain;
  txbuf[6] = (byte)vol;
  txbuf[7] = (byte)(vol >> 8);
  txbuf[8] = (byte)time;
  txbuf[9] = (byte)(time >> 8);
  txbuf[10] = stopFlag;
  txbuf[11] = 0x55;
  WTSerial.write(txbuf, 12);
}

// **************************************************************
void wavTrigger::trackCrossFade(int trkFrom, int trkTo, int gain, int time) {

byte txbuf[12];
unsigned short vol;

  // Start the To track with -40 dB gain
  trackGain(trkTo, -40);
  trackPlayPoly(trkTo);

  // Start a fade-in to the target volume
  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x0c;
  txbuf[3] = CMD_TRACK_FADE;
  txbuf[4] = (byte)trkTo;
  txbuf[5] = (byte)(trkTo >> 8);
  vol = (unsigned short)gain;
  txbuf[6] = (byte)vol;
  txbuf[7] = (byte)(vol >> 8);
  txbuf[8] = (byte)time;
  txbuf[9] = (byte)(time >> 8);
  txbuf[10] = 0x00;
  txbuf[11] = 0x55;
  WTSerial.write(txbuf, 12);

  // Start a fade-out on the From track
  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x0c;
  txbuf[3] = CMD_TRACK_FADE;
  txbuf[4] = (byte)trkFrom;
  txbuf[5] = (byte)(trkFrom >> 8);
  vol = (unsigned short)-40;
  txbuf[6] = (byte)vol;
  txbuf[7] = (byte)(vol >> 8);
  txbuf[8] = (byte)time;
  txbuf[9] = (byte)(time >> 8);
  txbuf[10] = 0x01;
  txbuf[11] = 0x55;
  WTSerial.write(txbuf, 12);
}





