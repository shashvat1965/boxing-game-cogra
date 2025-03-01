#include "globals.hpp"

GLuint backgroundTexID;
bool textureLoaded = false;
int bgWidth, bgHeight;
std::vector<unsigned char> bgData;

void drawBackground();

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ,
              lookX, lookY, lookZ,
              0.0, 1.0, 0.0);
    
    drawBackground();
    
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
            armRotation += 6.0f;
            bicepRotation += 6.0f;
            handRotation += 6.0f;
            if (armRotation >= 90.0f) { // Max forward extension
                punchForward2 = false;
                if (hitTimer1 <= 0.0f)
                    hitTimer1 = 1.0f; // hit Boxer 2
            }
        } else {
            armRotation -= 6.0f;
            bicepRotation -= 6.0f;
            handRotation -= 6.0f;
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

bool loadPNGImage(const char* filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        std::cerr << "Error: Could not open PNG file " << filename << std::endl;
        return false;
    }

    // Read header (8 bytes)
    png_byte header[8];
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        std::cerr << "Error: File " << filename << " is not recognized as a PNG file" << std::endl;
        fclose(fp);
        return false;
    }

    // Initialize png read struct
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        std::cerr << "Error: png_create_read_struct failed" << std::endl;
        fclose(fp);
        return false;
    }

    // Initialize png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        std::cerr << "Error: png_create_info_struct failed" << std::endl;
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cerr << "Error during init_io" << std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(fp);
        return false;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    bgWidth = png_get_image_width(png_ptr, info_ptr);
    bgHeight = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth  = png_get_bit_depth(png_ptr, info_ptr);

    // Convert palette images to RGB, grayscale to 8-bit, and add alpha if needed.
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    
    // Convert grayscale images to RGB.
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);
    
    // Remove alpha channel if present
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA ||
        color_type == PNG_COLOR_TYPE_RGBA)
        png_set_strip_alpha(png_ptr);
    
    png_read_update_info(png_ptr, info_ptr);

    // Read image data
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    bgData.resize(bgHeight * rowbytes);
    std::vector<png_bytep> row_pointers(bgHeight);
    for (int y = 0; y < bgHeight; y++) {
        row_pointers[y] = (png_bytep)&bgData[y * rowbytes];
    }
    
    png_read_image(png_ptr, row_pointers.data());
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);

    // We now assume the image is RGB
    return true;
}

//------------------------------------------------------
// Setup texture from loaded image data
//------------------------------------------------------
void setupBackgroundTexture() {
    if (bgData.empty())
        return;
    
    glGenTextures(1, &backgroundTexID);
    glBindTexture(GL_TEXTURE_2D, backgroundTexID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Note: bgData's row length is bgWidth * 3 (RGB)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bgWidth, bgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bgData.data());
    
    textureLoaded = true;
}

//------------------------------------------------------
// 8. GLUT Callback Functions
//------------------------------------------------------
void drawBackground() {
    if (!textureLoaded)
        return;
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, backgroundTexID);
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 1.0f);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
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
    glutCreateWindow("Boxing Game Assignment - Jake Paul v. Md. Ali");
    
    glEnable(GL_DEPTH_TEST);

    loadModel();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
    if (loadPNGImage("background.png"))
        setupBackgroundTexture();
    else
        std::cerr << "Failed to load background.png" << std::endl;
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(30, onTimer, 0);
    
    glutMainLoop();
    return 0;
}
