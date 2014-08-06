export PKG_CONFIG_PATH=/home/ubuntu/ffmpeg_build/lib/pkgconfig:$PKG_CONFIG_PATH
g++ screenshot.cpp -o screenshot $(pkg-config --libs --cflags libavformat libswscale) -lrt
g++ search.cpp -o search $(pkg-config --libs --cflags libavformat libswscale) -lrt

