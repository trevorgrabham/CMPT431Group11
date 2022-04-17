#compiler setup
CXX = g++
MPICXX = mpic++
CXXFLAGS = -std=c++14 -O3

COMMON= core/utils.h core/cxxopts.h core/get_time.h core/graph.h core/quick_sort.h
DIJKSTRAR= DistributedSSSP_Dijkstra_relaxed
DIJKSTRA= DistributedSSSP_Dijkstra
BELLMAN= DistributedSSSP_Bellman
ALL= $(BELLMAN) $(DIJKSTRA) $(DIJKSTRAR) 

all : $(ALL)

$(ALL): % : %.cpp $(COMMON)	
	$(MPICXX) $(CXXFLAGS) -o $@ $<

.PHONY : clean
clean :
	rm -f *.o *.obj $(ALL)
