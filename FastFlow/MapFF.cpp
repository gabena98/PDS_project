#include <iostream>         // For input/output stream
#include <vector>           // For using the std::vector container
#include <cmath>            // For mathematical functions, here specifically for std::cbrt
#include <ff/ff.hpp>        // For FastFlow framework
#include <ff/farm.hpp>      // For FastFlow farm
#include <hpc_helpers.hpp>  // This file include TIMER macros
#include <barrier>          // C++20 barrier for thread synchronization

using namespace ff;

// Define a struct to represent a task that each worker will execute
struct Task {
    uint64_t start, end, num_workers, index;
    std::vector<std::vector<double>>* M;
};

// Define a worker class that inherits from ff_node_t and processes tasks
struct Worker : ff_node_t<Task> {
    Worker(std::barrier<> &barrier) : barrier(barrier) {}

    Task* svc(Task* task) {
        auto& M = *task->M; // Matrix on which computation takes
        uint64_t start = task->start; // Computation starting point diagonal
        uint64_t end = task->end; // Computation ending point on diagonal

        // Process each upper diagonal
        for (uint64_t k = 1; k < M.size(); ++k) {
            // Recompute variable for k-th diagonal
            uint64_t chunk_size = (M.size() - k) / task->num_workers;
            uint64_t remainder = (M.size() - k) % task->num_workers;

            // Recompute the interval for each worker
            start = task->index * chunk_size + (task->index < remainder ? task->index : remainder);
            end = (task->index + 1) * chunk_size + (task->index < remainder ? (task->index + 1) : remainder);

            // Process elements in the k-th diagonal assigned to this worker
            for (uint64_t m = start; m < end; ++m) {
                //if (m >= (M.size() - k)) break;
                double dot_product = 0.0;
                for (uint64_t i = 0; i < k; ++i) {
                    dot_product += M[m][m + i] * M[m + k - i][m + k];
                }
                M[m][m + k] = std::cbrt(dot_product);
            }
            barrier.arrive_and_wait(); // Synchronize after processing each diagonal
        }
        delete task; // Clean up task after processing
        return GO_ON;
    }

    std::barrier<> &barrier;
};

// Define an emitter class that inherits from ff_monode_t and generates tasks
struct Emitter : ff_monode_t<Task> {
    Emitter(const std::vector<Task>& tasks) : tasks(tasks), task_index(0) {}

    Task* svc(Task*) {
        if (task_index >= tasks.size())
            return EOS;
        return new Task(tasks[task_index++]);
    }

    std::vector<Task> tasks;
    size_t task_index;
};

// Function to perform wavefront computation on matrix M of size N with num_workers
void wavefront(std::vector<std::vector<double>> &M, const uint64_t &N, const unsigned int &num_workers) {
    std::vector<Task> tasks;
    unsigned int workers_to_use = (N-1) < num_workers ? (N-1) : num_workers;
    std::barrier barrier(workers_to_use);
    // Compute interval values for the first upper diagonal
    uint64_t chunk_size = (N-1) / workers_to_use;
    uint64_t remainder = (N-1) % workers_to_use;
    uint64_t start = 0;
    // Create as many tasks as workers_to_use
    for (unsigned int w = 0; w < workers_to_use; ++w) {
        uint64_t end = start + chunk_size + (w < remainder ? 1 : 0);
        tasks.push_back(Task{start, end, workers_to_use, w, &M});
        start = end;
    }
    // Pass tasks to emitter
    Emitter emitter(tasks);
    std::vector<std::unique_ptr<ff_node>> workers;
    for (unsigned int i = 0; i < workers_to_use; ++i) {
        workers.push_back(make_unique<Worker>(barrier));
    }
    // Create MAP
    ff_Farm<Task> map(std::move(workers), emitter);
    map.remove_collector();    // Remove collector as we don't need to collect results
    map.set_scheduling_ondemand(); // Set scheduling policy
    // deactivate mapping function if workers are more than physical cores.
    if(workers_to_use > (ff_numCores()/2)){
        map.no_mapping();
    }
    // run Map
    if (map.run_and_wait_end() < 0) {
        error("running farm");
    }
}

// Function to print the upper triangular part of the matrix
void printUpperTriangular(const std::vector<std::vector<double>>& matrix) {
    std::size_t rows = matrix.size();

    // Check if the matrix is square
    if (rows == 0 || rows != matrix[0].size()) {
        std::cout << "The matrix is not square." << std::endl;
        return;
    }

    // Print the upper triangular matrix
    for (std::size_t i = 0; i < rows; ++i) {
        for (std::size_t j = 0; j < rows; ++j) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char *argv[]) {
    uint64_t N = 512; // Default size of the matrix (NxN)
    unsigned int num_workers = ff_numCores(); // Default number of workers
    
    if (argc < 2 || argc > 3) {
        std::printf("use: %s N [num_workers]\n", argv[0]);
        std::printf("     N size of the square matrix\n");
        std::printf("     num_workers number of workers to use\n");
        return -1;
    }
    if (argc > 1) {
        N = std::stol(argv[1]);
    }
    if (argc > 2) {
        num_workers = std::stoul(argv[2]);
    }

    // Allocate and initialize the matrix
    std::vector<std::vector<double>> M(N, std::vector<double>(N, 0));
    auto init = [&]() {
        for (uint64_t i = 0; i < N; ++i) {
            M[i][i] = static_cast<double>(i + 1) / N;
        }
    };
    init();

    std::printf("Matrix initialized.\n");

    // Measure the time taken by the wavefront computation
    TIMERSTART(wavefront);
    wavefront(M, N, num_workers);
    TIMERSTOP(wavefront);

    // Print the value in the upper right corner of the matrix
    std::cout << "Value in the upper right corner of the matrix: " << M[0][N-1] << std::endl;
    return 0;
}