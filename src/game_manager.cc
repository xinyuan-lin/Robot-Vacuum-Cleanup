#include <iostream>
#include "game_manager.h"
#include <string>
#include <chrono>
#include <algorithm>

using namespace enviro;

// Spawns a new dust agent at a random valid location
void GameManagerController::spawn_dust() {
    if (!game_running) return;

    for (int tries = 0; tries < 50; tries++) {
        double x = x_dist(generator);
        double y = y_dist(generator);

        // Check if position is inside furniture bounding boxes
        bool in_sofa = (x >= -220 && x <= -120 && y >= 80 && y <= 140);
        bool in_table = (x >= 60 && x <= 170 && y >= -20 && y <= 40);
        bool in_cabinet = (x >= -40 && x <= 40 && y >= -140 && y <= -70);

        if (!in_sofa && !in_table && !in_cabinet) {
            // Determine dust tier and point value based on probability
            double p = std::uniform_real_distribution<double>(0, 1)(generator);
            int value = 1;
            std::string color = "gold";

            if (p > 0.95) { value = 5; color = "red"; }
            else if (p > 0.80) { value = 3; color = "blue"; }

            // Add the agent to the simulation
            Agent& dust = add_agent("Dust", x, y, 0, {
                {"fill", color},
                {"stroke", "black"}
            });

            int id = dust.get_id();
            dust_ids.push_back(id);
            dust_value_map[id] = value;
            return;
        }
    }
}

// Clears all existing dust agents from the world
void GameManagerController::clear_all_dust() {
    for (int id : dust_ids) {
        if (agent_exists(id)) remove_agent(id);
    }
    dust_ids.clear();
    dust_value_map.clear();
}

// Resets game state, score, timer, and spawns initial dust batch
void GameManagerController::reset_game() {
    clear_all_dust();

    score = 0;
    current_time_left = round_duration;
    game_running = true;

    auto now = std::chrono::steady_clock::now();
    round_start_time = now;
    last_spawn_time = now;
    last_timer_update = now;

    // Sync state with other agents via events
    emit(Event("score_updated", {{"score", score}}));
    emit(Event("timer_updated", {{"time_left", current_time_left}}));
    emit(Event("game_state_changed", {{"running", true}, {"status", ""}}));
    emit(Event("reset_positions"));

    // Spawn initial 10 dust agents as required by game mechanics
    for (int i = 0; i < 10; i++) {
        spawn_dust();
    }
}

void GameManagerController::init() {
    target_score = 50; // Win condition requirement

    // Listen for when the player collects dust
    watch("dust_collected", [&](Event& e) {
        if (!game_running) return;

        int dust_id = e.value()["id"];

        // Verify the agent exists in our tracked list
        if (std::find(dust_ids.begin(), dust_ids.end(), dust_id) == dust_ids.end()) {
            return;
        }

        // Add to delayed removal queue and update local list
        pending_remove_dust_ids.push_back(dust_id);

        // Calculate score based on the specific dust value stored in the map
        if (dust_value_map.count(dust_id)) {
            score += dust_value_map[dust_id];
            dust_value_map.erase(dust_id);
        } else {
            score += 1; // Default fallback
        }

        dust_ids.erase(
            std::remove(dust_ids.begin(), dust_ids.end(), dust_id),
            dust_ids.end()
        );

        emit(Event("score_updated", {{"score", score}}));
    });

    // Listen for when the pet consumes dust (no points awarded)
    watch("dust_removed_by_pet", [&](Event& e) {
        if (!game_running) return;

        int dust_id = e.value()["id"];
        if (std::find(dust_ids.begin(), dust_ids.end(), dust_id) == dust_ids.end()) {
            return;
        }

        pending_remove_dust_ids.push_back(dust_id);
        if (dust_value_map.count(dust_id)) {
            dust_value_map.erase(dust_id);
        }

        dust_ids.erase(
            std::remove(dust_ids.begin(), dust_ids.end(), dust_id),
            dust_ids.end()
        );
    });

    // Handle restart button interaction with safety checks
    watch("button_click", [&](Event& e) {
        auto value = e.value();
        auto it = value.find("value");
        if (it != value.end() && it->is_string()) {
            if (it->get<std::string>() == "restart") {
                reset_game();
            }
        }
    });
}

void GameManagerController::start() { 
    reset_game(); 
}

void GameManagerController::update() {
    // Safely remove agents registered in the previous cycle to avoid iterator errors
    if (!pending_remove_dust_ids.empty()) {
        for (int dust_id : pending_remove_dust_ids) {
            if (agent_exists(dust_id)) {
                remove_agent(dust_id);
            }
        }
        pending_remove_dust_ids.clear();
    }

    // Synchronize active dust IDs with other agents (Pet/Vacuum)
    json ids = json::array();
    for (int id : dust_ids) {
        if (agent_exists(id)) {
            ids.push_back(id);
        }
    }
    emit(Event("dust_list_updated", {{"ids", ids}}));

    // Update Head-Up Display (HUD) with Scoring Legend
    std::string legend = " (Gold=1, Blue=3, Red=5)";
    std::string ui_text =
        "Goal: " + std::to_string(target_score) + 
        "  Score: " + std::to_string(score) +
        "  Time: " + std::to_string(current_time_left) + legend;

    if (!game_running) {
        ui_text += (score >= target_score) ? "   WIN" : "   LOSE";
    }

    // Render the UI text (Position at 0,0 relative to GameManager)
    label(ui_text, 0, 0);

    if (!game_running) return;

    auto now = std::chrono::steady_clock::now();

    // Check if it is time to spawn new dust
    double spawn_elapsed = std::chrono::duration<double>(now - last_spawn_time).count();
    if (spawn_elapsed >= spawn_interval) {
        spawn_dust();
        last_spawn_time = now;
    }

    // Update game timer
    int elapsed_seconds = (int) std::chrono::duration<double>(now - round_start_time).count();
    int new_time_left = round_duration - elapsed_seconds;
    if (new_time_left < 0) new_time_left = 0;

    if (new_time_left != current_time_left) {
        current_time_left = new_time_left;
        emit(Event("timer_updated", {{"time_left", current_time_left}}));
    }

    // Handle end-of-round logic using unified target_score
    if (current_time_left <= 0) {
        game_running = false;
        std::string result = (score >= target_score) ? "WIN" : "LOSE";
        emit(Event("game_state_changed", {
            {"running", false},
            {"status", result}
        }));
    }
}

void GameManagerController::stop() {}