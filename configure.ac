AC_INIT([fastvideo], [0.1], [i.be.brett@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_LANG([C++])
AC_PROG_CXX
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
])

PKG_CHECK_MODULES([FFMPEG], [
	libavdevice,
	libavformat,
	libavdevice,
	libavfilter,
	libavcodec,
	libswresample,
	libswscale, 
	libavutil
])

PKG_CHECK_MODULES(X11, [x11])

AC_CHECK_HEADER([CImg.h])

AC_OUTPUT
