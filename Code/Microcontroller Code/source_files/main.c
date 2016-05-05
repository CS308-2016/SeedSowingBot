#include <util/delay.h>
#include "xbee.h"


void reset_servo() {
	servo_3(150);
	servo_2(120);
	servo_1(30);
	_delay_ms(1000);
	servo_1_free();
	servo_2_free();
	servo_3_free();
}

int main() {

	init_devices();
	reset_servo();
	// bot will start when user press 8 from numberpad in xbee consule in xctu
	while(1) {
		if (simulated) {
			simulate();
			break;
		}
	}
}


