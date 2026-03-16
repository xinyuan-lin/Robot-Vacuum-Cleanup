#ifndef __DUST_AGENT__H
#define __DUST_AGENT__H

#include "enviro.h"

using namespace enviro;

class DustController : public Process, public AgentInterface {

    public:
    DustController() : Process(), AgentInterface() {}

    void init();
    void start();
    void update();
    void stop();
};

class Dust : public Agent {
    public:
    Dust(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }

    private:
    DustController c;
};

DECLARE_INTERFACE(Dust)

#endif