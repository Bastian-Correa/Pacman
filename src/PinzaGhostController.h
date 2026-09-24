#pragma once
#include "Controller.h"
#include "FSMController.h" 

// * Fantasma Pinza
class PinzaState : public FSMState {
public:
	PinzaState(std::shared_ptr<Character> _character);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	~PinzaState();
};

class PinzaNonfrightenedState : public FSMState {
	std::shared_ptr<FSMState> chaseState;
	std::shared_ptr<FSMState> scatterState;
	std::shared_ptr<FSMState> activeSubState;
	std::shared_ptr<ChaseScatterTransition> swapTransition;
public:
	PinzaNonfrightenedState(std::shared_ptr<Character> _character, std::pair<int,int> scatterTarget);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	void onExit(const GameState& gs) override;
	~PinzaNonfrightenedState();
};

class PinzaStateMachine : public FiniteStateMachine {
public:
	PinzaStateMachine(std::shared_ptr<Character> _character, std::pair<int,int> scatterCorner);
	Move update(const GameState& gs) override;
	~PinzaStateMachine();
};

class PinzaGhostController: public Controller {
	std::shared_ptr<PinzaStateMachine> fsm;
	std::chrono::system_clock::time_point tiempoInicio; // ^ para el retraso de spawn
	int ultimoConteoPildoras = -1; // ^ para detectar cambios de nivel (mapa nuevo)
public:
	PinzaGhostController(std::shared_ptr<Character> character, std::pair<int,int> scatterCorner = std::make_pair(-1000,1000));
	virtual ~PinzaGhostController();
	virtual Move getMove(const GameState& game)override;
};