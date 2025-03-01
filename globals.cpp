#include "globals.hpp"

float camX = 0.0f;
float camY = 2.0f;
float camZ = 12.0f;
float lookX = 0.0f;
float lookY = 1.0f;
float lookZ = 0.0f;

int selectedBoxer = 0;
float punchAngleUpper1 = 0.0f;
float punchAngleFore1 = 0.0f;
float punchAnglePalm1 = 0.0f;
bool punching1 = false;
bool punchForward1 = true;

float armRotation = 0.0f;
float bicepRotation = 0.0f;
float handRotation = 0.0f;
bool punching2 = false;
bool punchForward2 = true;

const float maxUpper = 60.0f;
const float maxFore  = 120.0f;
const float maxPalm  = 120.0f;

float hitTimer1 = 0.0f;
float hitTimer2 = 0.0f;

float modelOffsetX = -0.4f;
float modelOffsetY = -1.2f;
float modelOffsetZ = 0.0f;
float modelRotationX = 0.0f;
float modelRotationY = 90.0f;
float modelRotationZ = 0.0f;
float modelScale = 2.3f;
