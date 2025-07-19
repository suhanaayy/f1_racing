#include "car.h"      // Defines the Car struct and function prototypes
#include "track.h"    // Defines track boundaries and isPositionOnTrack()
#include "game.h"     // Defines selectedTrackType and TrackType enum (assuming this exists in game.h)

#include <GL/glew.h>     // For OpenGL types (indirectly used via GLUT)
#include <GL/freeglut.h> // For rendering primitives like glutSolidCube
#include <math.h>        // For sinf, cosf, fabsf, fmodf, fmaxf, fminf, powf, sqrtf
#include <stdio.h>       // For optional debugging printf statements

// Define M_PI if not already defined by math.h
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
// Macro for converting degrees to radians
#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0f)


// --- Car Initialization ---
// Sets the initial state of the car based on the selected track.
void initCar(Car* car) {
    // Common initial state
    car->y = 0.25f;      // Half height, sitting on y=0 plane
    car->angle = 0.0f;     // Facing positive Z (generally 'up' the track initially)
    car->speed = 0.0f;

    // --- Set start position based on track type ---
    // This ensures the car starts on a valid part of the chosen track,
    // typically on the starting straight behind the finish line.
    if (selectedTrackType == TRACK_RECT) {
        // Start on the right straight for the rectangular track
        car->x = (RECT_INNER_X_POS + RECT_OUTER_X_POS) / 2.0f; // Center of the right road lane
        car->z = FINISH_LINE_Z - 20.0f; // Start back from the finish line Z coordinate
    } else { // TRACK_ROUNDED
        // Start on the right straight for the rounded track as well
        car->x = ROUND_TRACK_MAIN_WIDTH / 2.0f; // Center X of the right straight section
        car->z = FINISH_LINE_Z - 20.0f; // Start back from the finish line Z coordinate
    }


    // Initialize previous position to the starting position
    car->prev_x = car->x;
    car->prev_z = car->z;

    // --- Physics Parameters ---
    // These values can be tuned to change the car's handling characteristics.
    car->acceleration_rate = 7.0f;  // Units per second^2
    car->braking_rate = 15.0f;      // Units per second^2 (force opposing motion)
    car->friction = 2.0f;           // Drag factor applied when not accelerating/braking
    car->turn_speed = 140.0f;       // Adjusted for sharp corners
    car->max_speed = 40.0f;         // Maximum forward speed in units per second
    car->max_reverse_speed = -10.0f; // Maximum reverse speed

    // --- Control State Initialization ---
    // Ensure control flags start as 'off'.
    car->accelerating = 0; // 0 = false, 1 = true
    car->braking = 0;
    car->turning_left = 0;
    car->turning_right = 0;

    // --- Dimensions for Rendering and Collision ---
    // Used for scaling the car model and calculating corner positions.
    car->width = 1.0f;
    car->height = 0.5f;
    car->length = 2.2f;
}


// --- Corner Calculation Helper Function ---
// Calculates the world X, Z coordinates of the car's four corners based on its center,
// orientation, and dimensions. This is used for collision detection.
void calculateCarCorners(float center_x, float center_z, float angle_deg,
                         float width, float length,
                         float* fl_x, float* fl_z, // Front-Left output pointers
                         float* fr_x, float* fr_z, // Front-Right
                         float* rl_x, float* rl_z, // Rear-Left
                         float* rr_x, float* rr_z) // Rear-Right
{
    float angle_rad = DEG_TO_RAD(angle_deg); // Convert angle to radians
    float sin_a = sinf(angle_rad);
    float cos_a = cosf(angle_rad);

    // Calculate half dimensions for convenience
    float half_width = width / 2.0f;
    float half_length = length / 2.0f;

    // Define corner coordinates in the car's local space (relative to center, angle 0 = +Z)
    float local_fl_x = -half_width; float local_fl_z = +half_length; // Front-Left
    float local_fr_x = +half_width; float local_fr_z = +half_length; // Front-Right
    float local_rl_x = -half_width; float local_rl_z = -half_length; // Rear-Left
    float local_rr_x = +half_width; float local_rr_z = -half_length; // Rear-Right

    // Apply 2D rotation and translation to find world coordinates for each corner
    // Rotation formula: world_coord = center + R * local_coord
    // Where R is the rotation matrix [[cos, sin], [-sin, cos]] adjusted for our angle definition (0 = +Z)
    // World X = CenterX + LocalX * cos(angle) + LocalZ * sin(angle)
    // World Z = CenterZ - LocalX * sin(angle) + LocalZ * cos(angle)

    *fl_x = center_x + local_fl_x * cos_a + local_fl_z * sin_a;
    *fl_z = center_z - local_fl_x * sin_a + local_fl_z * cos_a;

    *fr_x = center_x + local_fr_x * cos_a + local_fr_z * sin_a;
    *fr_z = center_z - local_fr_x * sin_a + local_fr_z * cos_a;

    *rl_x = center_x + local_rl_x * cos_a + local_rl_z * sin_a;
    *rl_z = center_z - local_rl_x * sin_a + local_rl_z * cos_a;

    *rr_x = center_x + local_rr_x * cos_a + local_rr_z * sin_a;
    *rr_z = center_z - local_rr_x * sin_a + local_rr_z * cos_a;
}


// --- Car Update Logic ---
// Called every frame by updateGame() to calculate physics and collisions.
void updateCar(Car* car, float deltaTime) {
    // Store previous valid position *before* any updates. Used for collision response.
    car->prev_x = car->x;
    car->prev_z = car->z;

    // --- 1. Apply Turning --- (Code as provided by user)
    float current_turn_speed = car->turn_speed;
    if (fabsf(car->speed) > 1.0f) {
        float speed_factor = 1.0f - (fmaxf(0.0f, fabsf(car->speed) - car->max_speed * 0.3f) / (car->max_speed * 0.7f));
        current_turn_speed *= fmaxf(0.15f, speed_factor);
    }
    if (car->turning_left && fabsf(car->speed) > 0.1f) car->angle += current_turn_speed * deltaTime;
    if (car->turning_right && fabsf(car->speed) > 0.1f) car->angle -= current_turn_speed * deltaTime;
    car->angle = fmodf(car->angle + 360.0f, 360.0f);

    // --- 2. Apply Acceleration/Braking --- (Code as provided by user)
    float effective_accel = 0.0f;
    if (car->accelerating) effective_accel = car->acceleration_rate;
    if (car->braking) {
        if (car->speed > 0.01f) effective_accel -= car->braking_rate;
        else if (car->speed < -0.01f) effective_accel += car->braking_rate;
    }
    car->speed += effective_accel * deltaTime;

    // --- 3. Apply Friction --- (Code as provided by user)
    if (!car->accelerating && !car->braking && fabsf(car->speed) > 0.01f) {
        float friction_force = car->friction * deltaTime;
        if (car->speed > 0.0f) {
            car->speed -= friction_force; if (car->speed < 0.0f) car->speed = 0.0f;
        } else {
            car->speed += friction_force; if (car->speed > 0.0f) car->speed = 0.0f;
        }
    }

    // --- 4. Clamp Speed --- (Code as provided by user)
    car->speed = fmaxf(car->max_reverse_speed, fminf(car->max_speed, car->speed));

    // --- 5. Calculate Potential New Position and Check Corner Collisions ---
    if (fabsf(car->speed) > 0.001f) {
        float angle_rad = DEG_TO_RAD(car->angle);
        float dx = car->speed * sinf(angle_rad) * deltaTime;
        float dz = car->speed * cosf(angle_rad) * deltaTime;

        // Calculate potential new CENTER position
        float potential_x = car->x + dx;
        float potential_z = car->z + dz;

        // Calculate potential CORNER positions based on the potential center
        float pot_fl_x, pot_fl_z, pot_fr_x, pot_fr_z; // Front corners
        float pot_rl_x, pot_rl_z, pot_rr_x, pot_rr_z; // Rear corners
        calculateCarCorners(potential_x, potential_z, car->angle,
                            car->width, car->length,
                            &pot_fl_x, &pot_fl_z, &pot_fr_x, &pot_fr_z,
                            &pot_rl_x, &pot_rl_z, &pot_rr_x, &pot_rr_z);

        // Check if ANY potential corner is off the track
        int collisionDetected = 0; // Use int for boolean (0 = false, 1 = true)
        if (!isPositionOnTrack(pot_fl_x, pot_fl_z)) collisionDetected = 1;
        if (!isPositionOnTrack(pot_fr_x, pot_fr_z)) collisionDetected = 1;
        if (!isPositionOnTrack(pot_rl_x, pot_rl_z)) collisionDetected = 1;
        if (!isPositionOnTrack(pot_rr_x, pot_rr_z)) collisionDetected = 1;

        // --- 6. Collision Detection and Response ---
        if (!collisionDetected) { // If collisionDetected is 0 (false)
            // Position is valid: Update the car's actual position.
            car->x = potential_x;
            car->z = potential_z;
        } else { // If collisionDetected is 1 (true)
            // Collision Occurred!
            // Simple Response: Revert to the last known valid position and stop the car.
            car->x = car->prev_x;
            car->z = car->prev_z;
            car->speed = 0.0f; // Bring car to a complete halt

            // Optional: Add sound effect or visual feedback here later.
            // printf("Corner Collision! Pos:(%.2f, %.2f) Speed set to 0.\n", car->x, car->z); // Debug output
        }
    } else {
         // If speed is near zero, explicitly set it to zero to prevent potential drift.
         car->speed = 0.0f;
    }
}


// --- Car Rendering --- (Code as provided by user)
// Draws the car model (currently a composite cube structure) at its current position and orientation.
void renderCar(const Car* car) {
    glPushMatrix(); // Save the current OpenGL matrix state

    // Apply transformations: Move to car's position and rotate to its angle.
    glTranslatef(car->x, car->y, car->z);
    glRotatef(car->angle, 0.0f, 1.0f, 0.0f); // Rotate around the Y-axis (vertical)

    // --- Car Body (Red) ---
    glPushMatrix();
    glScalef(car->width, car->height, car->length);
    glColor3f(1.0f, 0.0f, 0.0f); // Red color
    glutSolidCube(1.0f);
    glPopMatrix();

    // --- Wheels (Dark Grey Cubes) ---
    float wheelRadius = 0.35f * car->height;
    float wheelWidth = 0.15f * car->width;
    float wheelDistX = (car->width / 2.0f) + wheelWidth * 0.5f;
    float wheelDistZ = (car->length / 2.0f) * 0.7f;
    glColor3f(0.1f, 0.1f, 0.1f);
    // FL
    glPushMatrix();
    glTranslatef(-wheelDistX, 0.0f, wheelDistZ);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(wheelWidth, wheelRadius * 2.0f, wheelRadius * 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // FR
    glPushMatrix();
    glTranslatef(wheelDistX, 0.0f, wheelDistZ);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(wheelWidth, wheelRadius * 2.0f, wheelRadius * 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // RL
    glPushMatrix();
    glTranslatef(-wheelDistX, 0.0f, -wheelDistZ);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(wheelWidth, wheelRadius * 2.0f, wheelRadius * 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // RR
    glPushMatrix();
    glTranslatef(wheelDistX, 0.0f, -wheelDistZ);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(wheelWidth, wheelRadius * 2.0f, wheelRadius * 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // --- Driver Helmet Indicator (White Cube) ---
    glPushMatrix();
    glTranslatef(0.0f, car->height * 0.6f, -car->length * 0.1f);
    float helmetSize = 0.15f;
    glScalef(helmetSize, helmetSize, helmetSize);
    glColor3f(1.0f, 1.0f, 1.0f); // White color
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix(); // Restore the matrix state from before car transformations
}


// --- Car Control Input --- (Code as provided by user)
// Updates the car's control state flags based on keyboard input.
void setCarControls(Car* car, int key, int state) {
    switch (key) {
        case 'w': case 'W':
            car->accelerating = state; if(state) car->braking = 0; break;
        case 's': case 'S':
            car->braking = state; if(state) car->accelerating = 0; break;
        case 'a': case 'A':
            car->turning_left = state; break;
        case 'd': case 'D':
            car->turning_right = state; break;
    }
}