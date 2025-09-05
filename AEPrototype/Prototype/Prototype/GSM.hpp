#pragma once
#include "AEEngine.h"
#include "utils.hpp"

enum GS_STATES
{
	GS_LEVEL1 = 0,
	GS_QUIT,
	GS_RESTART
};

typedef void(*FP)(void);

extern int current;
extern int previous;
extern int next;

extern FP fpLoad, fpInitialize, fpUpdate, fpDraw, fpFree, fpUnload;

void GSM_Initialize(int startingState);
void GSM_Update();


void Level1_Load();
void Level1_Init();
void Level1_Update();
void Level1_Draw();
void Level1_Free();
void Level1_Unload();