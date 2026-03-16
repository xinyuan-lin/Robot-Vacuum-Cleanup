# Robot Vacuum Cleanup

Final project for EEP_520_Winter2026.

## Overview

**Robot Vacuum Cleanup** is a small interactive game built with **Enviro**.  
The player controls a vacuum robot inside a room with walls, furniture, randomly spawned dust, and a moving pet hazard.

The objective is to clean enough dust before time runs out.  

The game includes:
- player-controlled vacuum movement
- randomly spawning dust
- score and countdown timer
- win / lose end state
- restart button for multiple rounds
- a pet agent with range-sensor-based obstacle avoidance
- furniture obstacles and room boundaries


---

## Game Goal

Control the vacuum robot and collect enough dust before the timer reaches zero.

- Collect dust to increase your score
- The pet moves around the room and can interfere with cleanup
- Win by reaching the target score before the timer ends
- Otherwise, you lose

---

## Features

### Player Vacuum
- Controlled with **WASD**
- Uses omni-directional movement
- Collects dust for points
- Can be slowed down temporarily if hit by the pet

### Dust System
- Dust appears at random valid positions
- Dust avoids spawning inside furniture
- Multiple dust tiers are used:
  - standard dust
  - medium-value dust
  - high-value dust
- Different colors represent different values

### HUD / Game State
- Score is displayed on screen
- Countdown timer is displayed on screen
- End state clearly shows **WIN** or **LOSE**
- A **Restart** button resets the round

### Pet Agent
- Autonomous moving hazard
- Uses range sensors to detect obstacles
- Avoids walls and furniture
- Can remove dust as it moves
- Triggers a slowdown effect when it collides with the player

### Environment
- Enclosed room with boundary walls
- Furniture obstacles placed around the room
- Open space left for player movement and dust collection

---

## Why This Project Fits the Final Project Requirements

This project satisfies the course final project requirements by including:

- **Enviro-based simulation/game**
- **Randomness** through random dust spawning
- **Game mechanics** including score, timer, win/lose, and restart
- **Static obstacles** (walls and furniture)
- **Dynamic elements** (player, pet, dust)
- **Sensor integration** through the pet’s range sensors
- **Substantial behavior** through multi-agent interaction and event-based gameplay

---

## Key Challenges and How They Were Addressed

### 1. Keeping Dust Spawn Valid
Dust should not spawn inside walls or furniture.  
This was handled by checking random candidate positions against bounding regions for furniture and only creating dust at legal locations.

### 2. Managing Agent Communication
The project uses multiple agents:
- `VacuumBot`
- `Pet`
- `Dust`
- `GameManager`

To keep the code organized, events are used for communication, including:
- score updates
- timer updates
- reset events
- dust removal events
- pet collision events

### 3. Preventing Unsafe Immediate Deletion
Removing Enviro agents immediately during interaction can cause instability.  
To avoid this, dust removal is handled through a delayed removal queue in the `GameManager`, which safely removes dust during the update cycle.

### 4. Pet Obstacle Avoidance
The pet uses range sensors and a simple movement policy to avoid walls and furniture while continuing to move around the room.  
The behavior was tuned to reduce wall-sticking and improve recovery near corners.

### 5. Displaying HUD Outside the Room
The score/timer display is attached to the `GameManager` and positioned outside the room boundary so it remains visible without interfering with gameplay.

---

## How to Run

### Prerequisites
- Docker installed
- The Enviro Docker image available or pullable

This project was developed to run inside the Enviro Docker environment used in class.

### Start the Docker container

From the project directory, run:

```bash
docker run -it -p80:80 -p8765:8765 -v $PWD:/source klavins/enviro:v1.61 bash
```

### Build the project

Inside the container:

```bash
cd /source
make
```

### Start the frontend

In one terminal inside the container:

```bash
cd /source
esm start
```
### Start the Enviro backend
In another terminal inside the same container:

```bash
cd /source
enviro
```
### Open the game
In your browser:
```text
http://localhost
```

## File Structure

```text
robot_cleanup/
├── config.json
├── Makefile
├── README.md
├── defs/
│   ├── vacuum_bot.json
│   ├── pet.json
│   ├── dust.json
│   └── game_manager.json
├── src/
│   ├── vacuum_bot.h
│   ├── vacuum_bot.cc
│   ├── pet.h
│   ├── pet.cc
│   ├── dust.h
│   ├── dust.cc
│   ├── game_manager.h
│   └── game_manager.cc
└── lib/
```

## How to Play

### Controls

W: move up

A: move left

S: move down

D: move right

### Objective

- Collect dust before the timer reaches zero
- Reach the target score to win
- Press Restart to start a new round

### Win / Lose Condition

- The round lasts 60 seconds.
- The player wins by reaching the target score before time runs out.
- If the timer reaches zero before the target score is reached, the player loses.

### Gameplay Notes

- Different dust colors may represent different score values
- The pet moves on its own and may interfere with cleaning
- The player can be slowed down when bumps into the pet


## Main Agents

### VacuumBot
Handles:
- player input
- omni-directional movement
- collecting dust
- reacting to slowdown when hit by the pet

### Pet
Handles:
- autonomous movement
- range-sensor-based obstacle avoidance
- dust removal
- collision-based slowdown interaction with the player

### Dust
Represents collectible cleaning targets spawned in the room.

### GameManager
Handles:
- score
- timer
- win/lose logic
- restart
- dust spawning
- HUD display
- delayed dust removal for safe updates

## Sources / Acknowledgements

This project was built using:
- course lecture material for Elma and Enviro
- the provided Enviro Docker image and project structure
- class examples involving:
  - agent definitions
  - sensors
  - buttons
  - events
  - dynamic agent creation and removal

No external game engine or image asset system was used.  
All room elements, agents, and interactions were implemented using Enviro configuration files and C++ controllers.

## Known Limitations

- The pet movement logic is reactive and simple rather than globally planned.
- Furniture is represented using geometric Enviro shapes rather than image assets.
- Dust spawning uses bounding-box checks for valid positions rather than a full geometric occupancy test.
- The game is designed as a compact Enviro project for the course rather than a full commercial game system.


## License

This project is released under the MIT License.  
See the `LICENSE` file in the repository for details.