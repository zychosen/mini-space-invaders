#include "i2c.h"
#include "ssd1306.h"
#include "font5x8.h"
#include "assets.h"
#include "random.h"

uint8_t semaphore = 0;
uint8_t transition_delay = 10, game_over = 0;
Sprite E[3], player;
uint16_t adcVal;
uint8_t direction = 0;

void init(void) {
	uint8_t i;

	/* Enemy Setup */
	for(i = 0; i < 3; i++) {
		E[i].x = 45*i;
		E[i].y = 0;
		E[i].bitmap[0] = EnemyB;
		E[i].bitmap[1] = EnemyB_Attack;
		E[i].weapon.bitmap = Missile_Enemy;
		E[i].weapon.x = E[i].x + ENEMY_WIDTH/2;
		E[i].weapon.y = E[i].y + ENEMY_HEIGHT;
		E[i].weapon.life = 0;
		E[i].death = explosion;
		E[i].life = 1;
		E[i].velocity = 1;
	}

	/* Player setup */
	player.x = XPOS_PLAYER;
	player.y = YPOS_PLAYER;
	player.bitmap[0] = BattleShip;
	player.weapon.bitmap = Missile;
	player.weapon.x = player.x + PLAYER_WIDTH/2;
	player.weapon.y = player.y - 3;
	player.weapon.life = 0;

	/* Setup switch irq */
	P2DIR &= ~BIT1;
	P2REN |= BIT1;
	P2OUT |= BIT1;
	P2IES |= BIT1;
	P2IE |= BIT1;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;
	P2IFG &= ~BIT1;

	/*Timer setup */
	TB0CTL |= TBCLR;
	TB0CTL |= MC__UP;              // up mode for compare because timer will count to 2^16 even after compare flag is asserted
	TB0CTL |= TBSSEL__ACLK;
	TB0CCR0 = 60;                  // 30Hz rendering

	TB0CCTL0 |= CCIE;              // timer capture control register
	TB0CCTL0 &= ~CCIFG;

	/* ADC12 setup */
	P6SEL |= BIT0;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;
	ADC12CTL0 |= ADC12SHT02;
	ADC12CTL0 |= ADC12ON;
	ADC12CTL1 |= ADC12SHP;
	ADC12CTL1 |= ADC12SSEL_3;
	ADC12CTL0 |= ADC12ENC;
}

inline void change_enemy_state(void) {
	uint8_t i = 0;
	direction = !direction;          // placed out of the loop because change in direction doesn't depend on whether enemy is alive or not
	for(i = 0; i < 3; i++) {         // if placed inside, effects other enemy directions cause it isnt an attribute, but a global state
		if (E[i].life > 0) {
			if(E[i].y >= player.y) {
				game_over = 1;
				break;
			} else {
				if(direction == 0) E[i].y += 1;
			}
		}
	}
}

inline void move(void) {
	uint8_t i;
	if (!game_over) {
		if(transition_delay--) {
			for(i = 0; i < 3; i++) {
				if(E[i].life > 0) {
					if (direction) E[i].x -= E[i].velocity;
					else E[i].x += E[i].velocity;
				}
			}
		} else {
			change_enemy_state();
			transition_delay = 10;
		}

		float output = 0.04*adcVal; // mapping 0-2047 due to op-amp sat
		player.x = output;

		if(!player.weapon.life)
		player.weapon.x = output + PLAYER_WIDTH/2;

		if(player.weapon.life && ((player.weapon.y > 0))) {
			player.weapon.y  -= 12;
		} else {
			player.weapon.life = 0;
			player.weapon.y  = player.y - 5;
		}
	}
}

uint8_t checkCollision(Sprite *enemy, Sprite *player) {
	uint8_t XCollision = (enemy->x <= player->weapon.x) && (enemy->x + ENEMY_WIDTH >= player->weapon.x);
	uint8_t YCollision = (enemy->y + ENEMY_HEIGHT) >= player->weapon.y ;
	if(!(XCollision && YCollision))
		return 0;
	else
		return 1;
}

void draw(void) {
	clear_screen();
	if (game_over) {
		drawString("GAME OVER!", 35, 28);
	} else {
		uint8_t i, sel;
		sel = (random() >> 8) % 3;
		for(i = 0; i < 3; i++) {
			if(E[i].life > 0) {
				if (checkCollision(&E[i], &player)) {
					drawBitmap(E[i].death, EXPLOSION_WIDTH, EXPLOSION_HEIGHT, E[i].x, E[i].y);
					E[i].life = 0;
					player.weapon.life = 0;
				}
				else {
					if((sel == i)) {
						drawBitmap(E[i].bitmap[1], ENEMY_WIDTH, ENEMY_HEIGHT, E[i].x, E[i].y);
						//drawBitmap(E[i].weapon.bitmap, ENEMY_WEAPON_WIDTH, ENEMY_WEAPON_HEIGHT, E[i].weapon.x, E[i].weapon.y);
					}
					else
						drawBitmap(E[i].bitmap[0], ENEMY_WIDTH, ENEMY_HEIGHT, E[i].x, E[i].y);
				}
			}
		}

		if(player.weapon.life && player.weapon.y > 0)
			drawBitmap(player.weapon.bitmap, PLAYER_WEAPON_WIDTH, PLAYER_WEAPON_HEIGHT, player.weapon.x, player.weapon.y);
		drawBitmap(player.bitmap[0], PLAYER_WIDTH, PLAYER_HEIGHT, player.x, player.y);
	}
	update_screen();
}

int main(void)
{

	WDTCTL = WDTPW | WDTHOLD;	            /* stop watchdog timer */
	static const uint8_t address = 0x3C;    /* I2C address of slave */
	i2c_setup(address);
	ssd1306_setup();

	init();
	Seed(0);                  // randomize seed???
	draw();
	__enable_interrupt();
	while (1) {
		while (semaphore == 0);
		draw();
		semaphore = 0;
	}
}

#pragma vector = PORT2_VECTOR
__interrupt void ISR_Port_2 (void) {
	P2IFG &= ~BIT1;
	P1OUT ^= BIT0;
    player.weapon.life = 1;
}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_Timer_CCR0 (void) {
	TB0CCTL0 &= ~CCIFG;
	if(semaphore == 0) {
		ADC12CTL0 |= ADC12SC;
		adcVal = ADC12MEM0;
		move();
		semaphore = 1;
	}
}
