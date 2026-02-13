#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <reproc++/reproc.hpp>
#include <reproc++/drain.hpp>

// Helper to burn CPU
void burn_cpu(std::atomic<bool> &keep_running) {
    volatile unsigned long long counter = 0;
    while (keep_running) { counter = counter + 1; }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <command> [args...]" << std::endl;
        return 1;
    }

    // 1. Setup CPU Burners
    unsigned int num_threads = std::thread::hardware_concurrency();
    std::atomic<bool> keep_burning{true};
    std::vector<std::thread> threads;

    std::cout << "Starting " << num_threads << " stress threads..." << std::endl;
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back(burn_cpu, std::ref(keep_burning));
    }

    // 2. Prepare Command
    // reproc expects a vector of strings
    std::vector<std::string> cmd;
    for (int i = 1; i < argc; ++i) cmd.push_back(argv[i]);

    std::cout << "Running command..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    // 3. Start Process using Reproc
    reproc::process process;
    reproc::options options;
    options.redirect.parent = true; // Redirect output to parent's stdout/err

    // This single line handles fork/exec (Linux) and CreateProcess (Windows)
    std::error_code ec = process.start(cmd, options);

    if (ec) {
        std::cerr << "Failed to start process: " << ec.message() << std::endl;
        keep_burning = false;
        for (auto &t: threads) t.join();
        return 1;
    }

    // 4. Wait for it to finish
    // Reproc handles the complexity of waitpid vs WaitForSingleObject
    int status = 0;
    std::tie(status, ec) = process.wait(reproc::infinite);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    if (ec) {
        std::cerr << "Error waiting for process: " << ec.message() << std::endl;
    }

    // 5. Cleanup
    keep_burning = false;
    for (auto &t: threads) t.join();

    std::cout << "Subprocess duration :" << elapsed.count() << " s" << std::endl;
    std::cout << "Subprocess exit code:" << status << std::endl;

    return 0;
}
