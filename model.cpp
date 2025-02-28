#include <GL/glew.h>
#include <GL/freeglut.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <cmath>

const char* objFilePath = "./your_modelobj.obj";
const aiScene* scene;
Assimp::Importer importer;

// Punching motion variables
float armRotation = 0.0f;   
float bicepRotation = 0.0f; 
float handRotation = 0.0f;  
bool punching = false;  // Initially stopped
bool extending = true;

// Camera controls
float camX = 0.0f, camY = -1.0f, camZ = 1.5f;
float camSpeed = 0.5f;

// Color mapping for symmetric limbs
void setNodeColor(const std::string& nodeName) {
    if (nodeName == "upper_arm_L" || nodeName == "upper_arm_R") glColor3f(1.0f, 0.0f, 0.0f); // Red
    else if (nodeName == "bicep_L" || nodeName == "bicep_R") glColor3f(0.0f, 1.0f, 0.0f); // Green
    else if (nodeName == "hand_L" || nodeName == "hand_R") glColor3f(0.0f, 0.0f, 1.0f); // Blue
    else if (nodeName == "thigh_L" || nodeName == "thigh_R") glColor3f(1.0f, 1.0f, 0.0f); // Yellow
    else if (nodeName == "shin_L" || nodeName == "shin_R") glColor3f(1.0f, 0.5f, 0.0f); // Orange
    else if (nodeName == "foot_L" || nodeName == "foot_R") glColor3f(0.5f, 0.0f, 0.5f); // Purple
    else glColor3f(0.8f, 0.8f, 0.8f); // Default gray
}

// Draw a sphere at the joint location
void drawJoint(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(1.0f, 0.0f, 1.0f);  // White color for joints
    glutSolidSphere(0.05, 10, 10); // Small sphere for joint
    glPopMatrix();
}

// Fix pivot points for the joints
void renderNode(aiNode* node) {
    std::string nodeName = node->mName.C_Str();

    // Rotate from shoulder joint
    if (nodeName == "upper_arm_L") {
        glPushMatrix();
        glTranslatef(-0.3f, 0.7f, 0);  // Move pivot to shoulder
        // glRotatef(armRotation, 0, 0, 1);
        glTranslatef(0.3f, -0.7f, 0);
        // drawJoint(0.2f, 0.85f, 0); // Shoulder joint
    }

    // Rotate from elbow joint
    if (nodeName == "bicep_L") {
        glPushMatrix();
        glTranslatef(-0.2f, -0.85f, 0);  // Move pivot to elbow
        glRotatef(bicepRotation, 0, -1, 0);
        glTranslatef(0.2f, 0.85f, 0);
        drawJoint(0.2f, 0.85f, 0); // Shoulder joint
    }

    // Rotate from wrist joint
    if (nodeName == "hand_L") {
        glPushMatrix();
        glTranslatef(-0.3f, 0.2f, 0);  // Move pivot to wrist
        // glRotatef(handRotation, 1, 0, 0);
        glTranslatef(0.3f, -0.2f, 0);
        // drawJoint(0.2f, 0.85f, 0); // Shoulder joint
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

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Setup camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)800 / 600, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(camX, camY, camZ, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    renderNode(scene->mRootNode);
    glutSwapBuffers();
}

void updateAnimation(int value) {
    if (punching) {
        if (extending) {
            armRotation += 2.0f;
            bicepRotation += 3.0f;
            handRotation += 4.0f;
            if (armRotation >= 30.0f) { // Max forward extension
                extending = false;
            }
        } else {
            armRotation -= 2.0f;
            bicepRotation -= 3.0f;
            handRotation -= 4.0f;
            if (armRotation <= 0.0f) { // Return to starting position
                extending = true;
            }
        }
        glutPostRedisplay();
    }
    glutTimerFunc(16, updateAnimation, 0); // Keep running smoothly
}

void setup() {
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 1.0f, 0.1f, 10.0f);
    glMatrixMode(GL_MODELVIEW);
}

void handleKeypress(unsigned char key, int x, int y)
{
    if (key == 'w')
        camZ -= camSpeed;
    if (key == 's')
        camZ += camSpeed;
    if (key == 'a')
        camX -= camSpeed;
    if (key == 'd')
        camX += camSpeed;
    if (key == 'q')
        camY -= camSpeed;
    if (key == 'e')
        camY += camSpeed;
    if (key == 'p') {
        punching = !punching; // Toggle animation
        if (punching) glutTimerFunc(16, updateAnimation, 0); // Restart if needed
    }

    glutPostRedisplay();
}

void loadModel() {
    scene = importer.ReadFile(objFilePath, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error loading OBJ: " << importer.GetErrorString() << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Punching Animation with Joints");

    glewInit();
    setup();
    loadModel();
    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeypress);
    glutMainLoop();
    return 0;
}
