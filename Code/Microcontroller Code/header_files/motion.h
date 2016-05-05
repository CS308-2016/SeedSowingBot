#include <avr/io.h>
#include <avr/interrupt.h>
#include "servo.h"

/*
*  This file contain helper functions used to control the motion of the bot
*/

void buzzer_config(void);

void motion_pin_config (void);

void left_encoder_pin_config (void);

void right_encoder_pin_config (void);

void port_init();

void left_position_encoder_interrupt_init (void);

void right_position_encoder_interrupt_init (void);

void motion_set (unsigned char Direction);

void forward (void);

void back (void);

void left (void);

void right (void);

void soft_right (void);

void soft_left_2 (void);

void soft_right_2 (void);

void stop (void);

void angle_rotate(unsigned int Degrees);

void linear_distance_mm(unsigned int DistanceInMM);

void forward_mm(unsigned int DistanceInMM);

void back_mm(unsigned int DistanceInMM);

void left_degrees(unsigned int Degrees);

void right_degrees(unsigned int Degrees);

void soft_left_degrees(unsigned int Degrees);

void soft_right_degrees(unsigned int Degrees);

void soft_left_2_degrees(unsigned int Degrees);

void timer5_init();

void velocity (unsigned char left_motor, unsigned char right_motor);

void back_up(unsigned int Degrees);


