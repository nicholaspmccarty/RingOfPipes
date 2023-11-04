#include <iostream>
#include <cstring>
#include <ext/stdio_filebuf.h>
#include <unistd.h>
#include <sys/wait.h>
#include <expected>
#include <vector>
#include <chrono>
#include <iomanip>
#include <memory>
constexpr inline int READ = 0;
constexpr inline int WRITE = 1;

#define ASSERT(condition) { if(!(condition)){ std::cerr << "ERROR: " << #condition << " @ " << __FILE__ << " (" << __LINE__ << ")" << std::endl; return -1; } }

enum class [[nodiscard]] Status {
    ok,
    invalid_input,
    index_error,
    pipe_creation_error,
    file_buffer_error,
    io_error,
    process_error,
    file_close_error,
    unexpected_condition
};


template <typename T>
using StatusOr = std::expected<T, Status>;

/**
 * @brief Executes a magical operation on two sets of pipe file descriptors.
 *
 * This function operates on two sets of pipe file descriptors, pipefd1 and pipefd2
 * It first closes the write end of pipefd1 and the read end of pipefd2 to prepare for input and output.
 * Next, it sets up file buffers for input and output, and performs error checks upon the way (See STD::expected)
 * It reads data line by line from 'pipefd1', processes it in a magical manner, and writes the result to 'pipefd2'.
 * (e.g., 'invalid_input' or 'index_error'). Proper closing of file descriptors is also ensured.
 *
 * @param pipefd1 An array of two integers representing the pipe file descriptors for input.
 * @param pipefd2 An array of two integers representing the pipe file descriptors for output.
 * 
 * @return An 'std::expected' containing 'StatusOr<void>' to indicate success or specific error conditions.
 */
StatusOr<void> theMagic(int pipefd1[2], int pipefd2[2]) {
    close((pipefd1[WRITE]) >= 0);
    close((pipefd2[READ]) >= 0);
    
    // Pre-lim error checking.
    if (close(pipefd1[WRITE]) < 0 || close(pipefd2[READ]) < 0) {
        return std::unexpected(Status::index_error);
    }
    std::filebuf* in_fb = new __gnu_cxx::stdio_filebuf<char>(pipefd1[READ], std::ios::in, 1);
    std::filebuf* out_fb = new __gnu_cxx::stdio_filebuf<char>(pipefd2[WRITE], std::ios::out, 1);
    
    if (!in_fb || !out_fb) {
        return std::unexpected(Status::invalid_input);
    }

    std::istream* is = new std::istream(in_fb);
    std::ostream* os = new std::ostream(out_fb);

    std::string line(15, ' ');
    
    while (!is->eof() && is->good()) {
        getline(*is, line);
        if (!line.empty()) {
            (*os) << line << std::endl;
        }
    }
    
    close((pipefd1[READ]) >= 0);
    close((pipefd2[WRITE]) >= 0);
   return {};
}



int main() {
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
   int N;
do {
    std::cout << "Enter the number of pipes and processes (N): ";
    if (!(std::cin >> N) || N <= 0) {
        std::cout << "Invalid input. Please enter a positive integer for N." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
} while (N <= 0);
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    
    std::vector<std::pair<int, int>> pipe_fds;

    // Create N pipes and store the FDs in the vector
    for (uint64_t q = 0; static_cast<int>(q) < N; q++) {
        
        int in_pipefd[2], out_pipefd[2];
        if (pipe(in_pipefd) >= 0 && pipe(out_pipefd) >= 0) {
            pipe_fds.push_back({in_pipefd[0], out_pipefd[1]});
        } else {
            std::cerr << "ERROR: Could not create pipes." << std::endl;
        }
    }
  
    for (size_t z = 0; z < static_cast<size_t>(N - 1); z++) {
         pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "Fork failed." << std::endl;
            return 1;
        }
        if (pid == 0) {
            theMagic(&pipe_fds[z].first, &pipe_fds[z].first);
            exit(0);
        }
    }
    
    
    // Creating our file buffers
    std::filebuf* custom_write_fb = new __gnu_cxx::stdio_filebuf<char>(pipe_fds[0].first, std::ios::out);
    std::filebuf* custom_read_fb = new __gnu_cxx::stdio_filebuf<char>(pipe_fds[0].second, std::ios::in);


    // Creating our output streams
    // Using auto and make unique makes sure that we have automatic memory management and better readability.
    auto custom_write_stream = std::make_unique<std::ostream>(custom_write_fb);
    auto custom_read_stream = std::make_unique<std::istream>(custom_read_fb);

    auto custom_send_line = "Hello World!";
    (*custom_write_stream) << custom_send_line << std::endl;


    std::string custom_recv_line = custom_send_line;
    auto end_time = std::chrono::high_resolution_clock::now();
    while (!custom_read_stream->eof() && custom_read_stream->good()) {
    getline(*custom_read_stream, custom_recv_line);
        if (!custom_recv_line.empty()) {
            end_time = std::chrono::high_resolution_clock::now();
        
    }
    close(pipe_fds[0].first);
    close(pipe_fds[0].second);
}

    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    std::cout << "Time elapsed: " << elapsed_seconds.count() << "s" << std::endl;
    return 0;
}

