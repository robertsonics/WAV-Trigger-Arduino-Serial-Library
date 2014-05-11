// **************************************************************
//     Filename: wavTrigger.h
// Date Created: 2/23/2014
//
//     Comments: Robertsonics WAV Trigger serial control library
//
// Programmers: Jamie Robertson, jamie@robertsonics.com
//
// **************************************************************
//
// Revision History
//
// Date      Description
// --------  -----------
//
// 02/22/14  First version created.
//           LIMITATIONS: Hard-coded for AltSoftwareSerial Library.
//           Also only supports commands TO the WAV Trigger. Will
//           fix these things.
//
// 05/10/14  Tested with UNO. Added new functions for fades, cross-
//           fades and starting multiple tracks in sample sync.


#ifndef WAVTRIGGER_H
#define WAVTRIGGER_H

#define CMD_TRACK_CONTROL	3
#define CMD_STOP_ALL		4
#define CMD_TRACK_VOLUME	8
#define CMD_TRACK_FADE		10
#define CMD_RESUME_ALL_SYNC	11

#define TRK_PLAY_SOLO	0
#define TRK_PLAY_POLY	1
#define TRK_PAUSE		2
#define TRK_RESUME		3
#define TRK_STOP		4
#define TRK_LOOP_ON		5
#define TRK_LOOP_OFF	6
#define TRK_LOAD		7

#include "../AltSoftSerial/AltSoftSerial.h"

class wavTrigger
{
public:
	wavTrigger() {;}
	~wavTrigger() {;}
	void start(void);
	void stopAllTracks(void);
	void resumeAllInSync(void);
	void trackPlaySolo(int trk);
	void trackPlayPoly(int trk);
	void trackLoad(int trk);
	void trackStop(int trk);
	void trackPause(int trk);
	void trackResume(int trk);
	void trackLoop(int trk, bool enable);
	void trackGain(int trk, int gain);
	void trackFade(int trk, int gain, int time, bool stopFlag);
	void trackCrossFade(int trkFrom, int trkTo, int gain, int time);
private:
	void trackControl(int trk, int code);

	AltSoftSerial WTSerial;
};

#endif
