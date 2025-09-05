#include "GSM.hpp"


int current = 0, previous = 0, next = 0;

FP fpLoad = nullptr, fpInitialize = nullptr, fpUpdate = nullptr, fpDraw = nullptr, fpFree = nullptr, fpUnload = nullptr;

void GSM_Initialize(int startingState)
{
	current = previous = next = startingState;
}

void GSM_Update()
{
	switch (current)
	{

	case GS_LEVEL1:
		fpLoad = Level1_Load;
		fpInitialize = Level1_Init;
		fpUpdate = Level1_Update;
		fpDraw = Level1_Draw;
		fpFree = Level1_Free;
		fpUnload = Level1_Unload;
		break;

	case GS_RESTART: //to do: read file to reset counter but not lives
		break;
	case GS_QUIT: //free 
		break;
	default:
		break;
	}
}