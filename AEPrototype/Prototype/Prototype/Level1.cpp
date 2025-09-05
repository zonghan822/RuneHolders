#include <crtdbg.h> // To check for memory leaks 
#include <iostream>
#include <cmath>
#include "GSM.hpp"
#include "player.hpp"
#include "Utils.hpp"


void Level1_Load()
{
	create_all_meshes();
}

void Level1_Init()
{
	dt = 0;																 //init dt
	std::cout << "Init\n";
}

void Level1_Update() {
	dt = static_cast<float>(AEFrameRateControllerGetFrameTime());	     //update dt
	mouse_to_world();

	//player related
	update_p1Movement();


	if (AEInputCheckCurr(AEVK_P)) {
		next = GS_QUIT;
	}
}


void Level1_Draw() {


	AEGfxSetBackgroundColor(1.0, 1.0, 1.0); 
	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

	draw_p1();

	

}

void Level1_Free()
{
	std::cout << "free";
	destroy_all_meshes();

}

void Level1_Unload()
{
	std::cout << "unloaded";
}