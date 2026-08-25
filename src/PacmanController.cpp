#include "PacmanController.h"
#include <SDL2/SDL.h>

PacmanController::PacmanController(std::shared_ptr<Character> character):
	Controller(character){
}

PacmanController::~PacmanController() {

}




Move
PacmanController::getMove(const GameState& game){

	//para cerrar la ventana
	SDL_Event e;
	if( SDL_PollEvent( &e ) != 0 )
	{
		if( e.type == SDL_QUIT || 
			(e.type == SDL_KEYDOWN && 
				(e.key.keysym.sym==SDLK_ESCAPE || 
				e.key.keysym.sym==SDLK_q) ))
		{
			SDL_Quit();
			exit(0);
		}
	}
	
	return PASS;
	
}
