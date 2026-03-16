#ifndef __VACUUM_BOT_AGENT__H
#define __VACUUM_BOT_AGENT__H

#include "enviro.h"
#include <chrono>
#include <string>

using namespace enviro;

class VacuumBotController : public Process, public AgentInterface {

    public:
        VacuumBotController() : Process(), AgentInterface() {}

        void init();
        void start();
        void update();
        void stop();

    private:
        // Movement velocity components
        double vx = 0;
        double vy = 0;

        // Speed configurations for normal and penalty states
        double normal_speed = 80.0;
        double slowed_speed = 35.0;

        // Penalty state tracking
        bool slowed = false;
        std::chrono::steady_clock::time_point slowed_until;

        // Game state variables synchronized via events
        int score = 0;
        int time_left = 60;
        std::string game_status = "";
        bool game_running = true;

        // Initial coordinates for teleportation/reset
        double start_x = 0;
        double start_y = 0;

        // Internal helper methods for movement and UI
        double current_speed() const;
        void apply_slowdown(double duration_seconds);
        void clear_input();
        void update_hud();
};

class VacuumBot : public Agent {
    public:
        VacuumBot(json spec, World& world) : Agent(spec, world) {
            add_process(c);
        }

    private:
        VacuumBotController c;
};

DECLARE_INTERFACE(VacuumBot)

#endif