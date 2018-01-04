
#ifndef __CONFIGURATION__
#define __CONFIGURATION__


// Program settings ------------------------------------------

#define SPEAKER_PIN 7
#define BEEP_FREQUENCY 5000
#define BEEP_DURATION 100

#define EEPROM_START_ADDRESS 10 // Change from 0 to 10 because 0 is overused
#define EEPROM_WRITE_AFTER_ENCODER_SECONDS 5

#define STRIP_MOSFET_PIN 8 //5

#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
#define ENCODER_SWITCH_PIN 4 //6 //8 //5

#define ENCODER_MODE_CHANGE_INTERVAL 2000

#define START_SWITCH_PIN 6 //5 //7
#define STOP_SWITCH_PIN 7 //6

#define SETTIME_BLINK_INTERVAL 200

#define DISPLAY_PIN_CLK 12
#define DISPLAY_PIN_DIO 11

#define PROGRAMMER_LED_PIN 13

#endif // __CONFIGURATION__
