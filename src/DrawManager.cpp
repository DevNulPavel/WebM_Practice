#include "DrawManager.h"
#define GLFW_INCLUDE_GLU
#define GLFW_INCLUDE_GL3
#define GLFW_INCLUDE_GLEXT
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее
#include <GLFW/glfw3.h>     // Непосредственно сам GLFW
#include "Shaders.h"
#include "Helpers.h"


struct Vertex{
    vec2 pos;
    vec2 texCoord;
    
    // constructor
    Vertex(const vec2& inPos, const vec2& inTexCoord):
        pos(inPos),
        texCoord(inTexCoord){
    }
};

////////////////////////////////////////////////////////////////////////////////

DrawManager::DrawManager():
    _shaderProgram(0),
    _texture0Location(0),
    _matrixLocation(0),
    _projectionMatrix(0),
    _size(0),
    _vbo(0),
    _texture(0){

    createDecoder();
    createGLContext();
}

DrawManager::~DrawManager(){
    // Delete VBO
    glDeleteBuffers(1, &_vbo);
    // удаление текстуры
    glDeleteTextures(1, &_texture);
    // удаление
    glDeleteProgram(_shaderProgram);
}

void DrawManager::createDecoder(){
#ifdef _WIN32
    std::string filePath = "res\\big-buck-bunny_trailer.webm";
#else
    std::string filePath = "res/big-buck-bunny_trailer.webm";
#endif
    _decoder = make_shared<WebMVideoDecoder>(filePath);
    
    vec2 size = _decoder->getVideoDisplaySize();
    setSize(size.x, size.y);
}

void DrawManager::createGLContext(){
    // Шейдеры
    map<string, int> attributesLocations;
    attributesLocations["aPos"] = UI_POS_ATTRIBUTE_LOCATION;
    attributesLocations["aTexCoord"] = UI_TEX_COORD_ATTRIBUTE_LOCATION;
    _shaderProgram = create2DShader(attributesLocations);
    CHECK_GL_ERRORS();
    
    // юниформы шейдера
    _matrixLocation = glGetUniformLocation(_shaderProgram, "uModelViewProjMat");
    _texture0Location = glGetUniformLocation(_shaderProgram, "uTexture0");
    CHECK_GL_ERRORS();
    
    // размер текстуры
    vec2 textureSize = _decoder->getVideoSize();
    
    // создание текстуры
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize.x, textureSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERRORS();
    
    // отображаемый размер
    vec2 displaySize = _decoder->getVideoDisplaySize();
    
    // Вершины
    vector<Vertex> vertexes;
    vertexes.reserve(4);
    
    // вбиваем данные о вершинах
    vertexes.push_back(Vertex(vec2(0, displaySize.y),        vec2(0, 1)));
    vertexes.push_back(Vertex(vec2(0, 0),              vec2(0, 0)));
    vertexes.push_back(Vertex(vec2(displaySize.x, displaySize.y),  vec2(1, 1)));
    vertexes.push_back(Vertex(vec2(displaySize.x, 0),        vec2(1, 0)));
    
    // VBO, данные о вершинах
    glGenBuffers (1, &_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, _vbo);
    glBufferData (GL_ARRAY_BUFFER, 4 * sizeof(Vertex), (void*)(vertexes.data()), GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS();

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    CHECK_GL_ERRORS();

    // VBO align
    glVertexAttribPointer(UI_POS_ATTRIBUTE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, pos)); // Позиции
    glVertexAttribPointer(UI_TEX_COORD_ATTRIBUTE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, texCoord));    // Текстурные координаты
    CHECK_GL_ERRORS();

    // Unbind VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS();
}

vec2 DrawManager::getSize(){
    return _size;
}

void DrawManager::setSize(float width, float height){
    _size = vec2(width, height);
    // обязательно во флоатах, чтобы тип матрицы был верным
    _projectionMatrix = glm::ortho(0.0f, width, 0.0f, height);
}

float DrawManager::getFrameTime(){
    return _decoder->getFrameDuration();
}

void DrawManager::decodeNewFrame(){
    _decoder->decodeNewFrame();
    _decoder->copyDataToTexture(_texture);
}

void DrawManager::drawTexture(){
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    CHECK_GL_ERRORS();
    
    // Включение шейдера
    glUseProgram (_shaderProgram);
    CHECK_GL_ERRORS();
    
    // выставляем матрицу трансформа в координаты камеры
    glUniformMatrix4fv(_matrixLocation, 1, false, glm::value_ptr(_projectionMatrix));
    
    // говорим шейдеру, что текстура будет на 0 позиции (GL_TEXTURE0)
    glUniform1i(_texture0Location, 0);
    
    // активируем нулевую текстуру для для шейдера, включаем эту текстуру
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    CHECK_GL_ERRORS();
    
    // VBO enable arrays
    glEnableVertexAttribArray(UI_POS_ATTRIBUTE_LOCATION);
    glEnableVertexAttribArray(UI_TEX_COORD_ATTRIBUTE_LOCATION);
    CHECK_GL_ERRORS();

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    CHECK_GL_ERRORS();
    
    // VBO align
    glVertexAttribPointer(UI_POS_ATTRIBUTE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, pos)); // Позиции
    glVertexAttribPointer(UI_TEX_COORD_ATTRIBUTE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, texCoord));    // Текстурные координаты
    CHECK_GL_ERRORS();
    
    // draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    CHECK_GL_ERRORS();
    
    // VBO off
    glDisableVertexAttribArray(UI_POS_ATTRIBUTE_LOCATION);
    glDisableVertexAttribArray(UI_TEX_COORD_ATTRIBUTE_LOCATION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS();
    
    glDisable(GL_BLEND);
    glUseProgram (0);
}

