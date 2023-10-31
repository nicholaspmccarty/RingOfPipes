#include <iostream>
#include <cstring>
#include <ext/stdio_filebuf.h>
#include <unistd.h>
#include <sys/wait.h>
#include <expected>

constexpr inline int READ = 0;
constexpr inline int WRITE = 1;

#define ASSERT(condition) { if(!(condition)){ std::cerr << "ERROR: " << #condition << " @ " << __FILE__ << " (" << __LINE__ << ")" << std::endl; return -1; } }

enum class [[nodiscard]] Status {
    ok, invalid_input, index_error
};

template <typename T>
using StatusOr = std::expected<T, Status>;

[[nodiscard]]
int parent(int pipefd[]) {
    ASSERT(close(pipefd[READ]) >= 0);

    std::filebuf* fb = new __gnu_cxx::stdio_filebuf<char>(pipefd[WRITE], std::ios::out, 1);
    ASSERT(fb != nullptr);

    std::ostream* os = new std::ostream(fb);
    ASSERT(os != nullptr);

    for (int i = 0; (i < 10); i++) {
        std::cout << "Sent #" << i << std::endl;
        (*os) << "Testing #" << i << std::endl;
    }
    delete os;
    ASSERT(close(pipefd[WRITE]) >= 0);

    return 0;
}

[[nodiscard]]
int child(const int pipefd[]) {
    ASSERT(close(pipefd[WRITE]) >= 0);

    std::filebuf* fb = new __gnu_cxx::stdio_filebuf<char>(pipefd[READ], std::ios::in, 1);
    ASSERT(fb != nullptr);

    std::istream* is = new std::istream(fb);
    ASSERT(is != nullptr);

    std::string line;
    while (!is->eof() && is->good()) {
        getline(*is, line);
        if (!line.empty()) {
            std::cout << "*" << line << std::endl;
        }
    }

    delete is;
    ASSERT(close(pipefd[READ]) >= 0);

    return 0;
}

int main() {
    // Create Storage for two pipes
    int pipefd[2] = { 0,0 };

    // Fill pipes from os call
    ASSERT(pipe(pipefd) >= 0);

    // Fork parent process into two - parent and child
    pid_t pid = fork();
    ASSERT(pid >= 0);

    if (pid == 0) {
        ASSERT(child(pipefd) >= 0);
    } else {
        ASSERT(parent(pipefd) >= 0);

        int exitCode = 0;
        ASSERT(waitpid(pid, &exitCode, 0) >= 0);
    }

    return 0;
}
