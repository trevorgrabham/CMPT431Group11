#include "core/graph.h"
#include <queue>
#include <mpi.h>

void writeResults(Graph const &g, uintV source, std::vector<uintV> const &distance, std::vector<uintV> const &path,
                  std::string input_file_path, int world_rank, int world_size) {
  std::string output_file_path = "./output" + input_file_path.substr(input_file_path.find_last_of("/"), input_file_path.length()) + "_s" + std::to_string(source) + ".out";
  if (world_rank == 0) std::cout << "Writing results to " << output_file_path << "\n";

  if (world_rank > 0) { // wait for previous process to finish writing
    MPI_Recv(nullptr, 0, MPI_INT, world_rank-1, 0,  MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  std::ofstream output_file;
  if (world_rank == 0) { // open fresh file
    output_file.open(output_file_path, std::ios::out);
    output_file << "n: " << g.n_ << "\n"
                   "m: " << g.m_ << "\n";
  } else { // append file
    output_file.open(output_file_path, std::ios::app);
  }

  uintV v_start = world_rank * g.n_/world_size;
  uintV v_end = world_rank == world_size - 1? g.n_ : v_start + g.n_/world_size;
  for (uintV v = v_start; v < v_end; v++) {
    output_file << "v" << v << ": ";
    if (path[v] == g.n_)
      output_file << "no path found\n";
    else
      output_file << "shortest path = " << distance[v] << "; parent = " << path[v] << "\n";
  }
  output_file.close();

  if (world_rank < world_size-1) { // signal next waiting process
    MPI_Send(nullptr, 0, MPI_INT, world_rank+1, 0,  MPI_COMM_WORLD);
  }
}

typedef std::pair<uintV,uintV> pair;
void findShortestPaths(Graph const &g, uintV source, std::vector<uintV> &distance, std::vector<uintV> &path,
                       int world_rank, int world_size) {
  timer t1;
  t1.start();
  
  // Divide the nodes across the processes
  uintV v_start = world_rank * g.n_/world_size;
  uintV v_end = world_rank == world_size - 1? g.n_ : v_start + g.n_/world_size;

  // Initialize data structures
  std::priority_queue<pair, std::vector<pair>, std::greater<pair>> Q; //local minimum priority queue
  Q.push(std::make_pair(g.n_, source));
  Q.push(std::make_pair(0, source));
  distance[source] = 0;
  path[source] = 0;

  // Find all shortest paths
  while (!Q.empty()) {
    // find node with minimum distance
    uintV local_min[2];
    uintV global_min[2];
    local_min[0] = Q.top().first; // distance
    local_min[1] = Q.top().second; // node #
    MPI_Allreduce(local_min, global_min, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);

    // remove node from respective local queue
    uintV v = global_min[1];
    uintV v_distance = global_min[0];
    if (Q.top().second == v) Q.pop();

    // determine shortest path for all neighbors
    uintE out_degree = g.vertices_[v].getOutDegree();
    for (uintE e = 0; e < out_degree; e++) {
      uintV u = g.vertices_[v].getOutNeighbor(e);
      if (u >= v_start && u < v_end) {
        uintV new_path_dist = v_distance + 1; // all edge weights are 1
        if (distance[u] > new_path_dist) {
          // shorter path found
          path[u] = v;
          distance[u] = new_path_dist;
          Q.push(std::make_pair(new_path_dist, u));
        }
      }
    }
  }

  if (world_rank == 0) {
    std::cout << "All existing shortest paths found\n"
                 "Time taken: " <<  t1.stop() << "\n";
  }
}

int main(int argc, char *argv[]) {
  cxxopts::Options options(
    "DistributedSSSP",
    "Calculate all shortest paths from source - MPI version");
  options.add_options(
    "",
    {
      {"inputFile", "Input graph file path",
        cxxopts::value<std::string>()->default_value(
            "/scratch/input_graphs/roadNet-CA")},
      {"sourceNode", "Source node ID",
        cxxopts::value<uintV>()->default_value(
            "0")},      
      {"output", "Write results to file",
        cxxopts::value<bool>()->default_value(
            "0")},
    }
  );

  auto cl_options = options.parse(argc, argv);
  uintV output = cl_options["output"].as<bool>();
  uintV source = cl_options["sourceNode"].as<uintV>();
  std::string input_file_path = cl_options["inputFile"].as<std::string>();

  MPI_Init(NULL, NULL);

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of this process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0) {
    std::cout << "Number of processes : " << world_size << "\n"
                 "Reading graph\n";
  }

  Graph g;
  g.readGraphFromBinary<int>(input_file_path);

  if (world_rank == 0) {
    std::cout << "Graph Created\n"
                 "n: " << g.n_ << "\n"
                 "m: " << g.m_ << "\n"
                 "SourceNode: " << source << "\n"
                 "Searching for paths...\n";
  }

  std::vector<uintV> distance(g.n_, g.n_); // holds shortest path distance from source node (if no path exists holds n)
  std::vector<uintV> path(g.n_, g.n_); // holds the parent node ID on shortest path from source (if not on any path holds n)

  findShortestPaths(g, source, distance, path, world_rank, world_size);
  if (output)
    writeResults(g, source, distance, path, input_file_path, world_rank, world_size);

  MPI_Finalize();
  return 0;
}