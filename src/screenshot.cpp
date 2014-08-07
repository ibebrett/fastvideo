#define __STDC_CONSTANT_MACROS

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

#include <stdio.h>
#include <CImg.h>

using namespace cimg_library;

#include <string>
#include <sstream>
#include <iostream>
#include <set>
#include <cmath>

using namespace std;

CImg<uint8_t>* ProcessFrame(AVFrame *pFrame, int width, int height, CImg<uint8_t>* last_image) {
    CImg<uint8_t>* next_image = new CImg<uint8_t>();

    set<uint32_t> hist;
    next_image->assign(*pFrame->data, 3, width, height, 1, true);
    next_image->permute_axes("yzcx");

    for(int t = 0; t < 1000; t++) {
        int x = rand() % width;
        int y = rand() % height;
        uint32_t rgb = (uint32_t)(*next_image)(x,y,0,0);
        rgb = (rgb << 8) + (uint32_t)(*next_image)(x,y,0,1);
        rgb = (rgb << 8) + (uint32_t)(*next_image)(x,y,0,2);
        hist.insert(rgb);
    }

    if(hist.size() < 100) {
        delete next_image;
        return last_image;

    } else {
        if(last_image != NULL)
            delete last_image;
        return next_image;
    }
}

int main(int argc, char *argv[]) {
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
  struct SwsContext      *sws_ctx = NULL;
 
  CImg<uint8_t>* best_image = NULL;

  if(argc < 3) {
    printf("Please provide a movie file and an output file\n");
    return -1;
  }
  // Register all formats and codecs
  av_register_all();
  
  // Open video file
  if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
    return -1; // Couldn't open file
  
  // Retrieve stream information
  if(avformat_find_stream_info(pFormatCtx, NULL)<0)
    return -1; // Couldn't find stream information
  
  // Dump information about file onto standard error
  //av_dump_format(pFormatCtx, 0, argv[1], 0);
  
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
  
  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }
  // Open codec
  if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
    return -1; // Could not open codec
  
  // Allocate video frame
  pFrame=av_frame_alloc();
  
  // Allocate an AVFrame structure
  pFrameRGB=av_frame_alloc();
  if(pFrameRGB==NULL)
    return -1;
  
  // Determine required buffer size and allocate buffer
  numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
                  pCodecCtx->height);
  buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

  sws_ctx =
    sws_getContext
    (
        pCodecCtx->width,
        pCodecCtx->height,
        pCodecCtx->pix_fmt,
        pCodecCtx->width,
        pCodecCtx->height,
        PIX_FMT_RGB24,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
    );
  
  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
         pCodecCtx->width, pCodecCtx->height);
         
  // seek to the last 30 seconds of the video, if there isn't a valid screenshot there then we don't care
  // duration - 30*
  int64_t seek_pos = pFormatCtx->duration - 60*AV_TIME_BASE;
  if(seek_pos < 0)
        seek_pos = 0;
  int err = 0;
  err = av_seek_frame(pFormatCtx, videoStream, seek_pos, AVSEEK_FLAG_BACKWARD);
  if(err < 0) {
        char errcode[1024];
        av_strerror(err, errcode, 1024);
        cerr << "error while seeking to " << seek_pos << " with code " << errcode << endl;
        return 0;
  }
         
  // Read frames and save first five frames to disk
  i=0;
  int num_conversions=0;
  while(av_read_frame(pFormatCtx, &packet)>=0) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, 
               &packet);
      
      // Did we get a video frame?
      if(frameFinished && i % 10 == 0 ) { 
        // Convert the image from its native format to RGB
        sws_scale
        (
            sws_ctx,
            (uint8_t const * const *)pFrame->data,
            pFrame->linesize,
            0,
            pCodecCtx->height,
            pFrameRGB->data,
            pFrameRGB->linesize
        );

        best_image = ProcessFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, best_image);
        num_conversions+=1;
     }
     i++;
    }
    
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }
 
  // write the screenshot file if we can
  if(best_image != NULL) {
        cerr << "Converted " << num_conversions << " images" << endl;
        best_image->save_png(argv[2]);
        delete best_image;
  } else {
        cerr << "Could not generate screenshot" << endl;
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
