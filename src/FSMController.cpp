/*
 * FSMController.cpp
 *
 *  Created on: Apr 23, 2018
 *      Author: nbarriga
 */

#include "FSMController.h"
#include <iostream>

// ^ Ahora se implementa la maquina de estados
FSMController::FSMController(std::shared_ptr<Character> character, std::pair<int,int> scatterCorner): 
	Controller(character),
	e(rand()),
	uniform_dist(0,3),
	fsm(std::make_shared<ExampleStateMachine>(character, scatterCorner)) {
}

FSMController::~FSMController() {
	// TODO Auto-generated destructor stub
}

Move 
FSMController::getMove(const GameState& game){
	return fsm->update(game);
}


///////////////////////////////////PillTransition///////////////////////////////
PillTransition::PillTransition(std::shared_ptr<FSMState> next):last(0),_next(next){

}

bool PillTransition::isValid(const GameState& gs){
	int quedan=gs.getMaze().getPillPositions().size();
	if(last!=quedan && quedan%20==0){
		last =quedan;
		return true;
	}
	return false;
}
std::shared_ptr<FSMState> PillTransition::getNextState(){
	return _next;
}



//////////////////////////////ChaseState///////////////////////////////////////
// ^ Este estado es el que hace que el fantasma persiga a Pacman

ChaseState::ChaseState(std::shared_ptr<Character> _character):FSMState(_character){

}
void ChaseState::onEnter(const GameState& ){
	std::cout << "Chase (persiguiendo a Pacman)\n";
	// ~ std::dynamic_pointer_cast<Ghost>(character)->revert();
}
Move ChaseState::onUpdate(const GameState& game){
	std::vector<Move> moves;
	const auto pacmanCoord=game.getMaze().getNodePos(game.getPacmanPos());
	const auto myPos=character->getPos();
	// ~ const auto myCoord=game.getMaze().getNodePos(myPos);

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	float min=euclid2(
		game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[0])),
			pacmanCoord);
	int minI=0;
	for(unsigned int i=1;i<moves.size();i++){
		auto dist=euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[i])),
			pacmanCoord);
		if(dist<min){
			min=dist;
			minI=i;
		}
	}
	return moves[minI];
}
ChaseState::~ChaseState(){

}


/////////////////////////////////////BlinkyStateMachine/////////////////////////////
// ! Maquina de estados (parace que el profe se olvido cambiarle el nombre a esta cosa)
ExampleStateMachine::ExampleStateMachine(std::shared_ptr<Character> _character, std::pair<int,int> scatterCorner):FiniteStateMachine(_character){
	auto nonfrightened = std::make_shared<NonfrightenedState>(character, scatterCorner);
	auto frightened = std::make_shared<FrightenedState>(character);

	nonfrightened->addTransition(std::make_shared<PowerPillTransition>(frightened));
	frightened->addTransition(std::make_shared<UnfrightenTransition>(character, nonfrightened));

	states.push_back(nonfrightened);
	states.push_back(frightened);

	initialState = nonfrightened;
	activeState = initialState;
}


Move ExampleStateMachine::update(const GameState& gs){
	auto t=activeState->getActiveTransition(gs);
	if(t!=nullptr){
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState=t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}


ExampleStateMachine::~ExampleStateMachine(){

}

// * ///////////////////////////////////ScatterState///////////////////////////////////
// ^ Este estado es el que hace que el fantasma vaya a la esquina de su mapa

ScatterState::ScatterState(std::shared_ptr<Character> _character, std::pair<int,int> _target)
	:FSMState(_character), target(_target){
}

void ScatterState::onEnter(const GameState& ){
	std::cout << "Scatter (yendo a la esquina)\n";
}

Move ScatterState::onUpdate(const GameState& game){
	std::vector<Move> moves;
	const auto myPos=character->getPos();

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	// ^ Igual que ChaseState, pero el ahora su objetivo es la esquina fija, no Pacman
	float min=euclid2(
		game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[0])),
			target);
	int minI=0;
	for(unsigned int i=1;i<moves.size();i++){
		auto dist=euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[i])),
			target);
		if(dist<min){
			min=dist;
			minI=i;
		}
	}
	return moves[minI];
}
ScatterState::~ScatterState(){
}


// * ///////////////////////////////////FrightenedState/////////////////////////////////
// ^ Este estado es el que hace que el fantasma huya de Pacman

FrightenedState::FrightenedState(std::shared_ptr<Character> _character):FSMState(_character){
}
void FrightenedState::onEnter(const GameState& ){ 
	std::cout << "Frightened (huyendo asustado)\n";
}
Move FrightenedState::onUpdate(const GameState& game){
	std::vector<Move> moves;
	const auto pacmanCoord=game.getMaze().getNodePos(game.getPacmanPos());
	const auto myPos=character->getPos();

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	// ^ Huir: mismo cálculo que Chase, pero busca la mayor distancia, no la menor
	float max=euclid2(
		game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[0])),
			pacmanCoord);
	int maxI=0;
	for(unsigned int i=1;i<moves.size();i++){
		auto dist=euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[i])),
			pacmanCoord);
		if(dist>max){
			max=dist;
			maxI=i;
		}
	}
	return moves[maxI];
}
FrightenedState::~FrightenedState(){
}


// * //////////////////////////////NonfrightenedState//////////////////////////////
// ^ Este estado es el que contiene a los otros dos estados (Chase y Scatter)

NonfrightenedState::NonfrightenedState(std::shared_ptr<Character> _character, std::pair<int,int> scatterTarget)
	:FSMState(_character){
	chaseState = std::make_shared<ChaseState>(character);
	scatterState = std::make_shared<ScatterState>(character, scatterTarget);

	// ^ Un solo objeto de transición compartido entre ambos sub estados
	swapTransition = std::make_shared<ChaseScatterTransition>(chaseState, scatterState, true);
	chaseState->addTransition(swapTransition);
	scatterState->addTransition(swapTransition);

	activeSubState = scatterState; // ^ Los fantasmas parten en Scatter
}
void NonfrightenedState::onEnter(const GameState& gs){
	// ^ Nos aseguramos de que ya no sea comestible al volver de Frightened
	std::dynamic_pointer_cast<Ghost>(character)->revert();
	// ^ Retomamos el sub estado en el que estábamos "antes" de Frightened
	std::cout << "Nonfrightened (estado normal)\n";
	swapTransition->resume(); 
	activeSubState->onEnter(gs);
}
Move NonfrightenedState::onUpdate(const GameState& gs){
	auto t=activeSubState->getActiveTransition(gs);
	if(t!=nullptr){
		activeSubState->onExit(gs);
		t->onTransition(gs);
		activeSubState=t->getNextState();
		activeSubState->onEnter(gs);
	}
	return activeSubState->onUpdate(gs);
}
void NonfrightenedState::onExit(const GameState& gs){
	swapTransition->pause(); // ^ congela el cronometro antes de irse a Frightened
	activeSubState->onExit(gs);
}
NonfrightenedState::~NonfrightenedState(){
}


// * ///////////////////////////////////ChaseScatterTransition///////////////////////////////////
// ^ 7 segundos de Scatter, 20 segundos de Chase.
// mientras el fantasma esta en Frightened (ver pause()/resume() abajo).
ChaseScatterTransition::ChaseScatterTransition(std::shared_ptr<FSMState> _chase, std::shared_ptr<FSMState> _scatter, bool startsAtScatter)
	:atChase(!startsAtScatter), running(true), lastResume(std::chrono::system_clock::now()),
	 accumulated(std::chrono::system_clock::duration::zero()), chase(_chase), scatter(_scatter){
}
bool ChaseScatterTransition::isValid(const GameState&){
	if(!running) return false; // ^ pausado
	int segundosLimite = atChase ? 20 : 7; 
	auto transcurrido = accumulated + (std::chrono::system_clock::now() - lastResume);
	return transcurrido > std::chrono::seconds(segundosLimite);
}
std::shared_ptr<FSMState> ChaseScatterTransition::getNextState(){
	accumulated = std::chrono::system_clock::duration::zero(); // ^ reinicia el contador
	lastResume = std::chrono::system_clock::now();
	if(atChase){ 
		atChase=false; 
		return scatter; 
	}else{
		atChase=true;
		return chase;
	}
}

void ChaseScatterTransition::pause(){
	if(running){
		accumulated += std::chrono::system_clock::now() - lastResume; // ^ guarda lo acumulado hasta ahora
		running = false;
	}
}
void ChaseScatterTransition::resume(){
	if(!running){
		lastResume = std::chrono::system_clock::now(); // ^ el reloj vuelve a correr desde ahora
		running = true;
	}
}
	

// * ///////////////////////////////////PowerPillTransition///////////////////////////////////
// 
PowerPillTransition::PowerPillTransition(std::shared_ptr<FSMState> next):last(-1),_next(next){
}
bool PowerPillTransition::isValid(const GameState& gs){
	int quedan=gs.getMaze().getPowerPillPositions().size();
	if(last==-1){ // ^ primera llamada: solo guarda la base, no dispara todavía
		last=quedan;
		return false;
	}
	if(quedan<last){ // ^ se comió una power pill recién
		last=quedan;
		return true;
	}
	last=quedan;
	return false;
}
std::shared_ptr<FSMState> PowerPillTransition::getNextState(){
	return _next;
}


// * ///////////////////////////////////UnfrightenTransition/////////////////////////////
UnfrightenTransition::UnfrightenTransition(std::shared_ptr<Character> _character, std::shared_ptr<FSMState> next)
	:character(_character), _next(next){
}
bool UnfrightenTransition::isValid(const GameState&){
	auto ghost=std::dynamic_pointer_cast<Ghost>(character);
	return ghost && !ghost->isEdible();
}
std::shared_ptr<FSMState> UnfrightenTransition::getNextState(){
	return _next;
}
