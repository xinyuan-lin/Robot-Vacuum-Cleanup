#include <iostream>
#include <cmath>
#include "pet.h"
#include <chrono>

using namespace enviro;

// Initiates the escape state to avoid obstacles
void PetController::begin_escape(double turn) {
    escaping = true;
    turn_rate = turn;
    escape_until = std::chrono::steady_clock::now() + std::chrono::seconds(1);
}

// Randomly chooses a new turning rate for wandering behavior
void PetController::choose_new_wander_turn() {
    turn_rate = wander_turn_dist(generator);
    next_wander_change = std::chrono::steady_clock::now() + std::chrono::milliseconds(1500);
}

void PetController::init() {
    // Notify collision event when the pet hits the player vacuum
    notice_collisions_with("VacuumBot", [&](Event& e) {
        emit(Event("pet_collision"));
    });

    // Receive updated list of active dust agents from the GameManager
    watch("dust_list_updated", [&](Event& e) {
        known_dust_ids.clear();

        auto value = e.value();
        auto it = value.find("ids");

        // Validate event data structure
        if (it == value.end()) {
            return;
        }

        if (!it->is_array()) {
            return;
        }

        // Store new list of active dust IDs
        for (auto& id_json : *it) {
            if (id_json.is_number_integer()) {
                known_dust_ids.push_back(id_json.get<int>());
            }
        }
    });

    // Reset pet position and state when the game restarts
    watch("reset_positions", [&](Event& e) {
        teleport(120, 80, 0);
        escaping = false;
        known_dust_ids.clear();
        choose_new_wander_turn();
    });
}

void PetController::start() {
    // Initial spawning position
    teleport(120, 80, 0);
    escaping = false;
    choose_new_wander_turn();
}

void PetController::update() {
    // ---------- Step 1: Check for Dust interaction ----------
    int removed_dust_id = -1;
    std::vector<int> remaining_dust_ids;

    for (int dust_id : known_dust_ids) {
        // Skip IDs for agents that no longer exist
        if (!agent_exists(dust_id)) {
            continue;
        }

        // If an ID has already been picked for removal, store the rest
        if (removed_dust_id != -1) {
            remaining_dust_ids.push_back(dust_id);
            continue;
        }

        Agent& dust = find_agent(dust_id);

        // Calculate distance between Pet and Dust
        double dx = dust.x() - x();
        double dy = dust.y() - y();
        double dist = std::sqrt(dx * dx + dy * dy);

        // If within removal radius, register for deletion
        if (dist <= dust_remove_radius) {
            removed_dust_id = dust_id;
        } else {
            remaining_dust_ids.push_back(dust_id);
        }
    }

    // Update the local list of known dust
    known_dust_ids = remaining_dust_ids;

    // Send removal request to GameManager if a dust was consumed
    if (removed_dust_id != -1) {
        track_velocity(0, 0);

        emit(Event("dust_removed_by_pet", {
            {"id", removed_dust_id}
        }));
        return;
    }

    // ---------- Step 2: Obstacle Avoidance and Wandering ----------
    
    // Helper function to ignore dust agents in the sensor feedback
    auto get_sensor = [&](int idx) {
        std::string t = sensor_reflection_type(idx);
        if (t == "Dust") {
            return 100.0; // Return maximum distance so pet doesn't "see" dust as a wall
        }
        return sensor_value(idx);
    };

    double left_d = get_sensor(0);
    double front_d = get_sensor(1);
    double right_d = get_sensor(2);

    // Obstacle avoidance logic based on range sensor values
    if (!escaping) {
        if (front_d < 40) {
            begin_escape(left_d > right_d ? -2.6 : 2.6);
        } else if (left_d < 20) {
            begin_escape(2.2);
        } else if (right_d < 20) {
            begin_escape(-2.2);
        }
    }

    // Movement execution
    if (escaping) {
        // Execute escape turn
        track_velocity(0, turn_rate);
        if (std::chrono::steady_clock::now() >= escape_until) {
            escaping = false;
            choose_new_wander_turn();
        }
    } else {
        // Execute standard wandering
        if (std::chrono::steady_clock::now() >= next_wander_change) {
            choose_new_wander_turn();
        }
        track_velocity(forward_speed, turn_rate);
    }
}

void PetController::stop() {}