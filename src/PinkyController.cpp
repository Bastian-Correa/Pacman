#include "PinkyController.h"


PinkyController::PinkyController(std::shared_ptr<Character> character):
	Controller(character){
}

PinkyController::~PinkyController() {

}

Move
PinkyController::getMove(const GameState& game){
	return PASS;	

	
}
