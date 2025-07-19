#ifndef GAME_H
#define GAME_H

#include "car.h" // Includes Car struct definition

// --- Game States ---
typedef enum {
    STATE_MENU,      // Showing the track selection menu
    STATE_RACING     // Actively racing on a selected track
} GameState;

// --- Track Types ---
// Enum defining the different available track geometries.
typedef enum {
    TRACK_RECT,      // The sharp-cornered rectangle
    TRACK_ROUNDED    // The rectangle with rounded corners
    // Add more track types here if needed (remember to update NUM_TRACK_OPTIONS)
} TrackType;

// --- Menu Selection ---
// Defines how many track options are available in the menu.
// This MUST match the number of entries in the trackNames array in game.c
#define NUM_TRACK_OPTIONS 2

// --- Frame Timing ---
#define FRAME_RATE 60                // Target frames per second
#define FRAME_TIME_MS (1000 / FRAME_RATE) // Delay between updates in milliseconds
#define FRAME_TIME_SEC (1.0f / FRAME_RATE) // Delay between updates in seconds (for physics)

// --- Global Variables ---
// These are defined in game.c and declared here for access in other files (like main.c).
extern GameState currentGameState;           // Current state of the game (menu or racing)
extern TrackType selectedTrackType;        // Track type for the *current* race (set when race starts)
extern int menuSelectionIndex;           // Which track is highlighted in the menu (0-based)
extern Car playerCar;                    // The player's car object

// Timer variables used for lap timing in the HUD.
extern int lapStartTimeMs;                   // Time the current lap started (ms since GLUT init)
extern int currentLapTimeMs;                 // Duration of the current lap (ms)
extern int lastLapTimeMs;                  // Duration of the last completed lap (ms)
extern int bestLapTimeMs;                  // Duration of the best completed lap (ms)
extern int crossedFinishLineMovingForwardState; // State flag for lap detection (0=false, 1=true)

// --- Function Declarations ---
// Core game functions
void initGame();                           // Initializes car/timers for the selected track (called by startGame/reset)
void updateGame(int value);                // Main game loop update function (timer callback)
void setupCamera();                        // Configures the third-person camera view
void startGame(TrackType type);            // Transitions from menu to racing state with chosen track

// Rendering functions
void renderMenu(int windowWidth, int windowHeight); // Draws the track selection menu
void renderHUD(int windowWidth, int windowHeight);  // Draws the lap timer HUD

// Input handling functions (called by main.c based on game state)
void handleMenuKeyPress(unsigned char key);   // Handles regular keys in menu state
void handleMenuSpecialKey(int key);         // Handles special keys (arrows) in menu state
void handleRacingKeyPress(unsigned char key); // Handles regular keys in racing state
void handleRacingSpecialKey(int key);       // Handles special keys in racing state (currently none)

#endif // GAME_H