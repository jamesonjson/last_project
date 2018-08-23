rtems_tests_PROGRAMS = smpmutex03
smpmutex03_SOURCES = init.c tasks.c system_conf.h ../../support/src/locked_print.c

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(smpmutex03_OBJECTS)
LINK_LIBS = $(smpmutex03_LDLIBS)

smpmutex03$(EXEEXT): $(smpmutex03_OBJECTS) $(smpmutex03_DEPENDENCIES)
	@rm -f smpmutex03$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
