#include <stdio.h>       // Standard Input/Output functions (printf)
#include <GL/glew.h>     // OpenGL Extension Wrangler Library (must be included before freeglut)
#include <GL/freeglut.h> // FreeGLUT library for windowing, input, and basic shapes

#include "game.h"       // Includes GameState, TrackType, global variables, core functions
// Include BOTH track headers for rendering functions
#include "track_rect.h"
#include "track_round.h"
// car.h is included via game.h

// --- Function Prototypes for GLUT Callbacks ---
void display();                          // Main drawing function
void reshape(int width, int height);     // Window resize handler
void keyboardDown(unsigned char key, int x, int y); // Regular key press handler
void keyboardUp(unsigned char key, int x, int y);   // Regular key release handler
void specialKeyDown(int key, int x, int y); // Special key press handler (arrows, etc.)
// void specialKeyUp(int key, int x, int y); // Optional: Special key release handler (not needed currently)
void cleanup();                          // Function called when the GLUT window is closed

// --- Main Application Entry Point ---
int main(int argc, char** argv) {
    // 1. Initialize GLUT
    glutInit(&argc, argv); // Initialize the GLUT library
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Double buffered, RGB color, Depth buffer
    glutInitWindowSize(1280, 720);     // Set initial window dimensions
    glutInitWindowPosition(100, 100);  // Set initial window position
    glutCreateWindow("F1 Racer Prototype - Menu"); // Create window with title

    // 2. Initialize GLEW
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Error initializing GLEW: %s\n", glewGetErrorString(err));
        return 1; // Indicate failure
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    fprintf(stdout, "Status: Using OpenGL %s\n", glGetString(GL_VERSION));


    // 3. Basic OpenGL Setup
    glEnable(GL_DEPTH_TEST); // Enable depth testing
    glDepthFunc(GL_LEQUAL);  // Pixels with equal or lesser depth pass
    glClearColor(0.1f, 0.3f, 0.7f, 1.0f); // Background clear color (sky blue)
    glEnable(GL_CULL_FACE); // Enable face culling
    glCullFace(GL_BACK);    // Cull back-facing polygons


    // 4. Initial Game State Setup
    // Game starts in STATE_MENU by default (see game.c definition)
    // No need to call initGame() here initially.


    // 5. Register GLUT Callback Functions
    glutDisplayFunc(display);           // Main drawing function
    glutReshapeFunc(reshape);           // Window resize handler
    glutKeyboardFunc(keyboardDown);     // Regular key press handler
    glutKeyboardUpFunc(keyboardUp);       // Regular key release handler
    glutSpecialFunc(specialKeyDown);    // Special key press handler
    glutCloseFunc(cleanup);             // Window close handler


    // 6. Register the Fixed-Update Timer
    glutTimerFunc(FRAME_TIME_MS, updateGame, 0);


    // 7. Print Controls and Enter GLUT Main Loop
     printf("\n--- CONTROLS ---\n");
     printf(" Menu:\n");
     printf("   UP/DOWN Arrows: Select Track\n");
     printf("   ENTER: Start Race\n");
     printf(" Racing:\n");
     printf("   W/S: Accelerate/Brake\n");
     printf("   A/D: Turn Left/Right\n");
     printf("   R: Reset Race\n");
     printf(" General:\n");
     printf("   ESC: Return to Menu / Exit\n");
     printf("-----------------\n\n");

    glutMainLoop(); // Start processing events

    return 0; // Should not be reached
}


// --- GLUT Callback Implementations ---

// Main Drawing Function
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear buffers

    // Render based on the current game state
    if (currentGameState == STATE_MENU) {
        renderMenu(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)); // Draw the 2D menu
    } else { // STATE_RACING
        // --- Render 3D Racing Scene ---
        glMatrixMode(GL_PROJECTION); glLoadIdentity();
        gluPerspective(50.0f, (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT), 0.1f, 600.0f); // Set perspective
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        setupCamera(); // Position the camera

        // Render the appropriate track based on selection
        if (selectedTrackType == TRACK_RECT) {
            renderRectTrack();
            renderRectGuardrails();
        } else { // TRACK_ROUNDED
            renderRoundTrack();
            renderRoundGuardrails();
        }

        renderCar(&playerCar); // Draw the car

        // --- Render 2D HUD ---
        renderHUD(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)); // Draw timers
    }

    glutSwapBuffers(); // Display the rendered frame
}


// Window Resize Handler
void reshape(int width, int height) {
    if (height == 0) height = 1; // Avoid divide by zero
    float aspect = (float)width / (float)height;
    glViewport(0, 0, width, height); // Set viewport to new dimensions

    // Reset projection matrix for new aspect ratio (important for perspective)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0f, aspect, 0.1f, 600.0f);
    glMatrixMode(GL_MODELVIEW); // Switch back to modelview
}


// Regular Key Press Handler (Delegates based on Game State)
void keyboardDown(unsigned char key, int x, int y) {
    (void)x; (void)y; // Mark GLUT mouse coordinates as unused

    // Call the appropriate state-specific handler function (defined in game.c)
    if (currentGameState == STATE_MENU) {
        handleMenuKeyPress(key);
    } else { // STATE_RACING
        handleRacingKeyPress(key);
    }
}


// Regular Key Release Handler (Only relevant for Racing State)
void keyboardUp(unsigned char key, int x, int y) {
    (void)x; (void)y; // Mark unused

    // Pass key release info to car controls only when racing
    if (currentGameState == STATE_RACING) {
        // Allow case-insensitivity for releasing movement keys
        if (key == 'w' || key == 'W' || key == 'a' || key == 'A' || key == 's' || key == 'S' || key == 'd' || key == 'D') {
             setCarControls(&playerCar, key, 0); // 0 = key up
        }
    }
}


// Special Key Press Handler (Delegates based on Game State)
void specialKeyDown(int key, int x, int y) {
    (void)x; (void)y; // Mark unused

    // Call the appropriate state-specific handler function (defined in game.c)
    if (currentGameState == STATE_MENU) {
        handleMenuSpecialKey(key);
    } else { // STATE_RACING
        handleRacingSpecialKey(key);
    }
}


// Cleanup Function
void cleanup() {
    printf("Exiting application...\n");
    // Add any resource freeing code here if necessary in the future
}