# Wavefront Computation Project

## Overview

This project implements wavefront computation on a matrix using different parallel programming paradigms. The project includes implementations using MPI, FastFlow, and a sequential version for comparison. The goal is to evaluate the performance of these implementations under various configurations.

## Project Structure

```
.DS_Store
FastFlow/
	include/
		hpc_helpers.hpp
	Makefile
	MapFF.cpp
	ParForFF.cpp
	StrongFFMapFrontend.sh
	StrongFFMapInternal.sh
	StrongFFParForFrontend.sh
	StrongFFParForInternal.sh
	WeakFFMapFrontend.sh
	WeakFFMapInternal.sh
	WeakFFParForFrontend.sh
	WeakFFParForInternal.sh
MPI/
	calculate_stats.sh
	Makefile
	MPItest.sh
	MPIWavefront.cpp
	mybash.sh
Sequential/
	include/
		hpc_helpers.hpp
	Makefile
	ProgWavefront.cpp
	SequentialTestFrontend.sh
	SequentialTestInternal.sh
```

## Implementations

### MPI Implementation

The MPI implementation is located in the `MPI/` directory. The main source file is [`MPIWavefront.cpp`](MPI/MPIWavefront.cpp). This implementation uses MPI to distribute the computation across multiple processes.

To compile the MPI implementation, use the provided Makefile:

```sh
cd MPI
make
```

To run the MPI implementation, use the provided SLURM script [`mybash.sh`](MPI/mybash.sh):

```sh
sbatch mybash.sh
```

### FastFlow Implementation

The FastFlow implementation is located in the `FastFlow/` directory. There are two main source files: [`MapFF.cpp`](FastFlow/MapFF.cpp) and [`ParForFF.cpp`](FastFlow/ParForFF.cpp). These implementations use the FastFlow framework to parallelize the computation.

To compile the FastFlow implementation, use the provided Makefile:

```sh
cd FastFlow
make
```

To run the FastFlow implementation, use the provided scripts, such as [`StrongFFMapFrontend.sh`](FastFlow/StrongFFMapFrontend.sh):

```sh
./StrongFFMapFrontend.sh
```

### Sequential Implementation

The sequential implementation is located in the `Sequential/` directory. The main source file is [`ProgWavefront.cpp`](Sequential/ProgWavefront.cpp). This implementation performs the computation sequentially for comparison purposes.

To compile the sequential implementation, use the provided Makefile:

```sh
cd Sequential
make
```

To run the sequential implementation, use the provided script [`SequentialTestFrontend.sh`](Sequential/SequentialTestFrontend.sh):

```sh
./SequentialTestFrontend.sh
```

## Helper Functions

The project includes a helper header file [`hpc_helpers.hpp`](Sequential/include/hpc_helpers.hpp) that provides macros for timing and other utilities.

## Performance Evaluation

The project includes several scripts to evaluate the performance of the implementations under different configurations:

- [`MPItest.sh`](MPI/MPItest.sh): Runs the MPI implementation with various configurations.
- [`calculate_stats.sh`](MPI/calculate_stats.sh): Calculates statistics from the output of the MPI tests.
- [`StrongFFMapFrontend.sh`](FastFlow/StrongFFMapFrontend.sh): Runs the FastFlow Map implementation with strong scaling.
- [`WeakFFMapFrontend.sh`](FastFlow/WeakFFMapFrontend.sh): Runs the FastFlow Map implementation with weak scaling.
- [`StrongFFParForFrontend.sh`](FastFlow/StrongFFParForFrontend.sh): Runs the FastFlow ParFor implementation with strong scaling.
- [`WeakFFParForFrontend.sh`](FastFlow/WeakFFParForFrontend.sh): Runs the FastFlow ParFor implementation with weak scaling.
- [`SequentialTestFrontend.sh`](Sequential/SequentialTestFrontend.sh): Runs the sequential implementation.

## Acknowledgments

This project uses the FastFlow framework for parallel programming. More information about FastFlow can be found [here](https://github.com/fastflow/fastflow).
