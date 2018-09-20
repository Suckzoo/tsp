CXX+= -std=c++11
CXXFLAGS= -g -O2 -Wall
CXXFLAGS+= $(EXTRA_CXXFLAGS)

SRCS= $(wildcard *.cc)
HEADERS= $(wildcard *.hh)
OBJS= $(SRCS:.cpp=.o)

TARGET= tsp

all: pre-build build

pre-build:
	mkdir -p log

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f tsp *.o
	
.PHONY: all clean

