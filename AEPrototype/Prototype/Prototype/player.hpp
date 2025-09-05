#pragma once
#include "AEEngine.h"


const int PLAYER_SPEED = 300;

enum Direction{
	NEUTRAL = 0,
	UP,
	DOWN,
	LEFT,
	RIGHT
};

class Player {
public:
	//ctor and dtor 
	Player();
	~Player();

	AEVec2 pos;
	float radius; //for now its a circle, if using spritesheet i would recommend using a square...
	Direction direction;

};

extern Player player;										//anything that includes player.hpp will now have access to player




void update_p1Movement();
void draw_p1();