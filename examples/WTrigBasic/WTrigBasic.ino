// ****************************************************************************
//       Sketch: WTriggerUno
// Date Created: 4/22/2015
//
//     Comments: Demonstrates basic serial control of the WAV Trigger from an
//               Arduino.
//
//  Programmers: Jamie Robertson, info@robertsonics.com
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
//    (If you are using an Arduino with extra hardware serial ports, such as
//    an Arduino Mega or Teensy, you don't need AltSoftSerial, and you should
//    edit the wavTrigger.h library file to select the desired serial port
//    according to the documentation contained in that file. And use the
//    appropriate TX/RX pin connections to your Arduino)
//
// 4) Download and install the demo wav files onto the WAV Trigger's microSD
//    card. You can find them here:
//
//    http://robertsonics.com/2015/04/25/arduino-serial-control-tutorial/
//
//    You can certainly use your own tracks instead, although the demo may
//    not make as much sense. If you do, make sure your tracks are at least 10
//    to 20 seconds long and have no silence at the start of the file.

#include <Metro.h>
#include <AltSoftSerial.h>    // Arduino build environment requires this
#include <wavTrigger.h>

#define LED 13                // our LED

wavTrigger wTrig;             // Our WAV Trigger object

Metro gLedMetro(500);         // LED blink interval timer
Metro gSeqMetro(6000);        // Sequencer state machine interval timer

byte gLedState = 0;           // LED State
int  gSeqState = 0;           // Main program sequencer state
int  gRateOffset = 0;         // WAV Trigger sample-rate offset


// ****************************************************************************
void setup() {
  
  // Serial monitor
  Serial.begin(9600);
 
  // Initialize the LED pin
  pinMode(LED,OUTPUT);
  digitalWrite(LED,gLedState);

  // If the Arduino is powering the WAV Trigger, we should wait for the WAV
  //  Trigger to finish reset before trying to send commands.
  delay(1000);

  // WAV Trigger startup at 57600
  wTrig.start();
  delay(10);
  
  // Send a stop-all command and reset the sample-rate offset, in case we have
  //  reset while the WAV Trigger was already playing.
  wTrig.stopAllTracks();
  wTrig.samplerateOffset(0);
  
}


// ****************************************************************************
// This program uses a Metro timer to create a sequencer that steps through
//  states at 6 second intervals - you can change this rate above. Each state
//  Each state will demonstrate a WAV Trigger serial control feature.

void loop() {
  
int i;

  // If time to do so, perform the next WAV Trigger task and then increment
  //  the state machine variable 
  if (gSeqMetro.check() == 1) {
      
      switch (gSeqState) {
  
          // State 0: Demonstrates how to fade in a music track 
          case 0:
              wTrig.samplerateOffset(0);            // Reset our sample rate offset
              wTrig.masterGain(0);                  // Reset the master gain to 0dB
              
              wTrig.trackGain(2, -40);              // Preset Track 2 gain to -40dB
              wTrig.trackPlayPoly(2);               // Start Track 2
              wTrig.trackFade(2, 0, 2000, 0);       // Fade Track 2 up to 0dB over 2 secs
          break;

          // State 1: Demonstrates how to cross-fade music tracks
          case 1:
              wTrig.trackGain(1, -40);              // Preset Track 1 gain to -40dB
              wTrig.trackPlayPoly(1);               // Start Track 1
              wTrig.trackFade(1, 0, 3000, false);   // Fade Track 1 up to 0db over 3 secs
              wTrig.update();
              delay(2000);                          // Wait 2 secs
              wTrig.trackFade(2, -40, 3000, true);  // Fade Track 2 down to -40dB over 3 secs and stop
          break;
                                 
          // State 2: Fade down music and start looping dialog
          case 2:
              wTrig.trackFade(1, -6, 500, 0);
              wTrig.trackPlayPoly(4);               // Start Track 4 poly
              wTrig.trackLoop(4, 1);                // Enable Track 4 looping
          break;

          // State 3: Honk the horn 2 times
          case 3:
              wTrig.trackPlayPoly(5);               // Start Track 5 poly
              delay(500);
              wTrig.trackStop(5);                   // Stop Track 5
              delay(250);
              wTrig.trackPlayPoly(5);               // Start Track 5 poly
              delay(500);
              wTrig.trackStop(5);                   // Stop Track 5
          break;
          
          // State 4: Fade out and stop dialog
          case 4:
              wTrig.trackLoop(4, 0);                // Disable Track 4 looping
              wTrig.trackFade(4, -50, 5000, 1);     // Fade Track 4 to -50dB and stop
          break;
          
          // State 5: This demonstrates playing musical instrument samples, with decay on
          //  release
          case 5:
              wTrig.masterGain(-8);                 // Lower main volume
              wTrig.trackPlayPoly(6);               // Play first note
              delay(1000);
              wTrig.trackPlayPoly(7);               // Play second note
              delay(1000);
              wTrig.trackPlayPoly(8);               // Play third note
              delay(1000);
              wTrig.trackFade(6, -50, 5000, 1);     // Fade Track 6 to -50dB and stop
              wTrig.trackFade(7, -50, 5000, 1);     // Fade Track 7 to -50dB and stop
              wTrig.trackFade(8, -50, 5000, 1);     // Fade Track 8 to -50dB and stop
          break;
   
          // State 6: Demonstrates preloading tracks and starting them in sample-sync, and
          //  real-time samplerate control (pitch bending);
          case 6:
              wTrig.trackLoad(6);                   // Load and pause Track 6
              wTrig.trackLoad(7);                   // Load and pause Track 7
              wTrig.trackLoad(8);                   // Load and pause Track 8
              wTrig.resumeAllInSync();              // Start all in sample sync
              for (i = 0; i < 100; i++) {
                gRateOffset -= 200;
                wTrig.samplerateOffset(gRateOffset);
                delay(10);
              }
              for (i = 0; i < 100; i++) {
                gRateOffset += 200;
                wTrig.samplerateOffset(gRateOffset);
                delay(10);
              }
              delay(500);
              wTrig.stopAllTracks();                // Stop all
         break;
         
           
      } // switch
 
      // Increment our state
      gSeqState++;
      if (gSeqState > 6)
          gSeqState = 0;
          
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

