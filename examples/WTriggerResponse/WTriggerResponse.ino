// ****************************************************************************
//       Sketch: WTriggerResponse
// Date Created: 7/7/2016
//
//     Comments: Demonstrates polling and capturing responses from WAV trigger
//
//  Programmers: Jamie Robertson, jamie@robertsonics.com
//
// ****************************************************************************
//
// To use this sketch, you'll need to:
//
// 1) Download and install the AltSoftSerial library.
// 2) Connect 2 wires from the UNO to the WAV Trigger's serial connector:
//
//    Uno           WAV Trigger
//    ===           ===========
//    GND  <------> GND
//    Pin9 <------> RX
//    Pin8 <------> TX
//
//    If you want to power the WAV Trigger from the Uno, then close the 5V
//    solder jumper on the WAV Trigger and connect a 3rd wire:
//
//    5V   <------> 5V
//
// 4) Download and install the demo wav files onto the WAV Trigger's microSD
//    card. You can find them here:
//
//
//    You can certainly use your own tracks instead, although the demo may
//    not make as much sense. If you do, make sure your tracks are at least 10
//    to 20 seconds long and have no silence at the start of the file.

#include <AltSoftSerial.h>    // Arduino build environment requires this
#include <wavTrigger.h>

wavTrigger wTrig;             // Our WAV Trigger object

int volume = 0;
uint8_t* sysVersion;
uint8_t tracksPlayingCount;
uint16_t* tracksPlaying;


// ****************************************************************************
void setup() {

  // Serial monitor
  Serial.begin(9600);
  Serial.println("Hit any key to start");
  while (!Serial.available()) {}
  Serial.println("Starting!");

  // WAV Trigger startup at 57600
  wTrig.start();

  // If the Uno is powering the WAV Trigger, we should wait for the WAV Trigger
  //  to finish reset before trying to send commands.
  delay(1000);

  // If we're not powering the WAV Trigger, send a stop-all command in case it
  //  was already playing tracks. If we are powering the WAV Trigger, it doesn't
  //  hurt to do this.
  wTrig.stopAllTracks();
  // Also, this sketch controls volume, so lets reset this as well
  volume = -20;
  wTrig.masterGain(volume);

  // flush serial
  Serial.flush();

}


// ****************************************************************************
// This programs waits for user input and then executes that input

void loop() {
  optionsPrint();
  Serial.println();
  while (!Serial.available()) {
    // hold here for user input
  }

  int incomingByte = Serial.read();
  parseFirstByte(incomingByte);
  Serial.println();
}

void optionsPrint() {
  Serial.println("     Options are: ");
  Serial.println("          (p) Play sound ");
  Serial.println("          (s) Get info from WAV Trigger ");
  Serial.println("          (S) Stop all sounds playing ");
  Serial.println("          (+) Increase volume 1dB ");
  Serial.println("          (m) Set volume to 0dB ");
  Serial.println("          (-) Decrease volume 1dB ");
}

void parseFirstByte(int firstByte) {
  switch (firstByte) {
    case 'p':
      playOptions();
      break;
    case 's':
      infoOptions();
      break;
    case 'S':
      wTrig.stopAllTracks();
      break;
    case '+':
      volume++;
      if (volume > 10 || volume < -70) {
        volume = 10;
      }
      wTrig.masterGain(volume);
      Serial.print("Volume set to: ");
      Serial.println(volume);
      break;
    case 'm':
      wTrig.masterGain(0);
      Serial.print("Volume set to: ");
      Serial.println(volume);
      break;
    case '-':
      volume--;
      if (volume > 10 || volume < -70) {
        volume = -70;
      }
      wTrig.masterGain(volume);
      Serial.print("Volume set to: ");
      Serial.println(volume);
      break;
    default:
      break;
  }
}

void playOptions() {
  Serial.println();
  Serial.println("     To play a sound, enter a four digit number. ");
  Serial.print("          Entered data: ");
  int trackNumber = 0;
  for (uint8_t i = 4; i > 0; i--) {
    while (!Serial.available()) {}
    int incomingByte = Serial.read() - '0';
    if (incomingByte >= 0 && incomingByte <= 9) {
      Serial.print(incomingByte);
      trackNumber += incomingByte * pow(10,i - 1);
    } else {
      Serial.print("Bad data entered");
      break;
    }
  }
  Serial.println();
  Serial.println();
  if (trackNumber > 0 && trackNumber <= 2048) {
    Serial.print("Playing track number: ");
    Serial.println(trackNumber);
    wTrig.trackPlayPoly(trackNumber);
  } else {
    Serial.print("Bad track number: ");
    Serial.println(trackNumber);
  }
}

void infoOptions() {
  Serial.println();
  Serial.println("     Info options are: ");
  Serial.println("          (v) Version of WAV Trigger ");
  Serial.println("          (t) Tracks and voices available ");
  Serial.println("          (l) Tracks playing ");
  Serial.println("          (n) Number of tracks playing ");
  Serial.println();

  while (!Serial.available()) {}

  int incomingByte = Serial.read();
  switch (incomingByte) {
    case 'v':
      wTrig.getVersion();
      sysVersion = wTrig.returnSysVersion();
      Serial.print("Sys Version: ");
      for (uint8_t i = 0; i < 25; i++) {
        Serial.print(sysVersion[i], HEX);
      }
      Serial.println();
      break;
    case 't':
      wTrig.getSysInfo();
      Serial.print("Number of tracks: ");
      Serial.print(wTrig.returnSysinfoTracks());
      Serial.print(" -- Number of voices: ");
      Serial.print(wTrig.returnSysinfoVoices());
      Serial.println();
      break;
    case 'l':
      wTrig.getStatus();
      tracksPlayingCount = wTrig.returnTracksPlayingCount();
      tracksPlaying = wTrig.returnTracksPlaying();
      if (tracksPlayingCount < 1) {
        Serial.println("No tracks playing");
      } else {
        Serial.print("Tracks playing: ");
        for (uint8_t i = 0; i < tracksPlayingCount; i++) {
          Serial.print(tracksPlaying[i]);
          Serial.print(" ");
        }
        Serial.println();
      }
      break;
    case 'n':
      wTrig.getStatus();
      Serial.print("Number of tracks playing: ");
      Serial.println(wTrig.returnTracksPlayingCount());
      break;
    default:
      Serial.println("Bad command");
      break;
  }
}
