#include <crtdbg.h> // To check for memory leaks 
#include <iostream>
#include <cmath>
#include "GSM.hpp"

void Level1_Load()
{
	//std::cout << "Loaded\n";	

}

void Level1_Init()
{
	//dt = 0;																 //init dt
	//std::cout << "Init\n";
}

void Level1_Update() {
	//dt = static_cast<float>(AEFrameRateControllerGetFrameTime());	         //update dt
	//if (AEInputCheckCurr(AEVK_P)) {
	//	next = GS_QUIT;
	//}
}


void Level1_Draw() {

}



void Level1_Free()
{
	//std::cout << "free";
	//destroy_all_meshes();

}

void Level1_Unload()
{
	//std::cout << "unloaded";
}