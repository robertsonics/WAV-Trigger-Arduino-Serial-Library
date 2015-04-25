// ****************************************************************************
//       Sketch: WTriggerUno
// Date Created: 4/22/2015
//
//     Comments: Demonstrates basic serial control of the WAV Trigger from an
//               Arduino UNO.
//
//  Programmers: Jamie Robertson, jamie@robertsonics.com
//
// ****************************************************************************
//
// To use this sketch, you'll need to:
//
// 1) Download and install the AltSoftSerial library.
// 2) Download and install the Metro library.
// 3) Connect 2 wires from the UNO to the WAV Trigger's serial connector:
//
//    Uno           WAV Trigger
//    ===           ===========
//    GND  <------> GND
//    Pin9 <------> RX
//
//    If you want to power the WAV Trigger from the Uno, then close the 5V
//    solder jumper on the WAV Trigger and connect a 3rd wire:
//
//    5V   <------> 5V
//
// 4) Make sure you have tracks 1 through 4 installed on the WAV Trigger. This
//    means compatible (16-bit, stereo, 44.1kHz) wav files named 001xxx.wav
//    through 004xxx.wav.

#include "Metro.h"
#include "AltSoftSerial.h"    // Arduino build environment requires this
#include "wavTrigger.h"

#define LED 13                // our LED

wavTrigger wTrig;             // Our WAV Trigger object

Metro gLedMetro(500);         // LED blink interval timer
Metro gWTrigMetro(5000);      // WAV Trigger state machine interval timer

byte gLedState = 0;           // LED State
int  gWTrigState = 0;         // WAV Trigger state


// ****************************************************************************
void setup() {
  
  // Serial monitor
  Serial.begin(9600);
 
  // Initialize the LED pin
  pinMode(LED,OUTPUT);
  digitalWrite(LED,gLedState);

  // WAV Trigger startup at 57600
  wTrig.start();
  
  // If the Uno is powering the WAV Trigger, we should wait for the WAV Trigger
  //  to finish reset before trying to send commands.
  delay(1000);
  
  // If we're not powering the WAV Trigger, send a stop-all command in case it
  //  was already playing tracks. If we are powering the WAV Trigger, it doesn't
  //  hurt to do this.
  wTrig.stopAllTracks();
  
}


// ****************************************************************************
// This program uses a Metro timer to create a state machine that steps
//  through states at 5 second intervals - you can change this rate above.
//  Each state will execute a new wavTrigger command.

void loop() {  

  // If time to do so, perform the next WAV Trigger task and then increment
  //  the state machine variable 
  if (gWTrigMetro.check() == 1) {
      
      switch (gWTrigState) {
  
          // Fade in track 1    
          case 0:
              wTrig.trackGain(1, -40);              // Preset Track 1 gain to -40dB
              wTrig.trackPlayPoly(1);               // Start Track 1
              wTrig.trackFade(1, 0, 2000, 0);       // Fade Track 1 to 0dB
          break;

          case 1:
              wTrig.trackCrossFade(1, 2, 0, 2000);  // Cross-fade Track 1 to Track 2
          break;
                                 
          case 2:
              wTrig.trackPlayPoly(3);               // Start Track 3 poly
          break;

          case 3:
              wTrig.trackFade(3, -50, 5000, 1);     // Fade Track 3 to -50dB and stop
          break;
                    
          case 4:
              wTrig.stopAllTracks();                // Stop all tracks
          break;
           
      } // switch
 
      // Increment our state
      gWTrigState++;
      if (gWTrigState > 4)
          gWTrigState = 0;
          
  } // if (gWTrigState.check() == 1)
 
  // If time to do so, toggle the LED
  if (gLedMetro.check() == 1) {
      if (gLedState == 0) gLedState = 1;
      else gLedState = 0;
      digitalWrite(LED, gLedState);
  } // if (gLedMetro.check() == 1)

  // Delay 30 msecs
  delay(30);
}

