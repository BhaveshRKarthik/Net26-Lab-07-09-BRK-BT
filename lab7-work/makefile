CXXFILES = $(wildcard *.cpp)
CXXOBJS = $(patsubst %.cpp,%.o,$(CXXFILES))
CXXINCL = $(wildcard *.hpp) $(wildcard *.h)

CFILES = $(wildcard *.c)
COBJS = $(patsubst %.c,%.o,$(CFILES))
CINCL = $(wildcard *.h)

OBJS = $(COBJS) $(CXXOBJS)

LD = g++
CXX = g++
CC = gcc

COM_FLAGS = -g

LDFLAGS = 
CXXFLAGS = -std=c++23
CFLAGS = 

LDFLAGS += $(COM_FLAGS)
CXXFLAGS += $(COM_FLAGS)
CFLAGS += $(COM_FLAGS)

EXEC = sock.out

$(EXEC): %: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(COBJS): %.o: %.c $(CINCL)

$(CXXOBJS): %.o: %.cpp $(CXXINCL)

clean:
	rm -rvf $(OBJS) $(EXEC)