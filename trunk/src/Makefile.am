bin_PROGRAMS = tou
tou_SOURCES = proj.cpp
INCLUDES = -I./lib -I../boostlib/boost_1_40_0
FLAGS = -g
LIBS = -lpthread -L ../boostlib/boost_1_40_0/stage/lib -lboost_date_time -lboost_system -lboost_regex -lboost_thread -lpthread -static


# ../boostlib/boost_1_40_0/stage/lib/libboost_data_time.a \
# 	../boostlib/boost_1_40_0/stage/lib/libboost_system.a \
# 	../boostlib/boost_1_40_0/stage/lib/libboost_regex.a \
# 	../boostlib/boost_1_40_0/stage/lib/libboost_thread.a

LDADD = lib/libtou.a 

SUBDIRS = lib
CLEANFILES = *~ *.o tou
