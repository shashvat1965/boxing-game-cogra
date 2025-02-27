#define  GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

//------------------------------------------------------
// 1. Data Structure for Body Parts
//------------------------------------------------------
struct BodyPart {
    std::string name;
    std::string shape;  // "cylinder", "sphere", "cube"
    float posX, posY, posZ;         // Position
    float scaleX, scaleY, scaleZ;    // For cylinders: scaleX = radius, scaleY = height; for spheres: use scaleX as radius; for cubes: final dimensions.
    float colorR, colorG, colorB;    // RGB color
};

std::vector<BodyPart> g_parts;

//------------------------------------------------------
// 2. Global Variables for Camera and Animation
//------------------------------------------------------
float camX = 0.0f, camY = 1.0f, camZ = 6.0f;
float lookX = 0.0f, lookY = 0.0f, lookZ = 0.0f;

// Punch animation globals
float punchAngle = 0.0f;
bool punching = false;
bool punchForward = true;

//------------------------------------------------------
// 3. Shape Drawing Functions
//------------------------------------------------------
void drawCylinder(float radius, float height) {
    GLUquadric* quad = gluNewQuadric();
    glPushMatrix();
        glRotatef(-90.0f, 1, 0, 0);  // Align cylinder along Y-axis
        gluCylinder(quad, radius, radius, height, 20, 20);
        // Draw bottom cap
        gluDisk(quad, 0.0, radius, 20, 1);
        // Draw top cap
        glTranslatef(0.0f, 0.0f, height);
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
// 4. Draw a Single Body Part (with punching for right arm parts)
//------------------------------------------------------
void drawPart(const BodyPart& part) {
    glColor3f(part.colorR, part.colorG, part.colorB);
    glPushMatrix();
        // If this part is in the right arm, apply the punching transformation.
        // We assume the shoulder (pivot) is at (0.5, 1.0, 0).
        if (part.name == "rightArmUpper" || part.name == "rightArmFore" || part.name == "rightPalm") {
            // Translate to the pivot point
            glTranslatef(0.5f, 1.0f, 0.0f);
            // Apply the punch rotation about the Z-axis
            glRotatef(-punchAngle, 1.0f, 0.0f, 0.0f);
            // Translate to the part's relative position from the pivot
            glTranslatef(part.posX - 0.5f, part.posY - 1.0f, part.posZ);
        } else {
            // Otherwise, use the part's absolute position.
            glTranslatef(part.posX, part.posY, part.posZ);
        }

        // Draw the appropriate shape.
        if (part.shape == "cylinder") {
            drawCylinder(part.scaleX, part.scaleY);
        } else if (part.shape == "sphere") {
            drawSphere(part.scaleX);  // using scaleX as the radius
        } else if (part.shape == "cube") {
            drawCube(part.scaleX, part.scaleY, part.scaleZ);
        } else {
            std::cerr << "Unknown shape: " << part.shape << std::endl;
        }
    glPopMatrix();
}

//------------------------------------------------------
// 5. Load Boxer Parts from a Text File
//------------------------------------------------------
bool loadBoxerFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()){
        std::cerr << "Error: Could not open " << filename << std::endl;
        return false;
    }
    g_parts.clear();
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments or empty lines.
        if (line.empty() || line[0] == '#') continue;
        BodyPart part;
        std::istringstream iss(line);
        iss >> part.name >> part.shape 
            >> part.posX >> part.posY >> part.posZ 
            >> part.scaleX >> part.scaleY >> part.scaleZ 
            >> part.colorR >> part.colorG >> part.colorB;
        if (!iss.fail()) {
            g_parts.push_back(part);
        }
    }
    file.close();
    return true;
}

//------------------------------------------------------
// 6. GLUT Callback Functions
//------------------------------------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    // Use the global camera variables
    gluLookAt(camX, camY, camZ,
              lookX, lookY, lookZ,
              0.0, 1.0, 0.0);
    
    // Draw each body part
    for (const auto& part : g_parts) {
        drawPart(part);
    }
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w/h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// Keyboard callback for camera movement and triggering the punch.
void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'p':  // Start punching animation
            if (!punching) {
                punching = true;
                punchForward = true;
            }
            break;
        case 'w': camZ -= 0.5f; break;
        case 's': camZ += 0.5f; break;
        case 'a': camX -= 0.5f; break;
        case 'd': camX += 0.5f; break;
        case 'q': camY += 0.5f; break;
        case 'e': camY -= 0.5f; break;
        case 27: exit(0); break; // ESC to exit
    }
    glutPostRedisplay();
}

// Timer function to update the punching animation.
void onTimer(int value) {
    if (punching) {
        if (punchForward) {
            punchAngle += 10.0f;
            if (punchAngle >= 100.0f) {
                punchForward = false;
            }
        } else {
            punchAngle -= 10.0f;
            if (punchAngle <= 0.0f) {
                punchAngle = 0.0f;
                punching = false;
                punchForward = true;
            }
        }
        glutPostRedisplay();
    }
    glutTimerFunc(30, onTimer, 0);
}

//------------------------------------------------------
// 7. Main Function
//------------------------------------------------------
int main(int argc, char** argv) {
    std::string filename = "boxer.txt";
    if (argc > 1) {
        filename = argv[1];
    }
    if (!loadBoxerFromFile(filename)) {
        return 1;
    }
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Boxer with Punch Animation & Camera Control");
    
    glEnable(GL_DEPTH_TEST);
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(30, onTimer, 0);
    
    glutMainLoop();
    return 0;
}
