/*
  Dijkstra's algorithm for computing SSSP. Orignal algorithm: https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm
*/
#include <limits.h>
#include <queue>

#include "core/graph.h"

void writeResults(Graph &g, uintV source, int* distance, int* path, std::string input_file_path) {
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

void SSSP(int* dist, int* prev, Graph const &g, int source) {
  uintV V = g.n_;
  std::queue<int> Q;
  uintV vIndex;
  uintV uIndex;
  uintV vDist;
  uintE outDegree;

  for(uint i=0;i<V;i++) {
    dist[i] = g.n_;
    prev[i] = g.n_;
  }
  dist[source] = 0;
  prev[source] = source;
  Q.push(source);
  while(!Q.empty()) {
    vIndex= Q.front();
    Q.pop();
    vDist = dist[vIndex];
    outDegree = g.vertices_[vIndex].getOutDegree();
    for(uintE i=0;i<outDegree;i++) {
      uIndex = g.vertices_[vIndex].getOutNeighbor(i);
      if(dist[uIndex] > dist[vIndex] + 1) {
        dist[uIndex] = dist[vIndex] + 1;
        prev[uIndex] = vIndex;
        Q.push(uIndex);
      }
    }
  }
}
