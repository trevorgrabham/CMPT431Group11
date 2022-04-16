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
static std::vector<double> min_path; //Minimum path length from source to
				//other vertices

//static std::vector<int> partition;
//static std::vector<int> vertex_ptr;
//static std::vector<int> width_ptr;
static std::vector<uintV> neighbor;// Work set of vertices waiting to be traversed
static uintV source;

std::atomic<bool> is_changed(false);
//std::atomic<bool> degree_zero(false);
//std::atomic<bool> finished(false);

std::atomic<int> lock(0);
std::atomic<int> lock_(0);

std::atomic<uintE> width(1);// Current depth
std::atomic<uintE> next_width(0);
std::atomic<int> currV_idx(-1);// Current source vertex v = neighbor(currV_idx) 
std::atomic<int> length(1);// Distance from source, same for vertices of the same depth
std::atomic<int> count(1);// Number of v with visited[v] = 1

static int flag = 0;
static int flag_ = 0;
static int *record;// record[v] = 1 if v has all its neighbors been traversed
static int *visited;// visited[v] = 1 if v as a neighbor of others has been traversed
static bool isEnd = false;//Search finished, all threads can leave
static uintV v = source; 

//Two independent of atomic locks, only one is being used
void wait_(){
        while(true){
                while(lock_.load());
                if(lock_.compare_exchange_weak(flag_, 1))
                        return;
                else
                        flag_ = 0;
        }
}
void signal_(){lock_.store(0);}

void wait(){
        while(true){
                while(lock.load());
                if(lock.compare_exchange_weak(flag, 1))
                        return;
                else
                        flag= 0;
        }
}
void signal(){lock.store(0);}

void parallelBFS(int id, int n_threads, Graph *g, struct CustomBarrier *barrier1, struct CustomBarrier *barrier2){
	timer t;
	t.start(); 

	uintV n = g->n_;
	//isLast: TRUE is this thread get the trun to process the last neighbor of a vertex
	bool isLast = false;
	while(true){
		//isLast = true;
		/*if(id>=0)
			printf("Thread %d in A\n", id);
		*/
		//std::cout<<"Current in " << v << std::endl;

	//For each vertex
		uintE out_degree = g->vertices_[v].getOutDegree();
		//printf("out_degree: %d\n", out_degree);

		for(uintE i = id; i < out_degree; i+=n_threads){
			//printf("Thread %d: i = %d\n", id, i);
			uintV u = g->vertices_[v].getOutNeighbor(i);
			//printf("Thread %d, vertex %d neighbor(%d) %d\n", id, v, i, u);
			//std::cout<<"Goes to " << u << std::endl;
			
			//If this vertex is been visit as source the first time
			if(record[v] == 0){
				next_width += g->vertices_[u].getOutDegree();
				//std::cout<<"Next width: " << next_width << "\n";
				wait();
				neighbor.push_back(u);
				signal();
				//printf("%d push to neighbor(%ld)\n", u, neighbor.size()-1);
				is_changed.store(true);
				if(visited[u] == 0){
					//printf("%dvisited\n", u);
					min_path[u] = length.load();
					count++;
					printf("(%d / %d)\n", count.load(), n);
					//Mark u as a neighbor of other vertices has been visited
					visited[u] = 1;
				}
			}
			//One vertex on the same depth has been visited
			width--;

			if(i == out_degree -1)
				isLast = true;
			
			//printf("width: %d\n", width);
		}
		barrier1->wait();
		//If this vertex 0 out_degree, thread 0 is set to the last thread
		if(out_degree == 0){
			if(id == 0){
				isLast = true;
			}
		}

		//if(id>=0)
			//printf("Thread %d leave for loop\n", id);

		//Mark v as a source is been visited
		if((record[v] == 0)&&isLast){
			//printf("Thread %d update record[%d]\n", id, v);
			record[v] = 1;
		}
		/*
		if(id>=0)
			printf("Thread %d leave part B\n", id);
		*/


		//All neighbors of vertices on the current depth have been traversed
		if(width == 0){
			if(is_changed.load() == false){
				thread_time[id] = t.stop();
				isEnd = true;
				//printf("No Changed, thread %d left\n", id);
				barrier1->wait();
				return;
			}
			
			if(isLast){
				//if(id>=0)
				//	printf("Thread %d enter partC\n", id);
				width.store(next_width.load());
				//printf("Width update to %d\n", width.load());
				next_width.store(0);
				is_changed.store(false);
				length++;
			}
		}
		/*if(id>=0)
			printf("Thread %d leave part C\n", id);
		*/
		
		//Update the source vertex v to be processed next
		//if v exceed the size of neighbor (workset of source vertex)
		//all thread exit
		if(isLast){
			/*
			if(id>=0)
				printf("Thread %d enter part D\n", id);
			*/
			currV_idx++;
			//std::cout<<"currV_idx " << currV_idx << "\n";
			if(currV_idx.load()<neighbor.size()){
				v = neighbor[currV_idx.load()];
				//printf("Next source: %d\n", neighbor[currV_idx]);
			}
			else{
				std::cout<<"Thread "<<id<<" ";
				std::cout<<"currV_idx " << currV_idx.load();
				std::cout<<" greater than " << neighbor.size() << std::endl;
				std::cout<<"Overflow\n";
				/*
				printf("width: %d\n", width.load());
				printf("isChanged: %d\n", is_changed.load());
				printf("partition_idx: %d\n", partition_idx.load());
				*/
				isEnd = true;
				barrier1->wait();
				return;
			}
			//std::cout<<"next level\n\n";
		}
		isLast = false;
		//printf("Thread %d wait\n", id);
		barrier1->wait();
		//printf("Thread %d left\n", id);
		if(isEnd){
			//std::cout<< id << "leave\n";
			thread_time[id] = t.stop();
			return;
		}
	}
}

void graphSearchParallel(Graph& g, uint n_threads){
	uintV n = g.n_;
	int max_iters = n-1;
	if(g.vertices_[source].getOutDegree() == 0){
		std::cout<<"No Path Possible\n";
		return;
	}

	//Initialize the path length from the source to other vertices to be infinity (n+1)
	//and 0 for the source
	for (uintV i = 0; i < n; i++) {
		if(i == source)
			min_path.push_back(0);
		else
			min_path.push_back(n+1);
	}
	
	for(int i = 0; i < n_threads; i++)
		thread_time.push_back(0.0);

	record = new int[n]; 
	visited = new int[n]; 
	for(int i = 0; i < n; i++){
		record[i] = 0;
		visited[i] = 0;
	}
	width.store(g.vertices_[source].getOutDegree());
	//
	//satic_partition.txt could be inserted here
	//
	struct CustomBarrier barrier1 = {(int) n_threads};
	struct CustomBarrier barrier2 = {(int) n_threads};

	timer t;
	double time_taken = 0.0;
	t.start();
	//Creating threads
	std::cout<<"Creating Threads\n";
	std::vector<std::thread> all_threads;
	for(int i = 0; i<n_threads; i++){
		std::thread newthread(parallelBFS, i, n_threads, &g, &barrier1, &barrier2);
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
		if(min_path[i] == n+1)
			std::cout << "INF\n";
		else
			std::cout << min_path[i] << std::endl;
	}
	
	std::cout << "Total Time: " << time_taken << std::endl;

	delete[] record;
	delete[] visited;
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
