#ifndef VIDEO_DECODER_COLOR_CONVERTER
#define VIDEO_DECODER_COLOR_CONVERTER

namespace xgen {
namespace video {

typedef unsigned int uint;
typedef unsigned char uint8_t;

void yuv420ToRGBA(uint width, uint height,
                         const uint8_t* y, const uint8_t* u, const uint8_t* v,
                         unsigned int ystride, unsigned int ustride, unsigned int vstride,
                         unsigned char* out);

void yuv420AToRGBA(uint width, uint height,
                          const uint8_t* y, const uint8_t* u, const uint8_t* v, const uint8_t* a,
                          unsigned int ystride, unsigned int ustride, unsigned int vstride, unsigned int astride,
                          unsigned char* out);

void yuv420ToRGB(uint width, uint height,
                        const uint8_t* y, const uint8_t* u, const uint8_t* v,
                        unsigned int ystride, unsigned int ustride, unsigned int vstride,
                        unsigned char* out);

// Y в альфу компоненту буфферов
void yToA(uint width, uint height,
                 const uint8_t* a,
                 unsigned int ystride,
                 unsigned char* out);

}
}

#endif
