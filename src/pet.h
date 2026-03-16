#ifndef __PET_AGENT__H
#define __PET_AGENT__H

#include "enviro.h"
#include <chrono>
#include <random>
#include <vector>

using namespace enviro;

class PetController : public Process, public AgentInterface {

    public:
    PetController() : Process(), AgentInterface() {}

    void init();
    void start();
    void update();
    void stop();

    // Behavior state management methods
    void begin_escape(double turn);
    void choose_new_wander_turn();

    private:
    // Movement parameters
    double forward_speed = 40.0;
    double turn_rate = 0.0;

    // State flags
    bool escaping = false;

    // Timing for behavior transitions
    std::chrono::steady_clock::time_point escape_until;
    std::chrono::steady_clock::time_point next_wander_change;

    // Random number generation for wandering movement
    std::default_random_engine generator;
    std::uniform_real_distribution<double> wander_turn_dist{-0.6, 0.6};

    // Parameters for dust interaction
    std::vector<int> known_dust_ids; // List of dust IDs currently tracked in the world
    double dust_remove_radius = 40.0; // Distance at which pet "consumes" dust
};

class Pet : public Agent {
    public:
    Pet(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }

    private:
    PetController c;
};

DECLARE_INTERFACE(Pet)

#endif