export PKG_CONFIG_PATH=/home/ubuntu/ffmpeg_build/pkgconfig:$PKG_CONFIG_PATH
g++ screenshot.cpp -o screenshot $(pkg-config --libs --cflags libavformat libswscale) -lrt

