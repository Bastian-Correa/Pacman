#include "BlinkyController.h"


BlinkyController::BlinkyController(std::shared_ptr<Character> character):
	Controller(character){
}

BlinkyController::~BlinkyController() {

}

Move
BlinkyController::getMove(const GameState& game){
	return PASS;	

	
}
