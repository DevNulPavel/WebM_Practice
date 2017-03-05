#include "WebMVideoDecoder.h"
#include <chrono>
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее
#include "WebMYUVConverter.h"
extern "C" {
    #define HAVE_STDINT_H 1
    #include <libyuv.h>
}

using namespace std;


//#define speedtest_code
#ifdef speedtest_code           
    #define speedtest_begin(RANDOM_ID) std::chrono::high_resolution_clock::time_point t1##RANDOM_ID = std::chrono::high_resolution_clock::now();
    #define speedtest_end(RANDOM_ID) std::chrono::high_resolution_clock::time_point t2##RANDOM_ID = std::chrono::high_resolution_clock::now(); \
        auto duration##RANDOM_ID = std::chrono::duration_cast<std::chrono::microseconds>( t2##RANDOM_ID - t1##RANDOM_ID ).count(); \
        LOGD(#RANDOM_ID" execution time: %d microSec", duration##RANDOM_ID); //cout << #RANDOM_ID" executionTime: " << duration##RANDOM_ID << " microSec" << endl;
#else
    #define speedtest_begin(RANDOM_ID) {}
    #define speedtest_end(RANDOM_ID) {}
#endif


// читаем в буффер данные из потока
int decoderRead(void* buffer, size_t size, void* context) {
    ifstream* f = (ifstream*)context;
    f->read((char*)buffer, size);
    // success = 1
    // eof = 0
    // error = -1
    size_t currentCount = f->gcount();
    bool endOfFile = f->eof();
    return (currentCount == size) ? 1 : (endOfFile ? 0 : -1);
}

// смещаемся по потоку
int decoderSeek(int64_t n, int whence, void* context) {
    ifstream* f = (ifstream*)context;
    f->clear();
    ios_base::seekdir dir;
    
    switch (whence) {
        case NESTEGG_SEEK_SET:{
            dir = fstream::beg;
        }break;
            
        case NESTEGG_SEEK_CUR:{
            dir = fstream::cur;
        }break;
            
        case NESTEGG_SEEK_END:{
            dir = fstream::end;
        }break;
    }
    f->seekg(n, dir);
    if (!f->good()){
        return -1;
    }
    return 0;
}

int64_t decoderTell(void* context) {
    ifstream* f = (ifstream*)context;
    return f->tellg();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////


WebMVideoDecoder::WebMVideoDecoder(const std::string& filePath):
    _status(WebMDecodeStatus::FILE_READING),
    _filePath(filePath),
    _nesteg(nullptr),
    _videoDuration(0),
    _tracksCount(0),
    _videoCodecId(0),
    _videoCodecType(WebMCodecType::VP8),
    _decodedFrameDurationSec(0),
    _defaultFrameDurationNanoSec(0),
    _defaultFrameDurationSec(0),
    _curFrameDurationSec(0),
    _withAlpha(false),
    _vpxInterface(nullptr),
    _decodedBuffer(nullptr),
    _bufferStride(0),
    _decodedBufferSize(0),
    _deltaSumm(0.0f),
    _isLastFrameDecoded(false){
    
    // загрузка и инициализация
    int bufferInitStatus = loadDataStream();
    assert(bufferInitStatus == 0);
    
    int containerInitStatus = initializeWebMContainer();
    assert(containerInitStatus == 0);
    
    int codecInitStatus = initializeCodec();
    assert(codecInitStatus == 0);
    
    // статус - готов к декодированию
    setStatus(WebMDecodeStatus::READY);
}

WebMVideoDecoder::~WebMVideoDecoder(){
    // буффер с изображением для обновления
    if(_decodedBuffer != nullptr){
        free(_decodedBuffer);
    }
    
    // уничтожение кодеков
    if(vpx_codec_destroy(&_normalCodec)) {
        printf("Failed to destroy codec in file: %s", _filePath.c_str());
    }
    if (_withAlpha) {
        if(vpx_codec_destroy(&_alphaCodec)) {
            printf("Failed to destroy alpha codec in file: %s", _filePath.c_str());
        }
    }
    if (_nesteg) {
        nestegg_destroy(_nesteg);
    }
    
    // уничтожение потока
    _stream = nullptr;
}

#pragma mark - Getters and setters

WebMDecodeStatus WebMVideoDecoder::getStatus() const{
    return _status;
}

void WebMVideoDecoder::setStatus(WebMDecodeStatus newStatus){
    _status = newStatus;
}

// фактический размер
vec2 WebMVideoDecoder::getVideoSize() const {
    vec2 size(_nestegVideoParams.width, _nestegVideoParams.height);
    return size;
}

// размер, в котором нужно рисовать
vec2 WebMVideoDecoder::getVideoDisplaySize() const {
    vec2 size(_nestegVideoParams.display_width, _nestegVideoParams.display_height);
    return size;
}

// декодировали ли мы только что последний кадр видео?
bool WebMVideoDecoder::isLastFrameDecoded(){
    bool localCopy = _isLastFrameDecoded;
    return localCopy;
}

#pragma mark - Logic

// загрузка буффера файла
int WebMVideoDecoder::loadDataStream(){
    if (getStatus() != WebMDecodeStatus::FILE_READING) {
        return -1;
    }

    _stream = make_shared<std::ifstream>(_filePath, std::ifstream::in | std::ifstream::binary);
    _stream->sync();
    bool opened = _stream->is_open();

    // get length of file:
    _stream->seekg (0, _stream->end);
    size_t length = _stream->tellg();
    _stream->seekg (0, _stream->beg);
    printf("Video file length = %d\n", length);

    return (opened == true) ? 0 : -1;
}

int WebMVideoDecoder::initializeWebMContainer(){
    if (getStatus() != WebMDecodeStatus::FILE_READING) {
        return -1;
    }

    // обработчик данных
    nestegg_io nesteggIO;
    nesteggIO.read = decoderRead;
    nesteggIO.seek = decoderSeek;
    nesteggIO.tell = decoderTell;
    nesteggIO.userdata = (void*)(_stream.get());
    
    // открываем медиа-контейнер
    int r = nestegg_init(&_nesteg, nesteggIO, NULL, -1);
    assert(r == 0);
    
    // читаем длительность видео
    r = nestegg_duration(_nesteg, &_videoDuration);
    assert(r == 0);
    
    // читаем количество треков
    r = nestegg_track_count(_nesteg, &_tracksCount);
    assert(r == 0);
    
    // скейл времени перемотки
    r = nestegg_tstamp_scale(_nesteg, &_timeStampScale);
    if (_timeStampScale == 0) {
        _timeStampScale = 1;
    }
    assert(r == 0);
    
    // видео-параметры
    _nestegVideoParams.width = 0;
    _nestegVideoParams.height = 0;
    
    for (uint i = 0; i < _tracksCount; ++i) {
        // получаем id кодека
        _videoCodecId = nestegg_track_codec_id(_nesteg, i);
        assert(_videoCodecId >= 0);
        
        // тип трека
        int type = nestegg_track_type(_nesteg, i);
        
        // если у нас видео поток
        if (type == NESTEGG_TRACK_VIDEO) {
            // сохраняем кодек на будущее
            _videoCodecType = (_videoCodecId == NESTEGG_CODEC_VP9) ?
                                            WebMCodecType::VP9:
                                            WebMCodecType::VP8;
            
            // получим параметры текущего потока
            r = nestegg_track_video_params(_nesteg, i, &_nestegVideoParams);
            assert(r == 0);
            
            // получим частоту кадров видео
            _defaultFrameDurationNanoSec = 0;
            nestegg_track_default_duration(_nesteg, i, &_defaultFrameDurationNanoSec);
            if (_defaultFrameDurationNanoSec == 0) {
                _defaultFrameDurationNanoSec = static_cast<uint64>(1.0 / 24.0 * 1000.0 * 1000.0 * 1000.0);
            }
            _defaultFrameDurationSec = static_cast<double>(_defaultFrameDurationNanoSec) / 1000.0 / 1000.0 / 1000.0;
            
            
            // есть альфа?
            _withAlpha = (_nestegVideoParams.alpha_mode == 1);
            
            // выводим информацию
            const char* withAlphaText = _withAlpha ? "True" : "False";
            printf("Track: %d, codec id: %d, type: %d, default FPS: %d, "
                   "duration: %lldnSec, timestampScale: %lldnSec, "
                   "size: %dx%d, "
                   "display size: %dx%d "
                   "Alpha: %s",
                   i, _videoCodecType, type, static_cast<int>(1.0 / _defaultFrameDurationSec),
                   _videoDuration, _timeStampScale,
                   _nestegVideoParams.width, _nestegVideoParams.height,
                   _nestegVideoParams.display_width, _nestegVideoParams.display_height,
                   withAlphaText);
        }
        
        // аудио поток
        /*if (type == NESTEGG_TRACK_AUDIO) {
            nestegg_audio_params params;
            r = nestegg_track_audio_params(nesteg, i, &params);
            assert(r == 0);
            cout << params.rate << " " << params.channels << " channels " << " depth " << params.depth;
        }*/
    }
    
    return 0;
}

int WebMVideoDecoder::initializeCodec(){
    if (getStatus() != WebMDecodeStatus::FILE_READING) {
        return -1;
    }

    // Определяем какой кодек будем использовать (указатель на функцию)
    _vpxInterface = (_videoCodecId == NESTEGG_CODEC_VP9) ?
                                    &vpx_codec_vp9_dx_algo:
                                    &vpx_codec_vp8_dx_algo;
    
    // код ошибки
    vpx_codec_err_t res;
    
    //  config
    /*
    vpx_codec_dec_cfg_t codecConfig;
    memset(&codecConfig, 0, sizeof(codecConfig));
    codecConfig.threads = 4;
    codecConfig.w = 100;
    codecConfig.h = 100;
    
    vpx_codec_dec_cfg_t* configPtr = &codecConfig;
    int codecFlags = VPX_CODEC_CAP_FRAME_THREADING | VPX_CODEC_USE_FRAME_THREADING;
    */
    vpx_codec_dec_cfg_t* configPtr = nullptr;
    int codecFlags = 0;
    
    // Инициализация кодека для обычного видео
    if((res = vpx_codec_dec_init(&_normalCodec, _vpxInterface, configPtr, codecFlags))) {
        return -1;
    }
    
    // инициализация кодека для альфы
    if (_withAlpha){
        if((res = vpx_codec_dec_init(&_alphaCodec, _vpxInterface, configPtr, codecFlags))) {
            return -1;
        }
    }
    
    // TODO: может быть можно использовать 16ти битный формат? RGBA_4444
    // создаем временный блок памяти, в который будем выгружать данные и конвертировать в RGBA
    speedtest_begin(MALLOC_TIME);
    _decodedBufferSize = 0;
    _bufferStride = 4;
    _decodedBufferSize = _bufferStride * _nestegVideoParams.width * _nestegVideoParams.height;
    /*if (_withAlpha) {   // TODO: Сейчас поддержка отрисовки только с альфой
        _bufferStride = 4;
        _decodedBufferSize = _bufferStride * _nestegVideoParams.width * _nestegVideoParams.height;
    }else{
        _bufferStride = 3;
        _decodedBufferSize = _bufferStride * _nestegVideoParams.width * _nestegVideoParams.height;
    }*/
    _decodedBuffer = (u8*)malloc(_decodedBufferSize);
    speedtest_end(MALLOC_TIME);
    
    // Информация о кодеке
    printf("WebM decoder for file \"%s\": %s", _filePath.c_str(), vpx_codec_iface_name(_vpxInterface));
    
    return 0;
}

void WebMVideoDecoder::decodeNewFrame(){
    // проверка статуса
    if((getStatus() != WebMDecodeStatus::READY) && (getStatus() != WebMDecodeStatus::WAIT_DECODE)){
        return;
    }
    
    // статус - декодируем
    setStatus(WebMDecodeStatus::DECODING);

    // сброс флага последнего кадра
    _isLastFrameDecoded = false;
    
    // время текущего фрейма
    uint64_t frameDurationNanoSec = 0;

    // флаг завершения
    bool decodingComplete = false;

    nestegg_packet* packet = nullptr;
    while (!decodingComplete) {
        speedtest_begin(READ);
        int r = 0;
        // читаем пакет пока не прочитается
        // 1 = keep calling
        // 0 = eof
        // -1 = error
        r = nestegg_read_packet(_nesteg, &packet);
        // продолжаем читать
        if ((r == 1) && (packet == nullptr)){
            continue;
        }
        // завершилось видео
        if (r == 0) {
            // чистим пакет
            if(packet){
                nestegg_free_packet(packet);
            }
        
            // переход к началу
            for (uint i = 0; i < _tracksCount; ++i){
                nestegg_track_seek(_nesteg, i, 0);
            }
            continue;
        }
        // выход при ошибке
        if (r <= 0){
            if(packet){
                nestegg_free_packet(packet);
            }
        
            // статус - ошибка декодирования
            setStatus(WebMDecodeStatus::DECODE_ERROR);
            break;
        }
        speedtest_end(READ);
        
        // получаем трек из пакета
        unsigned int track = 0;
        r = nestegg_packet_track(packet, &track);
        assert(r == 0);
        
        // если трек-видео
        bool isVideo = (nestegg_track_type(_nesteg, track) == NESTEGG_TRACK_VIDEO);
        if (isVideo) {
            speedtest_begin(DECODE);
            // количество пакетов
            uint packetsCount = 0;
            r = nestegg_packet_count(packet, &packetsCount);
            assert(r == 0);
            
            // получаем длительность текущего отображения кадра
            
            for (uint j = 0; j < packetsCount; ++j) { // for (int j=0; j < 1; ++j) { // проверка проигрывания одного кадра
                // чтение
                // CODE HERE!!!
                
                // чтение альфы
                // CODE HERE!!!
                
                // Выполнение декодирования кадра
                // CODE HERE!!!
                
                // Если есть альфа, то декодим альфу
                if (_withAlpha) {
                    // Выполнение декодирования альфы кадра
                    // CODE HERE!!!
                }
            }
            speedtest_end(DECODE);
            
            // чтение
            speedtest_begin(CONVERT);
            if (_withAlpha) {
                // чтение с альфой
                // CODE HERE!!!
            } else{
                // чтение без альфы
                // CODE HERE!!!
            }
            speedtest_end(CONVERT);
            
            decodingComplete = true;
        }
        
        // текущее время кадра
        uint64_t curTimeStamp = 0;
        nestegg_packet_tstamp(packet, &curTimeStamp);
        
        speedtest_begin(PACKET_FREE);
        // чистим пакет
        nestegg_free_packet(packet);
        speedtest_end(PACKET_FREE);
        
        // декодинг завершился, статус - раскодировали
        // специально в самом конце, чтобы не было проблем с потоками
        if (decodingComplete == true) {
            // последний кадр (время)
            uint64_t end = 0;
            if (_defaultFrameDurationNanoSec) {
                end = (_videoDuration - _defaultFrameDurationNanoSec); // -1 кадр в конце
            }else if(frameDurationNanoSec){
                end = (_videoDuration - frameDurationNanoSec); // -1 кадр в конце
            }else{
                end = (_videoDuration - 1000 * 1000 * 500); // если у видео нету частоты кадров, то считаем концом -0.5 секунды от конца
            }
            if (curTimeStamp >= end) {
                _isLastFrameDecoded = true;
            }
            
            // статус
            setStatus(WebMDecodeStatus::DECODED);
        }
    }
}

// сохраняем декодированные данные в текстуру
void WebMVideoDecoder::copyDataToTexture(uint textureid){
    // скопировать можно только раскодированный фрейм
    if (getStatus() != WebMDecodeStatus::DECODED) {
        return;
    }
    
    // выводим в текстуру
    if (_decodedBuffer) {
        // CODE HERE!!!
    }
    
    // обнуляем дельту для поддержки фпс
    _deltaSumm = 0;
    
    // обновляем длительность текущего кадра
    double decodedFrameDuration = _decodedFrameDurationSec;
    _curFrameDurationSec = decodedFrameDuration;
    
    // удаляем буффер для декодирования
    /*speedtest_begin(FREE_TIME);
    if (_decodedBuffer) {
        _decodedBufferSize = 0;
        
        free(_decodedBuffer);
        _decodedBuffer = nullptr;
    }
    speedtest_end(FREE_TIME);*/
    
    // статус - можем сново декодировать
    setStatus(WebMDecodeStatus::READY);
}

float WebMVideoDecoder::getFrameDuration(){
    // для поддержки нужного значения FPS (delta/2.0 - хитрый способ компенсации несоответствия частоты кадров фильма и игры)
    if (_curFrameDurationSec <= 0.0f) {
        return _defaultFrameDurationSec;
    }
    return _curFrameDurationSec;
}

