#pragma once

#include "Controller.h"


class PinkyController: public Controller {

public:
	PinkyController(std::shared_ptr<Character> character);
	virtual ~PinkyController();
	virtual Move getMove(const GameState& game)override;
};

