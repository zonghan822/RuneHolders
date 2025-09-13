#include "player.hpp"
#include "utils.hpp"

Player::Player() : pos{ 0,0 }, radius{ 10 }, direction{ NEUTRAL } {}
Player::~Player() = default;

Player player;

void update_p1Movement() {

	player.direction = NEUTRAL;

	if (AEInputCheckCurr(AEVK_W)) {
		player.direction = UP;
	}
	else if (AEInputCheckCurr(AEVK_S)) {
		player.direction = DOWN;
	}
	else if (AEInputCheckCurr(AEVK_A)) {
		player.direction = LEFT;
	}
	else if (AEInputCheckCurr(AEVK_D)) {
		player.direction = RIGHT;
	}

	//if (player.direction == RIGHT && player.direction == LEFT  ) { player.direction = NEUTRAL; }
	//if (player.direction == UP    && player.direction == DOWN  ) { player.direction = NEUTRAL; }

	switch (player.direction) {
	case UP:
		player.pos.y += PLAYER_SPEED * dt;
		break;
	case DOWN:
		player.pos.y -= PLAYER_SPEED * dt;
		break;
	case RIGHT:
		player.pos.x += PLAYER_SPEED * dt;
		break;
	case LEFT:
		player.pos.x -= PLAYER_SPEED * dt;
		break;

	}
}

	void draw_p1() {
		AEGfxSetColorToAdd(0, 1, 0, 0);
		AEGfxSetColorToMultiply(1, 1, 1, 1);
		draw_circle(player.pos.x, player.pos.y, player.radius, 1);
		
	}
