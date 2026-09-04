/*
 * FSMController.h
 *
 *  Created on: Apr 23, 2018
 *      Author: nbarriga
 */

#ifndef FSMCONTROLLER_H_
#define FSMCONTROLLER_H_

#include "Controller.h"
#include <random>
#include <chrono>
#include "FSM.h"

class ExampleStateMachine;
class ChaseScatterTransition;

class FSMController: public Controller {
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
	std::shared_ptr<ExampleStateMachine> fsm;
public:
	// ~ FSMController(std::shared_ptr<Character> character);
	FSMController(std::shared_ptr<Character> character, std::pair<int,int> scatterCorner = std::make_pair(-1000,-1000));
	virtual ~FSMController();
	virtual Move getMove(const GameState& game)override;
};

class PillTransition:public FSMTransition{
	int last;
	std::shared_ptr<FSMState> _next;
public:
	PillTransition(std::shared_ptr<FSMState> next);
	bool isValid(const GameState& gs)override;
	std::shared_ptr<FSMState> getNextState()override;
};

class ChaseState:public FSMState{

public:
	ChaseState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~ChaseState();

};

class ExampleStateMachine: public FiniteStateMachine{

public:
	// ~ ExampleStateMachine(std::shared_ptr<Character> _character);
	ExampleStateMachine(std::shared_ptr<Character> _character, std::pair<int,int> scatterCorner = std::make_pair(-1000,-1000));
	Move update(const GameState& gs) override;
	~ExampleStateMachine();

};

class ScatterState:public FSMState{
	std::pair<int,int> target;
public:
	ScatterState(std::shared_ptr<Character> _character, std::pair<int,int> _target);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	~ScatterState();
};

class FrightenedState:public FSMState{
public:
	FrightenedState(std::shared_ptr<Character> _character);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	~FrightenedState();
};

// ^ Estado compuestos (subestados): contiene su propia mini FSM interna (Chase <-> Scatter).
class NonfrightenedState:public FSMState{
	std::shared_ptr<FSMState> chaseState; 
	std::shared_ptr<FSMState> scatterState;
	std::shared_ptr<FSMState> activeSubState; // ^ recuerda en que sub estado ibamos
	std::shared_ptr<ChaseScatterTransition> swapTransition; 
public:
	NonfrightenedState(std::shared_ptr<Character> _character, std::pair<int,int> scatterTarget);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	void onExit(const GameState& gs) override;
	~NonfrightenedState();
};

// ^ Transición compartida entre Chase y Scatter, ahora si basada en tiempo 
class ChaseScatterTransition:public FSMTransition{
	bool atChase; 
	bool running; 
	std::chrono::system_clock::time_point lastResume;
	std::chrono::system_clock::duration accumulated; // ^ tiempo ya acumulado antes de la pausa actual
	std::shared_ptr<FSMState> chase;
	std::shared_ptr<FSMState> scatter;
public:
	ChaseScatterTransition(std::shared_ptr<FSMState> chase, std::shared_ptr<FSMState> scatter, bool startsAtScatter);
	bool isValid(const GameState& gs)override;
	std::shared_ptr<FSMState> getNextState()override;
	void pause();
	void resume();
};

// ^ Nonfrightened a Frightened: se dispara cuando Pacman come una power pill
class PowerPillTransition:public FSMTransition{
	int last;
	std::shared_ptr<FSMState> _next;
public:
	PowerPillTransition(std::shared_ptr<FSMState> next);
	bool isValid(const GameState& gs)override;
	std::shared_ptr<FSMState> getNextState()override;
};

// ^ Frightened a Nonfrightened
class UnfrightenTransition:public FSMTransition{
	std::shared_ptr<Character> character;
	std::shared_ptr<FSMState> _next;
public:
	UnfrightenTransition(std::shared_ptr<Character> character, std::shared_ptr<FSMState> next);
	bool isValid(const GameState& gs)override;
	std::shared_ptr<FSMState> getNextState()override;
};

#endif /* FSMCONTROLLER_H_ */
