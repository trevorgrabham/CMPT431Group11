#include "core/graph.h"
// #include "core/two_lock_queue.h"
#include "core/non_blocking_queue.h"
#include <thread>
#include <vector>
#include <atomic>
#include <queue>

void writeResults(Graph &g, uintV source, std::vector<uintV> const &distance, std::vector<uintV> const &path, std::string input_file_path) {
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

std::atomic_int done(0);
std::atomic_int level(0);

void findShortestPaths(int id, int nThreads, Graph const &g, std::vector<uintV> &distance, std::vector<uintV> &path,
					             NonBlockingQueue<uintV> &Q, CustomBarrier &b) {
  uintV v;

  while (Q.dequeue(&v)) {
    // all processes must be on the same level
    if (distance[v] > level) {
      uintV l = level;
      // std::cout << "{"<<id<<"}"<< " wait1\n";
      b.wait();
      // std::cout << "{"<<id<<"}"<< " wait2\n";
      level.compare_exchange_weak(l, l+1); // only first process increments level
      b.wait();
    }

    // process node
    uintE out_degree = g.vertices_[v].getOutDegree();
    for (uintE e = 0; e < out_degree; e++) {
      uintV u = g.vertices_[v].getOutNeighbor(e);
      if (distance[u] > distance[v] + 1) {
        distance[u] = distance[v] + 1;
        path[u] = v;
        Q.enqueue(u);
      }
    }
	}
  if (++done == nThreads) b.wait();
  while (done < nThreads) b.wait();
}

void ParallelBFS(Graph const &g, uintV source, std::vector<uintV> &distance, 
				         std::vector<uintV> &path, int nThreads){
  timer t;
  t.start();

  std::thread threads[nThreads];

  // Initialize data structures
  NonBlockingQueue<uintV> Q; // thread safe queue
  Q.initQueue(g.n_);
  Q.enqueue(source);
  distance[source] = 0;
  path[source] = source;
  CustomBarrier b(nThreads);

  for (int i = 0; i < nThreads; i++) {
	  threads[i] = std::thread(findShortestPaths, i, nThreads, std::cref(g), 
      std::ref(distance), std::ref(path), std::ref(Q), std::ref(b));
  }

  for (int i = 0; i < nThreads; i++) {
	  threads[i].join();
  }
 
  Q.cleanup();

  std::cout << "All existing shortest paths found\n"
               "Time taken: " <<  t.stop() << "\n";
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

  std::cout <<  "Number of Threads : " << nThreads << "\n"
			          "Reading Graph\n";
  Graph g;
  g.readGraphFromBinary<int>(input_file_path);

  std::cout <<  "Graph Created\n"
                "n: " << g.n_ << "\n"
                "m: " << g.m_ << "\n"
                "SourceNode: " << source << "\n"
                "Searching for paths...\n";

  std::vector<uintV> distance(g.n_, g.n_); // holds shortest path distance from source node (if no path exists holds n)
  std::vector<uintV> path(g.n_, g.n_); // holds the parent node ID on shortest path from source (if not on any path holds n)

  ParallelBFS(g, source, distance, path, nThreads);
  if (output)
    writeResults(g, source, distance, path, input_file_path);

  return 0;
}