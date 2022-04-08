#include "serial.h"

int main(int argc, char** argv) {
  bool** g = new bool*[4];
  g[0] = new bool[4] {false, true, false, true};
  g[1] = new bool[4] {true, false, true, true};
  g[2] = new bool[4] {false, true, false, false};
  g[3] = new bool[4] {true, true, false, false};
  int* dist = new int[4];
  int* prev = new int[4];
  std::cout << "Running Dijkstra's algorithm\n";
  SSSP2(dist, prev, g, 4, 0);
  std::cout << "Algorithm completed\n";
  std::cout << "Dist: [";
  for(uint i=0;i<3;i++) {
    std::cout << dist[i] << ", ";
  }
  std::cout << dist[3] << "]\n";
  std::cout << "Prev: [";
  for(uint i=0;i<3;i++) {
    std::cout << prev[i] << ", ";
  }
  std::cout << prev[3] << "]\n";
  delete [] g;
  delete dist;
  delete prev;
  return 0;
}