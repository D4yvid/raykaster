#include "main.h"
#include "game.h"
#include "macros.h"
#include "renderer.h"
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	DEBUG("initializing renderer");
	renderer_init();

	DEBUG("initializing game");
	game_init();

	DEBUG("running game");
	game_run();

	DEBUG("disabling game");
	game_quit();

	DEBUG("disabling renderer subsystem");
	renderer_quit();

	DEBUG("exiting...");

	return 0;
}
