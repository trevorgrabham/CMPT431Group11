/*
  Dijkstra's algorithm for computing SSSP. Orignal algorithm: https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm
*/
#include <limits.h>
#include <queue>
#include <tuple>

#include "core/graph.h"

void SSSP2(int* d, int* p, bool** g, int len, int source) {
  // Local variables
  int V = len;
  int v_index = 0;
  int u;
  int u_dist;
  int n_neighbours;
  int neighbour;
  std::tuple<int,int> min_tup;
  std::priority_queue<std::tuple<int,int>, std::vector<std::tuple<int,int>>, std::greater<std::tuple<int,int>>> Q;

  // Initialize dist, prev, and Q
  for(;v_index<V;v_index++) {
    d[v_index] = INT_MAX;
    p[v_index] = -1;
    // Q.push(std::tuple<int,int>{INT_MAX, v_index});     // I don't think I need this here, should be able to just add our source, and then add each node as we encounter it. If we don't reach a node then its dist[node] should still be INT_MAX
  }
  Q.push(std::tuple<int,int>{0, source});
  d[source] = 0;
  p[source] = -1;

  // Iterative Loop
  while(!Q.empty()) {
    min_tup = Q.top(); 
    Q.pop();
    u_dist = std::get<0>(min_tup);
    u = std::get<1>(min_tup);
    if(d[u] < u_dist) continue;
    for(neighbour=0;neighbour<V;neighbour++) {
      if(!g[u][neighbour]) continue;
      if(u_dist+1 < d[neighbour]) {
        d[neighbour] = u_dist+1;
        p[neighbour] = u;
        Q.push(std::tuple<int,int>{u_dist+1, neighbour});
      }
    }
  }
  return;
}

void SSSP(int* d, int* p, Graph* g, int source) {
  // Local variables
  int V = g->n_;
  int v_index = 0;
  int u;
  int u_dist;
  int n_neighbours;
  int neighbour;
  std::tuple<int,int> min_tup;
  std::priority_queue<std::tuple<int,int>, std::vector<std::tuple<int,int>>, std::greater<std::tuple<int,int>>> Q;

  // Initialize dist, prev, and Q
  for(;v_index<V;v_index++) {
    d[v_index] = INT_MAX;
    p[v_index] = -1;
  }
  Q.push(std::tuple<int,int>{0, source});
  d[source] = 0;
  p[source] = -1;

  // Iterative Loop
  while(!Q.empty()) {
    min_tup = Q.top(); 
    Q.pop();
    u_dist = std::get<0>(min_tup);
    u = std::get<1>(min_tup);
    if(d[u] < u_dist) continue;
    n_neighbours = g->vertices_[u].getOutDegree();
    for(neighbour=0;neighbour<n_neighbours;neighbour++) {
      if(u_dist+1 < d[neighbour]) {
        d[neighbour] = u_dist+1;
        p[neighbour] = u;
        Q.push(std::tuple<int,int>{u_dist+1, neighbour});
      }
    }
  }
}
