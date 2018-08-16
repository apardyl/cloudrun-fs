srcdir := src
protodir := src/proto
gendir := gen
builddir := build
outdir := bin

CC := gcc
CXX := g++
PROTOC := protoc
LD := g++
CPREFLAGS := -D_FILE_OFFSET_BITS=64 -I $(gendir)
CFLAGS := -O0 -g -fPIC $(CPREFLAGS)
CXXFLAGS := $(CFLAGS)
LDFLAGS = -fPIC -lprotobuf -lfuse -pthread -g

CSOURCES := $(shell find $(srcdir) -type f -name *.c)
CXXSOURCES := $(shell find $(srcdir) -type f -name *.cpp)
PROTOSOURCES := src/proto/filesystem.proto src/proto/filedownload.proto

COBJS := $(CSOURCES:$(srcdir)/%.c=$(builddir)/%.c.o)
CXXOBJS := $(CXXSOURCES:$(srcdir)/%.cpp=$(builddir)/%.cxx.o)
CDEPS := $(SOURCES:$(srcdir)/%.c=$(builddir)/%.c.d)
CXXDEPS := $(CXXSOURCES:$(srcdir)/%.cpp=$(builddir)/%.cxx.d)

PROTOHEAD := $(PROTOSOURCES:$(srcdir)/%.proto=$(gendir)/%.pb.h)
PROTOOBJS := $(PROTOSOURCES:$(srcdir)/%.proto=$(builddir)/%.pb.o)
PROTOCFILES := $(PROTOSOURCES:$(srcdir)/%.proto=$(gendir)/%.pb.cc)

TARGET := $(outdir)/hashfs
INSTALLTARGET := /usr/local/bin/hashfs
 
all: $(TARGET)

generated: $(PROTOHEAD)
 
$(TARGET): $(COBJS) $(CXXOBJS) $(PROTOOBJS)
	@echo Linking $@
	@mkdir -p $(outdir)
	$(LD) $+ $(LDFLAGS) -o $@

$(builddir)/%.pb.o: generated
	@echo Building $@
	@mkdir -p "$(@D)"
	$(CXX) $(gendir)/$*.pb.cc $(CXXFLAGS) -c -o $@

$(gendir)/%.pb.h: $(srcdir)/%.proto
	@echo Generating proto $@
	@mkdir -p "$(@D)"
	$(PROTOC) $< -I $(<D) --cpp_out=$(@D)
 
$(builddir)/%.c.o: $(srcdir)/%.c generated $(builddir)/%.c.d
	@echo Building $@
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -c -o $@ $<

$(builddir)/%.cxx.o: $(srcdir)/%.cpp generated $(builddir)/%.cxx.d
	@echo Building $@
	@mkdir -p "$(@D)"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(builddir)/%.c.d: $(srcdir)/%.c generated
	@echo Generating dependency list for $<
	@mkdir -p "$(@D)"
	@echo -n "$(@D)/" > $@
	$(CC) -MM -MP $(CPREFLAGS) $< | sed -r "s^.o:^.o $@ :^" >> $@

$(builddir)/%.cxx.d: $(srcdir)/%.cpp generated
	@echo Generating dependency list for $<
	@mkdir -p "$(@D)"
	@echo -n "$(@D)/" > $@
	$(CXX) -MM -MP $(CPREFLAGS) $< | sed -r "s^.o:^.o $@ :^" >> $@

clean:
	rm -rf $(gendir)
	rm -rf $(builddir)
	rm -rf $(outdir)
install:
	cp $(TARGET) $(INSTALLTARGET)

uninstall:
	rm $(INSTALLTARGET)

test:
	echo $(COBJS) $(CDEPS) $(CXXOBJS) $(CXXDEPS)

.PHONY: all clean install uninstall
.SECONDARY:
.SUFFIXES:
#.SECONDARY: $(COBJS) $(CXXOBJS)
-include $(CDEPS) $(CXXDEPS)
 
