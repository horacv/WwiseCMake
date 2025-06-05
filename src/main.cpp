#include "core/game.h"

int main(int argc, char* argv[])
{
	Game game;
	game.Initialize();
	game.Run();
	game.Terminate();

	return 0;
}
