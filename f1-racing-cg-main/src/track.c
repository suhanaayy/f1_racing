#include "track.h"
#include "game.h" // Need access to selectedTrackType enum and global variable
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h> // Optional: For debug prints

// Define M_PI if not already defined by math.h
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
// Macro for converting degrees to radians
#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0f)

// --- Rounded Corner Helpers (For Lines) ---
// Renders a curved line segment (part of the boundary lines)
void renderCornerLineSegment(float center_x, float center_z, float radius, float start_angle_deg, int num_segments, float y_level) {
    float angle_step = DEG_TO_RAD(90.0f) / num_segments; // 90 degrees per corner
    float start_rad = DEG_TO_RAD(start_angle_deg);
    for (int i = 0; i <= num_segments; ++i) { // Include endpoint
        float current_angle = start_rad + i * angle_step;
        // Calculate point on circle circumference
        glVertex3f(center_x + radius * cosf(current_angle), y_level, center_z + radius * sinf(current_angle));
    }
}

// --- Rounded Corner Helpers (For Quad Strips - Used for Rails) ---
// Renders a curved quad strip segment with thickness (base_y to top_y)
void renderCornerQuadStripSegment(float center_x, float center_z, float inner_rad, float outer_rad, float start_angle_deg, int num_segments, float base_y, float top_y) {
     float angle_step = DEG_TO_RAD(90.0f) / num_segments;
     float start_rad = DEG_TO_RAD(start_angle_deg);
     for (int i = 0; i <= num_segments; ++i) {
         float current_angle = start_rad + i * angle_step;
         float cos_a = cosf(current_angle);
         float sin_a = sinf(current_angle);
         // Inner edge vertex pair (bottom, top)
         glVertex3f(center_x + inner_rad * cos_a, base_y, center_z + inner_rad * sin_a);
         glVertex3f(center_x + inner_rad * cos_a, top_y, center_z + inner_rad * sin_a); // Only difference if top_y != base_y
         // Outer edge vertex pair (bottom, top)
         glVertex3f(center_x + outer_rad * cos_a, base_y, center_z + outer_rad * sin_a);
         glVertex3f(center_x + outer_rad * cos_a, top_y, center_z + outer_rad * sin_a); // Only difference if top_y != base_y
     }
}

// --- Rounded Corner Helpers (For Quad Strips - SURFACE ONLY) ---
// Renders a curved quad strip segment for the flat track surface (no height)
void renderCornerSurfaceSegment(float center_x, float center_z, float inner_rad, float outer_rad, float start_angle_deg, int num_segments, float surface_y) {
     float angle_step = DEG_TO_RAD(90.0f) / num_segments;
     float start_rad = DEG_TO_RAD(start_angle_deg);
     for (int i = 0; i <= num_segments; ++i) { // Include endpoint
         float current_angle = start_rad + i * angle_step;
         float cos_a = cosf(current_angle);
         float sin_a = sinf(current_angle);
         // Inner edge vertex
         glVertex3f(center_x + inner_rad * cos_a, surface_y, center_z + inner_rad * sin_a);
         // Outer edge vertex
         glVertex3f(center_x + outer_rad * cos_a, surface_y, center_z + outer_rad * sin_a);
     }
}


// --- Rectangular Wall Helper ---
// Draws a rectangular wall block between two points with given height and thickness
void drawWall(float x1, float z1, float x2, float z2, float height, float thickness) {
    // Calculate normalized direction vector and perpendicular vector
    float dx=x2-x1; float dz=z2-z1; float len=sqrtf(dx*dx+dz*dz); if(len<0.001f) return; // Avoid zero length
    float nx=dx/len; float nz=dz/len; float px=-nz; float pz=nx; float half_thick=thickness/2.0f;

    // Define 8 vertices of the wall block
    float v[8][3]={
        {x1 - px*half_thick, 0.0f,   z1 - pz*half_thick}, // 0: Front Bottom Left
        {x1 + px*half_thick, 0.0f,   z1 + pz*half_thick}, // 1: Back Bottom Left
        {x2 + px*half_thick, 0.0f,   z2 + pz*half_thick}, // 2: Back Bottom Right
        {x2 - px*half_thick, 0.0f,   z2 - pz*half_thick}, // 3: Front Bottom Right
        {x1 - px*half_thick, height, z1 - pz*half_thick}, // 4: Front Top Left
        {x1 + px*half_thick, height, z1 + pz*half_thick}, // 5: Back Top Left
        {x2 + px*half_thick, height, z2 + pz*half_thick}, // 6: Back Top Right
        {x2 - px*half_thick, height, z2 - pz*half_thick}  // 7: Front Top Right
    };

    // Draw the 6 faces using GL_QUADS
    glBegin(GL_QUADS);
        // Top face (CCW winding: 4, 5, 6, 7)
        glVertex3fv(v[4]); glVertex3fv(v[5]); glVertex3fv(v[6]); glVertex3fv(v[7]);
        // Bottom face (CCW winding: 0, 3, 2, 1) - Optional as usually unseen
        // glVertex3fv(v[0]); glVertex3fv(v[3]); glVertex3fv(v[2]); glVertex3fv(v[1]);
        // Front face (CCW winding: 0, 3, 7, 4)
        glVertex3fv(v[0]); glVertex3fv(v[3]); glVertex3fv(v[7]); glVertex3fv(v[4]);
        // Back face (CCW winding: 1, 2, 6, 5)
        glVertex3fv(v[1]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[5]);
        // Left face (CCW winding: 0, 4, 5, 1)
        glVertex3fv(v[0]); glVertex3fv(v[4]); glVertex3fv(v[5]); glVertex3fv(v[1]);
        // Right face (CCW winding: 3, 7, 6, 2)
        glVertex3fv(v[3]); glVertex3fv(v[7]); glVertex3fv(v[6]); glVertex3fv(v[2]);
    glEnd();
}

// --- Main Track Rendering Function (Conditional) ---
void renderTrack() {
    float surface_y = 0.0f; // Y level for track surface
    float line_y = 0.01f;   // Y level for white lines (slightly above surface)
    float finish_y = 0.02f; // Y level for finish line (slightly higher)
    float groundSize;       // Size of the green ground plane

    // --- Render Ground Plane (Common to both tracks) ---
    glColor3f(0.2f, 0.6f, 0.2f); // Grassy Green color
    glBegin(GL_QUADS);
        // Determine ground size based on the larger dimension of the selected track
        if (selectedTrackType == TRACK_RECT) {
            groundSize = fmaxf(RECT_TRACK_MAIN_WIDTH, RECT_TRACK_MAIN_LENGTH) * 1.2f;
        } else { // TRACK_ROUNDED
             groundSize = fmaxf(ROUND_TRACK_MAIN_WIDTH, ROUND_TRACK_MAIN_LENGTH) * 1.2f;
        }
        // Define the four corners of the ground quad
        glVertex3f(-groundSize, -0.02f, -groundSize); // Bottom-Left
        glVertex3f(-groundSize, -0.02f,  groundSize); // Top-Left
        glVertex3f( groundSize, -0.02f,  groundSize); // Top-Right
        glVertex3f( groundSize, -0.02f, -groundSize); // Bottom-Right
    glEnd();

    // --- Render Track Surface (Asphalt Grey) ---
    glColor3f(0.4f, 0.4f, 0.45f);

    if (selectedTrackType == TRACK_RECT) {
        // --- Rectangular Surface ---
        // Draw the 4 rectangular strips that make up the track surface.
        // Top strip
        glBegin(GL_QUADS);
            glVertex3f(RECT_OUTER_X_NEG, surface_y, RECT_INNER_Z_POS); // TL Inner
            glVertex3f(RECT_OUTER_X_POS, surface_y, RECT_INNER_Z_POS); // TR Inner
            glVertex3f(RECT_OUTER_X_POS, surface_y, RECT_OUTER_Z_POS); // TR Outer
            glVertex3f(RECT_OUTER_X_NEG, surface_y, RECT_OUTER_Z_POS); // TL Outer
        glEnd();
        // Bottom strip
        glBegin(GL_QUADS);
            glVertex3f(RECT_OUTER_X_NEG, surface_y, RECT_OUTER_Z_NEG); // BL Outer
            glVertex3f(RECT_OUTER_X_POS, surface_y, RECT_OUTER_Z_NEG); // BR Outer
            glVertex3f(RECT_OUTER_X_POS, surface_y, RECT_INNER_Z_NEG); // BR Inner
            glVertex3f(RECT_OUTER_X_NEG, surface_y, RECT_INNER_Z_NEG); // BL Inner
        glEnd();
        // Left strip (fills gap between top/bottom inner/outer)
        glBegin(GL_QUADS);
            glVertex3f(RECT_OUTER_X_NEG, surface_y, RECT_INNER_Z_NEG); // Outer BL
            glVertex3f(RECT_INNER_X_NEG, surface_y, RECT_INNER_Z_NEG); // Inner BL
            glVertex3f(RECT_INNER_X_NEG, surface_y, RECT_INNER_Z_POS); // Inner TL
            glVertex3f(RECT_OUTER_X_NEG, surface_y, RECT_INNER_Z_POS); // Outer TL
        glEnd();
        // Right strip (fills gap between top/bottom inner/outer)
         glBegin(GL_QUADS);
            glVertex3f(RECT_INNER_X_POS, surface_y, RECT_INNER_Z_NEG); // Inner BR
            glVertex3f(RECT_OUTER_X_POS, surface_y, RECT_INNER_Z_NEG); // Outer BR
            glVertex3f(RECT_OUTER_X_POS, surface_y, RECT_INNER_Z_POS); // Outer TR
            glVertex3f(RECT_INNER_X_POS, surface_y, RECT_INNER_Z_POS); // Inner TR
        glEnd();

    } else { // TRACK_ROUNDED
        // --- Rounded Corner Surface (Using continuous GL_QUAD_STRIP) ---
        int straight_segments = 10; // Number of quads per straight section

        glBegin(GL_QUAD_STRIP);

            // 1. Right Straight (Start point: Bottom Right Straight Start)
            for(int i = 0; i <= straight_segments; ++i) {
                float t = (float)i / straight_segments; // Interpolation factor (0 to 1)
                // Interpolate Z from bottom straight end (-Z_LIMIT) to top straight start (+Z_LIMIT)
                float z = -ROUND_STRAIGHT_Z_LIMIT + (ROUND_STRAIGHT_Z_LIMIT - (-ROUND_STRAIGHT_Z_LIMIT)) * t;
                // X coordinates are constant for the right straight
                float inner_x = ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH;
                float outer_x = ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH;
                glVertex3f(inner_x, surface_y, z); // Inner edge vertex
                glVertex3f(outer_x, surface_y, z); // Outer edge vertex
            }

            // 2. Top Right Corner (Connects from Right Straight End to Top Straight Start)
            // Start Angle = 0 degrees (positive X axis)
            renderCornerSurfaceSegment(ROUND_CORNER_CENTER_TR_X, ROUND_CORNER_CENTER_TR_Z, ROUND_INNER_CORNER_RADIUS, ROUND_OUTER_CORNER_RADIUS, 0.0f, CORNER_SEGMENTS, surface_y);

            // 3. Top Straight (Connects from TR Corner End to TL Corner Start)
            for(int i = 0; i <= straight_segments; ++i) {
                float t = (float)i / straight_segments;
                // Interpolate X from right straight end (+X_LIMIT) to left straight start (-X_LIMIT)
                float x = ROUND_STRAIGHT_X_LIMIT - (ROUND_STRAIGHT_X_LIMIT - (-ROUND_STRAIGHT_X_LIMIT)) * t;
                // Z coordinates are constant for the top straight
                float inner_z = ROUND_TRACK_MAIN_LENGTH / 2.0f - ROUND_HALF_ROAD_WIDTH;
                float outer_z = ROUND_TRACK_MAIN_LENGTH / 2.0f + ROUND_HALF_ROAD_WIDTH;
                glVertex3f(x, surface_y, inner_z); // Inner edge vertex
                glVertex3f(x, surface_y, outer_z); // Outer edge vertex
            }

            // 4. Top Left Corner (Connects from Top Straight End to Left Straight Start)
            // Start Angle = 90 degrees (positive Z axis)
            renderCornerSurfaceSegment(ROUND_CORNER_CENTER_TL_X, ROUND_CORNER_CENTER_TL_Z, ROUND_INNER_CORNER_RADIUS, ROUND_OUTER_CORNER_RADIUS, 90.0f, CORNER_SEGMENTS, surface_y);

            // 5. Left Straight (Connects from TL Corner End to BL Corner Start)
            for(int i = 0; i <= straight_segments; ++i) {
                float t = (float)i / straight_segments;
                // Interpolate Z from top straight end (+Z_LIMIT) to bottom straight start (-Z_LIMIT)
                float z = ROUND_STRAIGHT_Z_LIMIT - (ROUND_STRAIGHT_Z_LIMIT - (-ROUND_STRAIGHT_Z_LIMIT)) * t;
                // X coordinates are constant for the left straight
                float inner_x = -ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH;
                float outer_x = -ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH;
                glVertex3f(inner_x, surface_y, z); // Inner edge vertex (careful with order for quad strip)
                glVertex3f(outer_x, surface_y, z); // Outer edge vertex
            }

            // 6. Bottom Left Corner (Connects from Left Straight End to Bottom Straight Start)
            // Start Angle = 180 degrees (negative X axis)
            renderCornerSurfaceSegment(ROUND_CORNER_CENTER_BL_X, ROUND_CORNER_CENTER_BL_Z, ROUND_INNER_CORNER_RADIUS, ROUND_OUTER_CORNER_RADIUS, 180.0f, CORNER_SEGMENTS, surface_y);

            // 7. Bottom Straight (Connects from BL Corner End to BR Corner Start)
            for(int i = 0; i <= straight_segments; ++i) {
                float t = (float)i / straight_segments;
                // Interpolate X from left straight end (-X_LIMIT) to right straight start (+X_LIMIT)
                float x = -ROUND_STRAIGHT_X_LIMIT + (ROUND_STRAIGHT_X_LIMIT - (-ROUND_STRAIGHT_X_LIMIT)) * t;
                // Z coordinates are constant for the bottom straight
                float inner_z = -ROUND_TRACK_MAIN_LENGTH / 2.0f - ROUND_HALF_ROAD_WIDTH;
                float outer_z = -ROUND_TRACK_MAIN_LENGTH / 2.0f + ROUND_HALF_ROAD_WIDTH;
                glVertex3f(x, surface_y, inner_z); // Inner edge vertex
                glVertex3f(x, surface_y, outer_z); // Outer edge vertex
            }

            // 8. Bottom Right Corner (Connects from Bottom Straight End back to Right Straight Start)
            // Start Angle = 270 degrees (negative Z axis)
            renderCornerSurfaceSegment(ROUND_CORNER_CENTER_BR_X, ROUND_CORNER_CENTER_BR_Z, ROUND_INNER_CORNER_RADIUS, ROUND_OUTER_CORNER_RADIUS, 270.0f, CORNER_SEGMENTS, surface_y);

            // 9. Close the Loop: Add the first vertex pair of the Right Straight again explicitly.
            // This ensures the last quad connects back seamlessly to the beginning.
            float z_start_right_straight = -ROUND_STRAIGHT_Z_LIMIT;
            float inner_x_start_right = ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH;
            float outer_x_start_right = ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH;
            glVertex3f(inner_x_start_right, surface_y, z_start_right_straight);
            glVertex3f(outer_x_start_right, surface_y, z_start_right_straight);

        glEnd(); // End the single QUAD_STRIP for the rounded track surface
    } // End TRACK_ROUNDED Surface

    // --- Render Track Markings (White Lines) ---
    glColor3f(1.0f, 1.0f, 1.0f); // White color
    glLineWidth(2.0f);          // Set line thickness

    if (selectedTrackType == TRACK_RECT) {
        // Rectangular boundary lines using GL_LINE_LOOP for automatic closing
        // Outer boundary
        glBegin(GL_LINE_LOOP);
            glVertex3f(RECT_OUTER_X_NEG, line_y, RECT_OUTER_Z_NEG); // BL
            glVertex3f(RECT_OUTER_X_POS, line_y, RECT_OUTER_Z_NEG); // BR
            glVertex3f(RECT_OUTER_X_POS, line_y, RECT_OUTER_Z_POS); // TR
            glVertex3f(RECT_OUTER_X_NEG, line_y, RECT_OUTER_Z_POS); // TL
        glEnd();
        // Inner boundary
        glBegin(GL_LINE_LOOP);
            glVertex3f(RECT_INNER_X_NEG, line_y, RECT_INNER_Z_NEG); // BL
            glVertex3f(RECT_INNER_X_POS, line_y, RECT_INNER_Z_NEG); // BR
            glVertex3f(RECT_INNER_X_POS, line_y, RECT_INNER_Z_POS); // TR
            glVertex3f(RECT_INNER_X_NEG, line_y, RECT_INNER_Z_POS); // TL
        glEnd();
     } else { // TRACK_ROUNDED
        // Rounded boundary lines using GL_LINE_STRIP and corner helper
        // Outer boundary
        glBegin(GL_LINE_STRIP);
            // Start at beginning of Top Right corner (end of right straight)
            glVertex3f(ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH, line_y, ROUND_STRAIGHT_Z_LIMIT);
            renderCornerLineSegment(ROUND_CORNER_CENTER_TR_X, ROUND_CORNER_CENTER_TR_Z, ROUND_OUTER_CORNER_RADIUS, 0.0f, CORNER_SEGMENTS, line_y); // TR Corner
            renderCornerLineSegment(ROUND_CORNER_CENTER_TL_X, ROUND_CORNER_CENTER_TL_Z, ROUND_OUTER_CORNER_RADIUS, 90.0f, CORNER_SEGMENTS, line_y); // TL Corner
            renderCornerLineSegment(ROUND_CORNER_CENTER_BL_X, ROUND_CORNER_CENTER_BL_Z, ROUND_OUTER_CORNER_RADIUS, 180.0f, CORNER_SEGMENTS, line_y); // BL Corner
            renderCornerLineSegment(ROUND_CORNER_CENTER_BR_X, ROUND_CORNER_CENTER_BR_Z, ROUND_OUTER_CORNER_RADIUS, 270.0f, CORNER_SEGMENTS, line_y); // BR Corner
            // Connect back to the start point to close the loop visually
            glVertex3f(ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH, line_y, ROUND_STRAIGHT_Z_LIMIT);
        glEnd();
         // Inner boundary
        glBegin(GL_LINE_STRIP);
             // Start at beginning of Top Right corner (end of right straight) - Inner
            glVertex3f(ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH, line_y, ROUND_STRAIGHT_Z_LIMIT);
            renderCornerLineSegment(ROUND_CORNER_CENTER_TR_X, ROUND_CORNER_CENTER_TR_Z, ROUND_INNER_CORNER_RADIUS, 0.0f, CORNER_SEGMENTS, line_y); // TR Corner Inner
            renderCornerLineSegment(ROUND_CORNER_CENTER_TL_X, ROUND_CORNER_CENTER_TL_Z, ROUND_INNER_CORNER_RADIUS, 90.0f, CORNER_SEGMENTS, line_y); // TL Corner Inner
            renderCornerLineSegment(ROUND_CORNER_CENTER_BL_X, ROUND_CORNER_CENTER_BL_Z, ROUND_INNER_CORNER_RADIUS, 180.0f, CORNER_SEGMENTS, line_y); // BL Corner Inner
            renderCornerLineSegment(ROUND_CORNER_CENTER_BR_X, ROUND_CORNER_CENTER_BR_Z, ROUND_INNER_CORNER_RADIUS, 270.0f, CORNER_SEGMENTS, line_y); // BR Corner Inner
            // Connect back to the start point to close the loop visually
            glVertex3f(ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH, line_y, ROUND_STRAIGHT_Z_LIMIT);
        glEnd();
     }

    // --- Finish line (Common logic, different coords based on track type) ---
    glColor3f(0.9f, 0.9f, 0.9f); // White-ish finish line
    glBegin(GL_QUADS);
        float finishLineXStart, finishLineXEnd, finishLineZPos, finishLineZNeg;
        // Determine finish line coordinates based on the selected track
        if (selectedTrackType == TRACK_RECT) {
            finishLineXStart = RECT_FINISH_LINE_X_START;
            finishLineXEnd = RECT_FINISH_LINE_X_END;
            finishLineZPos = FINISH_LINE_Z + RECT_FINISH_LINE_THICKNESS / 2.0f;
            finishLineZNeg = FINISH_LINE_Z - RECT_FINISH_LINE_THICKNESS / 2.0f;
        } else { // TRACK_ROUNDED (Place on right straight)
            finishLineXStart = ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH;
            finishLineXEnd = ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH;
            finishLineZPos = FINISH_LINE_Z + ROUND_FINISH_LINE_THICKNESS / 2.0f;
            finishLineZNeg = FINISH_LINE_Z - ROUND_FINISH_LINE_THICKNESS / 2.0f;
         }
        // Define the four vertices of the finish line quad
        glVertex3f(finishLineXStart, finish_y, finishLineZPos); // Top Left
        glVertex3f(finishLineXEnd,   finish_y, finishLineZPos); // Top Right
        glVertex3f(finishLineXEnd,   finish_y, finishLineZNeg); // Bottom Right
        glVertex3f(finishLineXStart, finish_y, finishLineZNeg); // Bottom Left
    glEnd();

    glLineWidth(1.0f); // Reset line width to default
}

// --- Guardrail Rendering (Conditional) ---
void renderGuardrails() {
    float railHeight = 0.8f;     // Height of the guardrails
    float railThickness = 0.4f;  // Thickness of the guardrails
    float margin = 0.15f;        // Distance rails are placed outside track lines
    float base_y = 0.0f;         // Bottom Y level of rails
    float top_y = railHeight;    // Top Y level of rails
    glColor3f(0.8f, 0.1f, 0.1f); // Red color for guardrails

    if (selectedTrackType == TRACK_RECT) {
        // --- Rectangular Guardrails ---
        // Define coordinates slightly outside the track boundaries
        float ox1=RECT_OUTER_X_NEG-margin; float oz1=RECT_OUTER_Z_NEG-margin;
        float ox2=RECT_OUTER_X_POS+margin; float oz2=RECT_OUTER_Z_NEG-margin;
        float ox3=RECT_OUTER_X_POS+margin; float oz3=RECT_OUTER_Z_POS+margin;
        float ox4=RECT_OUTER_X_NEG-margin; float oz4=RECT_OUTER_Z_POS+margin;
        // Draw outer walls using the helper function
        drawWall(ox1, oz1, ox2, oz2, railHeight, railThickness); // Bottom wall
        drawWall(ox2, oz2, ox3, oz3, railHeight, railThickness); // Right wall
        drawWall(ox3, oz3, ox4, oz4, railHeight, railThickness); // Top wall
        drawWall(ox4, oz4, ox1, oz1, railHeight, railThickness); // Left wall

        // Define inner boundary coordinates slightly inside
        float ix1=RECT_INNER_X_NEG+margin; float iz1=RECT_INNER_Z_NEG+margin;
        float ix2=RECT_INNER_X_POS-margin; float iz2=RECT_INNER_Z_NEG+margin;
        float ix3=RECT_INNER_X_POS-margin; float iz3=RECT_INNER_Z_POS-margin;
        float ix4=RECT_INNER_X_NEG+margin; float iz4=RECT_INNER_Z_POS-margin;
         // Draw inner walls using the helper function
        drawWall(ix1, iz1, ix2, iz2, railHeight, railThickness); // Bottom wall
        drawWall(ix2, iz2, ix3, iz3, railHeight, railThickness); // Right wall
        drawWall(ix3, iz3, ix4, iz4, railHeight, railThickness); // Top wall
        drawWall(ix4, iz4, ix1, iz1, railHeight, railThickness); // Left wall

    } else { // TRACK_ROUNDED
        // --- Rounded Guardrails ---
        // Draw straight sections using drawWall
        float outerRailYPos = ROUND_TRACK_MAIN_LENGTH / 2.0f + ROUND_HALF_ROAD_WIDTH + margin;
        float outerRailYNeg = -outerRailYPos;
        float innerRailYPos = ROUND_TRACK_MAIN_LENGTH / 2.0f - ROUND_HALF_ROAD_WIDTH - margin;
        float innerRailYNeg = -innerRailYPos;
        float outerRailXPos = ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH + margin;
        float outerRailXNeg = -outerRailXPos;
        float innerRailXPos = ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH - margin;
        float innerRailXNeg = -innerRailXPos;

        // Outer straight walls
        drawWall(-ROUND_STRAIGHT_X_LIMIT, outerRailYPos,  ROUND_STRAIGHT_X_LIMIT, outerRailYPos, railHeight, railThickness); // Top
        drawWall(-ROUND_STRAIGHT_X_LIMIT, outerRailYNeg,  ROUND_STRAIGHT_X_LIMIT, outerRailYNeg, railHeight, railThickness); // Bottom
        drawWall( outerRailXPos, -ROUND_STRAIGHT_Z_LIMIT, outerRailXPos,  ROUND_STRAIGHT_Z_LIMIT, railHeight, railThickness); // Right
        drawWall( outerRailXNeg, -ROUND_STRAIGHT_Z_LIMIT, outerRailXNeg,  ROUND_STRAIGHT_Z_LIMIT, railHeight, railThickness); // Left
        // Inner straight walls
        drawWall(-ROUND_STRAIGHT_X_LIMIT, innerRailYPos,  ROUND_STRAIGHT_X_LIMIT, innerRailYPos, railHeight, railThickness); // Top
        drawWall(-ROUND_STRAIGHT_X_LIMIT, innerRailYNeg,  ROUND_STRAIGHT_X_LIMIT, innerRailYNeg, railHeight, railThickness); // Bottom
        drawWall( innerRailXPos, -ROUND_STRAIGHT_Z_LIMIT, innerRailXPos,  ROUND_STRAIGHT_Z_LIMIT, railHeight, railThickness); // Right
        drawWall( innerRailXNeg, -ROUND_STRAIGHT_Z_LIMIT, innerRailXNeg,  ROUND_STRAIGHT_Z_LIMIT, railHeight, railThickness); // Left


        // Draw curved sections using QUAD_STRIP helper
        // Calculate radii for the *centerline* of the thick guardrail wall
        float outerRailCenterRadius = ROUND_OUTER_CORNER_RADIUS + margin;
        float innerRailCenterRadius = fmaxf(0.1f + railThickness/2.0f, ROUND_INNER_CORNER_RADIUS - margin); // Ensure positive radius even after subtracting margin

        // Render curved rails using renderCornerQuadStripSegment (Pass same radius for inner/outer, use drawWall thickness)
        // We should actually use drawWall in segments for curved rails too for consistency.
        // Let's refine this: Draw curved rails as segmented straight walls.

        // Outer Curved Rails (Segmented Walls)
        float prev_x = ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH + margin; // Start at end of right straight
        float prev_z = ROUND_STRAIGHT_Z_LIMIT;
        for (int corner = 0; corner < 4; ++ corner) { // Loop through TR, TL, BL, BR corners
            float center_x, center_z, start_angle_deg;
            if (corner == 0) { center_x = ROUND_CORNER_CENTER_TR_X; center_z = ROUND_CORNER_CENTER_TR_Z; start_angle_deg = 0.0f; }
            else if (corner == 1) { center_x = ROUND_CORNER_CENTER_TL_X; center_z = ROUND_CORNER_CENTER_TL_Z; start_angle_deg = 90.0f; }
            else if (corner == 2) { center_x = ROUND_CORNER_CENTER_BL_X; center_z = ROUND_CORNER_CENTER_BL_Z; start_angle_deg = 180.0f; }
            else { center_x = ROUND_CORNER_CENTER_BR_X; center_z = ROUND_CORNER_CENTER_BR_Z; start_angle_deg = 270.0f; }

            float angle_step = DEG_TO_RAD(90.0f) / CORNER_SEGMENTS;
            float start_rad = DEG_TO_RAD(start_angle_deg);

            for (int i = 1; i <= CORNER_SEGMENTS; ++i) { // Start from segment 1
                 float current_angle = start_rad + i * angle_step;
                 float current_x = center_x + outerRailCenterRadius * cosf(current_angle);
                 float current_z = center_z + outerRailCenterRadius * sinf(current_angle);
                 drawWall(prev_x, prev_z, current_x, current_z, railHeight, railThickness);
                 prev_x = current_x;
                 prev_z = current_z;
            }
            // After corner, update prev_x/z to the start of the next straight wall section
             if (corner == 0) { prev_x = -ROUND_STRAIGHT_X_LIMIT; prev_z = outerRailYPos; } // Start of Top Straight
             else if (corner == 1) { prev_x = outerRailXNeg; prev_z = ROUND_STRAIGHT_Z_LIMIT; } // Start of Left Straight
             else if (corner == 2) { prev_x = ROUND_STRAIGHT_X_LIMIT; prev_z = outerRailYNeg; } // Start of Bottom Straight
             // else: after BR corner, prev_x/z are correctly set for the start of Right Straight
        }


         // Inner Curved Rails (Segmented Walls)
        prev_x = ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH - margin; // Start at end of right straight
        prev_z = ROUND_STRAIGHT_Z_LIMIT;
        for (int corner = 0; corner < 4; ++ corner) { // Loop through TR, TL, BL, BR corners
            float center_x, center_z, start_angle_deg;
            if (corner == 0) { center_x = ROUND_CORNER_CENTER_TR_X; center_z = ROUND_CORNER_CENTER_TR_Z; start_angle_deg = 0.0f; }
            else if (corner == 1) { center_x = ROUND_CORNER_CENTER_TL_X; center_z = ROUND_CORNER_CENTER_TL_Z; start_angle_deg = 90.0f; }
            else if (corner == 2) { center_x = ROUND_CORNER_CENTER_BL_X; center_z = ROUND_CORNER_CENTER_BL_Z; start_angle_deg = 180.0f; }
            else { center_x = ROUND_CORNER_CENTER_BR_X; center_z = ROUND_CORNER_CENTER_BR_Z; start_angle_deg = 270.0f; }

            float angle_step = DEG_TO_RAD(90.0f) / CORNER_SEGMENTS;
            float start_rad = DEG_TO_RAD(start_angle_deg);

            for (int i = 1; i <= CORNER_SEGMENTS; ++i) { // Start from segment 1
                 float current_angle = start_rad + i * angle_step;
                 float current_x = center_x + innerRailCenterRadius * cosf(current_angle);
                 float current_z = center_z + innerRailCenterRadius * sinf(current_angle);
                 drawWall(prev_x, prev_z, current_x, current_z, railHeight, railThickness);
                 prev_x = current_x;
                 prev_z = current_z;
            }
             if (corner == 0) { prev_x = -ROUND_STRAIGHT_X_LIMIT; prev_z = innerRailYPos; }
             else if (corner == 1) { prev_x = innerRailXNeg; prev_z = ROUND_STRAIGHT_Z_LIMIT; }
             else if (corner == 2) { prev_x = ROUND_STRAIGHT_X_LIMIT; prev_z = innerRailYNeg; }
        }
    } // End TRACK_ROUNDED Guardrails
}


// --- Collision Detection (Conditional) ---
// Checks if the given (x, z) position is within the track boundaries.
// Returns 1 if on track, 0 if off track.
int isPositionOnTrack(float x, float z) {
    if (selectedTrackType == TRACK_RECT) {
        // --- Rectangular Collision ---
        // Define boundaries with collision tolerance
        float outerXPosEps = RECT_OUTER_X_POS + COLLISION_EPSILON;
        float outerXNegEps = RECT_OUTER_X_NEG - COLLISION_EPSILON;
        float outerZPosEps = RECT_OUTER_Z_POS + COLLISION_EPSILON;
        float outerZNegEps = RECT_OUTER_Z_NEG - COLLISION_EPSILON;
        float innerXPosEps = RECT_INNER_X_POS - COLLISION_EPSILON;
        float innerXNegEps = RECT_INNER_X_NEG + COLLISION_EPSILON;
        float innerZPosEps = RECT_INNER_Z_POS - COLLISION_EPSILON;
        float innerZNegEps = RECT_INNER_Z_NEG + COLLISION_EPSILON;

        // Check if outside the outer rectangle
        if (x > outerXPosEps || x < outerXNegEps || z > outerZPosEps || z < outerZNegEps) {
            return 0; // Off track (outside)
        }
        // Check if inside the inner hole rectangle
        if (x < innerXPosEps && x > innerXNegEps && z < innerZPosEps && z > innerZNegEps) {
             return 0; // Off track (inside hole)
        }
        // If not outside outer and not inside inner, must be on track
        return 1; // On track

    } else { // TRACK_ROUNDED
        // --- Rounded Corner Collision ---
        float absX = fabsf(x);
        float absZ = fabsf(z);
        // Calculate squared radii with tolerance for efficient comparison
        float innerRadiusSq = powf(fmaxf(0.0f, ROUND_INNER_CORNER_RADIUS - COLLISION_EPSILON), 2);
        float outerRadiusSq = powf(ROUND_OUTER_CORNER_RADIUS + COLLISION_EPSILON, 2);
        // Calculate half road width with tolerance
        float halfRoadWidthWithEps = ROUND_HALF_ROAD_WIDTH + COLLISION_EPSILON;

        // Check Straight Sections first (more common)
        // Check if within X limits of horizontal straights AND Z is within road width
        if (absX <= ROUND_STRAIGHT_X_LIMIT) {
             if (absZ >= (ROUND_TRACK_MAIN_LENGTH / 2.0f - halfRoadWidthWithEps) &&
                 absZ <= (ROUND_TRACK_MAIN_LENGTH / 2.0f + halfRoadWidthWithEps)) {
                 return 1; // On top or bottom straight
             }
        }
        // Check if within Z limits of vertical straights AND X is within road width
        if (absZ <= ROUND_STRAIGHT_Z_LIMIT) {
             if (absX >= (ROUND_TRACK_MAIN_WIDTH / 2.0f - halfRoadWidthWithEps) &&
                 absX <= (ROUND_TRACK_MAIN_WIDTH / 2.0f + halfRoadWidthWithEps)) {
                 return 1; // On left or right straight
             }
        }

        // Check Corner Sections if not on a straight
        float corner_center_x = 0.0f, corner_center_z = 0.0f;
        int in_corner_zone = 0; // Flag to indicate if in a corner's bounding box

        // Determine which corner zone the point might be in
        if (x > ROUND_STRAIGHT_X_LIMIT && z > ROUND_STRAIGHT_Z_LIMIT) { // Top-Right Zone
            corner_center_x = ROUND_CORNER_CENTER_TR_X; corner_center_z = ROUND_CORNER_CENTER_TR_Z; in_corner_zone = 1;
        } else if (x < -ROUND_STRAIGHT_X_LIMIT && z > ROUND_STRAIGHT_Z_LIMIT) { // Top-Left Zone
            corner_center_x = ROUND_CORNER_CENTER_TL_X; corner_center_z = ROUND_CORNER_CENTER_TL_Z; in_corner_zone = 1;
        } else if (x < -ROUND_STRAIGHT_X_LIMIT && z < -ROUND_STRAIGHT_Z_LIMIT) { // Bottom-Left Zone
            corner_center_x = ROUND_CORNER_CENTER_BL_X; corner_center_z = ROUND_CORNER_CENTER_BL_Z; in_corner_zone = 1;
        } else if (x > ROUND_STRAIGHT_X_LIMIT && z < -ROUND_STRAIGHT_Z_LIMIT) { // Bottom-Right Zone
            corner_center_x = ROUND_CORNER_CENTER_BR_X; corner_center_z = ROUND_CORNER_CENTER_BR_Z; in_corner_zone = 1;
        }

        // If potentially in a corner zone, check distance from corner center
        if (in_corner_zone) {
            float dx = x - corner_center_x;
            float dz = z - corner_center_z;
            float dist_sq = dx * dx + dz * dz; // Squared distance

            // Check if distance is between inner and outer radii (using squared values)
            if (dist_sq >= innerRadiusSq && dist_sq <= outerRadiusSq) {
                return 1; // On track (in corner)
            }
        }

        // If none of the conditions above were met, the point is off-track
        return 0; // Off track
    }
}