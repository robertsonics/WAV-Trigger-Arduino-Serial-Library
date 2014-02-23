WAV-Trigger-Arduino-Serial-Library
==================================

WAV Trigger Serial Control Arduino Library

First preliminary version is written as part of a demo for controlling the WAV
Trigger from an Arduino YUN via a browser interface over wireless. This libraray
uses the AltSoftwareSerial library for compatibility with the YUN, and currently
only sends commands TO the WAV Trigger.

I make no attempt to throttle the amount of messages that are sent. If you send
continuous volume commands at full speed, you risk overflowing the WAV Trigger's
serial input buffer and/or causing clicks in the WAV Triggers audio output due to
excessive serial interrupt processing stealing cycles from audio playback. If you
are connecting a continuous controller that can change rapidly for volume control,
you should use a timer to send changes only every 20 - 50 msecs.
