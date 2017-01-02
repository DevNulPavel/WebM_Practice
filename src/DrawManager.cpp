#include "DrawManager.h"
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
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

DrawManager::DrawManager(){
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
    
    // создание текстуры
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERRORS();
    
    // Вершины
    vector<Vertex> vertexes;
    vertexes.reserve(4);
    
    // вбиваем данные о вершинах
    vertexes.push_back(Vertex(vec2(0, _size.y),        vec2(0, 1)));
    vertexes.push_back(Vertex(vec2(0, 0),              vec2(0, 0)));
    vertexes.push_back(Vertex(vec2(_size.x, _size.y),  vec2(1, 1)));
    vertexes.push_back(Vertex(vec2(_size.x, 0),        vec2(1, 0)));
    
    // VBO, данные о вершинах
    _vbo = 0;
    glGenBuffers (1, &_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, _vbo);
    glBufferData (GL_ARRAY_BUFFER, 4 * sizeof(Vertex), (void*)(vertexes.data()), GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS();
}

DrawManager::~DrawManager(){
    // удаление текстуры
    glDeleteTextures(1, &_texture);
    glDeleteBuffers(1, &_vbo);
    // удаление
    glDeleteProgram(_shaderProgram);
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
    return 0.0f;
}

void DrawManager::draw(){
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Включение шейдера
    glUseProgram (_shaderProgram);
        
    // выставляем матрицу трансформа в координаты камеры
    glUniformMatrix4fv(_matrixLocation, 1, false, glm::value_ptr(_projectionMatrix));
    
    // говорим шейдеру, что текстура будет на 0 позиции (GL_TEXTURE0)
    glUniform1i(_texture0Location, 0);
    
    // активируем нулевую текстуру для для шейдера, включаем эту текстуру
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    
    // рисуем
    //      sizeof(Vertex) - размер блока одной информации о вершине
    //      OFFSETOF(Vertex, color) - смещение от начала
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    CHECK_GL_ERRORS();
    
    // VBO enable arrays
    glEnableVertexAttribArray(UI_POS_ATTRIBUTE_LOCATION);
    glEnableVertexAttribArray(UI_TEX_COORD_ATTRIBUTE_LOCATION);
    CHECK_GL_ERRORS();
    
    // VBO align
    glVertexAttribPointer(UI_POS_ATTRIBUTE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, pos)); // Позиции
    glVertexAttribPointer(UI_TEX_COORD_ATTRIBUTE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, texCoord));    // Текстурные координаты
    CHECK_GL_ERRORS();
    
    // draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // VBO off
    glDisableVertexAttribArray(UI_POS_ATTRIBUTE_LOCATION);
    glDisableVertexAttribArray(UI_TEX_COORD_ATTRIBUTE_LOCATION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS();
    
    glDisable(GL_BLEND);
    glUseProgram (0);
}
