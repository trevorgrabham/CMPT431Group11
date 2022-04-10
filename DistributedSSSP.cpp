#include "core/graph.h"
#include <queue>

/* Indexed Minimum Prority Queue Class
// Adapted from: https://www.geeksforgeeks.org/indexed-priority-queue-with-implementation/
class indexed_min_priority_queue {
  // Storing indices of values using key
  std::unordered_map<uintV, uintV, std::hash<uintV>> m;
  std::vector<std::pair<uintV, uintV> > v; // underlying container
  uintV numElements;
  uintV infinity; // defintion for infinity

  // heapify the container
  void heapify(std::vector<std::pair<uintV, uintV> >& v, uintV heap_size, uintV index) {
    uintV leftChild = 2 * index + 1;
    uintV rightChild = 2 * index + 2;
    uintV suitableNode = index;

    if (leftChild < heap_size && v[suitableNode].second > v[leftChild].second) {
        suitableNode = leftChild;
    }

    if (rightChild < heap_size && v[suitableNode].second > v[rightChild].second) {
        suitableNode = rightChild;
    }

    if (suitableNode != index) {
      // swap the value
      std::pair<uintV, uintV> temp = v[index];
      v[index] = v[suitableNode];
      v[suitableNode] = temp;

      // updating the map
      m[v[index].first] = index + 1;
      m[v[suitableNode].first] = suitableNode + 1;

      // heapify other affected nodes
      heapify(v, numElements, suitableNode);
    }
  }
  
public:
  indexed_min_priority_queue(uintV _infinity) : infinity(_infinity), numElements(0) {
    m.clear();
    v.clear();
  }

  void push(uintV key, uintV value) {
    // Adding element
    v.push_back(std::make_pair(key, value));
    numElements++;
    m[key] = numElements;

    uintV index = numElements - 1;

    // Comparing to parent node
    while (index != 0 && v[(index - 1) / 2].second > v[index].second) {
      // swap the value
      std::pair<uintV, uintV> temp = v[index];
      v[index] = v[(index - 1) / 2];
      v[(index - 1) / 2] = temp;

      // updating the map
      m[v[index].first] = index + 1;
      m[v[(index - 1) / 2].first] = (index - 1) / 2 + 1;

      // updating index in map
      index = (index - 1) / 2;
    }
  }

  uintV pop() {
    uintV top = v[0].first;
    // Removing element
    v.erase(v.begin());
    numElements--;
    heapify(v, numElements, 0);
    return top;
  }

  bool empty() { return numElements == 0 || v[0].second == infinity; }

  void changeAtKey(uintV key, uintV value) {
    std::cout<<"changeAtKey() BEFORE: v0=<"<<v[0].first<<","<<v[0].second<<">, v1=<"<<v[1].first<<","<<v[1].second<<">, v2=<"<<v[2].first<<","<<v[2].second<<">, v3=<"<<v[3].first<<","<<v[3].second<<">\n";

    uintV index = m[key] - 1;
    v[index].second = value;

    // Comparing to child nodes
    heapify(v, numElements, index);

    // Comparing to Parent Node
    while (index != 0 && v[(index - 1) / 2].second > v[index].second) {
      // swap the value
      std::pair<uintV, uintV> temp = v[index];
      v[index] = v[(index - 1) / 2];
      v[(index - 1) / 2] = temp;

      // updating the map
      m[v[index].first] = index + 1;
      m[v[(index - 1) / 2].first] = (index - 1) / 2 + 1;

      // updating index in map
      index = (index - 1) / 2;
    }
        std::cout<<"changeAtKey() AFTER: v0=<"<<v[0].first<<","<<v[0].second<<">, v1=<"<<v[1].first<<","<<v[1].second<<">, v2=<"<<v[2].first<<","<<v[2].second<<">, v3=<"<<v[3].first<<","<<v[3].second<<">\n";
  }
};
*/

void writeResults(Graph &g, uintV source, std::vector<uintV> const &distance, std::vector<uintV> const &path, std::string input_file_path) {
  std::string output_file_path = "./output" + input_file_path.substr(input_file_path.find_last_of("/"), input_file_path.length()) + "_s" + std::to_string(source) + ".out";
  std::cout << "Writing results to " << output_file_path << "\n";

  std::ofstream output_file;
  output_file.open(output_file_path, std::ios::out);

  output_file << "n: " << g.n_ << "\n"
                 "m: " << g.m_ << "\n";

  for (uintV v = 0; v < g.n_; v++) {
    output_file << "v" << v << ": ";
    if (path[g.n_] == g.n_) {
      output_file << "no path found\n";
      continue;
    } else {
      output_file << "shortest path = " << distance[v] << "; parent = " << path[v] << "\n";
    }
  }
  output_file.close();
}

typedef std::pair<uintV,uintV> pair;
void findShortestPaths(Graph &g, uintV source, std::vector<uintV> &distance, std::vector<uintV> &path) {
  uintV infinity = g.n_;
  timer t1;
  t1.start();
  
  // Initialize data structures
  std::priority_queue<pair, std::vector<pair>, std::greater<pair>> Q;;
  Q.push(std::make_pair(0, source));
  distance[source] = 0;
  path[source] = 0;

  // Find all shortest paths
  while (!Q.empty()) {
    uintV v = Q.top().second; Q.pop();    
    uintE out_degree = g.vertices_[v].getOutDegree();

    for (uintE e = 0; e < out_degree; e++) {
      uintV u = g.vertices_[v].getOutNeighbor(e);
      uintV new_path_dist = distance[v] + 1; // all edge weights are 1
      if (distance[u] > new_path_dist) {
        // Shorter path found
        path[u] = v;
        distance[u] = new_path_dist;
        Q.push(std::make_pair(new_path_dist, u));
      }
    }
  }
  
  double time_taken = t1.stop();
  std::cout << "All existing shortest paths found\n";
  std::cout << "Time taken: " << time_taken << "\n";
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

  Graph g;
  std::cout << "Reading graph\n";
  g.readGraphFromBinary<int>(input_file_path);
  std::cout << "Created graph\n";

  std::vector<uintV> distance(g.n_, g.n_); // holds shortest path distance from source node (if no path exists holds n)
  std::vector<uintV> path(g.n_, g.n_); // holds the parent node ID on shortest path from source (if not on any path holds n)

  findShortestPaths(g, source, distance, path);
  if (output) writeResults(g, source, distance, path, input_file_path);

  return 0;
}