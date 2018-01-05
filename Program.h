/*
 * Project: UV PCB Exposure Box
 * URL: https://lytrax.io/blog/projects/diy-uv-exposure-box
 * Youtube Video: https://www.youtube.com/watch?v=ZAlxNNI-BVM
 * Author: Christos Lytras <christos.lytras@gmail.com>
 * Description: I'm using an older scanner as a box to create a UV PCB exposure box. I'll go through all steps of the hardware and software design emphasizing more to software logic.
 * Version: 1.1
 *
 * 1.1
 * - Change EEPROM write, to write only after 5 seconds the encoder has stopped and when the program goes at running mode
 *
 * */

#include <Arduino.h>
#include <TM1637Display.h>
#include <CL_RotaryEncoder.h>
#include <CL_Button.h>
#include <toneAC.h>
#include <EEPROM.h>

#include "Configuration.h"
#include "Globals.h"

#ifndef __PROGRAM__
#define __PROGRAM__

class Program
{
public:
	Program();

	static Program *self; // Static self for use in static functions

	enum Modes {
		ModeSetD1 = 0,
		ModeSetD2 = 1,
		ModeSetD3 = 2,
		ModeSetD4 = 3, // Default
		ModeRunning = 4,
		ModePaused = 5,
		ModeFinished = 6
	};

	void initialize();
	void run(bool encoderSwitchHandler = true);
	static void run_btnDoEvents(Program* pProgram);
	void setMode(Modes mode);
	Modes getMode() { return m_mode; }
	void nextSetMode();
	void playMode();

	// Mode run functions

	void runSetMode(bool forceUpdate = false);
	void runRunningMode();
	void runFinishedMode();
	void runPausedMode(bool forcePausedState = false);
	void runEncoderSwitchHandler();

	// Mode change functions

	void doModeSet();
	void doModeStart();
	void doModeStop();
	void doModePause();
	void doModeResume();
	void doModeFinish();

	// Events

	static void onEncoderLeft(CL_RotaryEncoder *enc, Program *prog);
	static void onEncoderRight(CL_RotaryEncoder *enc, Program *prog);
	void onEncoderSwitchPush();
	void onEncoderSwitchLongPush();
	void onEncoderChange(int steps);
	static void onButtonStartPush(CL_Button *button);
	static void onButtonStopPush(CL_Button *button);

	// ATMEGA328 ROM Data

	void writeEEPROMData();
	void readEEPROMData();

	// LED Strip power on/off
	void ledUVPower(bool bSet = true);
	void ledUVToggle();

	// Speaker tone functions

	void beep();
	void finishedBeep();
	void hourChangeBeep();
	void tone(uint16_t frequency,
			uint32_t duration,
			uint8_t beepsCount = 1,
			uint32_t pauseDuration = 0 /* 0 = duration */);
	void runToneHandler();
	void stopBeep();

	TM1637Display display;
	CL_RotaryEncoder encoder;

	Modes m_mode;
	CL_Button m_startBtn, m_stopBtn;

	bool m_colon;
	uint32_t m_colonTimer;
	bool m_setTimeBlinkState;
	uint32_t m_setTimeBlinkTimer;
	uint32_t m_eepromWriteTimer;

private:
	uint16_t m_beepFrequency;
	uint32_t m_beepDuration;
	uint8_t m_beepBeepsCount;
	uint32_t m_beepPauseDuration;
	uint32_t m_beepCycle;
	uint32_t m_beepPauseCycle;
	bool m_beepActive;

	int16_t m_setTimeCurrentValue;
	int16_t m_secsCountdown;
	uint32_t m_countdownTimer;
	int16_t m_countdownSecs;
	char m_currentDigits[5];
	bool m_ledUVPower;
};

#endif // __PROGRAM__
