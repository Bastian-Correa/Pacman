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
#include "FSM.h"

class ExampleStateMachine;

class FSMController: public Controller {
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
	std::shared_ptr<ExampleStateMachine> fsm;
public:
	FSMController(std::shared_ptr<Character> character);
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
	ExampleStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~ExampleStateMachine();

};
#endif /* FSMCONTROLLER_H_ */
