#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <png.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>

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

//------------------------------------------------------
// 2. Global Variables for Camera, Selection & Animation
//------------------------------------------------------
float camX = 0.0f, camY = 2.0f, camZ = 12.0f;
float lookX = 0.0f, lookY = 1.0f, lookZ = 0.0f;

GLuint backgroundTexID;
bool textureLoaded = false;
int bgWidth, bgHeight;
std::vector<unsigned char> bgData;

int selectedBoxer = 1;

float punchAngleUpper1 = 0.0f, punchAngleFore1 = 0.0f, punchAnglePalm1 = 0.0f;
bool punching1 = false;
bool punchForward1 = true;
float punchAngleUpper2 = 0.0f, punchAngleFore2 = 0.0f, punchAnglePalm2 = 0.0f;
bool punching2 = false;
bool punchForward2 = true;

const float maxUpper = 60.0f;
const float maxFore  = 120.0f;
const float maxPalm  = 120.0f;

float hitTimer1 = 0.0f;
float hitTimer2 = 0.0f;

//------------------------------------------------------
// 3. Shape Drawing Functions
//------------------------------------------------------
void customRotatef(float angle, float axisX, float axisY, float axisZ) {
    float rad = angle * M_PI / 180.0f;
    float c = cos(rad);
    float s = sin(rad);
    float one_c = 1 - c;

    float length = sqrt(axisX * axisX + axisY * axisY + axisZ * axisZ);
    if (length < 0.0001f) return;
    float x = axisX / length;
    float y = axisY / length;
    float z = axisZ / length;

    float m[16] = {
        x*x*one_c + c,     y*x*one_c + z*s,  z*x*one_c - y*s,  0.0f,
        x*y*one_c - z*s,   y*y*one_c + c,    z*y*one_c + x*s,  0.0f,
        x*z*one_c + y*s,   y*z*one_c - x*s,  z*z*one_c + c,    0.0f,
        0.0f,              0.0f,             0.0f,             1.0f
    };
    glMultMatrixf(m);
}

void customTranslatef(float x, float y, float z) {
    float m[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x,    y,    z,    1.0f
    };
    glMultMatrixf(m);
}

void drawCylinder(float radius, float height) {
    GLUquadric* quad = gluNewQuadric();
    glPushMatrix();
        customRotatef(-90.0f, 1, 0, 0);
        gluCylinder(quad, radius, radius, height, 20, 20);
        gluDisk(quad, 0.0, radius, 20, 1);
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
void drawPartForBoxer(const BodyPart& part, int boxerID) {
    if ((boxerID == 1 && hitTimer1 > 0.0f) || (boxerID == 2 && hitTimer2 > 0.0f))
        glColor3f(1.0f, 0.0f, 0.0f);
    else
        glColor3f(part.colorR, part.colorG, part.colorB);
    
    glPushMatrix();
    if ((part.name == "rightArmUpper" || part.name == "rightArmFore" || part.name == "rightPalm") &&
         (boxerID == selectedBoxer))
    {
        glTranslatef(0.5f, 1.0f, 0.0f);
        if (part.name == "rightArmUpper") {
            if (boxerID == 1)
                customRotatef(-punchAngleUpper1, 1.0f, 0.0f, 0.0f);
            else
                customRotatef(-punchAngleUpper2, 1.0f, 0.0f, 0.0f);
        } else if (part.name == "rightArmFore") {
            if (boxerID == 1)
                customRotatef(-punchAngleFore1, 1.0f, 0.0f, 0.0f);
            else
                customRotatef(-punchAngleFore2, 1.0f, 0.0f, 0.0f);
        } else if (part.name == "rightPalm") {
            if (boxerID == 1)
                customRotatef(-punchAnglePalm1, 1.0f, 0.0f, 0.0f);
            else
                customRotatef(-punchAnglePalm2, 1.0f, 0.0f, 0.0f);
        }
        glTranslatef(part.posX - 0.5f, part.posY - 1.0f, part.posZ);
    } else {
        glTranslatef(part.posX, part.posY, part.posZ);
    }
    
    if (part.shape == "cylinder")
        drawCylinder(part.scaleX, part.scaleY);
    else if (part.shape == "sphere")
        drawSphere(part.scaleX);
    else if (part.shape == "cube")
        drawCube(part.scaleX, part.scaleY, part.scaleZ);
    else
        std::cerr << "Unknown shape: " << part.shape << std::endl;
    
    glPopMatrix();
}

//------------------------------------------------------
// 5. Draw a Boxer Given Its ID
//------------------------------------------------------
void drawBoxer(int boxerID) {
    glPushMatrix();
        if (boxerID == 1) {
            glTranslatef(0.4f, 0.0f, 0.0f);
            customRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        } else {
            glTranslatef(-0.4f, 0.0f, 0.0f);
            customRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        }
    
        for (const auto& part : g_baseParts)
            drawPartForBoxer(part, boxerID);
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
// 7. PNG Loader Using libpng
//------------------------------------------------------
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

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, lookX, lookY, lookZ, 0.0, 1.0, 0.0);
    
    drawBackground();
    drawBoxer(1);
    drawBoxer(2);
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0)
        h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w/h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

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
        case 'w': camZ -= 0.5f; break;
        case 's': camZ += 0.5f; break;
        case 'a': camX -= 0.5f; break;
        case 'd': camX += 0.5f; break;
        case 'q': camY += 0.5f; break;
        case 'e': camY -= 0.5f; break;
        case 27: exit(0); break;
    }
    glutPostRedisplay();
}

void onTimer(int value) {
    if (punching1) {
        if (punchForward1) {
            if (punchAngleUpper1 < maxUpper)
                punchAngleUpper1 += 3.0f;
            if (punchAngleFore1 < maxFore)
                punchAngleFore1 += 5.0f;
            if (punchAnglePalm1 < maxPalm)
                punchAnglePalm1 += 5.0f;
            if (punchAngleUpper1 >= maxUpper) {
                punchForward1 = false;
                if (hitTimer2 <= 0.0f)
                    hitTimer2 = 1.0f;
            }
        } else {
            if (punchAngleUpper1 > 0.0f)
                punchAngleUpper1 -= 3.0f;
            if (punchAngleFore1 > 0.0f)
                punchAngleFore1 -= 5.0f;
            if (punchAnglePalm1 > 0.0f)
                punchAnglePalm1 -= 5.0f;
            if (punchAngleUpper1 <= 0.0f) {
                punchAngleUpper1 = punchAngleFore1 = punchAnglePalm1 = 0.0f;
                punching1 = false;
                punchForward1 = true;
            }
        }
    }
    if (punching2) {
        if (punchForward2) {
            if (punchAngleUpper2 < maxUpper)
                punchAngleUpper2 += 3.0f;
            if (punchAngleFore2 < maxFore)
                punchAngleFore2 += 5.0f;
            if (punchAnglePalm2 < maxPalm)
                punchAnglePalm2 += 5.0f;
            if (punchAngleUpper2 >= maxUpper) {
                punchForward2 = false;
                if (hitTimer1 <= 0.0f)
                    hitTimer1 = 1.0f;
            }
        } else {
            if (punchAngleUpper2 > 0.0f)
                punchAngleUpper2 -= 3.0f;
            if (punchAngleFore2 > 0.0f)
                punchAngleFore2 -= 5.0f;
            if (punchAnglePalm2 > 0.0f)
                punchAnglePalm2 -= 5.0f;
            if (punchAngleUpper2 <= 0.0f) {
                punchAngleUpper2 = punchAngleFore2 = punchAnglePalm2 = 0.0f;
                punching2 = false;
                punchForward2 = true;
            }
        }
    }
    
    if (hitTimer1 > 0.0f) {
        hitTimer1 -= 0.03f;
        if (hitTimer1 < 0.0f)
            hitTimer1 = 0.0f;
    }
    if (hitTimer2 > 0.0f) {
        hitTimer2 -= 0.03f;
        if (hitTimer2 < 0.0f)
            hitTimer2 = 0.0f;
    }
    
    glutPostRedisplay();
    glutTimerFunc(30, onTimer, 0);
}

//------------------------------------------------------
// 9. Main Function
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
    glutCreateWindow("Two Boxers Facing Each Other – PNG Background & Improved Punch Animation");

    glEnable(GL_DEPTH_TEST);
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
