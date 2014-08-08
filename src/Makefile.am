bin_PROGRAMS = fvsearch fvscreenshot

fvsearch_SOURCES = search.cpp
fvsearch_CPPFLAGS = $(X11_CFLAGS) $(FFMPEG_CFLAGS)
fvsearch_LDADD = $(X11_LIBS) $(FFMPEG_LIBS) $(BOOST_PROGRAM_OPTIONS_LIB)

fvscreenshot_SOURCES = screenshot.cpp
fvscreenshot_CPPFLAGS = $(X11_CFLAGS) $(FFMPEG_CFLAGS)
fvscreenshot_LDADD = $(X11_LIBS) $(FFMPEG_LIBS) $(BOOST_PROGRAM_OPTIONS_LIB)
