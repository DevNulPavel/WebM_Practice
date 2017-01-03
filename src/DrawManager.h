#ifndef UIMANAGER
#define UIMANAGER

#include <memory>
#include <list>
#include <vector>
#include <glm.hpp>
#include "WebMVideoDecoder.h"

using namespace std;
using namespace glm;

#define UI_POS_ATTRIBUTE_LOCATION 0
#define UI_TEX_COORD_ATTRIBUTE_LOCATION 1

class DrawManager{
public:
    DrawManager();
    ~DrawManager();
    float getFrameTime();
    vec2 getSize();
    void draw();

private:
    // Decoder
    WebMVideoDecoderPtr _decoder;
    // OpenGL
    int _shaderProgram;
    uint _texture0Location;
    uint _matrixLocation;
    mat4 _projectionMatrix;
    vec2 _size;
    uint _vbo;
    uint _texture;

private:
    void createDecoder();
    void createGLContext();
    void setSize(float width, float height);
    void decodeNewFrame();
    void drawTexture();
};

#endif
