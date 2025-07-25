#ifndef TRACK_H
#define TRACK_H

#include "game.h" // Need TrackType

// --- Common ---
#define CORNER_SEGMENTS 20      // Segments per 90-degree corner (Rounded track)
#define COLLISION_EPSILON 0.2f
#define FINISH_LINE_Z 0.0f      // Common finish line Z

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

// --- Rounded Corner Track Dimensions ---
#define ROUND_TRACK_MAIN_WIDTH 80.0f
#define ROUND_TRACK_MAIN_LENGTH 120.0f
#define ROUND_TRACK_ROAD_WIDTH 12.0f
#define ROUND_CORNER_RADIUS (ROUND_TRACK_ROAD_WIDTH * 1.8f) // Centerline radius
#define ROUND_HALF_ROAD_WIDTH (ROUND_TRACK_ROAD_WIDTH / 2.0f)
#define ROUND_INNER_CORNER_RADIUS (ROUND_CORNER_RADIUS - ROUND_HALF_ROAD_WIDTH)
#define ROUND_OUTER_CORNER_RADIUS (ROUND_CORNER_RADIUS + ROUND_HALF_ROAD_WIDTH)
#define ROUND_STRAIGHT_X_LIMIT (ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_CORNER_RADIUS)
#define ROUND_STRAIGHT_Z_LIMIT (ROUND_TRACK_MAIN_LENGTH / 2.0f - ROUND_CORNER_RADIUS)
#define ROUND_CORNER_CENTER_TR_X (ROUND_STRAIGHT_X_LIMIT)
#define ROUND_CORNER_CENTER_TR_Z (ROUND_STRAIGHT_Z_LIMIT)
#define ROUND_CORNER_CENTER_TL_X (-ROUND_STRAIGHT_X_LIMIT)
#define ROUND_CORNER_CENTER_TL_Z (ROUND_STRAIGHT_Z_LIMIT)
#define ROUND_CORNER_CENTER_BL_X (-ROUND_STRAIGHT_X_LIMIT)
#define ROUND_CORNER_CENTER_BL_Z (-ROUND_STRAIGHT_Z_LIMIT)
#define ROUND_CORNER_CENTER_BR_X (ROUND_STRAIGHT_X_LIMIT)
#define ROUND_CORNER_CENTER_BR_Z (-ROUND_STRAIGHT_Z_LIMIT)
// Finish line placed on right straight for consistency
#define ROUND_FINISH_LINE_X_START (ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH)
#define ROUND_FINISH_LINE_X_END   (ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH)
#define ROUND_FINISH_LINE_THICKNESS 2.0f


// Function declarations
void renderTrack();
void renderGuardrails();
int isPositionOnTrack(float x, float z); // Now uses selectedTrackType internally

#endif // TRACK_H