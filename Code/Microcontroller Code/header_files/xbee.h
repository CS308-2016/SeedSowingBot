
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include "grid_config.h"
#include "motion.h"
unsigned char data; //to store received data from UDR1

volatile int simulated = 0;


//Function To Initialize UART0
// desired baud rate:9600
// actual baud rate:9600 (error 0.0%)
// char size: 8 bit
// parity: Disabled
void uart0_init(void)
{
 UCSR0B = 0x00; //disable while setting baud rate
 UCSR0A = 0x00;
 UCSR0C = 0x06;
 UBRR0L = 0x5F; //set baud rate lo
 UBRR0H = 0x00; //set baud rate hi
 UCSR0B = 0x98;
}

void init_devices()
{
 cli(); //Clears the global interrupt
 port_init();  //Initializes all the ports
 left_position_encoder_interrupt_init();
 right_position_encoder_interrupt_init();
 timer5_init();
 buzzer_config();
 init_servos();
lcd_port_config();
lcd_set_4bit();
lcd_init();
 adc_pin_config();
 adc_init();
 uart0_init();
 sei();   // Enables the global interrupt
}

void dig_soil() {
	
	for (int i = 150; i >= 75; i--) {
	  servo_3(i);
	  _delay_ms(5);
  	}
	for (int i = 75; i <150; i++) {
		  servo_3(i);
		  _delay_ms(5);
	 }
	servo_3_free();
}

void sow_seed() {


	for (int i = 120; i >= 90; i--) {
	  servo_2(i);
	  _delay_ms(10);
	}
	for (int i = 90; i <120; i++) {
		servo_2(i);
		_delay_ms(10);
	}
	servo_2_free();


	for (int i = 30; i >= 0; i--) {
	  servo_1(i);
	  _delay_ms(10);
	}

	for (int i = 0; i <30; i++) {
			servo_1(i);
			_delay_ms(10);
	}

	servo_1_free();
}



void simulate() {
	velocity(255,255);
	int number_of_digs = length / seed_distance;
	for (int i = 0; i < breadth / column_seperation; i++) {
			int digs = 0;
			int sows = 0;
			for(int i=0;i<(dig_to_seed_distance)/seed_distance;i++) {

				if (digs < number_of_digs){
					dig_soil();
					_delay_ms(1000);
					digs++;
				}
				if (digs < number_of_digs) {
					forward_mm(seed_distance);
					back_up(6);
				}
			}

			while(1) {
				forward_mm((dig_to_seed_distance)%seed_distance);
				if (dig_to_seed_distance%seed_distance)
					back_up(6);
				if (sows < number_of_digs) {
					sow_seed();
					sows++;
				}
				else {
					break;
				}

				forward_mm(seed_distance - (dig_to_seed_distance)%seed_distance);
				if(seed_distance - (dig_to_seed_distance)%seed_distance)
					back_up(6);
				if (digs < number_of_digs) {
					dig_soil();
				}
			}

			_delay_ms(1000);
			velocity(149,150);
			if (i % 2 == 0) {
				right_degrees(95);
			}
			else {
				left_degrees(95);
			}
			_delay_ms(1000);
			velocity(255,255);
			forward_mm(column_seperation);
			velocity(149,150);
			_delay_ms(1000);
			if (i % 2 == 0) {
				right_degrees(95);
			}
			else {
				left_degrees(90);
			}
			_delay_ms(1000);
			velocity(255,255);
	}
}

ISR(USART0_RX_vect) 		// ISR for receive complete interrupt
{
	data = UDR0; 				//making copy of data from UDR0 in 'data' variable

	UDR0 = data; 				//echo data back to PC

	if (data == 0x38){
		simulated = 1;
	}
		
}

