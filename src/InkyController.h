#pragma once

#include "Controller.h"


class InkyController: public Controller {

public:
	InkyController(std::shared_ptr<Character> character);
	virtual ~InkyController();
	virtual Move getMove(const GameState& game)override;
};

