#include "/opt/mcb32tools/include/pic32mx.h"
#include "screen.h"
#include <stdint.h>

void write_leds(char value) {
	PORTE = value;
}

void artificial_delay(int count) {
	int i;
	for(i = 0; i < count; i++) {
	}
}

int counter = 0;

void user_isr() {
}

void buttons_test() {
	display_init();
	uint8_t data[128];
	{
		int i;
		for(i = 0; i < 128; i++) {
			data[i] = i;
		}
	}
	display_image(0, data);
	display_image(32, data);
	display_image(64, data);
	display_image(96, data);

	int buttons_mask = (1 << 5) | (1 << 6) | (1 << 7);
	
	//Init
	TRISD = buttons_mask;
	TRISE = 0x00;
	TRISF = (1 << 3) | (1 << 2);
	TRISD |= (1 << 0) | (1 << 8) | (1 << 2);

	while(1) {
		int up = PORTF & (1 << 3);
		int right = PORTD & (1 << 0);
		int down = PORTD & (1 << 8);
		int left = PORTD & (1 << 2);
		write_leds(up | right | (down >> 7) | left);

		//artificial_delay(1000 * 1000);
	}
}

//Main function modified from lab
int main() {
        /*
	  This will set the peripheral bus clock to the same frequency
	  as the sysclock. That means 80 MHz, when the microcontroller
	  is running at 80 MHz. Changed 2017, as recommended by Axel.
	*/
	SYSKEY = 0xAA996655;  /* Unlock OSCCON, step 1 */
	SYSKEY = 0x556699AA;  /* Unlock OSCCON, step 2 */
	while(OSCCON & (1 << 21)); /* Wait until PBDIV ready */
	OSCCONCLR = 0x180000; /* clear PBDIV bit <0,1> */
	while(OSCCON & (1 << 21));  /* Wait until PBDIV ready */
	SYSKEY = 0x0;  /* Lock OSCCON */
	
	/* Set up output pins */
	//AD1PCFG = 0xFFFF;
	//ODCE = 0x0;
	//TRISECLR = 0xFF;
	//PORTE = 0x0;
	
	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;
	
	/* Set up input pins */
	//TRISDSET = (1 << 8);
	//TRISFSET = (1 << 1);
	
	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	/* SPI2STAT bit SPIROV = 0; */
	SPI2STATCLR = 0x40;
	/* SPI2CON bit CKP = 1; */
    SPI2CONSET = 0x40;
	/* SPI2CON bit MSTEN = 1; */
	SPI2CONSET = 0x20;
	/* SPI2CON bit ON = 1; */
	SPI2CONSET = 0x8000;

	buttons_test();
	
	return 0;
}
