AR = ar
CXX = g++
CXXFLAGS = -g -gstabs+ -ggdb -Wall -Wno-deprecated
LDFLAGS = 

LIB_OBJS = block.o         \
           disksystem.o    \
           buffercache.o   \
           btree.o         \
           btree_ds.o      \

EXEC_OBJS = \
makedisk.o \
infodisk.o \
readdisk.o \
writedisk.o \
deletedisk.o \
readbuffer.o \
writebuffer.o \
freebuffer.o \
btree_init.o \
btree_insert.o \
btree_update.o \
btree_delete.o \
btree_lookup.o \
btree_show.o \
btree_sane.o \
btree_display.o \
sim.o 

EXECS=$(EXEC_OBJS:.o=)

OBJS = $(LIB_OBJS) $(EXEC_OBJS)


all: $(EXECS)

%.o : %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $(@F)

libbtreelab.a: $(LIB_OBJS)
	$(AR) ruv libbtreelab.a $(LIB_OBJS)


$(EXECS): % : %.o libbtreelab.a
	$(CXX) $(LDFLAGS) $< libbtreelab.a -o $(@F)

depend:
	$(CXX) $(CXXFLAGS) -MM $(OBJS:.o=.cc) > .dependencies

clean:
	rm -f $(OBJS) $(EXEC_OBJS:.o=) libbtreelab.a

include .dependencies
