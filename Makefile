#compiler setup
CXX = g++
CXXFLAGS = -std=c++14 -O3 $(MACRO)

COMMON= core/utils.h core/cxxopts.h core/get_time.h core/graph.h core/quick_sort.h
SERIAL= serial recordTime
ALL= $(SERIAL)


all : $(ALL)

% : %.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY : clean

clean :
	rm -f *.o *.obj $(ALL)
