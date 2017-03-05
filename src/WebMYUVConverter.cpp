#include "WebMYUVConverter.h"


namespace xgen {
namespace video {
        
void yuv420ToRGBA(uint width, uint height,
                  const uint8_t* y, const uint8_t* u, const uint8_t* v,
                  unsigned int ystride, unsigned int ustride, unsigned int vstride,
                  unsigned char* out){
    unsigned long int i, j;
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            unsigned long pointIndex = height - i;
            unsigned char* point = out + 4 * ((pointIndex * width) + j);
            
            int t_y = y[((i * ystride) + j)];
            int t_u = u[(((i / 2) * ustride) + (j / 2))];
            int t_v = v[(((i / 2) * vstride) + (j / 2))];
            t_y = t_y < 16 ? 16 : t_y;
            
            int r = (298 * (t_y - 16) + 409 * (t_v - 128) + 128) >> 8;
            int g = (298 * (t_y - 16) - 100 * (t_u - 128) - 208 * (t_v - 128) + 128) >> 8;
            int b = (298 * (t_y - 16) + 516 * (t_u - 128) + 128) >> 8;
            
            point[0] = (unsigned char)(r>255? 255 : r<0 ? 0 : r);
            point[1] = (unsigned char)(g>255? 255 : g<0 ? 0 : g);
            point[2] = (unsigned char)(b>255? 255 : b<0 ? 0 : b);
            point[3] = 0;
        }
    }
}

void yuv420AToRGBA(uint width, uint height,
                   const uint8_t* y, const uint8_t* u, const uint8_t* v, const uint8_t* a,
                   unsigned int ystride, unsigned int ustride, unsigned int vstride, unsigned int astride,
                   unsigned char* out){
    unsigned long int i, j;
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            unsigned long pointIndex = height - i;
            unsigned char* point = out + 4 * ((pointIndex * width) + j);
            
            int t_y = y[((i * ystride) + j)];
            int t_u = u[(((i / 2) * ustride) + (j / 2))];
            int t_v = v[(((i / 2) * vstride) + (j / 2))];
            t_y = t_y < 16 ? 16 : t_y;
            
            int t_a = a[((i * astride) + j)];
            
            int r = (298 * (t_y - 16) + 409 * (t_v - 128) + 128) >> 8;
            int g = (298 * (t_y - 16) - 100 * (t_u - 128) - 208 * (t_v - 128) + 128) >> 8;
            int b = (298 * (t_y - 16) + 516 * (t_u - 128) + 128) >> 8;
            
            point[0] = (unsigned char)(r>255? 255 : r<0 ? 0 : r);
            point[1] = (unsigned char)(g>255? 255 : g<0 ? 0 : g);
            point[2] = (unsigned char)(b>255? 255 : b<0 ? 0 : b);
            point[3] = (unsigned char)(t_a>255? 255 : t_a<0 ? 0: t_a);
        }
    }
}

void yuv420ToRGB(uint width, uint height,
                 const uint8_t* y, const uint8_t* u, const uint8_t* v,
                 unsigned int ystride, unsigned int ustride, unsigned int vstride,
                 unsigned char* out){
    unsigned long int i, j;
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            unsigned long pointIndex = height - i;
            unsigned char* point = out + 3 * ((pointIndex * width) + j);
            
            int t_y = y[((i * ystride) + j)];
            int t_u = u[(((i / 2) * ustride) + (j / 2))];
            int t_v = v[(((i / 2) * vstride) + (j / 2))];
            t_y = t_y < 16 ? 16 : t_y;
            
            int r = (298 * (t_y - 16) + 409 * (t_v - 128) + 128) >> 8;
            int g = (298 * (t_y - 16) - 100 * (t_u - 128) - 208 * (t_v - 128) + 128) >> 8;
            int b = (298 * (t_y - 16) + 516 * (t_u - 128) + 128) >> 8;
            
            point[0] = (unsigned char)(r>255? 255 : r<0 ? 0 : r);
            point[1] = (unsigned char)(g>255? 255 : g<0 ? 0 : g);
            point[2] = (unsigned char)(b>255? 255 : b<0 ? 0 : b);
        }
    }
}

// Y в альфу компоненту буфферов
void yToA(uint width, uint height,
          const uint8_t* a,
          unsigned int ystride,
          unsigned char* out){
    unsigned long int i, j;
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            unsigned long pointIndex = height - i;
            unsigned char* point = out + 4 * ((pointIndex * width) + j);
            
            int t_a = a[((i * ystride) + j)];
            
            point[3] = (unsigned char)(t_a>255? 255 : t_a<0 ? 0 : t_a);
        }
    }
}
        
}
}
