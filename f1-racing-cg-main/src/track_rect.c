#include "track_rect.h" // Specific header for this track
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>

// --- Wall Drawing Helper --- (Specific to this file now)
void drawWallRect(float x1, float z1, float x2, float z2, float height, float thickness) {
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

// --- Rectangular Track Rendering ---
void renderRectTrack() {
    float surface_y = 0.0f;
    float line_y = 0.01f;
    float finish_y = 0.02f;

    // --- Render Ground Plane ---
    glColor3f(0.2f, 0.6f, 0.2f); // Grassy Green
    glBegin(GL_QUADS);
        float groundSize = fmaxf(RECT_TRACK_MAIN_WIDTH, RECT_TRACK_MAIN_LENGTH) * 1.2f;
        glVertex3f(-groundSize, -0.02f, -groundSize); glVertex3f(-groundSize, -0.02f,  groundSize);
        glVertex3f( groundSize, -0.02f,  groundSize); glVertex3f( groundSize, -0.02f, -groundSize);
    glEnd();

    // --- Render Track Surface ---
    glColor3f(0.4f, 0.4f, 0.45f); // Asphalt Grey Color

    // Top strip
    glBegin(GL_QUADS);
        glVertex3f(RECT_OUTER_X_NEG, surface_y, RECT_INNER_Z_POS); glVertex3f(RECT_OUTER_X_POS, surface_y, RECT_INNER_Z_POS);
        glVertex3f(RECT_OUTER_X_POS, surface_y, RECT_OUTER_Z_POS); glVertex3f(RECT_OUTER_X_NEG, surface_y, RECT_OUTER_Z_POS);
    glEnd();
    // Bottom strip
    glBegin(GL_QUADS);
        glVertex3f(RECT_OUTER_X_NEG, surface_y, RECT_OUTER_Z_NEG); glVertex3f(RECT_OUTER_X_POS, surface_y, RECT_OUTER_Z_NEG);
        glVertex3f(RECT_OUTER_X_POS, surface_y, RECT_INNER_Z_NEG); glVertex3f(RECT_OUTER_X_NEG, surface_y, RECT_INNER_Z_NEG);
    glEnd();
    // Left strip
    glBegin(GL_QUADS);
        glVertex3f(RECT_OUTER_X_NEG, surface_y, RECT_INNER_Z_NEG); glVertex3f(RECT_INNER_X_NEG, surface_y, RECT_INNER_Z_NEG);
        glVertex3f(RECT_INNER_X_NEG, surface_y, RECT_INNER_Z_POS); glVertex3f(RECT_OUTER_X_NEG, surface_y, RECT_INNER_Z_POS);
    glEnd();
    // Right strip
     glBegin(GL_QUADS);
        glVertex3f(RECT_INNER_X_POS, surface_y, RECT_INNER_Z_NEG); glVertex3f(RECT_OUTER_X_POS, surface_y, RECT_INNER_Z_NEG);
        glVertex3f(RECT_OUTER_X_POS, surface_y, RECT_INNER_Z_POS); glVertex3f(RECT_INNER_X_POS, surface_y, RECT_INNER_Z_POS);
    glEnd();


    // --- Render Track Markings ---
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);

    // Outer boundary
    glBegin(GL_LINE_LOOP);
        glVertex3f(RECT_OUTER_X_NEG, line_y, RECT_OUTER_Z_NEG); glVertex3f(RECT_OUTER_X_POS, line_y, RECT_OUTER_Z_NEG);
        glVertex3f(RECT_OUTER_X_POS, line_y, RECT_OUTER_Z_POS); glVertex3f(RECT_OUTER_X_NEG, line_y, RECT_OUTER_Z_POS);
    glEnd();
    // Inner boundary
    glBegin(GL_LINE_LOOP);
        glVertex3f(RECT_INNER_X_NEG, line_y, RECT_INNER_Z_NEG); glVertex3f(RECT_INNER_X_POS, line_y, RECT_INNER_Z_NEG);
        glVertex3f(RECT_INNER_X_POS, line_y, RECT_INNER_Z_POS); glVertex3f(RECT_INNER_X_NEG, line_y, RECT_INNER_Z_POS);
    glEnd();

    // --- Start/Finish line ---
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
        glVertex3f(RECT_FINISH_LINE_X_START, finish_y, FINISH_LINE_Z + RECT_FINISH_LINE_THICKNESS / 2.0f);
        glVertex3f(RECT_FINISH_LINE_X_END,   finish_y, FINISH_LINE_Z + RECT_FINISH_LINE_THICKNESS / 2.0f);
        glVertex3f(RECT_FINISH_LINE_X_END,   finish_y, FINISH_LINE_Z - RECT_FINISH_LINE_THICKNESS / 2.0f);
        glVertex3f(RECT_FINISH_LINE_X_START, finish_y, FINISH_LINE_Z - RECT_FINISH_LINE_THICKNESS / 2.0f);
    glEnd();

    glLineWidth(1.0f); // Reset
}

// --- Rectangular Guardrail Rendering ---
void renderRectGuardrails() {
    float railHeight = 0.8f;
    float railThickness = 0.4f;
    float margin = 0.15f; // How far outside the track lines
    glColor3f(0.8f, 0.1f, 0.1f); // Red

    // Outer Guardrail coordinates
    float ox1=RECT_OUTER_X_NEG-margin; float oz1=RECT_OUTER_Z_NEG-margin;
    float ox2=RECT_OUTER_X_POS+margin; float oz2=RECT_OUTER_Z_NEG-margin;
    float ox3=RECT_OUTER_X_POS+margin; float oz3=RECT_OUTER_Z_POS+margin;
    float ox4=RECT_OUTER_X_NEG-margin; float oz4=RECT_OUTER_Z_POS+margin;
    // Draw outer walls
    drawWallRect(ox1, oz1, ox2, oz2, railHeight, railThickness); // Bottom
    drawWallRect(ox2, oz2, ox3, oz3, railHeight, railThickness); // Right
    drawWallRect(ox3, oz3, ox4, oz4, railHeight, railThickness); // Top
    drawWallRect(ox4, oz4, ox1, oz1, railHeight, railThickness); // Left

    // Inner Guardrail coordinates
    float ix1=RECT_INNER_X_NEG+margin; float iz1=RECT_INNER_Z_NEG+margin;
    float ix2=RECT_INNER_X_POS-margin; float iz2=RECT_INNER_Z_NEG+margin;
    float ix3=RECT_INNER_X_POS-margin; float iz3=RECT_INNER_Z_POS-margin;
    float ix4=RECT_INNER_X_NEG+margin; float iz4=RECT_INNER_Z_POS-margin;
     // Draw inner walls
    drawWallRect(ix1, iz1, ix2, iz2, railHeight, railThickness); // Bottom
    drawWallRect(ix2, iz2, ix3, iz3, railHeight, railThickness); // Right
    drawWallRect(ix3, iz3, ix4, iz4, railHeight, railThickness); // Top
    drawWallRect(ix4, iz4, ix1, iz1, railHeight, railThickness); // Left
}

// --- Rectangular Collision Detection ---
int isPositionOnRectTrack(float x, float z) {
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
    if (x > outerXPosEps || x < outerXNegEps || z > outerZPosEps || z < outerZNegEps) return 0;
    // Check if inside the inner hole rectangle
    if (x < innerXPosEps && x > innerXNegEps && z < innerZPosEps && z > innerZNegEps) return 0;
    // Otherwise, it's on the track
    return 1;
}