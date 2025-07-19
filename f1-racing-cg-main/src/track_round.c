#include "track_round.h" // Specific header for this track
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>

// Define DEG_TO_RAD locally if not available globally
#ifndef DEG_TO_RAD
#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0f)
#endif

// --- Rounded Corner Helpers (Local to this file) ---
void renderCornerLineSegmentRound(float center_x, float center_z, float radius, float start_angle_deg, int num_segments, float y_level) {
    float angle_step = DEG_TO_RAD(90.0f) / num_segments;
    float start_rad = DEG_TO_RAD(start_angle_deg);
    for (int i = 0; i <= num_segments; ++i) {
        float current_angle = start_rad + i * angle_step;
        glVertex3f(center_x + radius * cosf(current_angle), y_level, center_z + radius * sinf(current_angle));
    }
}

void renderCornerSurfaceSegmentRound(float center_x, float center_z, float inner_rad, float outer_rad, float start_angle_deg, int num_segments, float surface_y) {
     float angle_step = DEG_TO_RAD(90.0f) / num_segments;
     float start_rad = DEG_TO_RAD(start_angle_deg);
     for (int i = 0; i <= num_segments; ++i) {
         float current_angle = start_rad + i * angle_step;
         float cos_a = cosf(current_angle); float sin_a = sinf(current_angle);
         glVertex3f(center_x + inner_rad * cos_a, surface_y, center_z + inner_rad * sin_a);
         glVertex3f(center_x + outer_rad * cos_a, surface_y, center_z + outer_rad * sin_a);
     }
}

// --- Wall Drawing Helper (Also needed here for guardrails) ---
void drawWallRound(float x1, float z1, float x2, float z2, float height, float thickness) {
    float dx=x2-x1; float dz=z2-z1; float len=sqrtf(dx*dx+dz*dz); if(len<0.001f) return;
    float nx=dx/len; float nz=dz/len; float px=-nz; float pz=nx; float half_thick=thickness/2.0f;
    float v[8][3]={ {x1-px*half_thick,0.0f,z1-pz*half_thick},{x1+px*half_thick,0.0f,z1+pz*half_thick},{x2+px*half_thick,0.0f,z2+pz*half_thick},{x2-px*half_thick,0.0f,z2-pz*half_thick}, {x1-px*half_thick,height,z1-pz*half_thick},{x1+px*half_thick,height,z1+pz*half_thick},{x2+px*half_thick,height,z2+pz*half_thick},{x2-px*half_thick,height,z2-pz*half_thick} };
    glBegin(GL_QUADS);
    glVertex3fv(v[4]);glVertex3fv(v[5]);glVertex3fv(v[6]);glVertex3fv(v[7]); // Top
    glVertex3fv(v[0]);glVertex3fv(v[3]);glVertex3fv(v[7]);glVertex3fv(v[4]); // Front
    glVertex3fv(v[1]);glVertex3fv(v[5]);glVertex3fv(v[6]);glVertex3fv(v[2]); // Back
    glVertex3fv(v[0]);glVertex3fv(v[4]);glVertex3fv(v[5]);glVertex3fv(v[1]); // Left
    glVertex3fv(v[3]);glVertex3fv(v[2]);glVertex3fv(v[6]);glVertex3fv(v[7]); // Right
    glEnd();
}

// --- Rounded Track Rendering ---
void renderRoundTrack() {
    float surface_y = 0.0f;
    float line_y = 0.01f;
    float finish_y = 0.02f;
    int straight_segments = 10; // Number of quads per straight section

    // --- Render Ground Plane ---
    glColor3f(0.2f, 0.6f, 0.2f); // Grassy Green
    glBegin(GL_QUADS);
        float groundSize = fmaxf(ROUND_TRACK_MAIN_WIDTH, ROUND_TRACK_MAIN_LENGTH) * 1.2f;
        glVertex3f(-groundSize, -0.02f, -groundSize); glVertex3f(-groundSize, -0.02f,  groundSize);
        glVertex3f( groundSize, -0.02f,  groundSize); glVertex3f( groundSize, -0.02f, -groundSize);
    glEnd();

    // --- Render Track Surface (Asphalt Grey) ---
    glColor3f(0.4f, 0.4f, 0.45f);
    glBegin(GL_QUAD_STRIP);

        // 1. Right Straight (Start point: Bottom Right Straight Start)
        for(int i = 0; i <= straight_segments; ++i) {
            float t = (float)i / straight_segments;
            float z = -ROUND_STRAIGHT_Z_LIMIT + (ROUND_STRAIGHT_Z_LIMIT - (-ROUND_STRAIGHT_Z_LIMIT)) * t;
            float inner_x = ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH;
            float outer_x = ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH;
            glVertex3f(inner_x, surface_y, z); glVertex3f(outer_x, surface_y, z);
        }
        // 2. Top Right Corner
        renderCornerSurfaceSegmentRound(ROUND_CORNER_CENTER_TR_X, ROUND_CORNER_CENTER_TR_Z, ROUND_INNER_CORNER_RADIUS, ROUND_OUTER_CORNER_RADIUS, 0.0f, CORNER_SEGMENTS, surface_y);
        // 3. Top Straight
         for(int i = 0; i <= straight_segments; ++i) {
            float t = (float)i / straight_segments;
            float x = ROUND_STRAIGHT_X_LIMIT - (ROUND_STRAIGHT_X_LIMIT - (-ROUND_STRAIGHT_X_LIMIT)) * t;
            float inner_z = ROUND_TRACK_MAIN_LENGTH / 2.0f - ROUND_HALF_ROAD_WIDTH;
            float outer_z = ROUND_TRACK_MAIN_LENGTH / 2.0f + ROUND_HALF_ROAD_WIDTH;
             glVertex3f(x, surface_y, inner_z); glVertex3f(x, surface_y, outer_z);
         }
        // 4. Top Left Corner
        renderCornerSurfaceSegmentRound(ROUND_CORNER_CENTER_TL_X, ROUND_CORNER_CENTER_TL_Z, ROUND_INNER_CORNER_RADIUS, ROUND_OUTER_CORNER_RADIUS, 90.0f, CORNER_SEGMENTS, surface_y);
        // 5. Left Straight
        for(int i = 0; i <= straight_segments; ++i) {
            float t = (float)i / straight_segments;
            float z = ROUND_STRAIGHT_Z_LIMIT - (ROUND_STRAIGHT_Z_LIMIT - (-ROUND_STRAIGHT_Z_LIMIT)) * t;
            float inner_x = -ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH;
            float outer_x = -ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH;
             glVertex3f(inner_x, surface_y, z); glVertex3f(outer_x, surface_y, z); // Swapped order? Check winding. Let's keep consistent: Inner first.
             // Test: Should be Inner (-width - half_road), Outer (-width + half_road)
             //glVertex3f(-ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH, surface_y, z);
             //glVertex3f(-ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH, surface_y, z);

         }
        // 6. Bottom Left Corner
        renderCornerSurfaceSegmentRound(ROUND_CORNER_CENTER_BL_X, ROUND_CORNER_CENTER_BL_Z, ROUND_INNER_CORNER_RADIUS, ROUND_OUTER_CORNER_RADIUS, 180.0f, CORNER_SEGMENTS, surface_y);
        // 7. Bottom Straight
         for(int i = 0; i <= straight_segments; ++i) {
            float t = (float)i / straight_segments;
            float x = -ROUND_STRAIGHT_X_LIMIT + (ROUND_STRAIGHT_X_LIMIT - (-ROUND_STRAIGHT_X_LIMIT)) * t;
            float inner_z = -ROUND_TRACK_MAIN_LENGTH / 2.0f - ROUND_HALF_ROAD_WIDTH;
            float outer_z = -ROUND_TRACK_MAIN_LENGTH / 2.0f + ROUND_HALF_ROAD_WIDTH;
             glVertex3f(x, surface_y, inner_z); glVertex3f(x, surface_y, outer_z);
         }
        // 8. Bottom Right Corner
        renderCornerSurfaceSegmentRound(ROUND_CORNER_CENTER_BR_X, ROUND_CORNER_CENTER_BR_Z, ROUND_INNER_CORNER_RADIUS, ROUND_OUTER_CORNER_RADIUS, 270.0f, CORNER_SEGMENTS, surface_y);
        // 9. Close Loop by repeating the first vertex pair of the Right Straight
         float z_start_right_straight = -ROUND_STRAIGHT_Z_LIMIT;
         float inner_x_start_right = ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH;
         float outer_x_start_right = ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH;
         glVertex3f(inner_x_start_right, surface_y, z_start_right_straight);
         glVertex3f(outer_x_start_right, surface_y, z_start_right_straight);
    glEnd();

    // --- Render Track Markings ---
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
     // Outer boundary
    glBegin(GL_LINE_STRIP);
        glVertex3f( ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH, line_y, ROUND_STRAIGHT_Z_LIMIT);
        renderCornerLineSegmentRound(ROUND_CORNER_CENTER_TR_X, ROUND_CORNER_CENTER_TR_Z, ROUND_OUTER_CORNER_RADIUS, 0.0f, CORNER_SEGMENTS, line_y);
        renderCornerLineSegmentRound(ROUND_CORNER_CENTER_TL_X, ROUND_CORNER_CENTER_TL_Z, ROUND_OUTER_CORNER_RADIUS, 90.0f, CORNER_SEGMENTS, line_y);
        renderCornerLineSegmentRound(ROUND_CORNER_CENTER_BL_X, ROUND_CORNER_CENTER_BL_Z, ROUND_OUTER_CORNER_RADIUS, 180.0f, CORNER_SEGMENTS, line_y);
        renderCornerLineSegmentRound(ROUND_CORNER_CENTER_BR_X, ROUND_CORNER_CENTER_BR_Z, ROUND_OUTER_CORNER_RADIUS, 270.0f, CORNER_SEGMENTS, line_y);
        glVertex3f(ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH, line_y, ROUND_STRAIGHT_Z_LIMIT);
    glEnd();
     // Inner boundary
    glBegin(GL_LINE_STRIP);
        glVertex3f(ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH, line_y, ROUND_STRAIGHT_Z_LIMIT);
        renderCornerLineSegmentRound(ROUND_CORNER_CENTER_TR_X, ROUND_CORNER_CENTER_TR_Z, ROUND_INNER_CORNER_RADIUS, 0.0f, CORNER_SEGMENTS, line_y);
        renderCornerLineSegmentRound(ROUND_CORNER_CENTER_TL_X, ROUND_CORNER_CENTER_TL_Z, ROUND_INNER_CORNER_RADIUS, 90.0f, CORNER_SEGMENTS, line_y);
        renderCornerLineSegmentRound(ROUND_CORNER_CENTER_BL_X, ROUND_CORNER_CENTER_BL_Z, ROUND_INNER_CORNER_RADIUS, 180.0f, CORNER_SEGMENTS, line_y);
        renderCornerLineSegmentRound(ROUND_CORNER_CENTER_BR_X, ROUND_CORNER_CENTER_BR_Z, ROUND_INNER_CORNER_RADIUS, 270.0f, CORNER_SEGMENTS, line_y);
        glVertex3f(ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH, line_y, ROUND_STRAIGHT_Z_LIMIT);
    glEnd();

    // --- Finish line ---
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
        float finishLineXStart = ROUND_FINISH_LINE_X_START;
        float finishLineXEnd = ROUND_FINISH_LINE_X_END;
        float finishLineZPos = FINISH_LINE_Z + ROUND_FINISH_LINE_THICKNESS / 2.0f;
        float finishLineZNeg = FINISH_LINE_Z - ROUND_FINISH_LINE_THICKNESS / 2.0f;
        glVertex3f(finishLineXStart, finish_y, finishLineZPos); glVertex3f(finishLineXEnd, finish_y, finishLineZPos);
        glVertex3f(finishLineXEnd, finish_y, finishLineZNeg); glVertex3f(finishLineXStart, finish_y, finishLineZNeg);
    glEnd();

    glLineWidth(1.0f);
}

// --- Rounded Guardrail Rendering ---
void renderRoundGuardrails() {
    float railHeight = 0.8f;
    float railThickness = 0.4f;
    float margin = 0.15f;
    float base_y = 0.0f;
    float top_y = railHeight;
    glColor3f(0.8f, 0.1f, 0.1f);

    // --- Draw Straight Sections using drawWallRound ---
    float outerRailYPos = ROUND_TRACK_MAIN_LENGTH / 2.0f + ROUND_HALF_ROAD_WIDTH + margin;
    float outerRailYNeg = -outerRailYPos;
    float innerRailYPos = ROUND_TRACK_MAIN_LENGTH / 2.0f - ROUND_HALF_ROAD_WIDTH - margin;
    float innerRailYNeg = -innerRailYPos;
    float outerRailXPos = ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH + margin;
    float outerRailXNeg = -outerRailXPos;
    float innerRailXPos = ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH - margin;
    float innerRailXNeg = -innerRailXPos;

    drawWallRound(-ROUND_STRAIGHT_X_LIMIT, outerRailYPos,  ROUND_STRAIGHT_X_LIMIT, outerRailYPos, railHeight, railThickness); // Top Outer
    drawWallRound(-ROUND_STRAIGHT_X_LIMIT, outerRailYNeg,  ROUND_STRAIGHT_X_LIMIT, outerRailYNeg, railHeight, railThickness); // Bottom Outer
    drawWallRound( outerRailXPos, -ROUND_STRAIGHT_Z_LIMIT, outerRailXPos,  ROUND_STRAIGHT_Z_LIMIT, railHeight, railThickness); // Right Outer
    drawWallRound( outerRailXNeg, -ROUND_STRAIGHT_Z_LIMIT, outerRailXNeg,  ROUND_STRAIGHT_Z_LIMIT, railHeight, railThickness); // Left Outer
    drawWallRound(-ROUND_STRAIGHT_X_LIMIT, innerRailYPos,  ROUND_STRAIGHT_X_LIMIT, innerRailYPos, railHeight, railThickness); // Top Inner
    drawWallRound(-ROUND_STRAIGHT_X_LIMIT, innerRailYNeg,  ROUND_STRAIGHT_X_LIMIT, innerRailYNeg, railHeight, railThickness); // Bottom Inner
    drawWallRound( innerRailXPos, -ROUND_STRAIGHT_Z_LIMIT, innerRailXPos,  ROUND_STRAIGHT_Z_LIMIT, railHeight, railThickness); // Right Inner
    drawWallRound( innerRailXNeg, -ROUND_STRAIGHT_Z_LIMIT, innerRailXNeg,  ROUND_STRAIGHT_Z_LIMIT, railHeight, railThickness); // Left Inner


    // --- Draw Curved Sections as Segmented Walls ---
    float outerRailCenterRadius = ROUND_OUTER_CORNER_RADIUS + margin;
    float innerRailCenterRadius = fmaxf(0.1f + railThickness/2.0f, ROUND_INNER_CORNER_RADIUS - margin);

    // Outer Curved Rails
    float prev_x_out = ROUND_TRACK_MAIN_WIDTH / 2.0f + ROUND_HALF_ROAD_WIDTH + margin; // Start at end of right straight
    float prev_z_out = ROUND_STRAIGHT_Z_LIMIT;
    // Inner Curved Rails
    float prev_x_in = ROUND_TRACK_MAIN_WIDTH / 2.0f - ROUND_HALF_ROAD_WIDTH - margin; // Start at end of right straight
    float prev_z_in = ROUND_STRAIGHT_Z_LIMIT;

    for (int corner = 0; corner < 4; ++ corner) {
        float center_x, center_z, start_angle_deg;
        // Determine corner center and start angle
        if (corner == 0) { center_x = ROUND_CORNER_CENTER_TR_X; center_z = ROUND_CORNER_CENTER_TR_Z; start_angle_deg = 0.0f; }
        else if (corner == 1) { center_x = ROUND_CORNER_CENTER_TL_X; center_z = ROUND_CORNER_CENTER_TL_Z; start_angle_deg = 90.0f; }
        else if (corner == 2) { center_x = ROUND_CORNER_CENTER_BL_X; center_z = ROUND_CORNER_CENTER_BL_Z; start_angle_deg = 180.0f; }
        else { center_x = ROUND_CORNER_CENTER_BR_X; center_z = ROUND_CORNER_CENTER_BR_Z; start_angle_deg = 270.0f; }

        float angle_step = DEG_TO_RAD(90.0f) / CORNER_SEGMENTS;
        float start_rad = DEG_TO_RAD(start_angle_deg);

        // Draw segments for this corner
        for (int i = 1; i <= CORNER_SEGMENTS; ++i) {
             float current_angle = start_rad + i * angle_step;
             // Outer wall segment
             float current_x_out = center_x + outerRailCenterRadius * cosf(current_angle);
             float current_z_out = center_z + outerRailCenterRadius * sinf(current_angle);
             drawWallRound(prev_x_out, prev_z_out, current_x_out, current_z_out, railHeight, railThickness);
             prev_x_out = current_x_out;
             prev_z_out = current_z_out;
             // Inner wall segment
             float current_x_in = center_x + innerRailCenterRadius * cosf(current_angle);
             float current_z_in = center_z + innerRailCenterRadius * sinf(current_angle);
             drawWallRound(prev_x_in, prev_z_in, current_x_in, current_z_in, railHeight, railThickness);
             prev_x_in = current_x_in;
             prev_z_in = current_z_in;
        }
        // Update previous points to the start of the *next* straight section after the corner
        if (corner == 0) { // After TR corner -> Top Straight start
             prev_x_out = -ROUND_STRAIGHT_X_LIMIT; prev_z_out = outerRailYPos;
             prev_x_in = -ROUND_STRAIGHT_X_LIMIT; prev_z_in = innerRailYPos;
        } else if (corner == 1) { // After TL corner -> Left Straight start
             prev_x_out = outerRailXNeg; prev_z_out = ROUND_STRAIGHT_Z_LIMIT;
             prev_x_in = innerRailXNeg; prev_z_in = ROUND_STRAIGHT_Z_LIMIT;
        } else if (corner == 2) { // After BL corner -> Bottom Straight start
             prev_x_out = ROUND_STRAIGHT_X_LIMIT; prev_z_out = outerRailYNeg;
             prev_x_in = ROUND_STRAIGHT_X_LIMIT; prev_z_in = innerRailYNeg;
        } // After BR corner, prev points automatically match start of Right Straight
    }
}


// --- Rounded Collision Detection ---
int isPositionOnRoundTrack(float x, float z) {
    float absX = fabsf(x);
    float absZ = fabsf(z);
    float innerRadiusSq = powf(fmaxf(0.0f, ROUND_INNER_CORNER_RADIUS - COLLISION_EPSILON), 2);
    float outerRadiusSq = powf(ROUND_OUTER_CORNER_RADIUS + COLLISION_EPSILON, 2);
    float halfRoadWidthWithEps = ROUND_HALF_ROAD_WIDTH + COLLISION_EPSILON;

    // Check Straight Sections
    if (absX <= ROUND_STRAIGHT_X_LIMIT) {
         if (absZ >= (ROUND_TRACK_MAIN_LENGTH / 2.0f - halfRoadWidthWithEps) && absZ <= (ROUND_TRACK_MAIN_LENGTH / 2.0f + halfRoadWidthWithEps)) return 1;
    }
    if (absZ <= ROUND_STRAIGHT_Z_LIMIT) {
         if (absX >= (ROUND_TRACK_MAIN_WIDTH / 2.0f - halfRoadWidthWithEps) && absX <= (ROUND_TRACK_MAIN_WIDTH / 2.0f + halfRoadWidthWithEps)) return 1;
    }

    // Check Corner Sections
    float corner_center_x = 0.0f, corner_center_z = 0.0f; int in_corner_zone = 0;
    if (x > ROUND_STRAIGHT_X_LIMIT && z > ROUND_STRAIGHT_Z_LIMIT) { corner_center_x = ROUND_CORNER_CENTER_TR_X; corner_center_z = ROUND_CORNER_CENTER_TR_Z; in_corner_zone = 1; }
    else if (x < -ROUND_STRAIGHT_X_LIMIT && z > ROUND_STRAIGHT_Z_LIMIT) { corner_center_x = ROUND_CORNER_CENTER_TL_X; corner_center_z = ROUND_CORNER_CENTER_TL_Z; in_corner_zone = 1; }
    else if (x < -ROUND_STRAIGHT_X_LIMIT && z < -ROUND_STRAIGHT_Z_LIMIT) { corner_center_x = ROUND_CORNER_CENTER_BL_X; corner_center_z = ROUND_CORNER_CENTER_BL_Z; in_corner_zone = 1; }
    else if (x > ROUND_STRAIGHT_X_LIMIT && z < -ROUND_STRAIGHT_Z_LIMIT) { corner_center_x = ROUND_CORNER_CENTER_BR_X; corner_center_z = ROUND_CORNER_CENTER_BR_Z; in_corner_zone = 1; }

    if (in_corner_zone) {
        float dx = x - corner_center_x; float dz = z - corner_center_z;
        float dist_sq = dx * dx + dz * dz;
        if (dist_sq >= innerRadiusSq && dist_sq <= outerRadiusSq) return 1;
    }
    return 0; // Off track
}