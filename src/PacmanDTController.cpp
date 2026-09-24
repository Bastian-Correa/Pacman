#include "PacmanDTController.h"
#include <cstdlib>
#include <SDL2/SDL.h>

PacmanDTController::PacmanDTController(std::shared_ptr<Character> _character) : Controller(_character) 
{
}

PacmanDTController::~PacmanDTController() 
{
}

// ^ Se encarga de movimiento de Pacman 
Move PacmanDTController::getMove(const GameState& game) {
    const Maze& laberinto = game.getMaze();
    int posPacman = game.getPacmanPos();

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

    // ^ Detección de estancamiento
    int scoreActual = game.getScore();
    if(scoreActual == ultimoScoreVisto){
        ticksSinProgreso++;
    } else {
        ticksSinProgreso = 0;
        ultimoScoreVisto = scoreActual;
    }
    
    // ^ ~3 segundos reales sin progreso (150 ticks * 20ms/tick) -> modo arriesgado
    bool estancado = ticksSinProgreso > 150;

    // ^ 1. Separar amenazas 
    const int UMBRAL_PELIGRO = estancado ? 3 : 9;   // ^ baja a 3 si llevamos estancados
    const int UMBRAL_CAZA = 10;

    std::vector<std::pair<int,int>> coordsAmenazas; // ^ todas las amenazas dentro del umbral
    int distComestible = 1000000, posComestible = -1;

    for (int i = 0; i < 4; i++) {
        int posFantasma = game.getGhostsPos(i);
        int dist = distanciaEnSaltos(laberinto, posPacman, posFantasma);
        if (game.isGhostEdible(i)) {
            if (dist < distComestible) { distComestible = dist; posComestible = posFantasma; }
        } else if (dist <= UMBRAL_PELIGRO) {
            coordsAmenazas.push_back(laberinto.getNodePos(posFantasma));
        }
    }

    Move dirActual = character->getDirection();
    std::vector<Move> movimientosPosibles = laberinto.getPossibleMoves(posPacman);

    // ^ 2. Si hay AL MENOS UNA amenaza cerca, huir considerando a todas las amenazas
    if (!coordsAmenazas.empty()) {
        Move mejorMovimiento = dirActual;
        int mejorPeorDistancia = -1;

        // ^ Aqui se busca el movimiento que maximice la distancia mínima a la amenaza más cercana
        for (Move m : movimientosPosibles) {
            int vecino = laberinto.getNeighbour(posPacman, m);
            if (vecino < 0) continue;
            auto coordsVecino = laberinto.getNodePos(vecino);

            int distMinEnEsteMovimiento = 1000000000;
            for (auto& coordsAmenaza : coordsAmenazas) {
                int dx = coordsVecino.first - coordsAmenaza.first;
                int dy = coordsVecino.second - coordsAmenaza.second;
                int dist = dx*dx + dy*dy;
                if (dist < distMinEnEsteMovimiento) distMinEnEsteMovimiento = dist;
            }

            if (distMinEnEsteMovimiento > mejorPeorDistancia || 
               (distMinEnEsteMovimiento == mejorPeorDistancia && rand() % 2 == 0)) {
                mejorPeorDistancia = distMinEnEsteMovimiento;
                mejorMovimiento = m;
            }
        }
        return mejorMovimiento;
    }

    // ^ 3. Sin peligro inmediato: decidir objetivo (comestible cercano, o pildora)
    std::pair<int,int> coordsObjetivo;

    if (posComestible != -1 && distComestible <= UMBRAL_CAZA) {
        coordsObjetivo = laberinto.getNodePos(posComestible);
    } else {
        int nodoObjetivo = buscarNodoMasCercano(laberinto, posPacman,
            [&](int idx){ return laberinto.hasPowerPill(idx); });

        if (nodoObjetivo == -1) {
            nodoObjetivo = buscarNodoMasCercano(laberinto, posPacman,
                [&](int idx){ return laberinto.hasPill(idx); });
        }

        if (nodoObjetivo == -1) {
            return character->getDirection();
        }
        coordsObjetivo = laberinto.getNodePos(nodoObjetivo);
    }

    // ^ 4. Perseguir el objetivo (comestible o pildora) busca la menor distancia
    Move mejorMovimiento = dirActual;
    int mejorDistancia = 10000000;

    for (Move m : movimientosPosibles) {
        bool esOpuesto = (m == UP && dirActual == DOWN) || 
                         (m == DOWN && dirActual == UP) || 
                         (m == LEFT && dirActual == RIGHT) || 
                         (m == RIGHT && dirActual == LEFT);
        if (esOpuesto && movimientosPosibles.size() > 1) continue;

        int vecino = laberinto.getNeighbour(posPacman, m);
        if (vecino < 0) continue;

        auto coordsVecino = laberinto.getNodePos(vecino);
        int dx = coordsVecino.first - coordsObjetivo.first;
        int dy = coordsVecino.second - coordsObjetivo.second;
        int distCuadrada = dx * dx + dy * dy;

        if (distCuadrada < mejorDistancia) {
            mejorDistancia = distCuadrada;
            mejorMovimiento = m;
        } 
        else if (distCuadrada == mejorDistancia && rand() % 2 == 0) {
            mejorMovimiento = m;
        }
    }

    return mejorMovimiento;
}