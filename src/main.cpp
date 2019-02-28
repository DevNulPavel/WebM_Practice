// #define GLFW_INCLUDE_GLCOREARB 1 // Tell GLFW to include the OpenGL core profile header
#define GLFW_INCLUDE_GLU
#define GLFW_INCLUDE_GL3
#define GLFW_INCLUDE_GLEXT
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <math.h>
#include <stdio.h>
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее
#include <GLFW/glfw3.h>     // Непосредственно сам GLFW
#include <glm.hpp>          // библиотека графической математики
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include "Helpers.h"
#include "Shaders.h"
#include "DrawManager.h"
#include "WebMVideoDecoder.h"

#define MATH_PI 3.14159265


// Документация
// https://www.opengl.org/sdk/docs/man/html/

using namespace std;
using namespace glm;


// Текущие переменные для модели
bool leftButtonPressed = false;
bool rightPressed = false;
double lastCursorPosX = 0.0;
double lastCursorPosY = 0.0;

DrawManager* drawManager = nullptr;

//////////////////////////////////////////////////////////////////////////////////////////////////

void glfwErrorCallback(int error, const char* description) {
    printf("OpenGL error = %d\n description = %s\n\n", error, description);
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Выходим по нажатию Escape
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    // по пробелу включаем или выключаем вращение автоматом
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
    }
}

void glfwMouseButtonCallback(GLFWwindow* window, int button, int state, int mod) {
    // обработка левой кнопки
    if(button == GLFW_MOUSE_BUTTON_1){
        if(state == GLFW_PRESS){
            leftButtonPressed = true;
        }else{
            leftButtonPressed = false;
        }
    }
    // обработка правой кнопки
    if(button == GLFW_MOUSE_BUTTON_2){
        if(state == GLFW_PRESS){
            rightPressed = true;
        }else{
            rightPressed = false;
        }
    }
}

void glfwCursorCallback(GLFWwindow* window, double x, double y) {
    // при нажатой левой кнопки - вращаем по X и Y
    if(leftButtonPressed){
        float xAngle = 0.0;
        float yAngle = 0.0;
    
        xAngle += (y - lastCursorPosY) * 0.5;
        yAngle += (x - lastCursorPosX) * 0.5;
        // ограничение вращения
        if (xAngle < -80) {
           xAngle = -80;
        }
        if (xAngle > 80) {
           xAngle = 80;
        }
    }

    // при нажатой левой кнопки - перемещаем по X Y
    if(rightPressed){
        vec3 modelPos;
    
        float offsetY = (y - lastCursorPosY) * 0.02;
        float offsetX = (x - lastCursorPosX) * 0.02;
        float newX = modelPos.x + offsetX;
        float newY = modelPos.y - offsetY;
        if (newX < -3){
            newX = -3;
        }
        if (newX > 3){
            newX = 3;
        }
        if (newY < -3){
            newY = -3;
        }
        if (newY > 3){
            newY = 3;
        }
        modelPos = vec3(newX, newY, modelPos.z);
    }

    lastCursorPosX = x;
    lastCursorPosY = y;
}

void glfwScrollCallback(GLFWwindow* window, double scrollByX, double scrollByY) {
    float size = 0;

    size += scrollByY * 0.2;
    if(size < 0.5){
        size = 0.5;
    }
    if(size > 5.0){
        size = 5.0;
    }
}

GLFWwindow* createWindow(){
    // окно
    GLFWwindow* window = 0;
    
    // обработчик ошибок
    glfwSetErrorCallback(glfwErrorCallback);
    
    // инициализация GLFW
    if (!glfwInit()){
        exit(EXIT_FAILURE);
    }
    
    // создание окна
#ifdef __APPLE__
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#else
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(1, 1, "Simple example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);        // вертикальная синхронизация
    
    // Обработка клавиш и прочего
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
    glfwSetCursorPosCallback(window, glfwCursorCallback);
    glfwSetScrollCallback(window, glfwScrollCallback);
    
    // инициализация расширений
    glewExperimental = GL_TRUE;
    if(glewInit() != 0){
        printf("GLEW init failed");
        exit(-2);
    }
    
    CHECK_GL_ERRORS();

    // Инициализация отладки
    if(glDebugMessageCallback){
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        
        // Коллбек ошибок OpenGL
        glDebugMessageCallback((GLDEBUGPROC)glDebugOut, 0);
        
        // Более высокий уровень отладки
        // GLuint unusedIds = 0;
        // glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
    }
    
    const unsigned char* version = glGetString(GL_VERSION);
    printf("OpenGL version = %s\n", version);
    fflush(stdout);

    CHECK_GL_ERRORS();
    
    return window;
}


int main(int argc, char *argv[]) {
    GLFWwindow* window = createWindow();
    
    // графический интерфейс
    drawManager = new DrawManager();
    
    // получение размера
    vec2 size = drawManager->getSize();
    int width = size.x;
    int height = size.y;
    
    // изменение размера окна
    glfwSetWindowSize(window, width, height);
    
    // задаем отображение
    glViewport(0, 0, width, height);
    CHECK_GL_ERRORS();
    
    // текущее время
    while (!glfwWindowShouldClose(window)){
        // wipe the drawing surface clear
        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render
        double frameDuration = 0.0;
        if (drawManager) {
            drawManager->drawTexture();
            frameDuration = drawManager->getFrameTime();
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // decode
        double frameDecodeBegin = glfwGetTime();
        if (drawManager) {
            drawManager->decodeNewFrame();
        }
        double frameDecodeEnd = glfwGetTime();
        double frameDecodeTime = frameDecodeEnd - frameDecodeBegin;
        
        // время отображения текущего кадра
        double sleepTime = std::max(frameDuration - frameDecodeTime, 0.0);
        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepTime * 1000.0)));
        }
    }

    delete drawManager;

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}

//! [code]
