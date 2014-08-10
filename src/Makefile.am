bin_PROGRAMS = fvsearch fvscreenshot

fvsearch_SOURCES = search.cpp image_process.cpp video_process.cpp image_process.hpp video_process.hpp
fvsearch_CPPFLAGS = $(X11_CFLAGS) $(FFMPEG_CFLAGS)
fvsearch_LDADD = $(BOOST_PROGRAM_OPTIONS_LIB) $(X11_LIBS) $(FFMPEG_LIBS)

fvscreenshot_SOURCES = screenshot.cpp 
fvscreenshot_CPPFLAGS = $(X11_CFLAGS) $(FFMPEG_CFLAGS)
fvscreenshot_LDADD = $(BOOST_PROGRAM_OPTIONS_LIB) $(X11_LIBS) $(FFMPEG_LIBS)
