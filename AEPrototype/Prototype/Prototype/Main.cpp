// ---------------------------------------------------------------------------
// includes

#include <crtdbg.h> // To check for memory leaks mainmerged.cpp
#include "AEEngine.h"
#include "GSM.hpp"
#include "Utils.hpp"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(882);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	//_CrtSetBreakAlloc(3461);

	// System Initialize
	AESysInit(hInstance, nCmdShow, static_cast<s32>(WINDOW_WIDTH), static_cast<s32>(WINDOW_HEIGHT), 1, 60, true, NULL);

	// Changing the window title
	AESysSetWindowTitle("Solo Project!");

	// Game State Management Initialize
	GSM_Initialize(GS_LEVEL1);

	// Game State Manager Loop
	while (current != GS_QUIT)
	{
		if (current != GS_RESTART) {
			GSM_Update();
			fpLoad();
		}
		else {
			next = previous;
			current = previous;
		}

		fpInitialize();

		while (next == current) {
			AESysFrameStart();
			fpUpdate();
			fpDraw();

			if (0 == AESysDoesWindowExist()) {
				next = GS_QUIT;
			}

			AESysFrameEnd();
		}
		fpFree();
		if (next != GS_RESTART) {
			fpUnload();
		}

		previous = current;
		current = next;
	}

	// System Exit (terminate)
	AESysExit();
	return 0;
}