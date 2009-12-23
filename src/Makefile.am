AUTOMAKE_OPTIONS = foreign no-dependencies

if COMPILER_IS_GCC
AM_CFLAGS = -Wall -Werror
if BUILD_WITH_DEBUG
AM_CFLAGS += -Wextra -g -O0
endif
endif

include_HEADERS = routeros_api.h routeros_version.h
lib_LTLIBRARIES = librouteros.la

librouteros_la_LDFLAGS = -version-info @LIBROUTEROS_CURRENT@:@LIBROUTEROS_REVISION@:@LIBROUTEROS_AGE@
librouteros_la_LDFLAGS += -export-symbols-regex "^ros_"
librouteros_la_LIBADD = -lgcrypt
if BUILD_WITH_LIBSOCKET
librouteros_la_LIBADD += -lsocket
endif
librouteros_la_SOURCES = main.c routeros_api.h routeros_version.h \
			 ros_parse.c ros_parse.h \
			 registration_table.c \
			 interface.c \
			 system_resource.c

bin_PROGRAMS = ros

ros_SOURCES = ros.c
ros_LDADD = librouteros.la
