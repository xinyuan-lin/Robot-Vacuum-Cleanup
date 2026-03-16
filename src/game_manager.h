#ifndef __GAME_MANAGER_AGENT__H
#define __GAME_MANAGER_AGENT__H

#include "enviro.h"
#include <random>
#include <chrono>
#include <vector>
#include <unordered_map>

using namespace enviro;

class GameManagerController : public Process, public AgentInterface {

    public:
    GameManagerController() : Process(), AgentInterface() {}

    void init();
    void start();
    void update();
    void stop();

    private:

    int score = 0;

    // Random number generation for spawning positions
    std::default_random_engine generator;
    std::uniform_real_distribution<double> x_dist{-280, 280};
    std::uniform_real_distribution<double> y_dist{-130, 130};

    // Timing and spawning variables
    double spawn_interval = 2.0;
    std::chrono::steady_clock::time_point last_spawn_time;
    std::chrono::steady_clock::time_point round_start_time;
    std::chrono::steady_clock::time_point last_timer_update;

    // Game rules and state - Unified target score to 50
    int round_duration = 60;
    int target_score = 50; 
    int current_time_left = 60;

    bool game_running = true;

    // List of active dust agent IDs currently in the world
    std::vector<int> dust_ids;

    // Delayed removal queue to prevent segmentation faults during physics updates
    std::vector<int> pending_remove_dust_ids;

    // Map to store point values associated with each dust ID
    std::unordered_map<int, int> dust_value_map;

    // Internal helper methods for game flow
    void spawn_dust();
    void reset_game();
    void clear_all_dust();
};

class GameManager : public Agent {
    public:
    GameManager(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }

    private:
    GameManagerController c;
};

DECLARE_INTERFACE(GameManager)

#endif