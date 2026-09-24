#include "InkyController.h"

InkyController::InkyController(std::shared_ptr<Character> character):
	Controller(character), root(std::make_shared<Selector>()){

	// ^ 1. Si esta comestible huye
	auto frightenedFilter = std::make_shared<Filter>();
	frightenedFilter->addCondition(std::make_shared<Powerpill>());
	frightenedFilter->addAction(std::make_shared<Frightened>());
	root->addChild(frightenedFilter);

	// ^ 2. Ventana de Scatter (7 de cada 27 seg)  ir a su esquina (inferior derecha)
	auto scatterFilter = std::make_shared<Filter>();
	scatterFilter->addCondition(std::make_shared<TimeOut>());
	scatterFilter->addAction(std::make_shared<Scatter>(std::make_pair(1000,1000)));
	root->addChild(scatterFilter);

	// ^ 3. Si no, comportamiento clasico de peseguir a Pacman
	root->addChild(std::make_shared<InkyChase>());
}

InkyController::~InkyController() {
}

Move
InkyController::getMove(const GameState& game){
	Info::getInfo()->in_character=character;
	Info::getInfo()->in_gamestate=&game;
	root->tick();

	return Info::getInfo()->out_move;
}

Status InkyChase::update(){
	auto personaje = Info::getInfo()->in_character;
	auto gs = Info::getInfo()->in_gamestate;
	const Maze& laberinto = gs->getMaze();

	int posPacman = gs->getPacmanPos();
	Move direccionPacman = static_cast<Move>(gs->getPacmanDir());

	// ^ Punto 2 casillas adelante de Pacman
	int posIntermedio = posPacman;
	if(direccionPacman != PASS){
		for(int i=0; i<2; i++){
			int siguienteNodo = laberinto.getNeighbour(posIntermedio, direccionPacman);
			if(siguienteNodo < 0) break;
			posIntermedio = siguienteNodo;
		}
	}
	auto coordsIntermedio = laberinto.getNodePos(posIntermedio);

	// ^ Posición de Blinky 
	int posBlinky = gs->getGhostsPos(0);
	auto coordsBlinky = laberinto.getNodePos(posBlinky);

	// ^ Vector desde Blinky hasta el punto intermedio, lo duplico para haya objetivo
	int vx = coordsIntermedio.first - coordsBlinky.first;
	int vy = coordsIntermedio.second - coordsBlinky.second;
	auto coordsObjetivo = std::make_pair(coordsBlinky.first + 2*vx, coordsBlinky.second + 2*vy);

	Move mejorMovimiento = PASS;
	std::vector<Move> movimientosPosibles;
	if(personaje->getDirection()==PASS) {
		movimientosPosibles = laberinto.getPossibleMoves(personaje->getPos());
	} else {
		movimientosPosibles = laberinto.getGhostLegalMoves(personaje->getPos(), personaje->getDirection());
	}

	float mejorDistancia = 1000000000;
	for(auto movimiento: movimientosPosibles){
		if(movimiento==PASS) break;
		float distancia = euclid2(coordsObjetivo, laberinto.getNodePos(laberinto.getNeighbour(personaje->getPos(), movimiento)));
		if(distancia < mejorDistancia){
			mejorDistancia = distancia;
			mejorMovimiento = movimiento;
		}
	}
	Info::getInfo()->out_move = mejorMovimiento;
	return BH_SUCCESS;
}