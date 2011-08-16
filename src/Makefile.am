AM_CFLAGS = $(CWARNFLAGS) $(libev_CFLAGS)

lib_LTLIBRARIES = libeh.la

libeh_la_SOURCES = \
	eh_alloc.c eh_buffer.c eh_connection.c eh_fmt_cstr.c \
	eh_fmt_int.c eh_log.c eh_serial.c eh_server.c eh_socket.c

include_HEADERS = \
	eh.h eh_alloc.h eh_buffer.h eh_connection.h eh_fd.h \
	eh_fmt.h eh_list.h eh_log.h eh_serial.h eh_server.h \
	eh_socket.h eh_watcher.h
