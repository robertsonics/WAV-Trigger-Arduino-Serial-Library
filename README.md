WAV-Trigger-Arduino-Serial-Library
==================================

WAV Trigger Serial Control Arduino Library

Because the UNO's single serial port is used for programming, this library makes use
of the AltSoftwareSerial library from PJRC by default. If you're using an UNO, you'll
therefore want to download and install that library as well. Be sure to include both
library headers at the top of your sketch. (See the example sketches)

```
#include <AltSoftSerial.h>
#include <wavTrigger.h>
```

However, if you're using an Arduino with at least one additional hardware serial
port, you will not need AltSoftSerial. Instead, just make one small change to the
library's **wavTrigger.h file**. Near the top of the file, look for:

```
// ==================================================================
// The following defines are used to control which serial class is
//  used. Uncomment only the one you wish to use. If all of them are
//  commented out, the library will use Hardware Serial
#define __WT_USE_ALTSOFTSERIAL__
//#define __WT_USE_SERIAL1__
//#define __WT_USE_SERIAL2__
//#define __WT_USE_SERIAL3__
// ==================================================================
```

Comment out the `__WT_USER_ALTSOFTSERIAL__` line and uncomment the line correspond-
ing to the hardware port you want to use. If all the lines are commented out, the
library will use Serial (the only hardware serial port on an Uno.)

I make no attempt to throttle the amount of messages that are sent. If you send
continuous volume or sample-rate commands at full speed, you risk overflowing the
WAV Trigger's serial input buffer and/or causing clicks in the WAV Triggers audio
output due to excessive serial interrupt processing stealing cycles from audio
playback. If you are connecting a continuous controller that can change rapidly
for volume or sample-rate control, you should use a timer to send changes only
every 10 or more msecs. You can, of course, experiment with this. If you're only
ever playing 1 or 2 tracks at a time, you'll likely be able to get away with send-
ing volume changes more frequently than if you are playing 8 tracks at a time.

The library now includes two example sketches that demonstrate many of the library
commands with a set of demo tracks which you can download from

http://robertsonics.com/2015/04/25/arduino-serial-control-tutorial/

The basic sketch uses one-way communication to the WAV Trigger. The advanced sketch
demonstrates bi-directional communication which allows you to schedule events based
on when tracks finish.

Some advanced functions depend on the later version of the WAV Trigger firmware.
See the comments below.

Usage:
======

In all cases below, the range for t (track number) is 1 through 4096;

wavTrigger wTtrig;

**wTrig.start()** - you must call this method first to initialize the serial
  communications.

**wTrig.getVersion(char \*pDst, int len)** - this function will return **len** bytes of
  the WAV Trigger version string to the location specified by **pDst**. The function
  returns TRUE if successful, and FALSE if the string is not available. This
  function requires bi-directional communication with the WAV Trigger.

**wTrig.getNumTracks()** - Returns number of tracks on the WAV Trigger's microSD
  card. This function requires bi-directional communication with the WAV Trigger.

**wTrig.masterGain(int gain)** - this function immediately sets the gain of the
  final output stage to the specified value. The range for gain is -70 to +4. If
  audio is playing, you will hear the result immediately. If audio is not playing,
  the new gain will be used the next time a track is started.

**wTrig.setAmpPwr(bool enable)** - this function enabled/disables the on-board
  amplifier. If you’re planning to use the on-board amplifier, read this first.

  <http://robertsonics.com/2015/05/16/update-on-wav-trigger-hardware-versions/>

  Please consider using a supply voltage of no more than 9V, preferably less.
  Playing loud audio with the on-board amp for extended periods can draw a lot of
  current and the lower the input voltage, the less warm the 5V regulator will get.
  This is especially important if you’re encasing the WAV Trigger in a small project box.
  I personally use 6V wall mount adapters. Alternatively, you can power
  the WAV Trigger with regulated 5V (5Vin) and bypass the on-board regulator completely.

**wTrig.samplerateOffset(int offset)** - this function immediately sets sample-
  rate offset, or playback speed / pitch, of the main output mix. The range for
  for the offset is -32767 to +32676, giving a speed range of 1/2x to 2x, or a
  pitch range of down one octave to up one octave. If audio is playing, you will
  hear the result immediately. If audio is not playing, the new sample-rate offset
  will be used the next time a track is started.

**wTrig.trackPlaySolo(int t)** - this function stops any and all tracks that are
  currently playing and starts track number **t** from the beginning.

**wTrig.trackPlayPoly(int t)** - this function starts track number **t** from the
  beginning, blending it with any other tracks that are currently playing,
  including potentially another copy of the same track.
  
**wTrig.trackLoad(int t)** - this function loads track number **t** and pauses it
  at the beginning of the track. Loading muiltiple tracks and then un-pausing them
  all with resumeAllInSync() function below allows for starting multiple tracks in
  sample sync.
  
**wTrig.trackStop(int t)** - this function stops track number **t** if it's currently
  playing. If track t is not playing, this function does nothing. No other
  tracks are affected.
  
**wTrig.trackPause(int t)** - this function pauses track number **t** if it's currently
  playing. If track t is not playing, this function does nothing. Keep in mind
  that a paused track is still using one of the 8 voice slots. A voice allocated
  to playing a track becomes free only when that sound is stopped or the track
  reaches the end of the file (and is not looping).
  
**wTrig.trackResume(int t)** - this function resumes track number **t** if it's currently
  paused. If track number **t** is not paused, this function does nothing.
  
**wTrig.trackLoop(int t, bool enable)** - this function enables (true) or disables
  (false) the loop flag for track **t**. This command does not actually start a track,
  only determines how it behaves once it is playing and reaches the end. If the
  loop flag is set, that track will loop continuously until it's stopped, in which
  case it will stop immediately but the loop flag will remain set, or until the loop
  flag is cleared, in which case it will stop when it reaches the end of the track.
  This command may be used either before a track is started or while it's playing.
  
**wTrig.trackGain(int t, int gain)** - this function immediately sets the gain of
  track **t** to the specified value. The range for gain is -70 to +10. A value of
  0 (no gain) plays the track at the nominal value in the wav file. This is the
  default gain for every track until changed. A value of -70 is completely
  muted. If the track is playing, you will hear the result immediately. If the
  track is not playing, the gain will be used the next time the track is started.
  Every track can have its own gain.
  
  Because the effect is immediate, large changes can produce ubrupt results. If
  you want to fade in or fade out a track, send small changes spaced out at regular
  intervals. Increment or decrementing by 1 every 20 to 50 msecs produces nice
  smooth fades. Better yet, use the new trackFade() and trackCrossFade() commands
  below.
  
**wTrig.stopAllTracks()** - this commands stops any and all tracks that are currently
  playing.
  
**wTrig.resumeAllInSync()** - this command resumes all paused tracks within the same
  audio buffer. Any tracks that were loaded using the trackLoad() function will start
  and remain sample locked (in sample sync) with one another.

**wTrig.trackFade(int t, int gain, int time, bool stopFlag)** - this command initiates
  a hardware volume fade on track number **t** if it is currently playing. The track
  volume will transition smoothly from the current value to the target gain in the
  specified number of milliseconds. If the stopFlag is non-zero, the track will be
  stopped at the completion of the fade (for fade-outs.)
  
  
The following library functions are supported by WAV Trigger firmware v1.30 and above,
and require bi-directional communication with the WAV Trigger. The advanced example
sketch demonstrates the use of these functions.

**wTrig.setReporting(bool enable)** - this function enables (TRUE) or disables
  (FALSE) track reporting. When enabled, the WAV Trigger will send a message whenever
  a track starts or ends, specifying the track number. Provided you call update()
  periodically, the library will use these messages to maintain status of all tracks,
  allowing you to query if particular tracks are playing or not.
  
**wTrig.update()** - this function should be called periodically when reporting is
  enabled. Doing so will process any incoming serial messages and keep the track status
  up to date.

**wTrig.isTrackPlaying(int trk)** - If reporting has been enabled, this function can be
  used to determine if a particular track is currently playing.

**wTrig.flush()** - This function clears the WAV Trigger communication buffer and resets
  the local track status info.
  
**wTrig.trackPlaySolo(int t, bool lock)** - this function stops any and all tracks that
  are currently playing and starts track number **t** from the beginning. If **lock** is
  TRUE, the track will not be subject to the WAV Trigger's voice stealing algorithm,
  and will not be stopped if the max number of voices is reached.

**wTrig.trackPlayPoly(int t, bool lock)** - this function starts track number **t** from
  the beginning, blending it with any other tracks that are currently playing,
  including potentially another copy of the same track. If **lock** is TRUE, the track will
  not be subject to the WAV Trigger's voice stealing algorithm, and will not be stopped
  if the max number of voices is reached.
  
**wTrig.trackLoad(int t, bool lock)** - this function loads track number **t** and pauses it
  at the beginning of the track. Loading muiltiple tracks and then un-pausing them
  all with resumeAllInSync() function allows for starting multiple tracks in sample
  sync.  If **lock** is TRUE, the track will not be subject to the WAV Trigger's voice
  stealing algorithm, and will not be stopped if the max number of voices is reached.

The following library functions are supported by WAV Trigger firmware v1.32 and above.

**wTrig.setTriggerBank(int bank)** - this function immediately sets the Trigger Bank to
  the specified value. Trigger Banks are numbered 1 to 32.

  
