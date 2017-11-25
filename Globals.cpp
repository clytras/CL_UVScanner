

#include "Arduino.h"
#include "Configuration.h"
//#include <NewTone.h>
#include <toneAC.h>



void beep()
{
  //static bool newtone = false;

  //if(!newtone)
	  //tone(SPEAKER_PIN, BEEP_FREQUENCY, BEEP_DURATION);
  //else
  //  NewTone(SPEAKER_PIN, BEEP_FREQUENCY, BEEP_DURATION);
	//pinMode(SPEAKER_PIN, OUTPUT);
	//analogWrite(SPEAKER_PIN, 120);

  //  newtone = !newtone;

  toneAC(BEEP_FREQUENCY);
}
