bin_PROGRAMS = fvsearch fvscreenshot fvtestimagecompare

fvtestimagecompare_SOURCES = test_image_compare.cpp image_process.cpp image_process.hpp
fvtestimagecompare_CPPFLAGS = $(X11_CFLAGS) $(FFMPEG_CFLAGS)
fvtestimagecompare_LDADD = $(BOOST_PROGRAM_OPTIONS_LIB) $(X11_LIBS)

fvsearch_SOURCES = search.cpp image_process.cpp video_process.cpp image_process.hpp video_process.hpp
fvsearch_CPPFLAGS = $(X11_CFLAGS) $(FFMPEG_CFLAGS)
fvsearch_LDADD = $(BOOST_PROGRAM_OPTIONS_LIB) $(X11_LIBS) $(FFMPEG_LIBS)

fvscreenshot_SOURCES = screenshot.cpp image_process.cpp video_process.cpp image_process.hpp video_process.hpp
fvscreenshot_CPPFLAGS = $(X11_CFLAGS) $(FFMPEG_CFLAGS)
fvscreenshot_LDADD = $(BOOST_PROGRAM_OPTIONS_LIB) $(X11_LIBS) $(FFMPEG_LIBS)
