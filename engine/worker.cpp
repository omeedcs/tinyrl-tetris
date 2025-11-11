/* The idea here is that we want to be able to do the following:
 * fetch "job" from the working queue
 * these jobs will include preallocated buffers to do our transformation for
 * execute for a given amount of steps (or until done)
 * return batched data with states, rewards
 * */
#include <thread>
#include <vector>
#include <iostream>

#include "tetrisGame.h"

constexpr int NUM_WORKERS = 100;

int rl_loop(TetrisGame& game, int steps, int id) {
    // TODO: implement RL training loop; placeholder keeps compiler happy
    for (int i = 0; i < steps; i++) {
        game.step(0);
        //print pid and step count
        // std::cout << "PID: " << id << ", Step: " << i << std::endl;
    }
}

int main() {
    // let's test running 3 tetris env simulations in parallel
    std::vector<std::thread> threads;
    threads.reserve(NUM_WORKERS);

    std::vector<TetrisGame> games;
    games.reserve(NUM_WORKERS);
    for (int i = 0; i < NUM_WORKERS; ++i) {
        games.emplace_back(TimeManager::SIMULATION);
    }

    for (int i = 0; i < NUM_WORKERS; ++i) {
        threads.emplace_back([steps = 100, id = i, &games]() {
            rl_loop(games[id], steps, id);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // print the stats of each game
    for (int i = 0; i < NUM_WORKERS; ++i) {
        std::cout << "Game " << i << " score: " << games[i].score << std::endl;
    }
}
