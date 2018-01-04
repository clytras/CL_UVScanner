/*
 * Project: UV PCB Exposure Box
 * URL: https://lytrax.io/blog/projects/diy-uv-exposure-box
 * Youtube Video: https://www.youtube.com/watch?v=ZAlxNNI-BVM
 * Author: Christos Lytras <christos.lytras@gmail.com>
 * Description: I'm using an older scanner as a box to create a UV PCB exposure box. I'll go through all steps of the hardware and software design emphasizing more to software logic.
 * Version: 1.0
 *
 * */

#include "Program.h"

Program *Program::self;

Program::Program() :
display(DISPLAY_PIN_CLK, DISPLAY_PIN_DIO),
encoder(ENCODER_PIN_A, ENCODER_PIN_B),
m_mode(ModeSetD4),
m_colonTimer(0),
m_beepBeepsCount(0),
m_colon(true),
m_eepromWriteTimer(0)
{
	Program::self = this;
}

void Program::initialize()
{
	pinMode(ENCODER_SWITCH_PIN, INPUT);
	digitalWrite(ENCODER_SWITCH_PIN, HIGH);

	pinMode(STRIP_MOSFET_PIN, OUTPUT);
	m_ledUVPower = false;

	m_startBtn.onButtonPress(&this->onButtonStartPush);
	m_startBtn.onDoEvents(&this->run_btnDoEvents, (void*)this);
	m_startBtn.init(START_SWITCH_PIN, HIGH);
	m_stopBtn.onButtonPress(&this->onButtonStopPush);
	m_stopBtn.onDoEvents(&this->run_btnDoEvents, (void*)this);
	m_stopBtn.init(STOP_SWITCH_PIN, HIGH);

	readEEPROMData();

	display.setBrightness(LIGHT_BRIGHTEST); 
	display.showNumberInt(m_secsCountdown);

	encoder.setDataArg(this);
	encoder.enable();
	encoder.onLeft(&this->onEncoderLeft);
	encoder.onRight(&this->onEncoderRight);
}

void Program::run(bool encoderSwitchHandler /*= true*/)
{
	encoder.update();
	m_startBtn.update(true);
	m_stopBtn.update(true);

	playMode();
	runToneHandler();
	if(encoderSwitchHandler)
		runEncoderSwitchHandler();
}

void Program::run_btnDoEvents(Program* pProgram) {
	pProgram->encoder.update();
	pProgram->runToneHandler();
}

void Program::onButtonStartPush(CL_Button *button) {
	Program::self->doModeStart();
}

void Program::onButtonStopPush(CL_Button *button) {
	Program::self->doModeStop();
}

void Program::setMode(Modes mode)
{
	m_mode = mode;
}

void Program::nextSetMode()
{
	switch(m_mode)
	{
	case ModeSetD1:
		setMode(ModeSetD4);
		break;
	case ModeSetD2:
		setMode(ModeSetD1);
		break;
	case ModeSetD3:
		setMode(ModeSetD2);
		break;
	case ModeSetD4:
		setMode(ModeSetD3);
		break;
	}
}

void Program::playMode()
{
	switch(m_mode)
	{
	case ModeSetD1:
	case ModeSetD2:
	case ModeSetD3:
	case ModeSetD4: // Default
		runSetMode();
		break;
	case ModeRunning:
		runRunningMode();
		break;
	case ModePaused:
		runPausedMode();
		break;
	case ModeFinished:
		runFinishedMode();
		break;
	}
}

void Program::runSetMode(bool forceUpdate /*= false*/)
{
	char digits[5];
	char padDigits[5];
	itoa(m_secsCountdown, digits, 10);
	size_t digitsLen = strlen(digits);

	memset(padDigits, ' ', sizeof(padDigits));
	padDigits[4 - digitsLen] = 0;
	strcat(padDigits, digits);

	for(int i = 4; i >= m_mode; i--) {
		if(padDigits[i] == ' ')
			padDigits[i] = '0';
	}

	uint32_t currentMillis = millis();

	if(currentMillis - m_setTimeBlinkTimer >= 499)
	{
		m_setTimeBlinkState = !m_setTimeBlinkState;
		m_setTimeBlinkTimer = millis();
	}

	if(m_mode != ModeSetD4 && !m_setTimeBlinkState)
		padDigits[m_mode] = 0x20;

	if(forceUpdate || strcmp(m_currentDigits, padDigits) != 0)
	{
		display.showString(padDigits);
		strcpy(m_currentDigits, padDigits);
	}

	if(m_eepromWriteTimer != 0 && (currentMillis - m_eepromWriteTimer) >= (EEPROM_WRITE_AFTER_ENCODER_SECONDS * 1000))
	{
		writeEEPROMData();
		m_eepromWriteTimer = 0;
	}
}

void Program::doModeSet()
{
	if(m_mode != ModeRunning)
	{
		stopBeep();
		m_mode = ModeSetD4;
		runSetMode(true);
	}
}

void Program::doModeStart()
{
	if(m_mode == ModeRunning)
		doModePause();
	else if(m_mode == ModePaused)
		doModeResume();
	else if(m_mode == ModeFinished)
		doModeSet();
	else
	{
		stopBeep();
		m_setTimeBlinkState = true;
		m_setTimeBlinkTimer = millis();
		m_mode = ModeSetD4;
		runSetMode(true);
		m_mode = ModeRunning;
		m_countdownSecs = m_secsCountdown;
		m_countdownTimer = millis();
		ledUVPower(true);
	}
}

void Program::doModeStop()
{
	if(m_mode == ModeRunning)
	{
		m_mode = ModeFinished;
		ledUVPower(false);
	}
	else if(m_mode == ModePaused || m_mode == ModeFinished)
	{
		doModeSet();
	}
}

void Program::doModePause()
{
	if(m_mode == ModeRunning)
	{
		m_mode = ModePaused;
		ledUVPower(false);
	}
}

void Program::doModeResume()
{
	if(m_mode == ModePaused)
	{
		runPausedMode(true); // Set display to on immediately; Eliminates on/off delay
		m_mode = ModeRunning;
		m_countdownTimer = millis();
		ledUVPower(true);
	}
}

void Program::doModeFinish()
{
	if(m_mode == ModeRunning)
	{
		m_mode = ModeFinished;
		ledUVPower(false);
	}
}

void Program::ledUVPower(bool bSet /*= true*/)
{
	m_ledUVPower = bSet;
	digitalWrite(STRIP_MOSFET_PIN, m_ledUVPower ? HIGH : LOW);
}

void Program::ledUVToggle()
{
	ledUVPower(!m_ledUVPower);
}

void Program::runFinishedMode()
{
	static bool displayState = false;
	static uint32_t finishedTimer = millis() + 500;
	static uint32_t finishedBeepTimer = millis() + 7000;

	if(millis() - finishedTimer > 499) {
		displayState = !displayState;

		if(displayState)
			display.showString("----");
		else
			display.showString("");

		finishedTimer = millis();
	}

	if(millis() - finishedBeepTimer > 6999)
	{
		finishedBeep();
		finishedBeepTimer = millis();
	}
}

void Program::runRunningMode()
{
	if(millis() - m_countdownTimer > 999)
	{
		m_countdownSecs--;
		if(m_countdownSecs <= 0)
		{
			doModeFinish();
		}
		else
		{
			display.showNumberInt(m_countdownSecs);
		}
		m_countdownTimer = millis();
	}
}

void Program::runPausedMode(bool forcePausedState /*= false*/)
{
	static uint32_t pausedTimer = millis() + 1200;
	static bool pausedState = false;

	if(forcePausedState || millis() - pausedTimer > (pausedState ? 1199 : 499))
	{
		pausedState = !forcePausedState ? !pausedState : true;
		if(pausedState)
			display.showNumberInt(m_countdownSecs);
		else
			display.showString("");
		pausedTimer = millis();
	}
}

void Program::writeEEPROMData()
{
	int addr = EEPROM_START_ADDRESS;
	EEPROM.put(addr, m_secsCountdown);
}

void Program::readEEPROMData()
{
	int addr = EEPROM_START_ADDRESS;
	EEPROM.get(addr, m_secsCountdown);

	if(m_secsCountdown < 0)
		m_secsCountdown = 1;
	else if(m_secsCountdown > 9999)
		m_secsCountdown = 9999;
}

void Program::beep()
{
  this->tone(BEEP_FREQUENCY, BEEP_DURATION);
}

void Program::hourChangeBeep()
{
  this->tone(BEEP_FREQUENCY, BEEP_DURATION, 2);
}

void Program::finishedBeep()
{
	this->tone(3500, 500, 2, 500);
}

void Program::tone(uint16_t frequency,
            uint32_t duration,
            uint8_t beepsCount /*= 1*/,
            uint32_t pauseDuration /*= 0*/ /* 0 = duration */)
{
  m_beepFrequency = frequency;
  m_beepDuration = duration;
  m_beepBeepsCount = beepsCount;
  m_beepPauseDuration = pauseDuration == 0 ? duration : pauseDuration;
  m_beepActive = false; 
  m_beepPauseCycle = millis() - m_beepPauseDuration;
}

void Program::runToneHandler()
{
  if(m_beepBeepsCount > 0)
  {
    if(!m_beepActive)
    {
      if(millis() - m_beepPauseDuration >= m_beepPauseCycle)
      {
        toneAC(m_beepFrequency);
        m_beepCycle = millis();
        m_beepActive = true;
      }
    }
    else
    {
      if(millis() - m_beepCycle >= m_beepDuration)
      {
        noToneAC();
        m_beepPauseCycle = millis();
        m_beepActive = false;
        m_beepBeepsCount--;
      }
    }
  }
}

void Program::stopBeep()
{
	m_beepBeepsCount = 0;
	noToneAC();
}

void Program::runEncoderSwitchHandler()
{
	unsigned long decouplingTimer = millis() + 50;

	if((millis() - decouplingTimer) < 50)
		return;

	if (digitalRead(ENCODER_SWITCH_PIN) == LOW)
	{
		//unsigned long timeBegan = millis();
		bool modeChanged = false;

		while(digitalRead(ENCODER_SWITCH_PIN) == LOW)
		{
			run(false);
			/*if(millis() - timeBegan > ENCODER_MODE_CHANGE_INTERVAL)
			{
				beep();
				nextMode();

				timeBegan = millis();
				modeChanged = true;
			}*/
		}

		decouplingTimer = millis();

		if(!modeChanged && digitalRead(ENCODER_SWITCH_PIN) == HIGH)
			onEncoderSwitchPush();
	}
}

void Program::onEncoderSwitchPush()
{
	if(m_mode == ModeRunning)
		doModePause();
	else if(m_mode == ModePaused)
		doModeResume();
	else if(m_mode == ModeFinished)
		doModeStop();
	else
	{
		nextSetMode();
		beep();
	}
}

void Program::onEncoderSwitchLongPush()
{
}

void Program::onEncoderChange(int steps)
{
	if(m_mode == ModeRunning)
		return;

	if(m_mode == ModeFinished)
	{
		m_mode = ModeSetD4;
		return;
	}

	m_setTimeBlinkState = true;
	m_setTimeBlinkTimer = millis();
	m_eepromWriteTimer = millis();
	runSetMode();
	bool leadingZeros = true;
	int16_t secsCountdown = m_secsCountdown;
	uint8_t numbers[4];
	int i;

	for(i = 0; i < 4; i++)
	{
		if(m_currentDigits[i] > '0' && m_currentDigits[i] <= '9') {
			numbers[i] = m_currentDigits[i] - 0x30;
			if(i < (m_mode - 0))
				leadingZeros = false;
		} else
			numbers[i] = 0;

	}

	if(leadingZeros && steps <= 0 && numbers[m_mode] == 0) {
		if(steps != 0 && m_mode < 3) {
			m_mode = (Modes)m_mode + 1;
		}
		return;
	}

	int power = ceil(pow(10, 4 - m_mode - 1));
	m_secsCountdown += steps * power;

	if(m_secsCountdown < 1)
		m_secsCountdown = 1;
	else if(m_secsCountdown > 9999)
		m_secsCountdown = secsCountdown;
}

void Program::onEncoderLeft(CL_RotaryEncoder *enc, Program *prog)
{
	Program::self->onEncoderChange(-enc->getSteps());
}

void Program::onEncoderRight(CL_RotaryEncoder *enc, Program *prog)
{
	Program::self->onEncoderChange(enc->getSteps());
}
