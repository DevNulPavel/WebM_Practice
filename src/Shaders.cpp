#include "Shaders.h"
#include "Helpers.h"


GLuint createShaderFromSources(const char* vertexShader, const char* fragmentShader,
                               const map<string,int>& attributeLocations){
    GLuint vs = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShader, NULL);
    glCompileShader(vs);
    CHECK_GL_ERRORS();

    GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShader, NULL);
    glCompileShader(fs);
    CHECK_GL_ERRORS();

    GLuint shaderProgram = glCreateProgram ();
    glAttachShader(shaderProgram, fs);
    glAttachShader(shaderProgram, vs);
    CHECK_GL_ERRORS();
    
    // расположение аттрибутов в шейдере
    map<string,int>::const_iterator it = attributeLocations.begin();
    while (it != attributeLocations.end()) {
        const string& attributeName = (*it).first;
        int attributeLocation = (*it).second;
        
        glBindAttribLocation(shaderProgram, attributeLocation, attributeName.c_str());
        
        it++;
    }
    CHECK_GL_ERRORS();

    glLinkProgram(shaderProgram);
    CHECK_GL_ERRORS();
    
    return shaderProgram;
}

GLuint create2DShader(const map<string,int>& attributeLocations){
    // Шейдер вершин
    const char* vertexShader = STRINGIFY_SHADER(
        //#version 200

        // vertex attribute
        attribute vec2 aPos;
        attribute vec2 aTexCoord;
        // uniforms
        uniform mat4 uModelViewProjMat;
        // output
        varying vec2 vTexCoord;

        void main () {
            vec4 vertexVec4 = vec4(aPos, 0.0, 1.0);      // последняя компонента 1, тк это точка
            // вычисляем позицию точки в пространстве OpenGL
            gl_Position = uModelViewProjMat * vertexVec4;
            // цвет и текстурные координаты просто пробрасываем для интерполяции
            vTexCoord = aTexCoord;
        }
    );
    const char* fragmentShader = STRINGIFY_SHADER(
        //#version 200

        // переменная текстурных координат
        varying vec2 vTexCoord;

        // текстура
        uniform sampler2D uTexture0;

        void main () {
            // текстура
            vec4 textureColor = texture2D(uTexture0, vTexCoord);

            gl_FragColor = textureColor;
        }
    );

    GLuint shader = createShaderFromSources(vertexShader, fragmentShader, attributeLocations);
    CHECK_GL_ERRORS();
    return shader;
}
