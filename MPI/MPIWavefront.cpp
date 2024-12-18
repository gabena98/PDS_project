#include <mpi.h>
#include <iostream>
#include <vector>
#include <cassert>
#include <cmath> // For std::cbrt

// Function to perform wavefront computation on matrix M
void wavefront(std::vector<std::vector<double>> &M, const int &N, const int rank, const int size) {
    for(int k = 1; k < N; ++k) {    // for each upper diagonal
        std::vector<int> counts(size);
        std::vector<int> displs(size);
     
        // Compute chunk_size and remainder given the k-th diagonal
        int chunk_size = (N - k) / size;
        int remainder = (N - k) % size;
        // Recompute the interval for each worker
        for (int i=0; i<size; ++i){
            auto start = i * chunk_size + (i < remainder ? i : remainder);
            auto end = (i + 1) * chunk_size + (i < remainder ? (i + 1) : remainder);
            // Compute offsets and displacements
            counts[i] = end - start;
            displs[i] = start;
        }

        std::vector<double> buffer; // Buffer to hold the computed results
        std::vector<double> collect((N-k)); // Vector to collect all results from all processes

        // Process elements in the k-th diagonal, subdivided between workers
        for (int m = displs[rank]; m < (displs[rank] + counts[rank]); ++m) {
            double dot_product = 0.0;
            // Compute incrementally dot product
            for (int i = 0; i < k; ++i) {
                dot_product += M[m][m + i] * M[m + k - i][m + k];
            }
            // Write final result in the matrix and append to the buffer
            M[m][m + k] = std::cbrt(dot_product);
            buffer.push_back(std::cbrt(dot_product));
        }

        // Gather results from all processes
        MPI_Allgatherv(buffer.data(), counts[rank], MPI_DOUBLE, collect.data(), counts.data(), displs.data(), MPI_DOUBLE, MPI_COMM_WORLD);
   
        // Update diagonal elements
        for(int h = 0; h < (N - k); ++h){
            if (h >= displs[rank] && h < (displs[rank] + counts[rank]))
                continue;
            M[h][h + k] = collect[h];
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

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = 512;   // Default size of the matrix (NxN)

    // Parse command line arguments for matrix size
    if (argc != 1 && argc != 2) {
        if(!rank){
            std::printf("use: %s N [min max]\n", argv[0]);
            std::printf("     N size of the square matrix\n");
        } 
        return -1;
    }
    if (argc > 1) {
        N = std::stol(argv[1]);
    }

    // Allocate the matrix
    std::vector<std::vector<double>> M(N, std::vector<double>(N, 0));

    // Initialize the matrix
    auto init = [&]() {
        for (int i = 0; i < N; ++i) {
            // Initialize the major diagonal elements
            M[i][i] = static_cast<double>(i + 1) / N;
        }
    };
    init();

    double start = MPI_Wtime(); // Start timing
    wavefront(M, N, rank, size);
    double end = MPI_Wtime(); // End timing
    
    // Measure the current time and print by process 0
    if(!rank){
        std::cout << "Time with " << size << " processes: " << end-start << " seconds" << std::endl;
        std::cout<< "Value in the upper right corner of the matrix: "<<M[0][N-1]<<std::endl;   
    }

    MPI_Finalize();
    return 0;
}