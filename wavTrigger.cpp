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


//**************************************************************************/
/*!
    @brief  Call this method to initialize the serial communications.
	@param baud Serial baud rate, 57600 works well.
	@param config Serial port address
	@param rxPin RX Serial Pin
	@param txPin TX Serial Pin
*/
/**************************************************************************/
void wavTrigger::start(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin) {

uint8_t txbuf[5];

	versionRcvd = false;
	sysinfoRcvd = false;
	WTSerial.begin(baud, config, rxPin, txPin);
	flush();

	// Request version string
	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x05;
	txbuf[3] = CMD_GET_VERSION;
	txbuf[4] = EOM;
	WTSerial.write(txbuf, 5);

	// Request system info
	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x05;
	txbuf[3] = CMD_GET_SYS_INFO;
	txbuf[4] = EOM;
	WTSerial.write(txbuf, 5);
}

/**************************************************************************/
/*!
    @brief  Clears the WAV Trigger communication buffer and resets the 
	local track status info.
*/
/**************************************************************************/
void wavTrigger::flush(void) {

int i;
uint8_t dat;

	rxCount = 0;
	rxLen = 0;
	rxMsgReady = false;
	for (i = 0; i < MAX_NUM_VOICES; i++) {
	  voiceTable[i] = 0xffff;
	}
	while(WTSerial.available())
		dat = WTSerial.read();
}


/**************************************************************************/
/*!
    @brief  Should be called periodically when reporting is enabled. Doing 
	so will process any incoming serial messages and keep the track status 
	up to date.
*/
/**************************************************************************/
void wavTrigger::update(void) {

int i;
uint8_t dat;
uint8_t voice;
uint16_t track;

	rxMsgReady = false;
	while (WTSerial.available() > 0) {
		dat = WTSerial.read();
		if ((rxCount == 0) && (dat == SOM1)) {
			rxCount++;
		}
		else if (rxCount == 1) {
			if (dat == SOM2)
				rxCount++;
			else {
				rxCount = 0;
				//Serial.print("Bad msg 1\n");
			}
		}
		else if (rxCount == 2) {
			if (dat <= MAX_MESSAGE_LEN) {
				rxCount++;
				rxLen = dat - 1;
			}
			else {
				rxCount = 0;
				//Serial.print("Bad msg 2\n");
			}
		}
		else if ((rxCount > 2) && (rxCount < rxLen)) {
			rxMessage[rxCount - 3] = dat;
			rxCount++;
		}
		else if (rxCount == rxLen) {
			if (dat == EOM)
				rxMsgReady = true;
			else {
				rxCount = 0;
				//Serial.print("Bad msg 3\n");
			}
		}
		else {
			rxCount = 0;
			//Serial.print("Bad msg 4\n");
		}

		if (rxMsgReady) {
			switch (rxMessage[0]) {

				case RSP_TRACK_REPORT:
					track = rxMessage[2];
					track = (track << 8) + rxMessage[1] + 1;
					voice = rxMessage[3];
					if (voice < MAX_NUM_VOICES) {
						if (rxMessage[4] == 0) {
							if (track == voiceTable[voice])
								voiceTable[voice] = 0xffff;
						}
						else
							voiceTable[voice] = track;
					}
					// ==========================
					//Serial.print("Track ");
					//Serial.print(track);
					//if (rxMessage[4] == 0)
					//	Serial.print(" off\n");
					//else
					//	Serial.print(" on\n");
					// ==========================
				break;

				case RSP_VERSION_STRING:
					for (i = 0; i < (VERSION_STRING_LEN - 1); i++)
						version[i] = rxMessage[i + 1];
					version[VERSION_STRING_LEN - 1] = 0;
					versionRcvd = true;
					// ==========================
					//Serial.write(version);
					//Serial.write("\n");
					// ==========================
				break;

				case RSP_SYSTEM_INFO:
					numVoices = rxMessage[1];
					numTracks = rxMessage[3];
					numTracks = (numTracks << 8) + rxMessage[2];
					sysinfoRcvd = true;
					// ==========================
					///\Serial.print("Sys info received\n");
					// ==========================
				break;

			}
			rxCount = 0;
			rxLen = 0;
			rxMsgReady = false;

		} // if (rxMsgReady)

	} // while (WTSerial.available() > 0)
}

/**************************************************************************/
/*!
    @brief  If reporting has been enabled, this function can be used to 
	determine if a particular track is currently playing.
	@param trk Track number.
	@returns True or False.
*/
/**************************************************************************/
bool wavTrigger::isTrackPlaying(int trk) {

int i;
bool fResult = false;

	update();
	for (i = 0; i < MAX_NUM_VOICES; i++) {
		if (voiceTable[i] == trk)
			fResult = true;
	}
	return fResult;
}

/**************************************************************************/
/*!
    @brief  Sets the gain of the final output stage to the specified value. 
	The range for gain is -70 to +4. If audio is playing, you will hear the
	result immediately. If audio is not playing, the new gain will be used 
	the next time a track is started.
	@param gain Set volume, -70 to +4.
*/
/**************************************************************************/
void wavTrigger::masterGain(int gain) {

uint8_t txbuf[7];
unsigned short vol;

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x07;
	txbuf[3] = CMD_MASTER_VOLUME;
	vol = (unsigned short)gain;
	txbuf[4] = (uint8_t)vol;
	txbuf[5] = (uint8_t)(vol >> 8);
	txbuf[6] = EOM;
	WTSerial.write(txbuf, 7);
}

/**************************************************************************/
/*!
    @brief  Enable / disable the on-board amplifier. If you’re planning to
	use the on-board amplifier, read this first.
	http://robertsonics.com/2015/05/16/update-on-wav-trigger-hardware-versions/
	@param enable True or False.
*/
/**************************************************************************/
void wavTrigger::setAmpPwr(bool enable) {

uint8_t txbuf[6];

    txbuf[0] = SOM1;
    txbuf[1] = SOM2;
    txbuf[2] = 0x06;
    txbuf[3] = CMD_AMP_POWER;
    txbuf[4] = enable;
    txbuf[5] = EOM;
    WTSerial.write(txbuf, 6);
}

/**************************************************************************/
/*!
    @brief  Enables / disables track reporting. When enabled, the WAV 
	Trigger will send a message whenever a track starts or ends, specifying
	the track number. Provided you call update() periodically, the library 
	will use these messages to maintain status of all tracks, allowing you 
	to query if particular tracks are playing or not.
	@param enable True or False.
*/
/**************************************************************************/
void wavTrigger::setReporting(bool enable) {

uint8_t txbuf[6];

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x06;
	txbuf[3] = CMD_SET_REPORTING;
	txbuf[4] = enable;
	txbuf[5] = EOM;
	WTSerial.write(txbuf, 6);
}

/**************************************************************************/
/*!
    @brief  Gets WAV Trigger version string. Requires bi-directional 
	communication with the WAV Trigger.
	@param pDst Pointer to a version string.
	@param len Lenght of the version string. Use VERSION_STRING_LEN
	@returns TRUE successful, and FALSE not available.
*/
/**************************************************************************/
bool wavTrigger::getVersion(char *pDst, int len) {

int i;

	update();
	if (!versionRcvd) {
		return false;
	}
	for (i = 0; i < (VERSION_STRING_LEN - 1); i++) {
		if (i >= (len - 1))
			break;
		pDst[i] = version[i];
	}
	pDst[++i] = 0;
	return true;
}

/**************************************************************************/
/*!
    @brief  Gets number of tracks on the WAV Trigger's microSD card. 
	Requires bi-directional communication with the WAV Trigger.
	@returns Number of tracks available.
*/
/**************************************************************************/
int wavTrigger::getNumTracks(void) {

	update();
	return numTracks;
}

/**************************************************************************/
/*!
    @brief  Stops any and all tracks that are currently playing and starts
	track number from the beginning.
    @param trk Track number.
*/
/**************************************************************************/
void wavTrigger::trackPlaySolo(int trk) {
  
	trackControl(trk, TRK_PLAY_SOLO);
}

/**************************************************************************/
/*!
    @brief  Stops any and all tracks that are currently playing and starts
	track number from the beginning.
    @param trk Track number.
	@param lock Lock, True or False
*/
/**************************************************************************/
void wavTrigger::trackPlaySolo(int trk, bool lock) {
  
	trackControl(trk, TRK_PLAY_SOLO, lock);
}

/**************************************************************************/
/*!
    @brief  Starts track number from the beginning, blending it with any 
	other tracks that are currently playing, including potentially another 
	copy of the same track.
    @param trk Track number.
*/
/**************************************************************************/
void wavTrigger::trackPlayPoly(int trk) {
  
	trackControl(trk, TRK_PLAY_POLY);
}

/**************************************************************************/
/*!
    @brief  Starts track number from the beginning, blending it with any 
	other tracks that are currently playing, including potentially another 
	copy of the same track.
    @param trk Track number.
	@param lock Lock, True or False
*/
/**************************************************************************/
void wavTrigger::trackPlayPoly(int trk, bool lock) {
  
	trackControl(trk, TRK_PLAY_POLY, lock);
}

/**************************************************************************/
/*!
    @brief  Loads track number and pauses it at the beginning of the track. 
	Loading muiltiple tracks and then un-pausing them with resumeAllInSync() 
	function allows for starting multiple tracks in sample sync.
    @param trk Track number.
*/
/**************************************************************************/
void wavTrigger::trackLoad(int trk) {
  
	trackControl(trk, TRK_LOAD);
}

/**************************************************************************/
/*!
    @brief  Loads track number and pauses it at the beginning of the track. 
	Loading muiltiple tracks and then un-pausing them with resumeAllInSync() 
	function allows for starting multiple tracks in sample sync.
    @param trk Track number.
	@param lock Lock, True or False
*/
/**************************************************************************/
void wavTrigger::trackLoad(int trk, bool lock) {
  
	trackControl(trk, TRK_LOAD, lock);
}

/**************************************************************************/
/*!
    @brief  Stops track number if it's currently playing. If track is not 
	playing, this function does nothing. No other tracks are affected.
    @param trk Track number.
*/
/**************************************************************************/
void wavTrigger::trackStop(int trk) {

	trackControl(trk, TRK_STOP);
}

/**************************************************************************/
/*!
    @brief  Pauses track number if it's currently playing. If track is not
	playing, this function does nothing. A paused track is still using one
	of the 8 voice slots. A voice allocated to playing a track becomes 
	free only when that track is stopped or the track reaches the end of 
	the file (and is not looping).
    @param trk Track number.
*/
/**************************************************************************/
void wavTrigger::trackPause(int trk) {

	trackControl(trk, TRK_PAUSE);
}

/**************************************************************************/
/*!
    @brief  Resumes track number t if it's currently paused. If track number 
	is not paused, this function does nothing.
    @param trk Track number.
*/
/**************************************************************************/
void wavTrigger::trackResume(int trk) {

	trackControl(trk, TRK_RESUME);
}

/**************************************************************************/
/*!
    @brief  Enables or disables the loop flag for track. This command does
	 not start a track, only determines how it behaves once it is playing
	and reaches the end. If the loop flag is set, that track will loop 
	continuously until it's stopped, in which case it will stop immediately
	but the loop flag will remain set, or until the loop flag is cleared, 
	in which case it will stop when it reaches the end of the track.
    @param trk Track number.
	@param enable Enable True, disable False.
*/
/**************************************************************************/
void wavTrigger::trackLoop(int trk, bool enable) {
 
	if (enable)
		trackControl(trk, TRK_LOOP_ON);
	else
		trackControl(trk, TRK_LOOP_OFF);
}

// **************************************************************
void wavTrigger::trackControl(int trk, int code) {
  
uint8_t txbuf[8];

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x08;
	txbuf[3] = CMD_TRACK_CONTROL;
	txbuf[4] = (uint8_t)code;
	txbuf[5] = (uint8_t)trk;
	txbuf[6] = (uint8_t)(trk >> 8);
	txbuf[7] = EOM;
	WTSerial.write(txbuf, 8);
}

// **************************************************************
void wavTrigger::trackControl(int trk, int code, bool lock) {
  
uint8_t txbuf[9];

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x09;
	txbuf[3] = CMD_TRACK_CONTROL_EX;
	txbuf[4] = (uint8_t)code;
	txbuf[5] = (uint8_t)trk;
	txbuf[6] = (uint8_t)(trk >> 8);
	txbuf[7] = lock;
	txbuf[8] = EOM;
	WTSerial.write(txbuf, 9);
}

/**************************************************************************/
/*!
    @brief  Stops any and all tracks that are currently playing.
*/
/**************************************************************************/
void wavTrigger::stopAllTracks(void) {

uint8_t txbuf[5];

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x05;
	txbuf[3] = CMD_STOP_ALL;
	txbuf[4] = EOM;
	WTSerial.write(txbuf, 5);
}

/**************************************************************************/
/*!
    @brief  Resumes all paused tracks within the same audio buffer. Any 
	tracks that were loaded using the trackLoad() function will start and 
	remain sample locked (in sample sync) with one another. 
*/
/**************************************************************************/
void wavTrigger::resumeAllInSync(void) {

uint8_t txbuf[5];

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x05;
	txbuf[3] = CMD_RESUME_ALL_SYNC;
	txbuf[4] = EOM;
	WTSerial.write(txbuf, 5);
}

/**************************************************************************/
/*!
    @brief  Sets the gain of track to the specified value. A value of 
	0 (no gain) plays the track at the nominal value in the wav file. 
	This is the default gain for every track until changed. A value of 
	-70 is completely muted. If the track is playing, you will hear the 
	result immediately. If the track is not playing, the gain will be 
	used the next time the track is started. Every track can have its 
	own gain.
	Because the effect is immediate, large changes can produce ubrupt 
	results. If you want to fade in or fade out a track, send small 
	changes spaced out at regular intervals. Increment or decrementing 
	by 1 every 20 to 50 msecs produces nice smooth fades. Better yet, 
	usetrackFade() and trackCrossFade() commands.
    @param trk Track number.
	@param gain Volume target, -70 to +10.
*/
/**************************************************************************/
void wavTrigger::trackGain(int trk, int gain) {

uint8_t txbuf[9];
unsigned short vol;

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
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

/**************************************************************************/
/*!
    @brief  Hardware volume fade on track number if it is currently playing.
	The track volume will transition smoothly from the current value to the 
	target gain in the specified number of milliseconds. 
    @param trk Track number.
	@param gain Volume target.
	@param time Fade time in milliseconds.
	@param stopFlag If the stopFlag is non-zero, the track will be stopped at 
	the completion of the fade (for fade-outs.)
*/
/**************************************************************************/
void wavTrigger::trackFade(int trk, int gain, int time, bool stopFlag) {

uint8_t txbuf[12];
unsigned short vol;

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
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

/**************************************************************************/
/*!
    @brief Sets sample-rate offset, or playback speed / pitch, of the main 
	output mix. Offset gives a speed range of 1/2x to 2x, or a pitch range of 
	down one octave to up one octave. If audio is playing, you will hear the 
	result immediately. If audio is not playing, the new sample-rate offset 
	will be used the next time a track is started.
    @param offset Offset, -32767 to +32676.
*/
/**************************************************************************/
void wavTrigger::samplerateOffset(int offset) {

uint8_t txbuf[7];
unsigned short off;

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x07;
	txbuf[3] = CMD_SAMPLERATE_OFFSET;
	off = (unsigned short)offset;
	txbuf[4] = (uint8_t)off;
	txbuf[5] = (uint8_t)(off >> 8);
	txbuf[6] = EOM;
	WTSerial.write(txbuf, 7);
}

/**************************************************************************/
/*!
    @brief Immediately sets the Trigger Bank to the specified value.
    @param bank Bank value 1 to 32.
*/
/**************************************************************************/
void wavTrigger::setTriggerBank(int bank) {

uint8_t txbuf[6];

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x06;
	txbuf[3] = CMD_SET_TRIGGER_BANK;
	txbuf[4] = (uint8_t)bank;
	txbuf[5] = EOM;
	WTSerial.write(txbuf, 6);
}