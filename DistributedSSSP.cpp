#include "core/graph.h"


void findShortestPaths(Graph &g, uintV source) {
  uintV n = g.n_;
  Vertex sourceNode = g.vertices_[source];

  timer t1;
  t1.start();



  double time_taken = t1.stop();
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
        cxxopts::value<std::string>()->default_value(
            "0")},
    }
  );

  auto cl_options = options.parse(argc, argv);
  uintV source = cl_options["sourceNode"].as<uintV>();
  std::string input_file_path = cl_options["inputFile"].as<std::string>();

  Graph g;
  std::cout << "Reading graph\n";
  g.readGraphFromBinary<int>(input_file_path);
  std::cout << "Created graph\n";
  findShortestPaths(g, source);

  return 0;
}