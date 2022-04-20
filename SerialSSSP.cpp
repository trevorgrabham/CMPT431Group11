#include "serial.h"

int main(int argc, char *argv[]) {
  cxxopts::Options options(
      "ParallelSSSP",
      "Calculate all shortest paths from source - Threads version");
  options.add_options(
    "",
    {
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

	std::cout << "Reading Graph\n";
  Graph g;
  g.readGraphFromBinary<int>(input_file_path);

  std::cout <<  "Graph Created\n"
                "n: " << g.n_ << "\n"
                "m: " << g.m_ << "\n"
                "SourceNode: " << source << "\n"
                "Searching for paths...\n";

  int* distance = new int[g.n_];
  int* path = new int[g.n_];

  SSSP(distance, path, g, source);
  if (output)
    writeResults(g, source, distance, path, input_file_path);

  delete distance;
  delete path;
  return 0;
}