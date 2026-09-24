#pragma once
#include "Controller.h"
#include "BehaviorTree.h"
#include "BTGhostController.h" 
#include "FSMController.h"     // ^ reutilizare distanciaEnSaltos, buscarNodoMasCercano

class PowerPillCloser : public Behavior{
public:
	virtual Status update() override;
};

class DefendPowerPill : public Behavior{
public:
	virtual Status update() override;
};

class GuardianController: public Controller {
private:
	std::shared_ptr<Composite> root;
public:
	GuardianController(std::shared_ptr<Character> character);
	virtual ~GuardianController();
	virtual Move getMove(const GameState& game)override;
};