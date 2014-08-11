#include "video_process.hpp"

#include <iostream>
#include <string>

void video_processor::print_error(int err, const std::string &message) {
    char errcode[1024];
    av_strerror(err, errcode, 1024);

    std::cerr << message << " "<< errcode << std::endl;
}

int video_processor::iterate(
    video_worker &worker,
    std::string video_path,
    int start_from, 
    int start_from_back,
    int frame_skip,
    int width,
    int height) {

    AVFormatContext *pFormatCtx = NULL;
    int             i, videoStream;
    AVCodecContext  *pCodecCtx = NULL;
    AVCodec         *pCodec = NULL;
    AVFrame         *pFrame = NULL; 
    AVFrame         *pFrameRGB = NULL;
    AVPacket        packet;
    int             frameFinished;
    int             numBytes;
    uint8_t         *buffer = NULL;

    AVDictionary    *optionsDict = NULL;
    SwsContext      *sws_ctx = NULL;

    // Register all formats and codecs
    av_register_all();

    // Open video file
    if(avformat_open_input(&pFormatCtx, video_path.c_str(), NULL, NULL)!=0)
        return -1; // Couldn't open file  

    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
        return -1; // Couldn't find stream information

    // Find the first video stream
    videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
        videoStream=i;
        break;
    }
    if(videoStream==-1)
        return -1; // Didn't find a video stream

    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;
    //pCodecCtx->lowres=2;
    //pCodecCtx->skip_frame=AVDISCARD_ALL;

    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
        return -1; // Could not open codec

    // set default width and height if they are 0
    if(width  == 0) width  = pCodecCtx->width;
    if(height == 0) height = pCodecCtx->height;

    // Allocate video frame
    pFrame=av_frame_alloc();

    // Allocate an AVFrame structure
    pFrameRGB=av_frame_alloc();
    if(pFrameRGB==NULL)
        return -1;

    // Determine required buffer size and allocate buffer
    numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    sws_ctx = sws_getContext(
        pCodecCtx->width,
        pCodecCtx->height,
        pCodecCtx->pix_fmt,
        width,
        height,
        PIX_FMT_RGB24,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
    );

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, width, height);

    // try to seek to the correct place start_from or start_from_back
    // is specified
    int64_t seek_pos = 0;

    if(start_from != 0) {
        seek_pos = start_from*AV_TIME_BASE;
    } else if(start_from_back != 0) {
        seek_pos = pFormatCtx->duration - start_from_back*AV_TIME_BASE;
    }
    // rescale time
    seek_pos = av_rescale_q(seek_pos, AV_TIME_BASE_Q,  pFormatCtx->streams[videoStream]->time_base);
    if(seek_pos < 0)
        seek_pos = 0;

    if(seek_pos != 0) {
        std::cerr << " seeking video to " << seek_pos << std::endl;
        int err = av_seek_frame(pFormatCtx, videoStream, seek_pos, 0);//, AVSEEK_FLAG_BACKWARD);
        if(err < 0) {
            print_error(err, "error during seek");
        }
    }

    // Read frames and save first five frames to disk
    int frames=0;
    int total_matches=0;

    CImg8 image_frame;
    while(av_read_frame(pFormatCtx, &packet)>=0) { 
        // Is this a packet from the video stream?
        if(packet.stream_index==videoStream) {
            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

            // Did we get a video frame?
            if(frameFinished) {
                if(frame_skip == 0 || frames % frame_skip == 0) {
                    // Convert the image from its native format to RGB
                    sws_scale(
                        sws_ctx,
                        (uint8_t const * const *)pFrame->data,
                        pFrame->linesize,
                        0,
                        pCodecCtx->height,
                        pFrameRGB->data,
                        pFrameRGB->linesize
                    );

                    // convert into CImg
                    image_frame.assign(*pFrameRGB->data, 3, width, height, 1, true);
                    image_frame.permute_axes("yzcx");

                    worker.process_frame(image_frame, frames);
                }
                frames++;
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }

    // Free the RGB image
    av_free(buffer);
    av_free(pFrameRGB);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    return 0;
}
