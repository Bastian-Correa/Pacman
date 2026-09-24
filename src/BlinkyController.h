#pragma once

#include "Controller.h"
#include "FSMController.h"

// ^ Cruise Elroy (comportamiento clasico de Blinky)
class ElroyTransition : public ChaseScatterTransition {
	int umbralPastillas;
public:
	ElroyTransition(std::shared_ptr<FSMState> chase, std::shared_ptr<FSMState> scatter, bool startsAtScatter, int _umbralPastillas);
	bool isValid(const GameState& gs) override;
};

class BlinkyNonfrightenedState : public FSMState {
	std::shared_ptr<FSMState> chaseState;
	std::shared_ptr<FSMState> scatterState;
	std::shared_ptr<FSMState> activeSubState;
	std::shared_ptr<ElroyTransition> swapTransition;
public:
	BlinkyNonfrightenedState(std::shared_ptr<Character> _character, std::pair<int,int> scatterTarget, int umbralPastillas);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	void onExit(const GameState& gs) override;
	~BlinkyNonfrightenedState();
};

class BlinkyStateMachine : public FiniteStateMachine {
public:
	BlinkyStateMachine(std::shared_ptr<Character> _character, std::pair<int,int> scatterCorner, int umbralPastillas);
	Move update(const GameState& gs) override;
	~BlinkyStateMachine();
};

class BlinkyController: public Controller {
	std::shared_ptr<BlinkyStateMachine> fsm;
public:
	BlinkyController(std::shared_ptr<Character> character, std::pair<int,int> scatterCorner = std::make_pair(1000,-1000), int umbralPastillas = 30);
	virtual ~BlinkyController();
	virtual Move getMove(const GameState& game)override;
};