WAV-Trigger-Arduino-Serial-Library
==================================

WAV Trigger Serial Control Arduino Library

This preliminary version is written as part of a demo sketch for controlling
the WAV Trigger from an Arduino YUN via a browser interface over wifi. It uses
the AltSoftwareSerial library from PJRC for compatibility with the YUN. For
the time being, if you want to use a hardware serial port, or the native
Arduino SoftwareSerial class, it's easy enough to change. I'll eventually make
this, as well as the baudrate, configurable through the class itself.

This version currently only sends commands TO the WAV Trigger. I've not yet
implemented any functions to receive info FROM the WAV Trigger.

I make no attempt to throttle the amount of messages that are sent. If you send
continuous volume commands at full speed, you risk overflowing the WAV Trigger's
serial input buffer and/or causing clicks in the WAV Triggers audio output due to
excessive serial interrupt processing stealing cycles from audio playback. If you
are connecting a continuous controller that can change rapidly for volume control,
you should use a timer to send changes only every 10 or more msecs. You can, of
course, experiment with this. If you're only ever playing 1 or 2 tracks at a time,
you'll likely be able to get away with sending volume changes more frequently
than if you are playing 8 tracks at a time.

Usage:
======

In all cases below, the range for t (track number) is 1 through 999;

wavTrigger wTtrig;

wTrig.start() - you must call this method first to initialize the serial
  communications.

wTrig.trackPlaySolo(int t) - this function stops any and all tracks that are
  currently playing and starts track number t from the beginning.

wTrig.trackPlayPoly(int t) - this function starts track number t from the
  beginning, blending it with any other tracks that are currently playing,
  including potentially another copy of the same track.
  
wTrig.trackStop(int t) - this function stops track number t if it's currently
  playing. If track t is not playing, this function does nothing. No other
  tracks are affected.
  
wTrig.trackPause(int t) - this function pauses track number t if it's currently
  playing. If track t is not playing, this function does nothing. Keep in mind
  that a paused track is still using one of the 8 voice slots. A voice allocated
  to playing a track becomes free only when that sound is stopped or the track
  reaches the end of the file (and is not looping).
  
wTrig.trackResume(int t) - this function resumes track number t if it's currently
  paused. If track number t is not paused, this function does nothing.
  
wTrig.trackLoop(int t, bool enable) - this function enables (true) or disables
  (false) the loop flag for track t. This command does not actually start a track,
  only determines how it behaves once it is playing and reaches the end. If the
  loop flag is set, that track will loop continuously until it's stopped, in which
  case it will stop immediately but the loop flag will remain set, or until the loop
  flag is cleared, in which case it will stop when it reaches the end of the track.
  This command may be used either before a track is started or while it's playing.
  
wTrig.trackGain(int t, int gain) - this function immediately sets the gain of
  track t to the specified value. The range for gain is -70 to +10. A value of
  0 (no gain) plays the track at the nominal value in the wav file. This is the
  default gain for every track until changed. A value of -70 is completely
  muted. If the track is playing, you will hear the result immediately. If the
  track is not playing, the gain will be used the next time the track is started.
  Every track can have its own gain.
  
  Because the effect is immediate, large changes can produce ubrupt results. If
  you want to fade in or fade out a track, send small changes spaced out at regular
  intervals. Increment or decrementing by 1 every 20 to 50 msecs produces nice
  smooth fades.
  
wTrig.stopAllTracks() - this commands stops any and all tracks that are currently
  playing.
  
  

  
