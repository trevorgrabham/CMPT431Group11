#include "core/graph.h"
#include <queue>
#include <mpi.h>

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

void findShortestPaths(Graph &g, uintV source, std::vector<uintV> &distance, std::vector<uintV> &path,
                       int world_rank, int world_size) {
  // uintV infinity = g.n_;
  timer t1;
  t1.start();
  
  // Initialize data structures
  distance[source] = 0;
  path[source] = 0;

  // Divide the vertices across the processes
  uintV ni = g.n_/world_size;
  uintV v_start = world_rank * ni;
  uintV v_end = v_start + ni;
  if (world_rank == world_size - 1) v_end = g.n_;

  // Find all shortest paths
  for (uintV round = 1; round < g.n_; round++) { // n-1 rounds
std::cout << "round " << round <<"\n";

    for (uintV v = 0; v < g.n_; v++) {
      uintE out_degree = g.vertices_[v].getOutDegree();

      for (uintE e = 0; e < out_degree; e++) {
        uintV u = g.vertices_[v].getInNeighbor(e);

        bool own_v = v >= v_start && v < v_end;
        bool own_u = u >= v_start && u < v_end;

        if (own_u != own_v) {
          if (own_v) {
            int u_rank = u/ni;
            if (world_rank > u_rank){
              // higher rank sends first
    // std::cout << world_rank <<":"<<v <<" -> " << u_rank<<":"<<u <<"\n";
              MPI_Send(&distance[v], 1, MPI_INT, u_rank, v, MPI_COMM_WORLD);
    // std::cout << world_rank <<":"<<v <<" <- " << u_rank<<":"<<u <<"\n";
              MPI_Recv(&distance[u], 1, MPI_INT, u_rank, u, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
    // std::cout << world_rank <<":"<<v <<" <- " << u_rank<<":"<<u <<"\n";
              MPI_Recv(&distance[u], 1, MPI_INT, u_rank, u, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // std::cout << world_rank <<":"<<v <<" -> " << u_rank<<":"<<u <<"\n";
              MPI_Send(&distance[v], 1, MPI_INT, u_rank, v, MPI_COMM_WORLD);
            }
          } else {
            int v_rank = v/ni;
            if (world_rank > v_rank){
              // higher rank sends first
    // std::cout << world_rank <<":"<<u <<" -> " << v_rank<<":"<<v <<"\n";
              MPI_Send(&distance[u], 1, MPI_INT, v_rank, u, MPI_COMM_WORLD);
    // std::cout << world_rank <<":"<<u <<" <- " << v_rank<<":"<<v <<"\n";
              MPI_Recv(&distance[v], 1, MPI_INT, v_rank, v, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
    // std::cout << world_rank <<":"<<u <<" <- " << v_rank<<":"<<v <<"\n";
              MPI_Recv(&distance[v], 1, MPI_INT, v_rank, v, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // std::cout << world_rank <<":"<<u <<" -> " << v_rank<<":"<<v <<"\n";
              MPI_Send(&distance[u], 1, MPI_INT, v_rank, u, MPI_COMM_WORLD);
            }

          }
        }
        // determine shortest path
        if (distance[v] > distance[u] + 1) { // all edge weights are 1
          distance[v] = distance[u] + 1;
          path[v] = u;
// std::cout << "update "<<v<< ": distance = "<< distance[v] << ", path = " << path[v] <<"\n";
        }
      }
    }
  }

  double time_taken = t1.stop();

  if (world_rank == 0) {
    std::cout << "All existing shortest paths found\n"
                 "Time taken: " << time_taken << "\n";
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
                 "m: " << g.m_ << "\n";
  }

  std::vector<uintV> distance(g.n_, g.n_); // holds shortest path distance from source node (if no path exists holds n)
  std::vector<uintV> path(g.n_, g.n_); // holds the parent node ID on shortest path from source (if not on any path holds n)

  findShortestPaths(g, source, distance, path, world_rank, world_size);

  if (output && world_rank == 0) 
    writeResults(g, source, distance, path, input_file_path);

  MPI_Finalize();
  return 0;
}