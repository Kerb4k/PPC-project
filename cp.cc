#include <vector>
#include <cmath>
#include <omp.h>
#include <chrono>
#include <iostream>

long long flops = 0;

void calculate_mean_deviation(int ny, int nx, const float* data, std::vector<double>& means, std::vector<double>& dev) {
    means.resize(ny);
    dev.resize(ny);

    #pragma omp parallel for schedule(dynamic, 4)
    for (int i = 0; i < ny; ++i) {
        const float* row_i = data + i * nx;
        double sum = 0;
        for (int x = 0; x < nx; ++x) {
            sum += double(row_i[x]);
            #pragma omp atomic
            flops += 1;  // Addition operation
        }
        double mean = sum / nx;
        means[i] = mean;

        #pragma omp atomic
        flops += 1;  // Division operation

        double variance_sum = 0;
        for (int x = 0; x < nx; ++x) {
            double diff = double(row_i[x]) - mean;
            variance_sum += pow(diff, 2);
            #pragma omp atomic
            flops += 3;  // Subtraction, multiplication (square), and addition operations
        }
        dev[i] = std::sqrt(variance_sum / nx);
        #pragma omp atomic
        flops += 2;  // Division and sqrt operations
    }
}

void calculate_correlation(int ny, int nx, const float* data, const std::vector<double>& means, const std::vector<double>& dev, float* result) {
    #pragma omp parallel for schedule(dynamic, 4)
    for (int i = 0; i < ny; ++i) {
        const float* row_i = data + i * nx;
        double mean_i = means[i];
        double sd_i = dev[i];
       
        for (int j = 0; j <= i; ++j) {
            const float* row_j = data + j * nx;
            double mean_j = means[j];
            double sd_j = dev[j];

            double correlation_sum = 0;
            for (int x = 0; x < nx; ++x) {
                correlation_sum += (double(row_i[x]) - mean_i) * (double(row_j[x]) - mean_j);
                #pragma omp atomic
                flops += 5;  // Two subtractions, two multiplications, and addition
            }
            result[i + j * ny] = float(correlation_sum / (nx * sd_i * sd_j));
            #pragma omp atomic
            flops += 3;  // Multiplication, division, and assignment
        }
    }
}

void correlate(int ny, int nx, const float* data, float* result) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<double> means;
    std::vector<double> devs;

    calculate_mean_deviation(ny, nx, data, means, devs);
    calculate_correlation(ny, nx, data, means, devs, result);
    auto finish = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = finish - start;
    //std::cout << "FLOPs: " << flops << "\n";
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";
    std::cout << "FLOPS: " << flops / elapsed.count() << "\n";
}
#include <random>

int main() {
    int ny = 2000;
    int nx = 2000;

    // Dynamic allocation of the data array
    float* data = new float[ny * nx];
    float* result = new float[ny * ny];

    // Fill the data array with random values
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    for (int i = 0; i < ny * nx; ++i) {
        data[i] = dis(gen);
    }

    // Call the correlate function
    correlate(ny, nx, data, result);

    // Deallocate the memory after use
    delete[] data;
    delete[] result;

    return 0;
}
