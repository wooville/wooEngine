#include <iostream>
#include "./Game/Game.h"

int main(int argc, char* argv[]) {
    Game game;

    game.Initialize();
    game.Run();         //loop run until game is over
    game.Destroy();

    return 0;
}
