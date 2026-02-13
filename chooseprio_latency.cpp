#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <future>
#include <chrono>
#include <iomanip>
#include <reproc++/reproc.hpp>

// --- Windows Timer Precision ---
// Ensure 16ms sleep is accurate on Windows.
#ifdef _WIN32
    #include <windows.h>
    #include <mmsystem.h>
    #pragma comment(lib, "winmm.lib")
    void enable_high_res_timer() { timeBeginPeriod(1); }
    void disable_high_res_timer() { timeEndPeriod(1); }
#else
    void enable_high_res_timer() {}
    void disable_high_res_timer() {}
#endif

// --- Statistics Helper ---
struct RunningStats {
    double average_ms = 0.0;
    long count = 0;

    void update(double new_val_ms) {
        count++;
        // Welford's online algorithm for stable mean
        average_ms += (new_val_ms - average_ms) / count;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <command> [args...]" << std::endl;
        return 1;
    }

    // 1. Prepare arguments for reproc
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    // 2. Setup High-Res Timer
    enable_high_res_timer();

    std::cout << "Starting process via reproc..." << std::endl;

    // 3. Start the process
    reproc::process process;
    reproc::options options;
    options.redirect.parent = true;

    std::error_code ec = process.start(args, options);

    if (ec) {
        std::cerr << "Error starting process: " << ec.message() << std::endl;
        return 1;
    }

    auto future = std::async(std::launch::async, [&process]() {
            // reproc wait using a delay of 0 returns an error. Block in a different thread.
            return process.wait(reproc::infinite);
        });

    RunningStats stats;
    bool is_running = true;
    int exit_status = 0;

    std::cout << std::fixed << std::setprecision(4);

    // 4. Main Monitoring Loop
    while (is_running) {
        // A. Measure Sleep
        auto start = std::chrono::high_resolution_clock::now();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;

        // B. Update Stats
        stats.update(elapsed.count());

        // C. Check if process is still running
        std::future_status status = future.wait_for(std::chrono::milliseconds(0));

        if (status == std::future_status::ready) {
            std::tie(exit_status, ec) = future.get();
            if (ec) {
                std::cerr << "\nError status: " << ec.message() << std::endl;
            }
            is_running = false;
        }

        // D. Live Output
        if (is_running && ((stats.count % 100) == 0)) {
            std::cout << "\rCount: " << stats.count
                      << " | Avg Sleep: " << stats.average_ms << "ms"
                      << " | Last: " << elapsed.count() << "ms    " << std::flush;
        }
    }

    disable_high_res_timer();

    std::cout << "\n\n--- Process Finished ---" << std::endl;
    std::cout << "Exit Code: " << exit_status << std::endl;
    std::cout << "Total Samples: " << stats.count << std::endl;
    std::cout << "Average Sleep Duration: " << stats.average_ms << " ms" << std::endl;

    return 0;
}