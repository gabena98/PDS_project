#include <iostream>   // For input and output
#include <vector>     // For using std::vector
#include <random>     // For random number generation
#include <cassert>    // For assert function
#include <cmath>      // For mathematical functions like std::cbrt
#include <ff/ff.hpp>  // FastFlow library for parallel programming
#include <ff/parallel_for.hpp>  // Parallel for loop in FastFlow
#include <hpc_helpers.hpp>  // This file include TIMER macros

using namespace ff; 

// Function to perform wavefront computation on matrix M of size N with num_workers
void wavefront(std::vector<std::vector<double>> &M, const uint64_t &N, ssize_t num_workers) {
    // ParallelForReduce object for parallel for loops
    ParallelForReduce<double> pf(num_workers); 
    // Loop over each upper diagonal of the matrix
    for(uint64_t k = 1; k < N; ++k) { 
        // Ensure num_workers does not exceed the number of elements in the current diagonal
        num_workers = num_workers > (ssize_t)(N - k) ? (ssize_t)(N - k) : num_workers; 
        // Parallel loop over each element in the current diagonal
        pf.parallel_for(0, N - k, 1, 0, [&](uint64_t m){
            double dot_product = 0.0;
            // Compute the dot product incrementally for the current element
            for (uint64_t i = 0; i < k; ++i) {
                dot_product += M[m][m + i] * M[m + k - i][m + k];
            }
            // Update the matrix element with the cube root of the dot product
            M[m][m + k] = std::cbrt(dot_product);
            },
         num_workers);  // Specify the number of workers for parallel execution
    }
}

// Function to print the upper triangular matrix
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
    uint64_t N = 512;   // Default size of the matrix (NxN)
    ssize_t num_workers = ff_numCores();  // Default number of workers as the number of CPU cores
    if (argc != 1 && argc != 2 && argc != 3) {
        std::printf("use: %s N num_workers\n", argv[0]);
        std::printf("     N size of the square matrix\n");
        return -1;
    }
    if (argc > 1) {
        N = std::stol(argv[1]);  // Get the matrix size from command line argument
        if (argc > 2) {
            num_workers = std::stol(argv[2]);  // Get the number of workers from command line argument
            if (num_workers <= 0) {
                std::cerr << "num_workers should be greater than 0\n";
                return -1;
            }
        }
    }
    // Allocate the matrix
    std::vector<std::vector<double>> M(N, std::vector<double>(N, 0));
    // Init function to initialize the matrix
    auto init = [&]() {
        for (uint64_t i = 0; i < N; ++i) {
            // Initialize the major diagonal elements
            M[i][i] = static_cast<double>(i + 1) / N;
        }
       // Uncomment to print the initial matrix
       // printUpperTriangular(M);
    };

    init();

    std::printf("Matrix initialized.\n");

    // Measure the time taken for wavefront computation
    TIMERSTART(wavefront);
    wavefront(M, N, num_workers);
    TIMERSTOP(wavefront);

    // Uncomment to print the final matrix
    // printUpperTriangular(M);
    
    // Output the value in the upper right corner of the matrix
    std::cout << "Value in the upper right corner of the matrix: " << M[0][N-1] << std::endl;   
    return 0;
}