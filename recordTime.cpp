#include <fstream>
#include <iostream>
#include "core/get_time.h"

#include "serial.h"

#define N_RUNS 1000

int main(int argc, char** argv) {
  if(argc != 3) {
    std::cout << "Incorrect number of inputs provided.\nPlease provide an output file name and the path to the input graph only.\n";
  }
  Graph g;
  g.readGraphFromBinary<int>(argv[2]);
  int* dist = new int[g.n_];
  int* prev = new int[g.n_];
  long double total_time = 0;
  timer t;
  for(uint i=0;i<N_RUNS;i++) {
    t.start();
    SSSP(dist, prev, g, 0);
    total_time += t.stop();
  }
  long double avg_time = total_time / N_RUNS;
  std::ofstream out_file;
  out_file.open(argv[1], std::ios::out);
  out_file << "Input graph used: " << argv[2] << std::endl;
  out_file << "Number on nodes in the graph: " << g.n_ << std::endl;
  out_file << "Number on edges in the graph: " << g.m_ << std::endl;
  out_file << "Calculating average time for " << N_RUNS << " runs.\n";
  out_file << "Total time: " << total_time << std::endl;
  out_file << "Average time: " << avg_time << std::endl;
  out_file.close();
  delete dist;
  delete prev;
  return 0;
}