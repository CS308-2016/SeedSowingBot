#include <avr/io.h>
#include <avr/interrupt.h>

// fucntions to control velocity of bot
void timer5_init();
void velocity (unsigned char left_motor, unsigned char right_motor);
