#include "SueController.h"
#include <iostream>

//* SueChaseState 
SueChaseState::SueChaseState(std::shared_ptr<Character> _character, std::pair<int,int> _esquina)
	:FSMState(_character), esquina(_esquina){
}
void SueChaseState::onEnter(const GameState& ){
	std::cout << "Sue Chase (evalua distancia a Pacman)\n";
}
Move SueChaseState::onUpdate(const GameState& game){
	const Maze& laberinto = game.getMaze();
	const auto pacmanCoord = laberinto.getNodePos(game.getPacmanPos());
	const auto myPos = character->getPos();

	// ^ Distancia real en casillas pues hacemos un DFS para contar el numero de nodos entre Pacman y Sue
	int distanciaEnCasillas = distanciaEnSaltos(laberinto, game.getPacmanPos(), myPos);

	auto objetivo = (distanciaEnCasillas >= 8) ? pacmanCoord : esquina;

	std::vector<Move> moves;
	if(character->getDirection()==PASS){
		moves = laberinto.getPossibleMoves(myPos);
	} else {
		moves = laberinto.getGhostLegalMoves(myPos, character->getDirection());
	}

	float min = euclid2(laberinto.getNodePos(laberinto.getNeighbour(myPos,moves[0])), objetivo);
	int minI = 0;
	for(unsigned int i=1; i<moves.size(); i++){
		auto dist = euclid2(laberinto.getNodePos(laberinto.getNeighbour(myPos,moves[i])), objetivo);
		if(dist<min){
			min=dist;
			minI=i;
		}
	}
	return moves[minI];
}
SueChaseState::~SueChaseState(){
}

//* SueNonfrightenedState 
SueNonfrightenedState::SueNonfrightenedState(std::shared_ptr<Character> _character, std::pair<int,int> scatterTarget)
	:FSMState(_character){
	sueChaseState = std::make_shared<SueChaseState>(character, scatterTarget);
	scatterState = std::make_shared<ScatterState>(character, scatterTarget);

	swapTransition = std::make_shared<ChaseScatterTransition>(sueChaseState, scatterState, true);
	sueChaseState->addTransition(swapTransition);
	scatterState->addTransition(swapTransition);

	activeSubState = scatterState;
}
void SueNonfrightenedState::onEnter(const GameState& gs){
	std::dynamic_pointer_cast<Ghost>(character)->revert();
	std::cout << "Sue Nonfrightened (estado normal)\n";
	swapTransition->resume();
	activeSubState->onEnter(gs);
}
Move SueNonfrightenedState::onUpdate(const GameState& gs){
	auto t = activeSubState->getActiveTransition(gs);
	if(t!=nullptr){
		activeSubState->onExit(gs);
		t->onTransition(gs);
		activeSubState = t->getNextState();
		activeSubState->onEnter(gs);
	}
	return activeSubState->onUpdate(gs);
}
void SueNonfrightenedState::onExit(const GameState& gs){
	swapTransition->pause();
	activeSubState->onExit(gs);
}
SueNonfrightenedState::~SueNonfrightenedState(){
}

//* SueStateMachine
SueStateMachine::SueStateMachine(std::shared_ptr<Character> _character, std::pair<int,int> scatterCorner)
	:FiniteStateMachine(_character){
	auto nonfrightened = std::make_shared<SueNonfrightenedState>(character, scatterCorner);
	auto frightened = std::make_shared<FrightenedState>(character);

	nonfrightened->addTransition(std::make_shared<PowerPillTransition>(frightened));
	frightened->addTransition(std::make_shared<UnfrightenTransition>(character, nonfrightened));

	states.push_back(nonfrightened);
	states.push_back(frightened);

	initialState = nonfrightened;
	activeState = initialState;
}
Move SueStateMachine::update(const GameState& gs){
	auto t = activeState->getActiveTransition(gs);
	if(t!=nullptr){
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}
SueStateMachine::~SueStateMachine(){
}

//* SueController
SueController::SueController(std::shared_ptr<Character> character, std::pair<int,int> scatterCorner)
	:Controller(character), fsm(std::make_shared<SueStateMachine>(character, scatterCorner)){
}
SueController::~SueController(){
}
Move SueController::getMove(const GameState& game){
	return fsm->update(game);
}