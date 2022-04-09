#compiler setup
CXX = g++
CXXFLAGS = -std=c++14 -O3

COMMON= core/utils.h core/cxxopts.h core/get_time.h core/graph.h core/quick_sort.h
SRCS= $(wildcard *.cpp)
BINS= $(SRCS:.cpp=)
all : $(BINS)

% : %.cpp $(COMMON)	
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY : clean
clean :
	rm -f *.o *.obj $(BINS)
