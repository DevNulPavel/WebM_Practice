#ifndef VIDEO_DECODER_XGEN_H
#define VIDEO_DECODER_XGEN_H

#include <memory>
#include <atomic>
#include <fstream>
#include <iostream>
#include <memory>
#include <glm.hpp>
extern "C" {
    #define HAVE_STDINT_H 1
    #include <nestegg/nestegg.h>
    #include <vpx/vpx_decoder.h>
    #include <vpx/vp8dx.h>
}


using namespace std;
using namespace glm;


enum class WebMDecodeStatus{
    FILE_READING = 0,
    READY = 1,
    WAIT_DECODE = 2,
    DECODING = 3,
    DECODED = 4,
    DECODE_ERROR = 5
};

enum WebMCodecType{
    VP8 = 0,
    VP9 = 1
};

class WebMVideoDecoder {
public:
    WebMVideoDecoder(const std::string& filePath);
    ~WebMVideoDecoder();
    // getters and setters
    WebMDecodeStatus getStatus() const;
    vec2 getVideoSize() const;           // фактический размер
    vec2 getVideoDisplaySize() const;    // размер, в котором нужно рисовать
    // methods
    float getFrameDuration();
    void copyDataToTexture(uint textureid);
    void decodeNewFrame();          // декодирование
    bool isLastFrameDecoded();      // декодировали ли мы только что последний кадр видео?
    
private:
    // meta
    WebMDecodeStatus _status;
    std::string _filePath;
    // file data
    std::shared_ptr<std::ifstream> _stream;
    // Container + codec
    nestegg* _nesteg;
    uint64 _videoDuration;
    uint64 _timeStampScale;
    uint _tracksCount;
    nestegg_video_params _nestegVideoParams;
    int _videoCodecId;
    WebMCodecType _videoCodecType;
    double _decodedFrameDurationSec;
    uint64 _defaultFrameDurationNanoSec;
    double _defaultFrameDurationSec;
    double _curFrameDurationSec;
    bool _withAlpha;
    vpx_codec_iface_t* _vpxInterface;
    vpx_codec_ctx_t _normalCodec;
    vpx_codec_ctx_t _alphaCodec;
    u8* _decodedBuffer;
    uint _bufferStride;
    size_t _decodedBufferSize;
    float _deltaSumm;
    bool _isLastFrameDecoded;
    
private:
    void setStatus(WebMDecodeStatus newStatus);
    int loadDataStream();
    int initializeWebMContainer();
    int initializeCodec();
};

typedef shared_ptr<WebMVideoDecoder> WebMVideoDecoderPtr;


#endif // VIDEO_SPRITE_XGEN_H
