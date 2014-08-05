g++   -L/opt/local/lib  -I/usr/X11R6/include -L/usr/X11R6/lib  -lavformat -lavcodec -lswscale -lavutil -lSDLmain -Wl,-framework,AppKit -lSDL -Wl,-framework,Cocoa  -lm screenshot.cpp -lpthread -lX11 -o screenshot

