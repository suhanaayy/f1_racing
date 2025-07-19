#include "game.h"       // Defines GameState, TrackType, Car, globals, function prototypes
// Include BOTH track headers - the code uses constants/functions from one based on selectedTrackType
#include "track_rect.h"
#include "track_round.h"
#include <GL/glew.h>    // For OpenGL types if needed (used by GLUT)
#include <GL/freeglut.h> // For rendering text, getting time, etc.
#include <stdio.h>      // For snprintf, printf (debugging)
#include <string.h>     // For strlen (used implicitly by snprintf etc.)
#include <math.h>       // For fabsf, fmaxf, fminf, sinf, cosf etc.
#include <limits.h>     // For INT_MAX (initial best lap time)

// Define M_PI if not already defined by math.h
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
// Macro for converting degrees to radians
#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0f)

// --- Global Variable Definitions ---
// Declared 'extern' in game.h, defined here with initial values.
GameState currentGameState = STATE_MENU;     // Start the game in the menu state
TrackType selectedTrackType = TRACK_ROUNDED; // Default track type for internal logic (will be overwritten by menu)
int menuSelectionIndex = 0;              // Index of the currently highlighted menu option (0-based)
Car playerCar;                           // The player's car object
int lapStartTimeMs = 0;                  // Milliseconds timestamp when the current lap started
int currentLapTimeMs = 0;                // Milliseconds duration of the lap currently in progress
int lastLapTimeMs = 0;                   // Milliseconds duration of the previously completed lap
int bestLapTimeMs = INT_MAX;             // Milliseconds duration of the fastest completed lap
int crossedFinishLineMovingForwardState = 0; // Boolean flag (0=false, 1=true) for lap detection


// --- Initialization Function (for RACING state) ---
// Called by startGame() or when 'R' is pressed during racing.
// Sets up the car and timers for the currently selected track.
void initGame() {
    // initCar() itself now checks 'selectedTrackType' for positioning etc.
    initCar(&playerCar); // initCar is defined in car.c

    // Initialize lap timing variables for the start of the race/reset.
    lapStartTimeMs = glutGet(GLUT_ELAPSED_TIME); // Get current time since GLUT started
    currentLapTimeMs = 0;
    lastLapTimeMs = 0;       // No previous lap yet on reset
    bestLapTimeMs = INT_MAX; // Reset best lap on reset (or load from save later)

    // Determine initial finish line state based on the car's starting Z and X position
    // relative to the finish line boundaries of the *selected* track.
    float finishLineXStart, finishLineXEnd;
    if (selectedTrackType == TRACK_RECT) {
        finishLineXStart = RECT_FINISH_LINE_X_START;
        finishLineXEnd = RECT_FINISH_LINE_X_END;
    } else { // TRACK_ROUNDED
        finishLineXStart = ROUND_FINISH_LINE_X_START;
        finishLineXEnd = ROUND_FINISH_LINE_X_END;
    }
    // Set flag to true (1) only if starting exactly on or past the line (unlikely with current setup)
    crossedFinishLineMovingForwardState = (playerCar.z >= FINISH_LINE_Z &&
                                           playerCar.x >= finishLineXStart &&
                                           playerCar.x <= finishLineXEnd);

    printf("Game Initialized for Track Type %d. Start time: %dms. Crossed Flag: %d\n",
           selectedTrackType, lapStartTimeMs, crossedFinishLineMovingForwardState);
}


// --- Function to start the game ---
// Called when the user selects a track from the menu and presses Enter.
void startGame(TrackType type) {
    printf("Starting game with Track Type %d\n", type);
    selectedTrackType = type;       // Store the chosen track type globally
    initGame();                     // Initialize car position, timers for this track
    currentGameState = STATE_RACING; // Change the game state to racing mode
    glutPostRedisplay();            // Ensure screen updates immediately
}


// --- Camera Setup Function ---
// Configures the view matrix to follow the car (third-person view).
void setupCamera() {
    // Camera parameters (adjust for desired view)
    float followDistance = 10.0f; // How far behind
    float followHeight = 5.0f;    // How high up
    float lookAtHeightOffset = 0.5f; // Point slightly above car's center Y

    // Calculate camera position using car's angle and position
    float carAngleRad = DEG_TO_RAD(playerCar.angle);
    float camX = playerCar.x - followDistance * sinf(carAngleRad);
    float camY = playerCar.y + followHeight; // Use car's actual y + offset
    float camZ = playerCar.z - followDistance * cosf(carAngleRad);

    // Calculate look-at point (center of the car)
    float lookAtX = playerCar.x;
    float lookAtY = playerCar.y + lookAtHeightOffset;
    float lookAtZ = playerCar.z;

    // Set the Modelview matrix using gluLookAt
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); // Reset matrix before setting camera
    gluLookAt(camX, camY, camZ,           // Camera position (eye)
              lookAtX, lookAtY, lookAtZ,  // Point to look at (center)
              0.0f, 1.0f, 0.0f);         // Up vector (positive Y)
}


// --- Fixed Timestep Update Function ---
// Contains the main game loop logic, called repeatedly by GLUT timer.
void updateGame(int value) {
    // --- Only update game logic if in RACING state ---
    if (currentGameState != STATE_RACING) {
        // Still need to reschedule the timer to keep the loop running for the menu
        glutTimerFunc(FRAME_TIME_MS, updateGame, 0);
        // Request redraw in case menu needs updating (e.g., animations, though none currently)
        glutPostRedisplay();
        return; // Skip physics, lap timing, etc., when in menu
    }
    // --- End of state check ---

    (void)value; // Mark the GLUT timer parameter as unused

    int timeNowMs = glutGet(GLUT_ELAPSED_TIME);

    // Update car physics, movement, and collision detection/response.
    // This function (in car.c) now internally calls the correct isPositionOn*Track
    updateCar(&playerCar, FRAME_TIME_SEC);

    // Update Lap Timers based on elapsed time.
    if (timeNowMs >= lapStartTimeMs) {
        currentLapTimeMs = timeNowMs - lapStartTimeMs;
    } else {
        // Handle potential timer wrap-around or reset during gameplay.
        lapStartTimeMs = timeNowMs;
        currentLapTimeMs = 0;
    }

    // --- Lap Completion Logic ---
    // Check if the car has crossed the finish line in the forward direction.
    float carZ = playerCar.z;
    float carPrevZ = playerCar.prev_z;
    float carX = playerCar.x;
    int movingForward = (playerCar.speed > 0.1f); // Check speed for direction

    // Get finish line X boundaries based on the currently selected track.
    float finishLineXStart, finishLineXEnd;
    if (selectedTrackType == TRACK_RECT) {
        finishLineXStart = RECT_FINISH_LINE_X_START;
        finishLineXEnd = RECT_FINISH_LINE_X_END;
    } else { // TRACK_ROUNDED
        finishLineXStart = ROUND_FINISH_LINE_X_START;
        finishLineXEnd = ROUND_FINISH_LINE_X_END;
    }
    // Check if the car is within the X span of the finish line.
    int withinFinishLineX = (carX >= finishLineXStart && carX <= finishLineXEnd);


    // --- Detect Crossing Finish Line FORWARD ---
    // Conditions: Z crossed the FINISH_LINE_Z threshold, moving forward, within X bounds.
    if (carPrevZ < FINISH_LINE_Z && carZ >= FINISH_LINE_Z && movingForward && withinFinishLineX) {
        // Only count lap completion if the 'crossedForward' flag is already set (meaning
        // we completed the previous part of the track and are genuinely finishing a lap).
        if (crossedFinishLineMovingForwardState == 1) {
            // --- LAP COMPLETED ---
            lastLapTimeMs = currentLapTimeMs; // Record the time
            // Update best lap if this one was faster (and valid).
            if (lastLapTimeMs > 0 && lastLapTimeMs < bestLapTimeMs) {
                bestLapTimeMs = lastLapTimeMs;
            }
            // Reset timer for the start of the *new* lap.
            lapStartTimeMs = timeNowMs;
            currentLapTimeMs = 0;
            // The flag remains 1 as we start the next lap from past the line.
        } else {
            // This is the *first* time crossing forward (either started before the line
            // or crossed backward then forward again). Set the flag and start the timer.
            crossedFinishLineMovingForwardState = 1; // Set flag to true
            lapStartTimeMs = timeNowMs;            // Start timing the first/next lap *now*.
            currentLapTimeMs = 0;
        }
    }
    // --- Detect Crossing Finish Line BACKWARD ---
    // Conditions: Z crossed the threshold backward, within X bounds.
    else if (carPrevZ >= FINISH_LINE_Z && carZ < FINISH_LINE_Z && withinFinishLineX) {
        // If the car goes backward over the line, reset the state flag. It will need
        // to cross forward again to set the flag before completing the *next* lap.
        crossedFinishLineMovingForwardState = 0; // Set flag to false
    }

    // Request GLUT to redraw the screen.
    glutPostRedisplay();
    // Reschedule this update function to be called again after the frame delay.
    glutTimerFunc(FRAME_TIME_MS, updateGame, 0);
}


// --- Menu Rendering Function ---
// Draws the track selection menu.
void renderMenu(int windowWidth, int windowHeight) {
    char menuText[100]; // Text buffer
    // Array of track names corresponding to TrackType enum order and NUM_TRACK_OPTIONS
    const char* trackNames[NUM_TRACK_OPTIONS] = {
        "Rectangular Circuit", // Index 0 -> TRACK_RECT
        "Rounded Circuit"      // Index 1 -> TRACK_ROUNDED
        // Add more names here if NUM_TRACK_OPTIONS increases
    };

    // --- Set up 2D Orthographic Projection ---
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight); // Map OpenGL coords directly to pixels (0,0 bottom-left)
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); // Reset modelview for 2D

    // --- Disable 3D effects that interfere with 2D text ---
    glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_FOG_BIT);
    glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D); glDisable(GL_FOG);

    // --- Render Menu Text Elements ---
    int textX = windowWidth / 2 - 150; // Base X position for roughly centered text
    int textY = windowHeight / 2 + 100; // Starting Y position (higher up)
    int lineHeight = 28;               // Vertical spacing between lines

    // Title
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow
    snprintf(menuText, sizeof(menuText), "F1 RACER PROTOTYPE");
    glRasterPos2i(textX, textY);
    for (char* c = menuText; *c != '\0'; c++) { glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c); }
    textY -= lineHeight * 2; // Move down

    // Instructions
    glColor3f(0.8f, 0.8f, 0.8f); // Light grey
    snprintf(menuText, sizeof(menuText), "Use UP/DOWN arrows to select");
    glRasterPos2i(textX, textY); for (char* c = menuText; *c != '\0'; c++) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); }
    textY -= (int)(lineHeight * 0.75); // Smaller gap
    snprintf(menuText, sizeof(menuText), "Press ENTER to start");
    glRasterPos2i(textX, textY); for (char* c = menuText; *c != '\0'; c++) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); }
    textY -= (int)(lineHeight * 1.5); // Larger gap

    // Track Options (Loop through and highlight the selected one)
    for (int i = 0; i < NUM_TRACK_OPTIONS; ++i) {
        if (i == menuSelectionIndex) {
            glColor3f(1.0f, 1.0f, 1.0f); // White for selected item
            snprintf(menuText, sizeof(menuText), "> %s <", trackNames[i]); // Add selection markers
        } else {
            glColor3f(0.6f, 0.6f, 0.6f); // Grey for non-selected items
            snprintf(menuText, sizeof(menuText), "  %s  ", trackNames[i]); // Add padding for alignment
        }
        glRasterPos2i(textX + 10, textY); // Indent the track names slightly
        for (char* c = menuText; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        textY -= lineHeight; // Move down for next option
    }
    textY -= lineHeight; // Extra space before exit prompt

    // Exit Instruction
    glColor3f(0.8f, 0.8f, 0.8f);
    snprintf(menuText, sizeof(menuText), "ESC to Exit");
    glRasterPos2i(textX, textY);
    for (char* c = menuText; *c != '\0'; c++) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); }

    // --- Restore OpenGL states and matrices ---
    glPopAttrib(); // Restore disabled states (depth, lighting etc.)
    glMatrixMode(GL_PROJECTION); glPopMatrix(); // Restore previous projection matrix
    glMatrixMode(GL_MODELVIEW); glPopMatrix();  // Restore previous modelview matrix
}


// --- Heads-Up Display (HUD) Rendering Function ---
// Draws the lap timers during the racing state.
void renderHUD(int windowWidth, int windowHeight) {
    char hudText[100]; // Buffer for formatted strings

    // --- Set up 2D Orthographic Projection ---
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight); // Pixel coordinates (0,0 bottom-left)
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); // Reset modelview

    // --- Disable 3D effects ---
    glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_FOG_BIT);
    glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D); glDisable(GL_FOG);

    // --- Render Timers ---
    glColor3f(1.0f, 1.0f, 1.0f); // White text color
    int textX = 10;                // X position from left edge
    int textY = windowHeight - 30; // Y position from *bottom* edge (near top-left)
    int lineHeight = 20;           // Vertical spacing

    // Current Lap Time
    int cur_mins=(currentLapTimeMs/1000)/60; int cur_secs=(currentLapTimeMs/1000)%60; int cur_ms=currentLapTimeMs%1000;
    snprintf(hudText, sizeof(hudText), "Current: %02d:%02d.%03d", cur_mins, cur_secs, cur_ms);
    glRasterPos2i(textX, textY); // Set position for text drawing
    for (char* c = hudText; *c != '\0'; c++) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); } // Draw character by character
    textY -= lineHeight; // Move down for next line

    // Last Lap Time
    if (lastLapTimeMs > 0) { // Only display if a lap has been completed
        int last_mins=(lastLapTimeMs/1000)/60; int last_secs=(lastLapTimeMs/1000)%60; int last_ms=lastLapTimeMs%1000;
        snprintf(hudText, sizeof(hudText), "Last:    %02d:%02d.%03d", last_mins, last_secs, last_ms);
    } else {
        snprintf(hudText, sizeof(hudText), "Last:    --:--.---"); // Placeholder if no laps completed
    }
    glRasterPos2i(textX, textY); for (char* c = hudText; *c != '\0'; c++) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); }
    textY -= lineHeight;

    // Best Lap Time
    if (bestLapTimeMs != INT_MAX) { // Only display if a best lap exists
        int best_mins=(bestLapTimeMs/1000)/60; int best_secs=(bestLapTimeMs/1000)%60; int best_ms=bestLapTimeMs%1000;
        snprintf(hudText, sizeof(hudText), "Best:    %02d:%02d.%03d", best_mins, best_secs, best_ms);
    } else {
        snprintf(hudText, sizeof(hudText), "Best:    --:--.---"); // Placeholder if no laps recorded
    }
    glRasterPos2i(textX, textY); for (char* c = hudText; *c != '\0'; c++) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); }

    // --- Restore OpenGL states and matrices ---
    glPopAttrib(); // Restore states disabled earlier
    glMatrixMode(GL_PROJECTION); glPopMatrix(); // Restore projection matrix
    glMatrixMode(GL_MODELVIEW); glPopMatrix();  // Restore modelview matrix
}


// --- Input Handling Helper Functions ---
// These are called by the main GLUT callbacks in main.c based on the current game state.

// Handles regular key presses when in the Menu state.
void handleMenuKeyPress(unsigned char key) {
     switch (key) {
         case 13: // ASCII for Enter key
            // Start the game with the track corresponding to the highlighted index.
            // The index directly maps to the TrackType enum value.
            startGame((TrackType)menuSelectionIndex);
            break;
        case 27: // ESC key
            printf("ESC pressed in menu. Exiting.\n");
            glutLeaveMainLoop(); // Exit the application.
            break;
     }
}

// Handles special key presses (like arrows) when in the Menu state.
void handleMenuSpecialKey(int key) {
    switch (key) {
        case GLUT_KEY_UP: // Up arrow pressed
            menuSelectionIndex--; // Move selection up
            // Wrap around if moving past the first option.
            if (menuSelectionIndex < 0) {
                menuSelectionIndex = NUM_TRACK_OPTIONS - 1; // Wrap to last item
            }
            glutPostRedisplay(); // Request a redraw to show the updated highlight.
            break;
        case GLUT_KEY_DOWN: // Down arrow pressed
            menuSelectionIndex++; // Move selection down
            // Wrap around if moving past the last option.
            if (menuSelectionIndex >= NUM_TRACK_OPTIONS) {
                menuSelectionIndex = 0; // Wrap to first item
            }
            glutPostRedisplay(); // Request a redraw to show the updated highlight.
            break;
    }
}

// Handles regular key presses when in the Racing state.
void handleRacingKeyPress(unsigned char key) {
    // Pass movement keys (W, A, S, D) to the car controller.
    // Allow case-insensitivity for movement keys.
    if (key == 'w' || key == 'W' || key == 'a' || key == 'A' || key == 's' || key == 'S' || key == 'd' || key == 'D') {
        setCarControls(&playerCar, key, 1); // 1 = key down
    }


    // Handle non-movement keys specific to racing.
    switch (key) {
        case 'r': // Reset key
        case 'R':
            printf("'R' pressed. Resetting race.\n");
            initGame(); // Re-initialize car and timers for the current track.
            break;
        case 27: // ESC key
            printf("ESC pressed in racing. Returning to Menu.\n");
            currentGameState = STATE_MENU; // Change state back to menu.
            // Optionally highlight the track we just left in the menu.
            menuSelectionIndex = (int)selectedTrackType;
            // Reset timers when returning to menu to avoid confusion.
            lastLapTimeMs = 0; bestLapTimeMs = INT_MAX; currentLapTimeMs = 0;
            glutPostRedisplay(); // Request redraw to show the menu immediately.
            break;
    }
}

// Handles special key presses when in the Racing state.
void handleRacingSpecialKey(int key) {
    // Currently, no special keys are used during racing (arrows are for menu).
    (void)key; // Mark the parameter as unused to avoid compiler warnings.
}