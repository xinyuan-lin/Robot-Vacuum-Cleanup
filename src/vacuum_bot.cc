#include <iostream>
#include "vacuum_bot.h"
#include <cmath>
#include <string>
#include <chrono>

using namespace enviro;

// Clears local agent labels as UI is managed globally by the GameManager
void VacuumBotController::update_hud() {
    label("", 0, 0);
}

// Determines movement speed based on current penalty status
double VacuumBotController::current_speed() const {
    return slowed ? slowed_speed : normal_speed;
}

// Activates a movement speed penalty for a specified duration
void VacuumBotController::apply_slowdown(double duration_seconds) {
    slowed = true;
    slowed_until = std::chrono::steady_clock::now() + std::chrono::milliseconds((int)(duration_seconds * 1000));
}

// Resets input velocity to zero
void VacuumBotController::clear_input() { vx = 0; vy = 0; }

void VacuumBotController::init() {
    // Requirement: Set moment of inertia to infinity for platformer-style movement
    prevent_rotation();

    // Keydown event handler for WASD movement control
    watch("keydown", [&](Event& e) {
        std::string k = e.value()["key"].get<std::string>();
        if (k == "w" || k == "W") vy = -1;
        else if (k == "s" || k == "S") vy = 1;
        else if (k == "a" || k == "A") vx = -1;
        else if (k == "d" || k == "D") vx = 1;
    });

    // Keyup event handler to stop movement when keys are released
    watch("keyup", [&](Event& e) {
        std::string k = e.value()["key"].get<std::string>();
        if ((k == "w" || k == "W") && vy < 0) vy = 0;
        if ((k == "s" || k == "S") && vy > 0) vy = 0;
        if ((k == "a" || k == "A") && vx < 0) vx = 0;
        if ((k == "d" || k == "D") && vx > 0) vx = 0;
    });

    // Handle collisions with Dust agents
    notice_collisions_with("Dust", [&](Event& e) {
        int dust_id = e.value()["id"];
        if (agent_exists(dust_id)) {
            // Requirement: Agent removal handling
            remove_agent(dust_id);
            emit(Event("dust_collected", {{"id", dust_id}}));
        }
    });

    // Sync score and timer from GameManager events
    watch("score_updated", [&](Event& e) { score = e.value()["score"]; update_hud(); });
    watch("timer_updated", [&](Event& e) { time_left = e.value()["time_left"]; update_hud(); });

    // Handle end-game state changes
    watch("game_state_changed", [&](Event& e) {
        game_running = e.value()["running"];
        game_status = e.value()["status"].get<std::string>();
        if (!game_running) clear_input();
        update_hud();
    });

    // Handle game restart position reset
    watch("reset_positions", [&](Event& e) { clear_input(); teleport(start_x, start_y, 0); });

    // React to collisions with the Pet dynamic hazard
    watch("pet_collision", [&](Event& e) { if (game_running) apply_slowdown(2.0); });
}

void VacuumBotController::start() {
    // Reset agent state on round start
    slowed = false;
    clear_input();
    score = 0;
    time_left = 60;
    game_status = "";
    game_running = true;
    teleport(start_x, start_y, 0);
    update_hud();
}

void VacuumBotController::update() {
    // Stop movement if game is finished
    if (!game_running) { omni_track_velocity(0, 0); return; }

    // Check for penalty expiration
    if (slowed && std::chrono::steady_clock::now() >= slowed_until) slowed = false;

    // Calculate normalized omni-directional velocity
    double dx = vx, dy = vy;
    double magnitude = std::sqrt(dx * dx + dy * dy);
    if (magnitude > 0) {
        double speed = current_speed();
        dx = dx / magnitude * speed;
        dy = dy / magnitude * speed;
    }

    // Requirement: Execute omni-directional movement
    omni_track_velocity(dx, dy);
}

void VacuumBotController::stop() {}