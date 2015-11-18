#include <iostream>
#include <array>

#include "glicko/rating.hpp"

int main(int argc, char** argv)
{
    // The test player. Example taken from
    // http://www.glicko.net/glicko/glicko2.pdf
    Glicko::Rating player(1500, 200);

    // The three opponents
    Glicko::Rating player1(1400, 30);
    Glicko::Rating player2(1550, 100);
    Glicko::Rating player3(1700, 300);

    // Simulate 3 games and update the ratings
    std::array< Glicko::Rating, 3 > players{ player1, player2, player3 };
    std::array< double, 3 > scores{1.0, 0.0, 0.0};
    player.Update(players, scores);
    player.Apply();

    std::cout << player << std::endl;

    return 0;
}
