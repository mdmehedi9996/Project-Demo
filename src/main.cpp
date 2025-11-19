//Electricity Manegment

#include <GL/glut.h>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

const int WIDTH = 800;
const int HEIGHT = 600;
const float RATE_PER_UNIT = 8.0f;
const float UPDATE_INTERVAL = 1.0f; // seconds

// --- Texture globals ---
GLuint acTexture = 0, tvTexture = 0, fridgeTexture = 0;

// Simple BMP loader (expects 24-bit uncompressed BMP)
GLuint loadBMP(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        cout << "Error: could not open " << filename << endl;
        return 0;
    }

    unsigned char header[54];
    if (fread(header, 1, 54, file) != 54) {
        cout << "Error: not a correct BMP file (header)" << endl;
        fclose(file);
        return 0;
    }

    // Check BMP signature
    if (header[0] != 'B' || header[1] != 'M') {
        cout << "Error: not BMP: " << filename << endl;
        fclose(file);
        return 0;
    }

    // Parse header
    unsigned int dataPos     = *(int*)&(header[0x0A]);
    unsigned int imageSize   = *(int*)&(header[0x22]);
    unsigned int width       = *(int*)&(header[0x12]);
    unsigned int height      = *(int*)&(header[0x16]);
    unsigned short bpp       = *(unsigned short*)&(header[0x1C]);

    if (imageSize == 0) imageSize = width * height * 3;
    if (dataPos == 0) dataPos = 54;

    if (bpp != 24) {
        cout << "Warning: " << filename << " is not 24-bit BMP (bpp=" << bpp << "). This loader expects 24-bit BMP.\n";
        // continue anyway, may fail if not 24-bit
    }

        
    unsigned char* data = new unsigned char[imageSize];
    fseek(file, dataPos, SEEK_SET);
    if (fread(data, 1, imageSize, file) != imageSize) {
        // Some BMPs include padding per row; try to read per-row instead
        // Fallback: read raw rows handling row padding
        fseek(file, dataPos, SEEK_SET);
        int row_padded = (width*3 + 3) & (~3);
        unsigned char* rowdata = new unsigned char[row_padded];
        int idx = 0;
        for (unsigned int y = 0; y < height; ++y) {
            if (fread(rowdata, 1, row_padded, file) != (size_t)row_padded) {
                break;
            }
            for (unsigned int x = 0; x < width; ++x) {
                data[idx++] = rowdata[x*3 + 0];
                data[idx++] = rowdata[x*3 + 1];
                data[idx++] = rowdata[x*3 + 2];
            }
        }
        delete[] rowdata;
    }
    fclose(file);

    // BMP stores BGR, convert to RGB
    for (unsigned int i = 0; i < imageSize; i += 3) {
        unsigned char tmp = data[i];
        data[i] = data[i+2];
        data[i+2] = tmp;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // texture params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload to GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    delete[] data;
    return textureID;
}

// Draw texture quad; if texture==0 draw colored fallback
void drawImage(GLuint texture, float x, float y, float w = 80.0f, float h = 60.0f) {
    if (texture != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBegin(GL_QUADS);
            glTexCoord2f(0.f, 0.f); glVertex2f(x, y);
            glTexCoord2f(1.f, 0.f); glVertex2f(x + w, y);
            glTexCoord2f(1.f, 1.f); glVertex2f(x + w, y + h);
            glTexCoord2f(0.f, 1.f); glVertex2f(x, y + h);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    } else {
        // fallback colored rectangle
        glColor3f(0.8f, 0.8f, 0.8f);
        glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + w, y);
            glVertex2f(x + w, y + h);
            glVertex2f(x, y + h);
        glEnd();
    }
}

class ElectricDevice {
public:
    string name;
    float powerConsumption; // kW
    bool isOn;
    float posX, posY;
    float totalUnits; // units (kWh)
    float totalCost;
    char key;

    ElectricDevice(string name, float power, float x, float y, char key) :
        name(name), powerConsumption(power), isOn(false),
        posX(x), posY(y), totalUnits(0.0f), totalCost(0.0f), key(key) {
    }

    void toggle() {
        isOn = !isOn;
    }

    // timeDelta in seconds (we'll treat unit conversion similar to original: units accumulate)
    void updateUsage(float timeDelta) {
        if (isOn) {
            // units = power (kW) * time(hours)
            // timeDelta is seconds, so time(hours) = timeDelta / 3600.0
            float units = (powerConsumption * timeDelta) / 3600.0f; // correct physics
            totalUnits += units;
            totalCost = totalUnits * RATE_PER_UNIT;
        }
    }

    void draw() {
        // Draw device background box (slightly different color when ON)
        if (isOn) glColor3f(0.7f, 0.95f, 0.85f);
        else       glColor3f(0.95f, 0.94f, 0.9f);

        glBegin(GL_QUADS);
            glVertex2f(posX, posY);
            glVertex2f(posX + 80, posY);
            glVertex2f(posX + 80, posY + 60);
            glVertex2f(posX, posY + 60);
        glEnd();

        // Draw the device image (texture) on top (texture may cover the box)
        if (name == "AC")      drawImage(acTexture, posX, posY, 80.0f, 60.0f);
        else if (name == "TV") drawImage(tvTexture, posX, posY, 80.0f, 60.0f);
        else if (name == "Fridge") drawImage(fridgeTexture, posX, posY, 80.0f, 60.0f);

        // draw label & stats
        glColor3f(0, 0, 0);
        glRasterPos2f(posX, posY - 10);
        string label = name + " (" + key + ")";
        for (char c : label) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);

        glRasterPos2f(posX, posY + 70);
        stringstream powerText;
        powerText << "Power: " << fixed << setprecision(2) << (isOn ? powerConsumption : 0.0f) << " kW";
        for (char c : powerText.str()) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);

        glRasterPos2f(posX, posY + 88);
        stringstream unitsText;
        unitsText << "Units: " << fixed << setprecision(5) << totalUnits;
        for (char c : unitsText.str()) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);

        glRasterPos2f(posX, posY + 106);
        stringstream costText;
        costText << "Cost: " << fixed << setprecision(2) << totalCost << " Tk";
        for (char c : costText.str()) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }
};

// Create devices
ElectricDevice ac("AC", 5.0f, 150.0f, 150.0f, '1');
ElectricDevice tv("TV", 4.0f, 300.0f, 150.0f, '2');
ElectricDevice fridge("Fridge", 2.0f, 450.0f, 150.0f, '3');

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Outer panel background
    glColor3f(0.95f, 0.95f, 0.95f);
    glBegin(GL_QUADS);
        glVertex2f(100, 100);
        glVertex2f(700, 100);
        glVertex2f(700, 500);
        glVertex2f(100, 500);
    glEnd();

    // Draw each device (they draw their own images & text)
    ac.draw();
    tv.draw();
    fridge.draw();

    // Total cost
    float totalCombinedCost = ac.totalCost + tv.totalCost + fridge.totalCost;
    glColor3f(0, 0, 0.6f);
    glRasterPos2f(300, 50);
    stringstream totalText;
    totalText << "TOTAL COST: " << fixed << setprecision(2) << totalCombinedCost << " Tk";
    for (char c : totalText.str()) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    // Header & controls
    glColor3f(0, 0, 0);
    glRasterPos2f(100, 550);
    string header = "Constant Power Consumption Monitor (updates every second)";
    for (char c : header) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);

    glRasterPos2f(100, 570);
    string controls = "Controls: Press 1 (AC), 2 (TV), 3 (Fridge). Press ESC to exit.";
    for (char c : controls) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '1': ac.toggle(); break;
    case '2': tv.toggle(); break;
    case '3': fridge.toggle(); break;
    case 27: exit(0); break; // ESC
    }
    glutPostRedisplay();
}

void timer(int value) {
    // Update usage by seconds
    ac.updateUsage(UPDATE_INTERVAL);
    tv.updateUsage(UPDATE_INTERVAL);
    fridge.updateUsage(UPDATE_INTERVAL);

    glutPostRedisplay();
    glutTimerFunc((unsigned int)(1000.0f * UPDATE_INTERVAL), timer, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Measurement Electric Power");

    // 2D orthographic: origin top-left like screen coordinates
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    gluOrtho2D(0, WIDTH, HEIGHT, 0);

    // Load textures (ensure these BMP files exist in the same folder)
    acTexture = loadBMP("ac.bmp");
    tvTexture = loadBMP("tv.bmp");
    fridgeTexture = loadBMP("fridge.bmp");

    if (!acTexture)      cout << "ac.bmp not loaded or invalid (expected 24-bit BMP)\n";
    if (!tvTexture)      cout << "tv.bmp not loaded or invalid (expected 24-bit BMP)\n";
    if (!fridgeTexture)  cout << "fridge.bmp not loaded or invalid (expected 24-bit BMP)\n";

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc((unsigned int)(1000.0f * UPDATE_INTERVAL), timer, 0);

    cout << "Electric Device Monitor Started" << endl;
    cout << "Press 1, 2, or 3 to toggle devices" << endl;
    cout << "Press ESC to exit" << endl;

    glutMainLoop();
    return 0;
}
