// **************************************************************
//     Filename: wavTrigger.cpp
// Date Created: 2/23/2014
//
//     Comments: Robertsonics WAV Trigger serial control library
//
// Programmers: Jamie Robertson, jamie@robertsonics.com
//
// **************************************************************

#include "Arduino.h"

#include "wavTrigger.h"

// **************************************************************
void wavTrigger::start(void) {
  WTSerial.begin(57600);
}

// **************************************************************
void wavTrigger::getVersion(void) {
  wavTrigger::response(CMD_GET_VERSION);
}
void wavTrigger::getSysInfo(void) {
  wavTrigger::response(CMD_GET_SYS_INFO);
}
void wavTrigger::getStatus(void) {
  wavTrigger::response(CMD_GET_STATUS);
}

uint8_t* wavTrigger::returnSysVersion(void) {
  return sysVersion;
}
uint8_t wavTrigger::returnSysinfoVoices(void) {
  return sysinfoVoices;
}
uint16_t wavTrigger::returnSysinfoTracks(void) {
  return sysinfoTracks;
}
uint8_t wavTrigger::returnTracksPlayingCount(void) {
  return tracksPlayingCount;
}
uint16_t* wavTrigger::returnTracksPlaying(void) {
  return tracksPlaying;
}

// **************************************************************
void wavTrigger::response(uint8_t responseCommand) {

    WTSerial.flush();
    WTSerial.clear();

    uint8_t txbuf[5];

    txbuf[0] = HEAD_1;
    txbuf[1] = HEAD_2;
    txbuf[2] = 0x05;
    txbuf[3] = responseCommand;
    txbuf[4] = EOM;
    WTSerial.write(txbuf, 5);

    wavTrigger::readResponse(2000);
}

// **************************************************************
void wavTrigger::readResponse(unsigned long wait) {

  unsigned long stop = millis() + wait;
  while (
    WTSerial.available() < 5 &&
    millis() < stop
  ) {
    // wait until we have some data back, or timeout
  }

  if (WTSerial.available() >= 5) {

    uint8_t i = 0;
    while (WTSerial.available()) {
      packet[i] = WTSerial.read();
      if (packet[i] == EOM) {
        break;
      } else {
        i++;
      }
      delay(20); // MCU was too fast so had to put in delay
    }

    // check packet
    if (
      packet[0] == HEAD_1 &&
      packet[1] == HEAD_2 &&
      packet[i] == EOM
    ) {
      // good packet! run trough parser
      wavTrigger::parseResponse();
    } else {
      // bad packet!
    }
  } else {
    // no serial data in timely manner
  }
}

// **************************************************************
void wavTrigger::parseResponse() {
  int8_t dataBytesCount = packet[2] - 5;
  switch (packet[3]) {

    case RSP_VERSION_STRING:
      for (uint8_t j = 4; j < dataBytesCount + 4; j++) {
        sysVersion[j - 4] = packet[j];
      }
      break;

    case RSP_SYS_INFO:
      sysinfoVoices = packet[4];
      sysinfoTracks = (packet[6] << 8) | packet[5];
      break;

    case RSP_STATUS:
      if (dataBytesCount < 2) {
        tracksPlayingCount = 0;
      } else {
        tracksPlayingCount = dataBytesCount / 2;
        uint8_t k = 0;
        for (int8_t j = 4; j < dataBytesCount + 4; j++, k++) {
          tracksPlaying[k] = (packet[j + 1] << 8) | packet[j++];
        }
      }
      break;

    default:
      break;
  }
}

// **************************************************************
void wavTrigger::masterGain(int gain) {

uint8_t txbuf[8];
unsigned short vol;

  txbuf[0] = HEAD_1;
  txbuf[1] = HEAD_2;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_MASTER_VOLUME;
  vol = (unsigned short)gain;
  txbuf[4] = (uint8_t)vol;
  txbuf[5] = (uint8_t)(vol >> 8);
  txbuf[6] = EOM;
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

  uint8_t txbuf[8];

  txbuf[0] = HEAD_1;
  txbuf[1] = HEAD_2;
  txbuf[2] = 0x08;
  txbuf[3] = CMD_TRACK_CONTROL;
  txbuf[4] = (uint8_t)code;
  txbuf[5] = (uint8_t)trk;
  txbuf[6] = (uint8_t)(trk >> 8);
  txbuf[7] = EOM;
  WTSerial.write(txbuf, 8);
}

// **************************************************************
void wavTrigger::stopAllTracks(void) {

  uint8_t txbuf[5];

  txbuf[0] = HEAD_1;
  txbuf[1] = HEAD_2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_STOP_ALL;
  txbuf[4] = EOM;
  WTSerial.write(txbuf, 5);
}

// **************************************************************
void wavTrigger::resumeAllInSync(void) {

  uint8_t txbuf[5];

  txbuf[0] = HEAD_1;
  txbuf[1] = HEAD_2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_RESUME_ALL_SYNC;
  txbuf[4] = EOM;
  WTSerial.write(txbuf, 5);
}

// **************************************************************
void wavTrigger::trackGain(int trk, int gain) {

  uint8_t txbuf[9];
  unsigned short vol;

  txbuf[0] = HEAD_1;
  txbuf[1] = HEAD_2;
  txbuf[2] = 0x09;
  txbuf[3] = CMD_TRACK_VOLUME;
  txbuf[4] = (uint8_t)trk;
  txbuf[5] = (uint8_t)(trk >> 8);
  vol = (unsigned short)gain;
  txbuf[6] = (uint8_t)vol;
  txbuf[7] = (uint8_t)(vol >> 8);
  txbuf[8] = EOM;
  WTSerial.write(txbuf, 9);
}

// **************************************************************
void wavTrigger::trackFade(int trk, int gain, int time, bool stopFlag) {

  uint8_t txbuf[12];
  unsigned short vol;

  txbuf[0] = HEAD_1;
  txbuf[1] = HEAD_2;
  txbuf[2] = 0x0c;
  txbuf[3] = CMD_TRACK_FADE;
  txbuf[4] = (uint8_t)trk;
  txbuf[5] = (uint8_t)(trk >> 8);
  vol = (unsigned short)gain;
  txbuf[6] = (uint8_t)vol;
  txbuf[7] = (uint8_t)(vol >> 8);
  txbuf[8] = (uint8_t)time;
  txbuf[9] = (uint8_t)(time >> 8);
  txbuf[10] = stopFlag;
  txbuf[11] = EOM;
  WTSerial.write(txbuf, 12);
}

// **************************************************************
void wavTrigger::trackCrossFade(int trkFrom, int trkTo, int gain, int time) {

  uint8_t txbuf[12];
  unsigned short vol;

  // Start the To track with -40 dB gain
  trackGain(trkTo, -40);
  trackPlayPoly(trkTo);

  // Start a fade-in to the target volume
  txbuf[0] = HEAD_1;
  txbuf[1] = HEAD_2;
  txbuf[2] = 0x0c;
  txbuf[3] = CMD_TRACK_FADE;
  txbuf[4] = (uint8_t)trkTo;
  txbuf[5] = (uint8_t)(trkTo >> 8);
  vol = (unsigned short)gain;
  txbuf[6] = (uint8_t)vol;
  txbuf[7] = (uint8_t)(vol >> 8);
  txbuf[8] = (uint8_t)time;
  txbuf[9] = (uint8_t)(time >> 8);
  txbuf[10] = 0x00;
  txbuf[11] = EOM;
  WTSerial.write(txbuf, 12);

  // Start a fade-out on the From track
  txbuf[0] = HEAD_1;
  txbuf[1] = HEAD_2;
  txbuf[2] = 0x0c;
  txbuf[3] = CMD_TRACK_FADE;
  txbuf[4] = (uint8_t)trkFrom;
  txbuf[5] = (uint8_t)(trkFrom >> 8);
  vol = (unsigned short)-40;
  txbuf[6] = (uint8_t)vol;
  txbuf[7] = (uint8_t)(vol >> 8);
  txbuf[8] = (uint8_t)time;
  txbuf[9] = (uint8_t)(time >> 8);
  txbuf[10] = 0x01;
  txbuf[11] = EOM;
  WTSerial.write(txbuf, 12);
}

// **************************************************************
void wavTrigger::samplerateOffset(int offset) {

  uint8_t txbuf[8];
  unsigned short off;

  txbuf[0] = HEAD_1;
  txbuf[1] = HEAD_2;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_SAMPLERATE_OFFSET;
  off = (unsigned short)offset;
  txbuf[4] = (uint8_t)off;
  txbuf[5] = (uint8_t)(off >> 8);
  txbuf[6] = EOM;
  WTSerial.write(txbuf, 7);
}
