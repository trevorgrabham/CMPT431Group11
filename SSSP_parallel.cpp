#include "core/graph.h"
#include "core/utils.h"
#include <atomic>
#include <cfloat>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <utility>
#include <vector>

static std::vector<double> thread_time;
static std::vector<double> min_path;
static uintV source;

std::atomic<bool> is_changed(false);
std::atomic<int> lock(0);
int flag = 0;

void wait(){
        while(true){
                while(lock.load());
                if(lock.compare_exchange_weak(flag, 1))
                        return;
                else
                        flag= 0;
        }
}

void bellmanFordParallel(int id, Graph* g, uintV startV, uintV endV, int max_iters, struct CustomBarrier *barrier){
	uintV n = g->n_;
	int iter = 0;
	timer t;
	t.start();
	do{
		is_changed.store(false);
		for (uintV v = startV; v <= endV ; v++) {
			// for each vertex 'currV', process all its outNeighbors 'u'
			uintE out_degree = g->vertices_[v].getOutDegree();

			for (uintE i = 0; i < out_degree; i++) {
				uintV u = g->vertices_[v].getOutNeighbor(i);
	      
				wait();
				if(min_path[u] > min_path[v]+1){
					//default edge weight = 1
					min_path[u] = min_path[v]+1;
					is_changed.store(true);
				}
				lock.store(0);
			}
		}
		iter++;
		//barrier->wait();
	}while ((iter < max_iters) && is_changed.load());
	thread_time[id] = t.stop();
}

void graphSearchParallel(Graph& g, uint n_threads){
	uintV n = g.n_;
	int max_iters = n-1;

	//Initialize the path length from the source to other vertices to be infinity
	//and 0 for the source
	for (uintV i = 0; i < n; i++) {
		if(i == source)
			min_path.push_back(0);
		else
			min_path.push_back(n+2);
	}
	


	for(int i = 0; i < n_threads; i++)
		thread_time.push_back(0.0);

	//Thread workset calculation (from Assignment3)
	//
	std::vector<uintV> startV;
	std::vector<uintV> endV;

	uintV  currV = 0;
	uintV min_vertices = n / n_threads;
	uintV excessV = n - min_vertices * n_threads;

	for (int i = 0; i < n_threads; i++) {
		startV.push_back(currV);
		if (excessV > 0) {
			endV.push_back(currV + min_vertices);
			excessV--;
		}
		else {
			endV.push_back(currV + min_vertices - 1);
		}
		currV = endV[i]+1;
	}

	struct CustomBarrier barrier = {(int)n_threads};

	timer t;
	double time_taken = 0.0;
	t.start();
	//Creating threads
	std::vector<std::thread> all_threads;
	for(int i = 0; i<n_threads; i++){
		std::thread newthread(bellmanFordParallel, i, &g, startV[i], endV[i], max_iters, &barrier);
		all_threads.push_back(std::move(newthread));
	}
	for (std::thread & th : all_threads){
		if (th.joinable())
			th.join();
	}
	time_taken = t.stop();

	//Output time and selected results
	std::cout << "Thread id, time taken\n";
	for(int i = 0; i < n_threads; i++)
		std::cout << i << ", " << thread_time[i] << std::endl;

	std::cout << "Selected results: \n";
	std::cout << "Source, destination, length\n";
	for(int i = 0; i < 10; i++){
		std::cout << source << ", " << i << ", "; 
		if(min_path[i] == n+2)
			std::cout << "INF\n";
		else
			std::cout << min_path[i] << std::endl;
	}
	std::cout << "Total Time: " << time_taken << std::endl;
}

int main(int argc, char *argv[]) {
  cxxopts::Options options(
      "SSSP_parallel",
      "Thread implementation: Calculate the shortest path from a given vertex to every other vertices");
  options.add_options(
      "",
      {
           {"nThreads", "Number of Threads",
           cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_THREADS)},
	   {"inputFile", "Input Graph Gile Path",
           cxxopts::value<std::string>()->default_value(
               "/scratch/input_graphs/roadNet-CA")},
	   {"source", "Start Vertex",
           cxxopts::value<uintV>()->default_value(DEFAULT_SOURCE_VERTEX)}
      });

  auto cl_options = options.parse(argc, argv);
  uint nThreads = cl_options["nThreads"].as<uint>();
  std::string input_file_path = cl_options["inputFile"].as<std::string>();
  source =  cl_options["source"].as<uintV>();

  Graph g;
  std::cout << "Reading graph\n";
  g.readGraphFromBinary<int>(input_file_path);
  std::cout << "Created graph\n";
  if(source >= g.n_){
	  std::cout << "Illegal Source Vertex\n";
	  return 1;
  }
  graphSearchParallel(g, nThreads);

  return 0;
}
