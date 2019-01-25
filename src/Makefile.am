AM_CPPFLAGS = -DPTHREADS $(DRM_CFLAGS) $(LIBVA_DEPS_CFLAGS)

backend_cflags = -Wall -fvisibility=hidden
backend_ldflags = -module -avoid-version -no-undefined -Wl,--no-undefined
backend_libs = -lpthread -ldl $(DRM_LIBS) $(LIBVA_DEPS_LIBS)

backend_c = dump.c object_heap.c config.c surface.c context.c buffer.c \
	header.c header_mpeg2.c header_h264.c header_h265.c picture.c \
	subpicture.c image.c

backend_h = dump.h object_heap.h config.h surface.h context.h buffer.h \
	header.h picture.h subpicture.h image.h

dump_drv_video_la_LTLIBRARIES = dump_drv_video.la
dump_drv_video_ladir = $(LIBVA_DRIVERS_PATH)
dump_drv_video_la_CFLAGS = $(backend_cflags)
dump_drv_video_la_LDFLAGS = $(backend_ldflags)
dump_drv_video_la_LIBADD = $(backend_libs)
dump_drv_video_la_SOURCES = $(backend_c)
noinst_HEADERS = $(backend_h)

MAINTAINERCLEANFILES = Makefile.in autoconfig.h.in
