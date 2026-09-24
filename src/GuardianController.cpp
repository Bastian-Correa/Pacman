#include "GuardianController.h"
#include "PinkyController.h" // ^ reutilizare Ambush (anticipación) (ahora es tryhard este fantasma)

GuardianController::GuardianController(std::shared_ptr<Character> character):
	Controller(character), root(std::make_shared<Selector>()){

	// ^ 1. Si esta comestible el huye
	auto frightenedFilter = std::make_shared<Filter>();
	frightenedFilter->addCondition(std::make_shared<Powerpill>());
	frightenedFilter->addAction(std::make_shared<Frightened>());
	root->addChild(frightenedFilter);

	// ^ 2. Doble Amenaza: defender la pildora de poder si tiene chance real
	auto defensaFilter = std::make_shared<Filter>();
	defensaFilter->addCondition(std::make_shared<PowerPillCloser>());
	defensaFilter->addAction(std::make_shared<DefendPowerPill>());
	root->addChild(defensaFilter);

	// ^ 3. Ventana de Scatter
	auto scatterFilter = std::make_shared<Filter>();
	scatterFilter->addCondition(std::make_shared<TimeOut>());
	scatterFilter->addAction(std::make_shared<Scatter>(std::make_pair(1000,1000)));
	root->addChild(scatterFilter);

	// ^ 4. Por defecto, persigue a Pacman
	root->addChild(std::make_shared<Ambush>());
}

GuardianController::~GuardianController(){
}

Move GuardianController::getMove(const GameState& game){
	Info::getInfo()->in_character=character;
	Info::getInfo()->in_gamestate=&game;
	root->tick();
	return Info::getInfo()->out_move;
}

// * PowerPillCloser
Status PowerPillCloser::update(){
	auto personaje = Info::getInfo()->in_character;
	auto gs = Info::getInfo()->in_gamestate;
	const Maze& laberinto = gs->getMaze();

	int posPacman = gs->getPacmanPos();
	int nodoPildora = buscarNodoMasCercano(laberinto, posPacman,
		[&](int idx){ return laberinto.hasPowerPill(idx); });

	if(nodoPildora == -1){
		return BH_FAILURE;
	}

	int distPacmanPildora = distanciaEnSaltos(laberinto, posPacman, nodoPildora);
	int distGuardianPildora = distanciaEnSaltos(laberinto, personaje->getPos(), nodoPildora);

	if(distGuardianPildora <= distPacmanPildora){
		return BH_SUCCESS;
	}
	return BH_FAILURE;
}

// * DefendPowerPill
Status DefendPowerPill::update(){
	auto personaje = Info::getInfo()->in_character;
	auto gs = Info::getInfo()->in_gamestate;
	const Maze& laberinto = gs->getMaze();

	int nodoPildora = buscarNodoMasCercano(laberinto, gs->getPacmanPos(),
		[&](int idx){ return laberinto.hasPowerPill(idx); });

	if(nodoPildora == -1){
		return BH_FAILURE;
	}
	auto objetivo = laberinto.getNodePos(nodoPildora);

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
		float distancia = euclid2(objetivo, laberinto.getNodePos(laberinto.getNeighbour(personaje->getPos(), movimiento)));
		if(distancia < mejorDistancia){
			mejorDistancia = distancia;
			mejorMovimiento = movimiento;
		}
	}
	Info::getInfo()->out_move = mejorMovimiento;
	return BH_SUCCESS;
}