#include <iostream>
#include <thread>
#include <mutex>

int main(int argc, char *argv[]) {

    size_t a = 0;
    std::mutex m_a;

    auto f = [&] {
        size_t a_local = 0;
        for (size_t i = 0; i < 1000 * 1000 * 1000; ++i) {
            ++a_local;
        }
        m_a.lock();
        a += a_local;
        m_a.unlock();
    };

    std::thread th1(f), th2(f);
    th1.join(); th2.join();
    std::cout << a << std::endl;

    return 0;
}