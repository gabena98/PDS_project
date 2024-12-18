#include <iostream>        // For input/output stream
#include <vector>          // For using the std::vector container
#include <cmath>           // For mathematical functions, here specifically for std::cbrt
#include <hpc_helpers.hpp> // Contains the TIMERSTART and TIMERSTOP macros

// Function to perform the wavefront computation on matrix M of size N x N
void wavefront(std::vector<std::vector<double>> &M, const uint64_t &N) {
    for(uint64_t k = 1; k < N; ++k) {        // Loop over each upper diagonal
        for(uint64_t m = 0; m < (N - k); ++m) { // Loop over each element in the diagonal
            double dot_product = 0.0;
            // Compute the dot product incrementally
            for (uint64_t i = 0; i < k; ++i) {
                dot_product += M[m][m + i] * M[m + k - i][m + k];
            }
            // Write the final result into the matrix
            M[m][m + k] = std::cbrt(dot_product); // Use cube root of the dot product
        }
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
    uint64_t N = 512;   // Default size of the matrix (NxN)
    
    if (argc != 1 && argc != 2) {
        std::printf("use: %s N [min max]\n", argv[0]);
        std::printf("     N size of the square matrix\n");
        return -1;
    }
    if (argc > 1) {
        N = std::stol(argv[1]);
    }

    // Allocate the matrix
    std::vector<std::vector<double>> M(N, std::vector<double>(N, 0));

    // Init function to initialize the major diagonal elements
    auto init = [&]() {
        for (uint64_t i = 0; i < N; ++i) {
            // Initialize the major diagonal elements
            M[i][i] = static_cast<double>(i + 1) / N;
        }
       //printUpperTriangular(M); // Uncomment to print initial matrix
    };
    
    init();

    std::printf("Matrix initialized.\n");
    
    // Measure the time taken by the wavefront computation
    TIMERSTART(wavefront);
    wavefront(M, N); 
    TIMERSTOP(wavefront);
    
    // Uncomment to print the final upper triangular matrix
    //printUpperTriangular(M);

    // Print the value in the upper right corner of the matrix
    std::cout << "Value in the upper right corner of the matrix: " << M[0][N-1] << std::endl;
    
    return 0;
}