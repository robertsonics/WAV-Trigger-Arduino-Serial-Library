// ****************************************************************************
//       Sketch: WTrigAdvanced - Wav Trigger Advanced Example
// Date Created: 11/7/2016
//
//     Comments: Demonstrates advanced, two-way, serial control of the WAV
//               Trigger from an Arduino. Requires WAV Trigger firmware v1.30
//               or above.
//
//  Programmers: Jamie Robertson, info@robertsonics.com
//
// ****************************************************************************
//
// To use this sketch with an UNO, you'll need to:
//
// 1) Download and install the AltSoftSerial library.
// 2) Download and install the Metro library.
// 3) Connect 3 wires from the UNO to the WAV Trigger's serial connector:
//
//    Uno           WAV Trigger
//    ===           ===========
//    GND  <------> GND
//    Pin9 <------> RX
//    Pin8 <------> TX
//
//    If you want to power the WAV Trigger from the Uno, then close the 5V
//    solder jumper on the WAV Trigger and connect a 4th wire:
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
//    not make as much sense. If you do, make sure your tracks are at least
//    10 to 20 seconds long and have no silence at the start of the file.

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
int  gNumTracks;              // Number of tracks on SD card

char gWTrigVersion[VERSION_STRING_LEN];    // WAV Trigger version string


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
  
  // Enable track reporting from the WAV Trigger
  wTrig.setReporting(true);
  
  // Allow time for the WAV Trigger to respond with the version string and
  //  number of tracks.
  delay(100); 
  
  // If bi-directional communication is wired up, then we should by now be able
  //  to fetch the version string and number of tracks on the SD card.
  if (wTrig.getVersion(gWTrigVersion, VERSION_STRING_LEN)) {
      Serial.print(gWTrigVersion);
      Serial.print("\n");
      gNumTracks = wTrig.getNumTracks();
      Serial.print("Number of tracks = ");
      Serial.print(gNumTracks);
      Serial.print("\n");
  }
  else
      Serial.print("WAV Trigger response not available");
}


// ****************************************************************************
// This program uses a Metro timer to create a sequencer that steps through
//  states at 6 second intervals - you can change this rate above. Each state
//  Each state will demonstrate a WAV Trigger serial control feature.
//
//  In this advanced example, some states wait for specific audio tracks to
//  stop playing before advancing to the next state.

void loop() {
  
int i;

  // Call update on the WAV Trigger to keep the track playing status current.
  wTrig.update();
  
  // Check if the sequencer timer has elapsed and perform the appropriate
  //  state action if so. States 3 and 5 wait for tracks to stop playing and
  //  are therefore not in the metro event. They are instead polled after the
  //  metro check.
  if (gSeqMetro.check() == 1) {
      
      switch (gSeqState) {
  
          // State 0: Demonstrates how to fade in a music track 
          case 0:
              wTrig.samplerateOffset(0);            // Reset our sample rate offset
              wTrig.masterGain(0);                  // Reset the master gain to 0dB
              
              wTrig.trackGain(2, -40);              // Preset Track 2 gain to -40dB
              wTrig.trackPlayPoly(2);               // Start Track 2
              wTrig.trackFade(2, 0, 2000, 0);       // Fade Track 2 up to 0dB over 2 secs
              gSeqState = 1;                        // Advance to state 1
          break;

          // State 1: Demonstrates how to cross-fade music tracks
          case 1:
              wTrig.trackGain(1, -40);              // Preset Track 1 gain to -40dB
              wTrig.trackPlayPoly(1);               // Start Track 1
              wTrig.trackFade(1, 0, 3000, false);   // Fade Track 1 up to 0db over 3 secs
              wTrig.update();
              delay(2000);                          // Wait 2 secs
              wTrig.trackFade(2, -40, 3000, true);  // Fade Track 2 down to -40dB over 3 secs and stop
              gSeqState = 2;                        // Advance to state 2
          break;
                                 
          // State 3: Honk the horn 2 times
          case 3:
              wTrig.trackPlayPoly(5);               // Start Track 5 poly
              wTrig.update();
              delay(500);
              wTrig.trackStop(5);                   // Stop Track 5
              wTrig.update();
              delay(250);
              wTrig.trackPlayPoly(5);               // Start Track 5 poly
              wTrig.update();
              delay(500);
              wTrig.trackStop(5);                   // Stop Track 5
              gSeqState = 4;                        // Advance to state 4
          break;
          
          // State 4: Fade out and stop dialog
          case 4:
              wTrig.trackLoop(4, 0);                // Disable Track 4 looping
              wTrig.trackFade(4, -50, 5000, 1);     // Fade Track 4 to -50dB and stop
              gSeqState = 5;                        // Advance to state 5
          break;
            
          // State 6: Demonstrates preloading tracks and starting them in sample-
          //  sync, and real-time samplerate control (pitch bending);
          case 6:
              wTrig.trackLoad(6);                   // Load and pause Track 6
              wTrig.trackLoad(7);                   // Load and pause Track 7
              wTrig.trackLoad(8);                   // Load and pause Track 8
              wTrig.resumeAllInSync();              // Start all in sample sync

              // Decrement the sample rate offset from 0 to -32767 (1 octave down)
              //  in 10 ms steps
              gRateOffset = 0;
              for (i = 0; i < 127; i++) {
                gRateOffset -= 256;
                wTrig.samplerateOffset(gRateOffset);
                delay(10);
              }
              gRateOffset = -32767;
              wTrig.samplerateOffset(gRateOffset);
              
              // Hold for 1 second
              delay(1000);
              
              // Now increment to +32767 (1 octave up) in 10ms steps
              for (i = 0; i < 255; i++) {
                gRateOffset += 256;
                wTrig.samplerateOffset(gRateOffset);
                delay(10);
              }
              gRateOffset = 32767;
              wTrig.samplerateOffset(gRateOffset);
              
              // Hold for 1 second, the stop all tracks
              delay(1000);
              wTrig.stopAllTracks();                // Stop all
              gSeqState = 0;                        // Advance to state 0
          break;
            
      } // switch
           
  } // if (gSeqState.check() == 1)
 
  // State 2: Wait for Track 2 to stop, then fade down the music and start the
  //  dialog track looping.
  if (gSeqState == 2) {
      gSeqMetro.reset();                            // Reset the sequencer metro
      if (!wTrig.isTrackPlaying(2)) {
          Serial.print("Track 2 done\n");
          wTrig.trackFade(1, -6, 500, 0);           // Lower the music volume           
          wTrig.trackPlayPoly(4);                   // Start Track 4 poly
          wTrig.trackLoop(4, 1);                    // Enable Track 4 looping
          gSeqState = 3;                            // Advance to state 3;
      }       
  }

  // State 5: Wait for Track 4 to stop, then play three tracks sequentially and
  //  stop all with a 5 sec fade to -50dB. This is how you can implement MIDI
  //  Note-On/Off control for: MIDI -> Arduino -> WAV Trigger.
  if (gSeqState == 5) {
      gSeqMetro.reset();
      if (!wTrig.isTrackPlaying(4)) {
          Serial.print("Track 4 done\n");
          wTrig.masterGain(-8);                     // Lower main volume
          wTrig.trackPlayPoly(6);                   // Play first note
          wTrig.update();
          delay(1000);
          wTrig.trackPlayPoly(7);                   // Play second note
          wTrig.update();
          delay(1000);
          wTrig.trackPlayPoly(8);                   // Play third note
          wTrig.update();
          delay(1000);
          wTrig.trackFade(6, -50, 5000, 1);         // Fade Track 6 to -50dB and stop
          wTrig.trackFade(7, -50, 5000, 1);         // Fade Track 7 to -50dB and stop
          wTrig.trackFade(8, -50, 5000, 1);         // Fade Track 8 to -50dB and stop
         gSeqState = 6;
      }       
  }
  
  // If time to do so, toggle the LED
  if (gLedMetro.check() == 1) {
      if (gLedState == 0) gLedState = 1;
      else gLedState = 0;
      digitalWrite(LED, gLedState);
  } // if (gLedMetro.check() == 1)

  // Delay 30 msecs
  delay(30);
}

