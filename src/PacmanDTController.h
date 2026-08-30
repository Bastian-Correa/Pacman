// ^ Esto me servira para prevenir doble llamado a nuestra clase
#ifndef PACMANDTCONTROLLER_H_
#define PACMANDTCONTROLLER_H_ 

#include "Controller.h"
#include <memory> 

class PacmanDTController : public Controller {
public:
    PacmanDTController(std::shared_ptr<Character> _character);  // ^ Vincular nuestro pacman con cerebro (character)
    virtual ~PacmanDTController(); 
    virtual Move getMove(const GameState& game) override; // ^ Llamar el motor de la IA y luego con el override llamos los frames para el tema de los movimientos para obtener la dirreción calculada de nuestro arbol de decisiones
};

// * me quedo largo el apunte. 

#endif 