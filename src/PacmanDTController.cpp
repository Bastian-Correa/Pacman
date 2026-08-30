#include "PacmanDTController.h"
#include <cstdlib>
#include <SDL2/SDL.h>
// * Probando, probando, githud cargo correctamente al fin??
PacmanDTController::PacmanDTController(std::shared_ptr<Character> _character) : Controller(_character) 
{
}

PacmanDTController::~PacmanDTController() 
{
}

// ^ Se encarga de movimiento de Pacman 
Move PacmanDTController::getMove(const GameState& game) {
    const Maze& make = game.getMaze();
    int posPacman = game.getPacmanPos();
    auto coordsPacman = make.getNodePos(posPacman);

    int posFantasmaCercano = -1;
    bool esComestible = false;
    int distMinima = 10000000;

    // ^ Buscar el fantasma más cercano a Pacman
    for (int i = 0; i < 4; i++) {
        int posFantasma = game.getGhostsPos(i);
        auto coordsFantasma = make.getNodePos(posFantasma);

        int dx = coordsFantasma.first - coordsPacman.first; // * Horizontal 
        int dy = coordsFantasma.second - coordsPacman.second; // * Vertical 
        int distCuadrada = dx * dx + dy * dy;

        if (distCuadrada < distMinima) { 
            distMinima = distCuadrada;
            posFantasmaCercano = posFantasma;
            esComestible = game.isGhostEdible(i);
        }
    }

    // ! Para cerrar la ventana
    SDL_Event e;
    if( SDL_PollEvent( &e ) != 0 )
    {
        if( e.type == SDL_QUIT || 
            (e.type == SDL_KEYDOWN && 
                (e.key.keysym.sym==SDLK_ESCAPE || 
                e.key.keysym.sym==SDLK_q) ))
        {
            SDL_Quit();
            exit(0);
        }
    }

    // ^ Si no hay fantasma detectado, mantener dirección actual
    if (posFantasmaCercano == -1) {
        return character->getDirection();
    }

    // ^ Evaluar nodos vecinos según la decisión (perseguir o huir)
    Move mejorMovimiento = character->getDirection();
    int mejorDistancia = esComestible ? 10000000 : -1;
    auto coordsObjetivo = make.getNodePos(posFantasmaCercano);

    std::vector<Move> movimientosPosibles = make.getPossibleMoves(posPacman);
    
    // ^ Guardar dirección del Pacman para saber de donde venia 
    Move dirActual = character->getDirection();

    for (Move m : movimientosPosibles) {
        // ^ Verificar si el movimiento se esta analizando los giros de 180 grados
        bool esOpuesto = (m == UP && dirActual == DOWN) || 
                         (m == DOWN && dirActual == UP) || 
                         (m == LEFT && dirActual == RIGHT) || 
                         (m == RIGHT && dirActual == LEFT);

        // ^ Ignorar si tenemos más de una salida y asegurar que Pacman solo se devuelva cuando se siente sin salida
        if (esOpuesto && movimientosPosibles.size() > 1) {
            continue;
        }

        int vecino = make.getNeighbour(posPacman, m);
        if (vecino < 0) continue;

        auto coordsVecino = make.getNodePos(vecino);
        int dx = coordsVecino.first - coordsObjetivo.first;
        int dy = coordsVecino.second - coordsObjetivo.second;
        int distCuadrada = dx * dx + dy * dy;

        if (esComestible) {
            // ^ Perseguir: Este busca la menor distancia
            if (distCuadrada < mejorDistancia) {
                mejorDistancia = distCuadrada;
                mejorMovimiento = m;
            } 

            // ^ Al final volvi a poner el tema de desempate aleatorio para tener una seguridad que no se congele entre dos opciones iguales
            else if (distCuadrada == mejorDistancia && rand() % 2 == 0) {
                mejorMovimiento = m;
            }
        } else {
            // ^ Huye: Busca la mayor distancia
            if (distCuadrada > mejorDistancia) {
                mejorDistancia = distCuadrada;
                mejorMovimiento = m;
            } 
            // ^ Desempate aleatorio también para la huida
            else if (distCuadrada == mejorDistancia && rand() % 2 == 0) {
                mejorMovimiento = m;
            }
        }
    }

    return mejorMovimiento;
}