#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*
* This file contains functions to control the servos
*/
void servo1_pin_config (void);

void servo2_pin_config (void);

void servo3_pin_config (void);

void port_init_servo(void);

void timer1_init(void);

void init_servos(void);

void servo_1(unsigned char degrees);

void servo_2(unsigned char degrees);

void servo_3(unsigned char degrees);

void servo_1_free (void);

void servo_2_free (void);

void servo_3_free (void);
