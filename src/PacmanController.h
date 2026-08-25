#pragma once

#include "Controller.h"


class PacmanController: public Controller {

public:
	PacmanController(std::shared_ptr<Character> character);
	virtual ~PacmanController();
	virtual Move getMove(const GameState& game)override;
};

