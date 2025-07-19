#ifndef CAR_H
#define CAR_H

// Basic struct to hold car state
typedef struct {
    // Position
    float x;
    float y;
    float z;

    // Previous position (for collision response)
    float prev_x;
    float prev_z;

    // Orientation (angle around Y-axis in degrees)
    float angle;

    // Physics
    float speed;
    float acceleration_rate;
    float braking_rate;
    float friction;
    float turn_speed;
    float max_speed;
    float max_reverse_speed;

    // Control state (using int for bool)
    int accelerating;
    int braking;
    int turning_left;
    int turning_right;

    // Dimensions (for rendering AND collision)
    float width;
    float height;
    float length;

} Car;

// Function declarations
void initCar(Car* car);
void updateCar(Car* car, float deltaTime);
void renderCar(const Car* car);
void setCarControls(Car* car, int key, int state); // 1 for down, 0 for up

// --- New Helper Function Prototype ---
// Calculates the world X, Z coordinates of the car's four corners
void calculateCarCorners(float center_x, float center_z, float angle_deg,
                         float width, float length,
                         float* fl_x, float* fl_z, // Front-Left
                         float* fr_x, float* fr_z, // Front-Right
                         float* rl_x, float* rl_z, // Rear-Left
                         float* rr_x, float* rr_z); // Rear-Right

#endif // CAR_H