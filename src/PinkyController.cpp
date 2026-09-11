#include "PinkyController.h"

PinkyController::PinkyController(std::shared_ptr<Character> character):
	Controller(character), root(std::make_shared<Selector>()){

	// ^ Si esta comestible (Pacman comio Super pastilla) debemos huir
	auto frightenedFilter = std::make_shared<Filter>();
	frightenedFilter->addCondition(std::make_shared<Powerpill>());
	frightenedFilter->addAction(std::make_shared<Frightened>());
	root->addChild(frightenedFilter);

	// ^ Si estamos dentro de la ventana de tiempo de Scatter (7 de cada 27 seg) vamos a ir a esquina
	// ^ Si estamos fuera de la ventana de tiempo de Scatter, emboscamos a Pacman
	auto scatterFilter = std::make_shared<Filter>();
	scatterFilter->addCondition(std::make_shared<TimeOut>());
	scatterFilter->addAction(std::make_shared<Scatter>(std::make_pair(-1000,-1000)));
	root->addChild(scatterFilter);

	// ^ Si ninguna de las anteriores aplica vamos a emboscar a Pacman 
	root->addChild(std::make_shared<Ambush>());
}

PinkyController::~PinkyController() { 

}

Move // ^ Se encarga de actualizar la entrada de la informacion  
PinkyController::getMove(const GameState& game){
	Info::getInfo()->in_character=character; 
	Info::getInfo()->in_gamestate=&game;
	root->tick(); // * El encargado de hacer desde raiz a sus hijos y el tick() se encarga de hacer llamado en update en cada nodo 

	return Info::getInfo()->out_move;
} 

Status Ambush::update(){
	auto personaje = Info::getInfo()->in_character;   
	auto gs = Info::getInfo()->in_gamestate;
	const Maze& laberinto = gs->getMaze();

	int posPacman = gs->getPacmanPos();
	Move direccionPacman = static_cast<Move>(gs->getPacmanDir());

	// ^ Avanzamos 4 nodos en la direccion actual de Pacman para hallar el punto de emboscada
	int posObjetivo = posPacman;
	if(direccionPacman != PASS){
		for(int i=0; i<4; i++){
			int siguienteNodo = laberinto.getNeighbour(posObjetivo, direccionPacman);
			if(siguienteNodo < 0) break; // ! Si choco con pared, nos quedamos en el ultimo nodo valido
			posObjetivo = siguienteNodo; 
		}
	}
	auto coordsObjetivo = laberinto.getNodePos(posObjetivo);

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
