AM_CPPFLAGS = \
	-DPTHREADS		\
	$(X11_DEPS_CFLAGS)\
	$(DRM_CFLAGS)		\
	$(LIBVA_DEPS_CFLAGS)

driver_cflags = \
	-Wall			\
	-fvisibility=hidden

driver_ldflags = \
	-module -avoid-version	\
	-no-undefined		\
	-Wl,--no-undefined

driver_libs = \
	-lpthread -ldl		\
	$(DRM_LIBS)		\
	$(X11_DEPS_LIBS)	\
	$(LIBVA_DEPS_LIBS)

source_c = dump.c object_heap.c buffer.c config.c context.c image.c picture.c \
	subpicture.c surface.c header.c

source_h = dump.h object_heap.h buffer.h config.h context.h image.h picture.h \
	subpicture.h surface.h header.h

dump_drv_video_la_LTLIBRARIES	= dump_drv_video.la
dump_drv_video_ladir		= $(LIBVA_DRIVERS_PATH)
dump_drv_video_la_CFLAGS	= $(driver_cflags)
dump_drv_video_la_LDFLAGS	= $(driver_ldflags)
dump_drv_video_la_LIBADD	= $(driver_libs)
dump_drv_video_la_SOURCES	= $(source_c)
noinst_HEADERS				= $(source_h)

MAINTAINERCLEANFILES = Makefile.in autoconfig.h.in
