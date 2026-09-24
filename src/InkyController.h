#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include "BTGhostController.h" 

class InkyController: public Controller {
private:
    std::shared_ptr<Composite> root;
public:
	InkyController(std::shared_ptr<Character> character);
	virtual ~InkyController();
	virtual Move getMove(const GameState& game)override;
};


class InkyChase : public Behavior{
public:
    virtual Status update() override;
};