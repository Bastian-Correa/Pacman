#include "InkyController.h"


InkyController::InkyController(std::shared_ptr<Character> character):
	Controller(character){
}

InkyController::~InkyController() {

}

Move
InkyController::getMove(const GameState& game){
	return PASS;	

	
}
