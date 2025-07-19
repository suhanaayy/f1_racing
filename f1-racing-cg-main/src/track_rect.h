#ifndef TRACK_RECT_H
#define TRACK_RECT_H

// --- Rectangular Track Dimensions ---
#define RECT_TRACK_MAIN_WIDTH 80.0f
#define RECT_TRACK_MAIN_LENGTH 120.0f
#define RECT_TRACK_ROAD_WIDTH 12.0f
#define RECT_HALF_ROAD_WIDTH (RECT_TRACK_ROAD_WIDTH / 2.0f)
#define RECT_OUTER_X_POS (RECT_TRACK_MAIN_WIDTH / 2.0f)
#define RECT_OUTER_X_NEG (-RECT_TRACK_MAIN_WIDTH / 2.0f)
#define RECT_OUTER_Z_POS (RECT_TRACK_MAIN_LENGTH / 2.0f)
#define RECT_OUTER_Z_NEG (-RECT_TRACK_MAIN_LENGTH / 2.0f)
#define RECT_INNER_X_POS (RECT_OUTER_X_POS - RECT_TRACK_ROAD_WIDTH)
#define RECT_INNER_X_NEG (RECT_OUTER_X_NEG + RECT_TRACK_ROAD_WIDTH)
#define RECT_INNER_Z_POS (RECT_OUTER_Z_POS - RECT_TRACK_ROAD_WIDTH)
#define RECT_INNER_Z_NEG (RECT_OUTER_Z_NEG + RECT_TRACK_ROAD_WIDTH)
#define RECT_FINISH_LINE_X_START RECT_INNER_X_POS // Place finish line on right straight
#define RECT_FINISH_LINE_X_END   RECT_OUTER_X_POS
#define RECT_FINISH_LINE_THICKNESS 2.0f

// --- Common Values ---
#define FINISH_LINE_Z 0.0f
#define COLLISION_EPSILON 0.2f

// --- Function Declarations ---
void renderRectTrack();
void renderRectGuardrails();
int isPositionOnRectTrack(float x, float z);

#endif // TRACK_RECT_H