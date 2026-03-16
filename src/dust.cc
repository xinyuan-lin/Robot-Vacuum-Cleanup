#include <iostream>
#include "dust.h"

using namespace enviro;

// Put your implementations here
void DustController::init() {}

void DustController::start() {
    prevent_rotation();
}

void DustController::update() {
    omni_track_velocity(0, 0);
}

void DustController::stop() {}