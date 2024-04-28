#include <iostream>
#include <thread>
#include <mutex>

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp("lock", argv[1]) != 0 && strcmp("nolock", argv[1]) != 0)) {
        std::cout << "Usage: " << argv[0] << " {lock, nolock}\n";
        return EXIT_FAILURE;
    }

    size_t a = 0;
    std::mutex m_a;
    bool use_locks = strcmp("lock", argv[1]) == 0;

    auto f = [&] {
        if (use_locks) 
            m_a.lock();
        for (size_t i = 0; i < 1000 * 1000 * 1000; ++i) {
            ++a;
        }
        if (use_locks) 
            m_a.unlock();
    };

    std::thread th1(f), th2(f);
    th1.join(); th2.join();
    std::cout << a << std::endl;

    return 0;
}