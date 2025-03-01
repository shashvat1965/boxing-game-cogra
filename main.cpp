#include "globals.hpp"

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
