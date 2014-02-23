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
void wavTrigger::trackPlaySolo(int trk) {
  
  trackControl(trk, TRK_PLAY_SOLO);
}

// **************************************************************
void wavTrigger::trackPlayPoly(int trk) {
  
  trackControl(trk, TRK_PLAY_POLY);
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





