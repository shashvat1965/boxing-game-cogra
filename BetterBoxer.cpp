// #define GL_SILENCE_DEPRECATION
#include "helpers.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

//------------------------------------------------------
// 1. Data Structure for Body Parts
//------------------------------------------------------
struct BodyPart {
    std::string name;        // e.g., "body", "head", "rightArmUpper", etc.
    std::string shape;       // "cylinder", "sphere", or "cube"
    float posX, posY, posZ;   // Relative position (base model)
    float scaleX, scaleY, scaleZ; // For cylinders: scaleX = radius, scaleY = height; for spheres: use scaleX as radius; for cubes: dimensions.
    float colorR, colorG, colorB; // Base color components
};

std::vector<BodyPart> g_baseParts; // Loaded from file

// Model Variables
const char* objFilePath = "./boxer_real.obj";
const aiScene* scene;
Assimp::Importer importer;

// Model transformation variables
float modelOffsetX = -0.5f;
float modelOffsetY = -1.2f;
float modelOffsetZ = 0.0f;
float modelRotationX = 0.0f;
float modelRotationY = 90.0f;
float modelRotationZ = 0.0f;
float modelScale = 2.3f;

// Node-specific offset variables
struct NodeOffset {
    float x, y, z;
};

// Define joint pivot offsets for specific nodes
std::map<std::string, NodeOffset> jointOffsets = {
    {"upper_arm_L", {0.2f, 0.5f, 0.0f}},
    {"bicep_L", {0.2f, 0.5f, 0.0f}},
    {"hand_L", {0.2f, 0.5f, 0.0f}}
    // Add more node-specific offsets as needed
};

//------------------------------------------------------
// 2. Global Variables for Camera, Selection & Animation
//------------------------------------------------------
// Camera parameters
float camX = 0.0f, camY = 2.0f, camZ = 12.0f;
float lookX = 0.0f, lookY = 1.0f, lookZ = 0.0f;

// Which boxer is selected for punching (1 or 2); 0 = none
int selectedBoxer = 0;

// For each boxer, we now animate three separate angles for the right arm:
// Upper arm, forearm, and palm.
// Boxer 1:
float punchAngleUpper1 = 0.0f, punchAngleFore1 = 0.0f, punchAnglePalm1 = 0.0f;
bool punching1 = false;
bool punchForward1 = true;
// Boxer 2:
float armRotation = 0.0f;   
float bicepRotation = 0.0f; 
float handRotation = 0.0f;  
bool punching2 = false;
bool punchForward2 = true;

// Maximum rotation angles for each limb (in degrees)
const float maxUpper = 60.0f;
const float maxFore  = 120.0f;
const float maxPalm  = 120.0f;

// Hit timers: when > 0, the boxer is tinted red.
float hitTimer1 = 0.0f;
float hitTimer2 = 0.0f;

void renderNode(aiNode* node);

//------------------------------------------------------
// 3. Shape Drawing Functions
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
// 4. Draw a Single Body Part for a Given Boxer
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
        // Apply a different rotation for each part:
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
    
    // Draw shape based on type
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
// 5. Draw a Boxer Given Its ID
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
// 6. Load Boxer Base Parts from a Text File
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

//------------------------------------------------------
// 7. GLUT Callback Functions
//------------------------------------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ,
              lookX, lookY, lookZ,
              0.0, 1.0, 0.0);
    
    // Draw both boxers.
    drawBoxer(1);
    // drawBoxer(2);
    
    // Apply global model transformations
    glPushMatrix();
    customTranslatef(modelOffsetX, modelOffsetY, modelOffsetZ);
    customRotatef(modelRotationX, 1.0f, 0.0f, 0.0f);
    customRotatef(modelRotationY, 0.0f, 1.0f, 0.0f);
    customRotatef(modelRotationZ, 0.0f, 0.0f, 1.0f);
    glScalef(modelScale, modelScale, modelScale);
    
    // Render the model
    renderNode(scene->mRootNode);
    
    glPopMatrix();
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0)
        h = 1;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w/h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// Keyboard callback: selection, punch trigger, camera movement.
void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case '1':
            selectedBoxer = 1;
            std::cout << "Boxer 1 selected." << std::endl;
            break;
        case '2':
            selectedBoxer = 2;
            std::cout << "Boxer 2 selected." << std::endl;
            break;
        case 'p':
            if (selectedBoxer == 1 && !punching1) {
                punching1 = true;
                punchForward1 = true;
            } else if (selectedBoxer == 2 && !punching2) {
                punching2 = true;
                punchForward2 = true;
            }
            break;
        // Camera movement controls
        case 'w': camZ -= 0.5f; break;
        case 's': camZ += 0.5f; break;
        case 'a': camX -= 0.5f; break;
        case 'd': camX += 0.5f; break;
        case 'q': camY += 0.5f; break;
        case 'e': camY -= 0.5f; break;
        case 27: exit(0); break; // ESC
    }
    glutPostRedisplay();
}

// Timer callback to update punching animation and hit timers.
void onTimer(int value) {
    // Update Boxer 1 punching animation
    if (punching1) {
        if (punchForward1) {
            if (punchAngleUpper1 < maxUpper)
                punchAngleUpper1 += 3.0f;
            if (punchAngleFore1 < maxFore)
                punchAngleFore1 += 5.0f;
            if (punchAnglePalm1 < maxPalm)
                punchAnglePalm1 += 5.0f;
            // When the upper arm reaches maximum, consider the punch extended.
            if (punchAngleUpper1 >= maxUpper) {
                punchForward1 = false;
                if (hitTimer2 <= 0.0f)
                    hitTimer2 = 1.0f; // hit Boxer 2
            }
        } else {
            if (punchAngleUpper1 > 0.0f)
                punchAngleUpper1 -= 3.0f;
            if (punchAngleFore1 > 0.0f)
                punchAngleFore1 -= 5.0f;
            if (punchAnglePalm1 > 0.0f)
                punchAnglePalm1 -= 5.0f;
            if (punchAngleUpper1 <= 0.0f) {
                punchAngleUpper1 = 0.0f;
                punchAngleFore1 = 0.0f;
                punchAnglePalm1 = 0.0f;
                punching1 = false;
                punchForward1 = true;
            }
        }
    }
    // Update Boxer 2 punching animation
    if (punching2) {
        if (punchForward2) {
            armRotation += 2.0f;
            bicepRotation += 2.0f;
            handRotation += 2.0f;
            if (armRotation >= 90.0f) { // Max forward extension
                punchForward2 = false;
                if (hitTimer1 <= 0.0f)
                    hitTimer1 = 1.0f; // hit Boxer 2
            }
        } else {
            armRotation -= 2.0f;
            bicepRotation -= 2.0f;
            handRotation -= 2.0f;
            if (armRotation <= 0.0f) { // Return to starting position
                punchForward2 = true;
                punching2 = false;
            }
        }
    }
    
    // Update hit timers (decrement by approx 0.03 sec per timer call)
    if (hitTimer1 > 0.0f) {
        hitTimer1 -= 0.03f;
        if (hitTimer1 < 0.0f) hitTimer1 = 0.0f;
    }
    if (hitTimer2 > 0.0f) {
        hitTimer2 -= 0.03f;
        if (hitTimer2 < 0.0f) hitTimer2 = 0.0f;
    }
    
    glutPostRedisplay();
    glutTimerFunc(30, onTimer, 0);
}

void setNodeColor(const std::string& nodeName) {
    if (nodeName == "upper_arm_L" || nodeName == "upper_arm_R") glColor3f(1.0f, 0.0f, 0.0f); // Red
    else if (nodeName == "bicep_L" || nodeName == "bicep_R") glColor3f(0.0f, 1.0f, 0.0f); // Green
    else if (nodeName == "hand_L" || nodeName == "hand_R") glColor3f(0.0f, 0.0f, 1.0f); // Blue
    else if (nodeName == "thigh_L" || nodeName == "thigh_R") glColor3f(1.0f, 1.0f, 0.0f); // Yellow
    else if (nodeName == "shin_L" || nodeName == "shin_R") glColor3f(1.0f, 0.5f, 0.0f); // Orange
    else if (nodeName == "foot_L" || nodeName == "foot_R") glColor3f(0.5f, 0.0f, 0.5f); // Purple
    else glColor3f(0.8f, 0.8f, 0.8f); // Default gray
}

// Model Boxer
void renderNode(aiNode* node) {
    std::string nodeName = node->mName.C_Str();
    NodeOffset offset = {0.0f, 0.0f, 0.0f};
    
    // Get node-specific offsets if available
    if (jointOffsets.find(nodeName) != jointOffsets.end()) {
        offset = jointOffsets[nodeName];
    }

    // Rotate from shoulder joint
    if (nodeName == "upper_arm_L") {
        glPushMatrix();
        customTranslatef(offset.x, offset.y, offset.z);  // Move to pivot point
        customRotatef(-armRotation, 0, 1, 0);
        customTranslatef(-offset.x, -offset.y, -offset.z);  // Move back
    }

    // Rotate from elbow joint
    if (nodeName == "bicep_L") {
        glPushMatrix();
        customTranslatef(offset.x, offset.y, offset.z);  // Move to pivot point
        customRotatef(-bicepRotation, 0, 1, 0);  // Rotate around Y axis
        customTranslatef(-offset.x, -offset.y, -offset.z);  // Move back
    }

    // Rotate from wrist joint
    if (nodeName == "hand_L") {
        glPushMatrix();
        customTranslatef(offset.x, offset.y, offset.z);  // Move to pivot point
        customRotatef(-handRotation, 0, 1, 0);
        customTranslatef(-offset.x, -offset.y, -offset.z);  // Move back
    }

    setNodeColor(nodeName); // Apply color

    // Draw mesh
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        glBegin(GL_TRIANGLES);
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                int index = face.mIndices[k];
                aiVector3D vertex = mesh->mVertices[index];
                glVertex3f(vertex.x, vertex.y, vertex.z);
            }
        }
        glEnd();
    }

    // Recursively render children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        renderNode(node->mChildren[i]);
    }

    if (nodeName == "upper_arm_L" || nodeName == "bicep_L" || nodeName == "hand_L") {
        glPopMatrix();
    }
}

void loadModel() {
    scene = importer.ReadFile(objFilePath, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error loading OBJ: " << importer.GetErrorString() << std::endl;
        exit(EXIT_FAILURE);
    }
}

//------------------------------------------------------
// 8. Main Function
//------------------------------------------------------
int main(int argc, char** argv) {
    std::string filename = "boxer.txt";
    if (argc > 1)
        filename = argv[1];
    
    if (!loadBoxerFromFile(filename))
        return 1;
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Two Boxers Facing Each Other – Improved Punch Animation");
    
    glEnable(GL_DEPTH_TEST);

    loadModel();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(30, onTimer, 0);
    
    glutMainLoop();
    return 0;
}
