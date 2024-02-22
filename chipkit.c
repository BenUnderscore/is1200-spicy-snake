#include "/opt/mcb32tools/include/pic32mx.h"
#include "screen.h"
#include <stdint.h>
#include "snake.h"

void write_leds(char value) {
	PORTE = value;
}

void initleds(){
	TRISECLR = 0xFF;
}

void artificial_delay(int count) {
	int i;
	for(i = 0; i < count; i++) {
	}
}

int counter = 0;

void user_isr() {
}

char screen_pages[4][4][32]; //pages, rows, columns

void update_screen(){
	int pagenum;
	for(pagenum = 0; pagenum < 4; pagenum++){
		display_image(32*pagenum,screen_pages[pagenum]);
	}
}

void render_pixel(int x, int y, char pixels) {
	int screen_x = x*2;
	int screen_y = y*2;
	int pagenum = screen_x >> 5; //divided by 32, round down for page
	int rownum = screen_y >> 3; 
	int colnum = screen_x & 0b11111;
	int bitnum = screen_y & 0b111;
	char *c1 = &screen_pages[pagenum][rownum][colnum];
	char *c2 = &screen_pages[pagenum][rownum][colnum+1];
	char mask = 0b11 << bitnum;
	*c1 = (*c1 & ~mask) | (mask & (pixels << bitnum));
	*c2 = (*c2 & ~mask) | (mask & (pixels >> 2 << bitnum));
}

void render_pixel_or(int x, int y, char pixels) {
	int screen_x = x*2;
	int screen_y = y*2;
	int pagenum = screen_x >> 5; //divided by 32, round down for page
	int rownum = screen_y >> 3; 
	int colnum = screen_x & 0b11111;
	int bitnum = screen_y & 0b111;
	char *c1 = &screen_pages[pagenum][rownum][colnum];
	char *c2 = &screen_pages[pagenum][rownum][colnum+1];
	char mask = 0b11 << bitnum;
	*c1 = *c1 | (mask & (pixels << bitnum));
	*c2 = *c2 | (mask & (pixels >> 2 << bitnum));
}

void render_snake(struct game_state *state){
	int x,y,screen_x,screen_y;
	int pagenum, rownum, colnum, bitnum;
	for(y = 0; y < state->config->field_size_y; y++){
		for(x = 0; x < state->config->field_size_x; x++){
			char segment = state->segments[y*state->config->field_size_x+x];
			render_pixel(x, y, segment == SNAKE_SEGMENT_NONE ? 0 : 0xF);
		}
	}

	render_pixel(state->food_x, state->food_y, 0b1001);
}

static char pixels_from_direction(int dx, int dy) {
	if(dx > 0) {
		return 0b1100;
	} else if(dx < 0) {
		return 0b0011;
	} else if(dy > 0) {
		return 0b1010;
	} else {
		return 0b0101;
	}
}

void render_animation(struct player_state* player_states, int player_count) {
	int player_num;
	for(player_num = 0; player_num < player_count; player_num++) {
		int x = player_states[player_num].head_x + player_states[player_num].dx;
		int y = player_states[player_num].head_y + player_states[player_num].dy;
		render_pixel_or(x, y, pixels_from_direction(player_states[player_num].dx, player_states[player_num].dy));
	}
}

void init_timer2(){
	// initialiserar timer 2 för 1/10s uppdatering
	T2CON = 0 | 1 << 15 | 0b111 << 4;
	PR2 = 31250;
}

int check_timer2(){
	if(IFS(0) >> 8 & 1){
		IFSCLR(0) = 1 << 8;
		return 1;
	}
	return 0;
}

struct game_state snake_state;

void snake_main(){
	struct player_state players[2];
	players[0].head_x = 8;
	players[0].head_y = 8;
	players[1].head_x = 40;
	players[1].head_y = 8;
	players[0].tail_x = players[0].head_x;
	players[0].tail_y = players[0].head_y;
	players[1].tail_x = players[1].head_x;
	players[1].tail_y = players[1].head_y;
	players[0].growth_backlog = 2;
	players[1].growth_backlog = 2;
	players[0].dx = 1;
	players[0].dy = 0;
	players[0].dead = 0;
	players[1].dx = -1;
	players[1].dy = 0;
	players[1].dead = 0;
	//Init
	//buttons of breadboard
	TRISD = 0b11101111;
	TRISF = 0b10;

	struct game_config config;
	config.field_size_x = 64;
	config.field_size_y = 16;

	init_snake_game(&snake_state, players, 2, &config, 18386);
	display_init();
	init_timer2();
	initleds();
	int counter = 0;
	int frame_counter = 0;
	int dx0 = 1;
	int dy0 = 0;
	int dx1 = -1;
	int dy1 = 0;
	while(1){
		char dbg  = PORTD & 0xF;
		write_leds(dbg);
		//pin 3, right
		if((PORTD >> 0 ) & 1){
			dx0 = 1;
			dy0 = 0;
		//pin 5, up
		} else if((PORTD >> 1) & 1){
			dx0 = 0;
			dy0 = 1;
		//pin 6, left
		} else if((PORTD >> 2) & 1){
			dx0 = -1;
			dy0 = 0;
		//pin 9, down
		} else if((PORTD >> 3) & 1){
			dx0 = 0;
			dy0 = -1;
		}
		//replacement for buttons on board to breadboard
		if((PORTD >> 7) & 1){
			dx1 = 1;
			dy1 = 0;
		} else if((PORTD >> 6) & 1){
			dx1 = -1;
			dy1 = 0;
		} else if((PORTD >> 5) & 1){
			dx1 = 0;
			dy1 = 1;
		} else if((PORTF >> 1) & 1){
			dx1 = 0;
			dy1 = -1;
		}
		if(check_timer2()){
			if((frame_counter & 1) == 0) {
				set_snake_direction(&snake_state, 0, dx0, dy0);
				set_snake_direction(&snake_state, 1, dx1, dy1);
				tick_snake_game(&snake_state);
				if(players[0].dead && players[1].dead){
					return;
				}
				render_snake(&snake_state);
				update_screen();
			}

			frame_counter++;
		}
	}
}

void buttons_test() {
	display_init();
	int counter = 0;
	int rownum, pagenum, colnum;
	for(pagenum = 0; pagenum < 4; pagenum++){
		for(rownum = 0; rownum < 4; rownum++){
			for(colnum = 0; colnum < 32; colnum++){
				screen_pages[pagenum][rownum][colnum] = counter++;
			}
		}
	}
	update_screen();
	// uint8_t data[128];
	// {
	// 	int i;
	// 	for(i = 0; i < 128; i++) {
	// 		data[i] = i;
	// 	}
	// }
	// display_image(0, data);
	// display_image(32, data);
	// display_image(64, data);
	// display_image(96, data);

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
		screen_pages[0][0][0] = PORTD & (1<<5);
		update_screen();

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

	snake_main();
	
	return 0;
}
