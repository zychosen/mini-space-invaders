#include "i2c.h"
#include "ssd1306.h"
#include "font5x8.h"
#include "assets.h"
/**
 * main.c
 */

int8_t flag = 0;
uint8_t state = 3, game_over = 0;
Sprite E[3], player;

void init(void) {
	uint8_t i;
	for(i = 0; i < 3; i++) {
		E[i].x = 50*i;
		E[i].y = 0;
		E[i].bitmap = EnemyB;
		E[i].weapon.bitmap = MissileEnemy;
		E[i].weapon.y = 20;
		E[i].death = explosion;
		E[i].life = 1;
		E[i].velocity = 1;
	}

	player.x = XPOS_PLAYER;
	player.y = YPOS_PLAYER;
	player.bitmap = BattleShip;
	player.weapon.bitmap = Missile;
	player.weapon.x = player.x + PLAYER_WIDTH/2;
	player.weapon.y = player.y - 3;

	/* Setup switch irq */
	P2DIR &= ~BIT1;
	P2REN |= BIT1;
	P2OUT |= BIT1;
	P2IES |= BIT1;
	P2IE |= BIT1;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;
	P2IFG &= ~BIT1;
}

void change_enemy_state(Sprite *s) {
	uint8_t i = 0;
	for(i = 0; i < 3; i++) {
		s[i].x = 50*i;
		if(s[i].y >= player.y) {
			game_over = 1;
			break;
		} else {
			s[i].y += 1;
		}
	}
}

void move(void) {
	uint8_t i;
	if (!game_over) {
		if(state--) {
			for(i = 0; i < 3; i++) {
				E[i].x += E[i].velocity;
			}
		} else {
			change_enemy_state(E);
			state = 3;
		}

		if(flag && ((player.weapon.y  > 0))) {
			player.weapon.y  -= 10;
		} else {
			flag = 0;
			player.weapon.y  = player.y - 5;
		}
	}

}

uint8_t checkCollision(Sprite enemy, Sprite player) {
	uint8_t XCollision = (enemy.x <= player.weapon.x) && (enemy.x + ENEMY_WIDTH >= player.weapon.x);
	uint8_t YCollision = (enemy.y + ENEMY_HEIGHT) >= player.weapon.y ;
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
		uint8_t i;
		for(i = 0; i < 3; i++) {
			if(E[i].life > 0) {
				if (checkCollision(E[i], player)) {
					drawBitmap(E[i].death, EXPLOSION_WIDTH, EXPLOSION_HEIGHT, E[i].x, E[i].y);
					E[i].life = 0;
				}
				else
					drawBitmap(E[i].bitmap, ENEMY_WIDTH, ENEMY_HEIGHT, E[i].x, E[i].y);
			}
		}
		if(flag && player.weapon.y > 0)
			drawBitmap(player.weapon.bitmap, PLAYER_WEAPON_WIDTH, PLAYER_WEAPON_HEIGHT, player.weapon.x, player.weapon.y);
		drawBitmap(player.bitmap, PLAYER_WIDTH, PLAYER_HEIGHT, player.x, player.y);
	}
	update_screen();
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	            /* stop watchdog timer */
	static const uint8_t address = 0x3C;    /* I2C address of slave */
	i2c_setup(address);
	ssd1306_setup();

	int i;
	init();
	draw();

	__enable_interrupt();
	while (1) {
		move();
		draw();
		//for (i = 500; i > 0; i--);
	}
}

#pragma vector = PORT2_VECTOR
__interrupt void ISR_Port_2 (void) {
	P2IFG &= ~BIT1;
	P1OUT ^= BIT0;
    flag = 1;
}

