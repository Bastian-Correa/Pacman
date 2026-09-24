#include "PinzaGhostController.h"
#include <iostream>

//* PinzaState
PinzaState::PinzaState(std::shared_ptr<Character> _character):FSMState(_character){
}
void PinzaState::onEnter(const GameState& ){
	std::cout << "Fantasma Pinza: Chase (evalua posicion de Clyde)\n";
}
Move PinzaState::onUpdate(const GameState& game){
	const Maze& laberinto = game.getMaze();
	const auto pacmanCoord = laberinto.getNodePos(game.getPacmanPos());
	const auto myPos = character->getPos();

	int posClyde = game.getGhostsPos(3);
	int distanciaClyde = distanciaEnSaltos(laberinto, game.getPacmanPos(), posClyde);

	std::pair<int,int> objetivo;
	if(distanciaClyde <= 8){
		// ^ Modo pinza: punto reflejado de Clyde respecto a Pacman
		auto coordsClyde = laberinto.getNodePos(posClyde);
		objetivo = std::make_pair(2*pacmanCoord.first - coordsClyde.first,
		                            2*pacmanCoord.second - coordsClyde.second);
	} else {
		// ^ Fuera de modo pinza: persigue la posicion anticipada de Pacman (3 casillas adelante)
		int posPacman = game.getPacmanPos();
		Move direccionPacman = static_cast<Move>(game.getPacmanDir());
		int posAnticipado = posPacman;
		if(direccionPacman != PASS){
			for(int i=0; i<3; i++){
				int siguienteNodo = laberinto.getNeighbour(posAnticipado, direccionPacman);
				if(siguienteNodo < 0) break;
				posAnticipado = siguienteNodo;
			}
		}
		objetivo = laberinto.getNodePos(posAnticipado);
	}
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
PinzaState::~PinzaState(){
}

//* PinzaNonfrightenedState 
PinzaNonfrightenedState::PinzaNonfrightenedState(std::shared_ptr<Character> _character, std::pair<int,int> scatterTarget)
	:FSMState(_character){
	chaseState = std::make_shared<PinzaState>(character);
	scatterState = std::make_shared<ScatterState>(character, scatterTarget);

	swapTransition = std::make_shared<ChaseScatterTransition>(chaseState, scatterState, true);
	chaseState->addTransition(swapTransition);
	scatterState->addTransition(swapTransition);

	activeSubState = scatterState;
}
void PinzaNonfrightenedState::onEnter(const GameState& gs){
	std::dynamic_pointer_cast<Ghost>(character)->revert();
	std::cout << "Fantasma Pinza: Nonfrightened (estado normal)\n";
	swapTransition->resume();
	activeSubState->onEnter(gs);
}
Move PinzaNonfrightenedState::onUpdate(const GameState& gs){
	auto t = activeSubState->getActiveTransition(gs);
	if(t!=nullptr){
		activeSubState->onExit(gs);
		t->onTransition(gs);
		activeSubState = t->getNextState();
		activeSubState->onEnter(gs);
	}
	return activeSubState->onUpdate(gs);
}
void PinzaNonfrightenedState::onExit(const GameState& gs){
	swapTransition->pause();
	activeSubState->onExit(gs);
}
PinzaNonfrightenedState::~PinzaNonfrightenedState(){
}

//* PinzaStateMachine 
PinzaStateMachine::PinzaStateMachine(std::shared_ptr<Character> _character, std::pair<int,int> scatterCorner)
	:FiniteStateMachine(_character){
	auto nonfrightened = std::make_shared<PinzaNonfrightenedState>(character, scatterCorner);
	auto frightened = std::make_shared<FrightenedState>(character);

	nonfrightened->addTransition(std::make_shared<PowerPillTransition>(frightened));
	frightened->addTransition(std::make_shared<UnfrightenTransition>(character, nonfrightened));

	states.push_back(nonfrightened);
	states.push_back(frightened);

	initialState = nonfrightened;
	activeState = initialState;
}
Move PinzaStateMachine::update(const GameState& gs){
	auto t = activeState->getActiveTransition(gs);
	if(t!=nullptr){
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}
PinzaStateMachine::~PinzaStateMachine(){
}

//* PinzaGhostController
PinzaGhostController::PinzaGhostController(std::shared_ptr<Character> character, std::pair<int,int> scatterCorner):
	Controller(character), fsm(std::make_shared<PinzaStateMachine>(character, scatterCorner)),
	tiempoInicio(std::chrono::system_clock::now()){
}
PinzaGhostController::~PinzaGhostController(){
}
Move PinzaGhostController::getMove(const GameState& game){
	// ^ Detecta cambio de nivel: si el numero de pildoras aumenta, reinicia el tiempo de spawn
	int pildorasAhora = (int)game.getMaze().getPillPositions().size();
	if(ultimoConteoPildoras != -1 && pildorasAhora > ultimoConteoPildoras){
		tiempoInicio = std::chrono::system_clock::now();
	}
	ultimoConteoPildoras = pildorasAhora;

	// ^ Se queda quieto en casa los primeros 5 segundos, luego empieza a moverse con la FSM
	auto transcurrido = std::chrono::system_clock::now() - tiempoInicio;
	if(transcurrido < std::chrono::seconds(5)){
		return PASS;
	}
	return fsm->update(game);
}