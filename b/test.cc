#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

void perform_operations(long long start, long long end, double &result) {
    for (long long i = start; i < end; ++i) {
        result += (i * 0.1) / (i + 1.0);
    }
}


float foo(long long max_iterations ){

    const unsigned num_threads = std::thread::hardware_concurrency();
     std::vector<double> results(num_threads, 0.0);

    std::vector<std::thread> threads;
    for (unsigned i = 0; i < num_threads; ++i) {
        long long start = i * (max_iterations / num_threads);
        long long end = (i + 1) * (max_iterations / num_threads);
        threads.push_back(std::thread(perform_operations, start, end, std::ref(results[i])));
    }

     // Wait for all threads to finish
    for (auto &thread : threads) {
        thread.join();
    }

    // Combine the results from each thread
    float result = 0.0;
    for (const auto &partial_result : results) {
        result += partial_result;
    }

    return result;
}



int main() {
    auto start_time = std::chrono::high_resolution_clock::now();

    long long input = 5e10;
    
    float result = foo(input);




    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;

    std::cout << "Result: " << result << std::endl;
    std::cout << "Time taken: " << duration.count() << "s" << std::endl;

    // Estimating the FLOPS
    long long total_operations = 3 * input; // we perform 3 operations per iteration
    double flops = total_operations / duration.count();
    std::cout << "Estimated FLOPS: " << flops << std::endl;

    return 0;
}
