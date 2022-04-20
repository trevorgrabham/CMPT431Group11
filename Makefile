CXX = g++
MPICXX = mpic++
CXXFLAGS = -std=c++14 -march=native -pthread -O3 
LDFLAGS = -Llib -lalloc431 -lpthread

COMMON= core/utils.h core/cxxopts.h core/get_time.h core/graph.h core/quick_sort.h core/allocator.h
SERIAL= SerialSSSP recordTime
PARALLEL= ParallelSSSP
DISTRIBUTED= DistributedSSSP

ALL= $(SERIAL) $(PARALLEL) $(DISTRIBUTED) 

all : $(ALL)

$(DISTRIBUTED): % : %.cpp $(COMMON)	
	$(MPICXX) $(CXXFLAGS) -o $@ $<

$(PARALLEL) : % : %.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

$(Serial) : % : %.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY : clean
clean :
	rm -f *.o *.obj $(ALL)
