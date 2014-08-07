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
#include <vector>
#include <cmath>

using namespace std;

inline uint32_t getImagePixel(CImg<uint8_t>* pImage, int x, int y) {
    uint32_t rgb = (uint32_t)(*pImage)(x,y,0,0);
    rgb = (rgb << 8) + (uint32_t)(*pImage)(x,y,0,1);
    rgb = (rgb << 8) + (uint32_t)(*pImage)(x,y,0,2);
    return rgb;
}


int pixelDiff(CImg<uint8_t>* pImage1, CImg<uint8_t>* pImage2, int x, int y) {
    // take difference between rg,b
    uint8_t r_diff = abs(((*pImage1)(x,y,0,0)-(*pImage2)(x,y,0,0)))/4;
    uint8_t g_diff = abs(((*pImage1)(x,y,0,1)-(*pImage2)(x,y,0,1)))/4;
    uint8_t b_diff = abs(((*pImage1)(x,y,0,2)-(*pImage2)(x,y,0,2)))/4;
    
    return r_diff + g_diff + b_diff;
}

int diffRegion(CImg<uint8_t>* pImage1, CImg<uint8_t>* pImage2, int sx, int sy, int width, int height) {
    int sum = 0;
    for(int x = sx; x < sx+width; x++) {
        for(int y = sy; y < sy+height; y++) {
            sum += pixelDiff(pImage1, pImage2, x, y);
        }
    }
    return sum;
}

int regionedDiff(CImg<uint8_t>* pImage1, CImg<uint8_t>* pImage2, int width, int height) {   
    int smallest = -1;
   
    for(int base_y = 0; base_y < 30; base_y+=10) {
        for( int base_x = 0; base_x < 30; base_x+=10) {
            int diff = diffRegion(pImage1, pImage2, base_x, base_y, 10, 10);
            if(smallest == -1 || diff < smallest) {
                smallest = diff;
            }
        }
    }

    return smallest;
}

int checkFrame(AVFrame* pFrame, vector<CImg<uint8_t>*> screenshots, int width, int height) {
    // first get from the frame
    CImg<uint8_t> pImage;// new CImg<uint8_t>();

    pImage.assign(*pFrame->data, 3, width, height, 1, true);
    pImage.permute_axes("yzcx");

    for(vector<CImg<uint8_t>*>::iterator it1 = screenshots.begin(); it1 != screenshots.end(); ++it1) { 
        int diff = regionedDiff((*it1), &pImage, 30, 30);
        if(diff < 500.0) {
            return 1;
        }
    }

    return 0;
}

vector<CImg<uint8_t>*> loadScreenshots(char** argv, int start, int argc) {
    vector<CImg<uint8_t>*> screenshots;
    
    for(int i = start; i < argc; i++) {
        CImg<uint8_t>* pNewImage = new CImg<uint8_t>();
        pNewImage->load(argv[i]);
        pNewImage->resize(30, 30); // resize using fastest resize algo 
        screenshots.push_back(pNewImage);
    }
    return screenshots;
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
 
  vector<CImg<uint8_t>*> screenshots = loadScreenshots(argv, 2, argc);

  // Register all formats and codecs
  av_register_all();
  
  // Open video file
  if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
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
        30,//pCodecCtx->width,
        30,//pCodecCtx->height,
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
        30, 30);
         
         
    // Read frames and save first five frames to disk
    i=0;
    int frames=0;
    int total_matches=0;
    while(av_read_frame(pFormatCtx, &packet)>=0) { 
      // Is this a packet from the video stream?
      if(packet.stream_index==videoStream) {
    // Decode video frame
        avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, 
               &packet);
      
      // Did we get a video frame?
      if(frameFinished) {
        frames++;
        if(frames % 5 == 0) {
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
            total_matches += checkFrame(pFrameRGB, screenshots, 30, 30);
        }
     }
     i++;
    }
    
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }
  cout << i << " iterations " << frames << " frames " << (double)total_matches/(double)frames * 100.0 << endl;
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
