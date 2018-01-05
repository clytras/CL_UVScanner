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


#include "Program.h"

Program program;


void setup()
{
	program.initialize();
}

void loop()
{
	program.run();
}
