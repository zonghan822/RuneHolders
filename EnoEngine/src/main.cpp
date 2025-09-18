#include "engine/eno_engine.hpp"

int main()
{
	Engine::EnoEngine engine;

	try
	{
		engine.setup();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}