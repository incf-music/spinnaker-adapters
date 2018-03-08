## Process this file with Automake to create Makefile.in

bin_PROGRAMS = spinnmusic-in spinnmusic-out


spinnmusic_in_SOURCES = spinnmusic-in.cpp MusicOutputAdapter.cpp MusicOutputAdapter.h rtclock.cpp rtclock.h
spinnmusic_in_CXXFLAGS = -I$(top_srcdir)/@LIBSPYNNAKER_EXTDEV_DIR@ @MPI_CXXFLAGS@
spinnmusic_in_LDADD = -L$(top_srcdir)/@LIBSPYNNAKER_EXTDEV_DIR@ -lspynnaker_external_device_lib -lmusic @MPI_LDFLAGS@ -lpthread -lsqlite3


spinnmusic_out_SOURCES = spinnmusic-out.cpp MusicInputAdapter.cpp MusicInputAdapter.h rtclock.cpp rtclock.h
spinnmusic_out_CXXFLAGS = -I$(top_srcdir)/@LIBSPYNNAKER_EXTDEV_DIR@ @MPI_CXXFLAGS@
spinnmusic_out_LDADD = -L$(top_srcdir)/@LIBSPYNNAKER_EXTDEV_DIR@ -lspynnaker_external_device_lib -lmusic @MPI_LDFLAGS@ -lpthread -lsqlite3
