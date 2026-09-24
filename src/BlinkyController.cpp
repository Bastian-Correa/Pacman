#include "BlinkyController.h"
#include <iostream>

// * ElroyTransition
ElroyTransition::ElroyTransition(std::shared_ptr<FSMState> chase, std::shared_ptr<FSMState> scatter, bool startsAtScatter, int _umbralPastillas)
	: ChaseScatterTransition(chase, scatter, startsAtScatter), umbralPastillas(_umbralPastillas){
}
bool ElroyTransition::isValid(const GameState& gs){
	int pildorasRestantes = gs.getMaze().getPillPositions().size();
	if(pildorasRestantes <= umbralPastillas && atChase){
		return false; // ^ Cruise Elroy activo: nunca deja pasar a Scatter
	}
	return ChaseScatterTransition::isValid(gs);
}

// * BlinkyNonfrightenedState
BlinkyNonfrightenedState::BlinkyNonfrightenedState(std::shared_ptr<Character> _character, std::pair<int,int> scatterTarget, int umbralPastillas)
	:FSMState(_character){
	chaseState = std::make_shared<ChaseState>(character); // ^ Chase generico, clasico
	scatterState = std::make_shared<ScatterState>(character, scatterTarget);

	swapTransition = std::make_shared<ElroyTransition>(chaseState, scatterState, true, umbralPastillas);
	chaseState->addTransition(swapTransition);
	scatterState->addTransition(swapTransition);

	activeSubState = scatterState;
}
void BlinkyNonfrightenedState::onEnter(const GameState& gs){
	std::dynamic_pointer_cast<Ghost>(character)->revert();
	std::cout << "Blinky Nonfrightened (estado normal)\n";
	swapTransition->resume();
	activeSubState->onEnter(gs);
}
Move BlinkyNonfrightenedState::onUpdate(const GameState& gs){
	auto t = activeSubState->getActiveTransition(gs);
	if(t!=nullptr){
		activeSubState->onExit(gs);
		t->onTransition(gs);
		activeSubState = t->getNextState();
		activeSubState->onEnter(gs);
	}
	return activeSubState->onUpdate(gs);
}
void BlinkyNonfrightenedState::onExit(const GameState& gs){
	swapTransition->pause();
	activeSubState->onExit(gs);
}
BlinkyNonfrightenedState::~BlinkyNonfrightenedState(){
}

// *BlinkyStateMachine
BlinkyStateMachine::BlinkyStateMachine(std::shared_ptr<Character> _character, std::pair<int,int> scatterCorner, int umbralPastillas)
	:FiniteStateMachine(_character){
	auto nonfrightened = std::make_shared<BlinkyNonfrightenedState>(character, scatterCorner, umbralPastillas);
	auto frightened = std::make_shared<FrightenedState>(character);

	nonfrightened->addTransition(std::make_shared<PowerPillTransition>(frightened));
	frightened->addTransition(std::make_shared<UnfrightenTransition>(character, nonfrightened));

	states.push_back(nonfrightened);
	states.push_back(frightened);

	initialState = nonfrightened;
	activeState = initialState;
}
Move BlinkyStateMachine::update(const GameState& gs){
	auto t = activeState->getActiveTransition(gs);
	if(t!=nullptr){
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}
BlinkyStateMachine::~BlinkyStateMachine(){
}

// * BlinkyController
BlinkyController::BlinkyController(std::shared_ptr<Character> character, std::pair<int,int> scatterCorner, int umbralPastillas):
	Controller(character), fsm(std::make_shared<BlinkyStateMachine>(character, scatterCorner, umbralPastillas)){
}
BlinkyController::~BlinkyController() {
}
Move BlinkyController::getMove(const GameState& game){
	return fsm->update(game);
}