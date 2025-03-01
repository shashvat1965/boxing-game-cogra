#include "globals.hpp"
#include <fstream>
#include <sstream>
#include <vector>

//------------------------------------------------------
// Data Structure for Body Parts
//------------------------------------------------------
struct BodyPart {
    std::string name;        
    std::string shape;       // "cylinder", "sphere", or "cube"
    float posX, posY, posZ;   
    float scaleX, scaleY, scaleZ; // For cylinders: scaleX = radius, scaleY = height; for spheres: use scaleX as radius; for cubes: dimensions.
    float colorR, colorG, colorB; 
};

std::vector<BodyPart> g_baseParts; // Loaded from file

//------------------------------------------------------
// Shape Drawing Functions
//------------------------------------------------------
void drawCylinder(float radius, float height) {
    GLUquadric* quad = gluNewQuadric();
    glPushMatrix();
        customRotatef(-90.0f, 1, 0, 0);  // Align along Y-axis
        gluCylinder(quad, radius, radius, height, 20, 20);
        // Bottom cap
        gluDisk(quad, 0.0, radius, 20, 1);
        // Top cap
        customTranslatef(0.0f, 0.0f, height);
        gluDisk(quad, 0.0, radius, 20, 1);
    glPopMatrix();
    gluDeleteQuadric(quad);
}

void drawSphere(float radius) {
    glutSolidSphere(radius, 20, 20);
}

void drawCube(float sizeX, float sizeY, float sizeZ) {
    glPushMatrix();
        glScalef(sizeX, sizeY, sizeZ);
        glutSolidCube(1.0);
    glPopMatrix();
}

//------------------------------------------------------
// Draw a Single Body Part for a Given Boxer
//------------------------------------------------------
// For right arm parts, we apply different rotations if this boxer is selected for punching.
void drawPartForBoxer(const BodyPart& part, int boxerID) {
    // If the boxer is hit, override the part's color to red.
    if ((boxerID == 1 && hitTimer1 > 0.0f) || (boxerID == 2 && hitTimer2 > 0.0f)) {
        glColor3f(1.0f, 0.0f, 0.0f); // red tint
    } else {
        glColor3f(part.colorR, part.colorG, part.colorB);
    }
    
    glPushMatrix();
    // For right arm parts, if this boxer is the selected one, apply the punching transformation.
    if ((part.name == "rightArmUpper" || part.name == "rightArmFore" || part.name == "rightPalm") &&
         (boxerID == selectedBoxer))
    {
        // Translate to the shoulder pivot (assumed at (0.5, 1.0, 0) in the base model)
        customTranslatef(0.5f, 1.0f, 0.0f);
        if (part.name == "rightArmUpper") {
            if (boxerID == 1)
                customRotatef(-punchAngleUpper1, 1.0f, 0.0f, 0.0f);
        } else if (part.name == "rightArmFore") {
            if (boxerID == 1)
                customRotatef(-punchAngleFore1, 1.0f, 0.0f, 0.0f);
        } else if (part.name == "rightPalm") {
            if (boxerID == 1)
                customRotatef(-punchAnglePalm1, 1.0f, 0.0f, 0.0f);
        }
        // Translate back to the part’s relative position
        customTranslatef(part.posX - 0.5f, part.posY - 1.0f, part.posZ);
    } else {
        customTranslatef(part.posX, part.posY, part.posZ);
    }
    
    if (part.shape == "cylinder") {
        drawCylinder(part.scaleX, part.scaleY);
    } else if (part.shape == "sphere") {
        drawSphere(part.scaleX);
    } else if (part.shape == "cube") {
        drawCube(part.scaleX, part.scaleY, part.scaleZ);
    } else {
        std::cerr << "Unknown shape: " << part.shape << std::endl;
    }
    glPopMatrix();
}

//------------------------------------------------------
// Draw a Boxer Given Its ID
//------------------------------------------------------
// Position and orient the boxers so they face each other.
// Boxer 1: placed at (-2,0,0), rotated so base +Z becomes +X.
// Boxer 2: placed at (2,0,0), rotated so base +Z becomes -X.
void drawBoxer(int boxerID) {
    glPushMatrix();
        if (boxerID == 1) {
            customTranslatef(0.4f, 0.0f, 0.0f);
            customRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        } else if (boxerID == 2) {
            customTranslatef(-0.4f, 0.0f, 0.0f);
            customRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        }
    
        for (const auto& part : g_baseParts) {
            drawPartForBoxer(part, boxerID);
        }
    glPopMatrix();
}

//------------------------------------------------------
// Load Boxer Base Parts from a Text File
//------------------------------------------------------
bool loadBoxerFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()){
        std::cerr << "Error: Could not open " << filename << std::endl;
        return false;
    }
    g_baseParts.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0]=='#')
            continue;
        BodyPart part;
        std::istringstream iss(line);
        iss >> part.name >> part.shape 
            >> part.posX >> part.posY >> part.posZ 
            >> part.scaleX >> part.scaleY >> part.scaleZ 
            >> part.colorR >> part.colorG >> part.colorB;
        if (!iss.fail())
            g_baseParts.push_back(part);
    }
    file.close();
    return true;
}