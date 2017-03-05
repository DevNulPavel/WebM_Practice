#ifndef HELPERS
#define HELPERS

#define GLFW_INCLUDE_GLU
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее

// вычисление смещения в структуре/классе
#define OFFSETOF(TYPE, FIELD) ((void*)&(((TYPE*)NULL)->FIELD))
// Превращаем текущий текст в строку шейдера
#define STRINGIFY_SHADER(TEXT) ("#version 120\n "#TEXT)
// проверка ошибок
#define CHECK_GL_ERRORS() checkOpenGLerror(__FILE__, __LINE__)

// создать проперти с геттером для получаения значения
#define PROPERTY_GET(TYPE, VAR, METHOD_NAME) \
    private: \
    TYPE VAR; \
    public: \
    const TYPE& get##METHOD_NAME() const{ \
        return VAR; \
    }

// создать проперти с сеттером и геттером
#define PROPERTY_SET_GET(TYPE, VAR, METHOD_NAME) \
    private:\
    TYPE VAR;   \
    public: \
    const TYPE& get##METHOD_NAME() const{ \
        return VAR; \
    }\
    void set##METHOD_NAME(const TYPE& val){ \
        VAR = val; \
    }

// создать геттер для получаения ссылки значения
#define GETTER(TYPE, VAR, METHOD_NAME) \
    const TYPE& get##METHOD_NAME() const{ \
        return VAR; \
    }

// создать геттер для получаения копии значения
#define GETTER_COPY(TYPE, VAR, METHOD_NAME) \
    TYPE get##METHOD_NAME() const{ \
        return VAR; \
    }

// сеттер/геттер для установки/получения ссылки значения
#define SETTER_GETTER(TYPE, VAR, METHOD_NAME) \
    const TYPE& get##METHOD_NAME() const{ \
        return VAR; \
    } \
    void set##METHOD_NAME(const TYPE& val){ \
        VAR = val; \
    }

// сеттер/геттер для установки/получения копии значения
#define SETTER_GETTER_COPY(TYPE, VAR, METHOD_NAME) \
    TYPE get##METHOD_NAME() const{ \
        return VAR; \
    } \
    void set##METHOD_NAME(const TYPE& val){ \
        VAR = val; \
    }


typedef unsigned int uint;

int checkOpenGLerror(const char *file, int line);
void glDebugOut(uint source, uint type, uint id, uint severity, int length, const char* message, void* userParam);


#endif
