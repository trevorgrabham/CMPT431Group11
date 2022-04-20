## CMPT 431 Project Group 11

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