# FastVideo
A set of tools to quickly perform video tasks

fv_screenshot - Searches through the video for a "nice" screenshot as close to the end as possible.

fv_search - Compares a video to a list of noisy screenshots to determine the likelihood that those screenshots came from that video file.

# Dependencies

You need ffmpeg, CImg, Boost, and X11. To do conversions to useful formats it is preferable to have imagemagick installed (CImg will use it).

For ffmpeg, building from source is the best bet; this guide for compiling on ubuntu works well:

https://trac.ffmpeg.org/wiki/CompilationGuide/Ubuntu

To build fast video, the ffmpeg libraries must be on the PKG_CONFIG_PATH. Additionally there seems to be a bug in ffmpeg that doesn't add librt as a dependancy for the ubuntu build of avutil. I suggest changing its package config file to contain the switch -lrt. 

