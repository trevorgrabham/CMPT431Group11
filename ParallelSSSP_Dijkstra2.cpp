#include "core/graph.h"
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

void writeResults(Graph const &g, uintV source, std::vector<uintV> const &distance, std::vector<uintV> const &path, std::string input_file_path) {
  std::string output_file_path = "./output" + input_file_path.substr(input_file_path.find_last_of("/"), input_file_path.length()) + "_s" + std::to_string(source) + ".out";
  std::cout << "Writing results to " << output_file_path << "\n";

  std::ofstream output_file;
  output_file.open(output_file_path, std::ios::out);

  output_file << "n: " << g.n_ << "\n"
                 "m: " << g.m_ << "\n";

  for (uintV v = 0; v < g.n_; v++) {
    output_file << "v" << v << ": ";
    if (path[v] == g.n_) {
      output_file << "no path found\n";
      continue;
    } else {
      output_file << "shortest path = " << distance[v] << "; parent = " << path[v] << "\n";
    }
  }
  output_file.close();
}

typedef std::pair<uintV,uintV> pair;

std::vector<pair> mins;
std::mutex m;

void findShortestPaths(int i, int nThreads, uintV source, uintV v0, uintV vn, Graph const &g,
					             std::vector<uintV> &distance, std::vector<uintV> &path, CustomBarrier &b) {
  std::priority_queue<pair, std::vector<pair>, std::greater<pair>> Q; //local minimum priority queue
  Q.push(std::make_pair(g.n_, g.n_));
  Q.push(std::make_pair(0, source)); // **USE SOURCE**

  // Find all shortest paths
  while (!Q.empty()) {
    // find global min
    pair min = *std::min_element(mins.begin(), mins.end());
	  uintV v = min.second;
    uintV v_distance = min.first;
    if (Q.top().second == v) Q.pop();
    // std::cout<< "{"<<i<<"} v_distance="<<v_distance<<", v="<<v<<"\n";

    // determine shortest path for all neighbors
    uintE out_degree = g.vertices_[v].getOutDegree();
    for (uintE e = 0; e < out_degree; e++) {
      if (e%nThreads == i) { // split neighbors equally (option 1)
        uintV u = g.vertices_[v].getOutNeighbor(e);
        // if (u >= v0 && u < vn) { // pre assigned node set (option 2)
          uintV new_path_dist = v_distance + 1; // all edge weights are 1
          if (distance[u] > new_path_dist) {
            // shorter path found
            path[u] = v;
            distance[u] = new_path_dist;
            Q.push(std::make_pair(new_path_dist, u));
          }
    	}
    }

    b.wait();
    mins[i] = Q.top(); // record local min
    // std::cout<< "min["<<i<<"]: "<< mins[i].first<<" "<<mins[i].second <<"\n";
    b.wait();
  }
}

void ParallelDijkstra(Graph const &g, uintV source, std::vector<uintV> &distance, 
					   std::vector<uintV> &path, int nThreads) {
  timer t1;
  t1.start();
   
  std::thread threads[nThreads];

  // Divide the nodes across the threads
  std::vector<uintV> v_start(nThreads);
  std::vector<uintV> v_end(nThreads);
  for (int i = 0; i < nThreads; i++) {
	v_start[i] = g.n_/nThreads * i;
	v_end[i] = v_start[i] + g.n_/nThreads;
  }
  v_end[nThreads - 1] = g.n_;

  // Initialize data structures
  distance[source] = 0;
  path[source] = source;
  CustomBarrier b(nThreads);
  mins.resize(nThreads,{0, source});

  for (int i = 0; i < nThreads; i++) {
	threads[i] = std::thread(findShortestPaths, i, nThreads, source, v_start[i], v_end[i],
							 std::cref(g), std::ref(distance), std::ref(path), std::ref(b));
  }

  for (int i = 0; i < nThreads; i++) {
	threads[i].join();
  }

  std::cout << "All existing shortest paths found\n"
               "Time taken: " <<  t1.stop() << "\n";
}

int main(int argc, char *argv[]) {
  cxxopts::Options options(
      "ParallelSSSP",
      "Calculate all shortest paths from source - Threads version");
  options.add_options(
    "",
    {
      {"nThreads", "Number of Threads",
        cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_THREADS)},
      {"inputFile", "Input graph file path",
        cxxopts::value<std::string>()->default_value(
            "/scratch/input_graphs/roadNet-CA")},
      {"sourceNode", "Source node ID",
        cxxopts::value<uintV>()->default_value("0")},      
      {"output", "Write results to file",
        cxxopts::value<bool>()->default_value("0")},
    }
  );

  auto cl_options = options.parse(argc, argv);
  uintV output = cl_options["output"].as<bool>();
  uintV source = cl_options["sourceNode"].as<uintV>();
  std::string input_file_path = cl_options["inputFile"].as<std::string>();
  uint nThreads = cl_options["nThreads"].as<uint>();

  std::cout << "Number of Threads : " << nThreads << "\n"
			   "Reading Graph\n";
  Graph g;
  g.readGraphFromBinary<int>(input_file_path);

  std::cout << "Graph Created\n"
			   "n: " << g.n_ << "\n"
			   "m: " << g.m_ << "\n"
			   "SourceNode: " << source << "\n"
			   "Searching for paths...\n";

  std::vector<uintV> distance(g.n_, g.n_); // holds shortest path distance from source node (if no path exists holds n)
  std::vector<uintV> path(g.n_, g.n_); // holds the parent node ID on shortest path from source (if not on any path holds n)

  ParallelDijkstra(g, source, distance, path, nThreads);
  if (output)
    writeResults(g, source, distance, path, input_file_path);

  return 0;
}
