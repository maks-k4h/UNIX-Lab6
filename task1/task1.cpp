#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <string>
#include <thread>


class MatrixGenerator {
public:
    MatrixGenerator(size_t rows, size_t cols) 
    : rows_(rows), cols_(cols)
    {
    }

    std::vector<std::vector<float> > generate() {
        std::vector<std::vector<float> > res(rows_, std::vector<float>(cols_));
        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < cols_; ++j) {
                res[i][j] = (rand() / float(RAND_MAX)) * 2 - 1;
            }
        }
        return res;
    }
    
private:
    size_t rows_;
    size_t cols_;
};


template<class T>
void show_matrix(const std::vector<std::vector<T> > &m) {
    if (m.empty()) {
        std::cout << "|  |\n";
    }

    std::cout << std::fixed;
    std::cout.precision(4);
    for (auto row : m) {
        std::cout << "| ";
        for (auto e : row) {
            std::cout << ' '  << std::setw(7) << e;
        }
        std::cout << " |\n";
    }
}


class MatrixProcessingContext {
public:
    MatrixProcessingContext(size_t num_threads) 
    : nt_(num_threads) 
    {
    }

    template<class T>
    std::vector<std::vector<T> > dot(const std::vector<std::vector<T> > &m1, const std::vector<std::vector<T> > &m2) {
        size_t rows = m1.size(), cols = m2[0].size();
        std::vector<std::vector<T> > res(rows, std::vector<T>(cols));

        auto f = [&] (size_t i, size_t j, size_t n) {
            // only perform multiplication for given elements
            // std::cout << "I'm a worker. I will be processing " << n << " elements starting from " << i << ' ' << j << std::endl;
            for (size_t k = 0; k < n; ++k) {
                // i,j — valid
                res[i][j] = 0;
                for (size_t h = 0; h < m2.size(); ++h) {
                    res[i][j] += m1[i][h] * m2[h][j];
                }
                // std::cout << i << '-' << j << " = " << res[i][j] << std::endl;

                // update i,j
                if (j + 1 < cols) {
                    ++j;
                }
                else {
                    ++i;
                    j = 0;
                }
            }
        };

        size_t i = 0, j = 0;  // keep track of current element
        auto effective_nt = std::min(nt_, rows * cols);  // number of threads to be created
        auto ept = rows * cols / effective_nt + (rows * cols % effective_nt > 0);  // number of matrix Elements Per Thread
        // create threads 
        std::list<std::thread> threads;
        for (size_t w = 0; w < effective_nt; ++w) {
            size_t epct = std::min(ept, rows * cols - i * cols - j);  // Elements Per Current Thread
            threads.push_back(std::thread(f, i, j, epct));

            i += epct / cols + (j + epct%cols) / cols;
            j = (j + epct) % cols;
        }

        // join threads
        for (auto &t : threads) {
            t.join();
        }

        return res;
    }

private:
    size_t nt_;
};


int main(int argc, char *argv[]) {
    if (!(argc == 5 || (argc == 6 && strcmp(argv[5], "-v") == 0))) {
        std::cout << "Usage: " << argv[0] << " <n> <m> <k> <num_threads> [-v]\n";
        return EXIT_FAILURE;
    }

    int n = std::stoi(argv[1]), m = std::stoi(argv[2]), k = std::stoi(argv[3]), nt = std::stoi(argv[4]);
    bool verbose = argc == 6;

    MatrixGenerator genA(n, m);
    MatrixGenerator genB(m, k);
    std::cout << "Matrix sizes: " << n << 'x' << m << " and " << m << 'x' << k << std::endl;
    auto Ma = genA.generate();
    auto Mb = genB.generate();
    if (verbose) {
        std::cout << std::endl << "Matrix A:" << std::endl;
        show_matrix(Ma);
        std::cout << std::endl << "Matrix B:" << std::endl;
        show_matrix(Mb);
    }

    MatrixProcessingContext ctx(nt);
    auto M = ctx.dot(Ma, Mb);
    if (verbose) {
        std::cout << std::endl << "Multiplication result. Matrix M:" << std::endl;
        show_matrix(M);
    }

    return 0;
}