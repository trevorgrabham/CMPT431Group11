## CMPT 431 Project Group 11

## Algorithm

<p>Perform a Breath First Search started at source vertex, all vertices in the same
depth are within the same distance to the source. If the current depth no vertices has their distence updated, algorithm terminated.</p>

`static_partition.cpp` has the code for pre-assign the workset used by threads, all vertices are pushed to `neighbor` by BFS<br />
`partition[i]` is the index in `neighbor` of the first neighbor of the i-th vertex `vertex-ptr[i]`<br />
`width-ptr` is the index in `neighbor` of the first vertex of depth i. (Based on the idea of CSR/CSC matrix format).</p>

## Compile

```bash
make
```

## Run

```bash
./serial --source 0 --inputFile absolute_path_of_input_graph
./ParallelSSSP_BFS --source 0 --inputFile absolute_path_of_input_graph --nThreads 4
./DistributedSSSP_Dijkstra --source 0 --inputFile absolute_path_of_input_graph
```

## Output

Output can optionally be recorded using a `--output 1` flag for any of the implementations

## Input graphs

Input files can be found on the slurm servers under the scratch folder

## Serial Timing

```bash
./recordTime
```
