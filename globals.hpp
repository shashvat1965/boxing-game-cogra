//------------------------------------------------------
// Global Variables for Camera, Selection & Animation
//------------------------------------------------------
#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <GL/freeglut.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <map>
#include <png.h>
#include <vector>

// Camera parameters
extern float camX;
extern float camY;
extern float camZ;
extern float lookX;
extern float lookY;
extern float lookZ;

// Boxer selection and animation parameters
extern int selectedBoxer;
extern float punchAngleUpper1;
extern float punchAngleFore1;
extern float punchAnglePalm1;
extern bool punching1;
extern bool punchForward1;
extern float armRotation;
extern float bicepRotation;
extern float handRotation;
extern bool punching2;
extern bool punchForward2;
extern const float maxUpper;
extern const float maxFore;
extern const float maxPalm;
extern float hitTimer1;
extern float hitTimer2;

// Model transformation variables
extern float modelOffsetX;
extern float modelOffsetY;
extern float modelOffsetZ;
extern float modelRotationX;
extern float modelRotationY;
extern float modelRotationZ;
extern float modelScale;

extern const aiScene* scene;

// Declare helper functions to be used in BetterBoxer.cpp
void customRotatef(float angle, float axisX, float axisY, float axisZ);
void customTranslatef(float x, float y, float z);

void drawBoxer(int boxerID);
bool loadBoxerFromFile(const std::string& filename);

void loadModel();
void renderNode(aiNode* node);

#endif // GLOBALS_HPP