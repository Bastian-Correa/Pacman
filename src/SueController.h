#pragma once

#include "Controller.h"
#include "FSMController.h" 

// ^ Comportamiento de Sue FSM
class SueChaseState : public FSMState {
	std::pair<int,int> esquina;
public:
	SueChaseState(std::shared_ptr<Character> _character, std::pair<int,int> _esquina);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	~SueChaseState();
};

class SueNonfrightenedState : public FSMState {
	std::shared_ptr<FSMState> sueChaseState;
	std::shared_ptr<FSMState> scatterState;
	std::shared_ptr<FSMState> activeSubState;
	std::shared_ptr<ChaseScatterTransition> swapTransition;
public:
	SueNonfrightenedState(std::shared_ptr<Character> _character, std::pair<int,int> scatterTarget);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	void onExit(const GameState& gs) override;
	~SueNonfrightenedState();
};

class SueStateMachine : public FiniteStateMachine {
public:
	SueStateMachine(std::shared_ptr<Character> _character, std::pair<int,int> scatterCorner);
	Move update(const GameState& gs) override;
	~SueStateMachine();
};

class SueController: public Controller {
	std::shared_ptr<SueStateMachine> fsm;
public:
	SueController(std::shared_ptr<Character> character, std::pair<int,int> scatterCorner = std::make_pair(-1000,1000));
	virtual ~SueController();
	virtual Move getMove(const GameState& game)override;
};