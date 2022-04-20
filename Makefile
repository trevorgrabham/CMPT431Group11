#compiler setup
CXX = g++
CXXFLAGS = -std=c++14 -march=native -pthread -O3 
LDFLAGS = -Llib -lalloc431 -lpthread

COMMON= core/utils.h core/cxxopts.h core/get_time.h core/graph.h core/quick_sort.h core/allocator.h
PARALLEL= SSSP_parallel_ver0 SSSP_parallel_ver1 ParallelSSSP_Dijkstra ParallelSSSP_Dijkstra2 ParallelSSSP_BFS
ALL= $(PARALLEL)

all : $(ALL)

$(ALL) : % : %.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

.PHONY : clean

clean :
	rm -f *.o *.obj $(ALL)
