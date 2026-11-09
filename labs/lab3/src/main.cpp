#include <mpi.h>

#include <chrono>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

typedef long long Value;

struct Matrix
{
    int n;
    std::vector<Value> a;
    explicit Matrix(int size = 0) : n(size), a((size_t)size * size, 0) {}
};

inline Matrix read_matrix(const std::string &path)
{
    FILE *f = std::fopen(path.c_str(), "r");
    if (!f)
        throw std::runtime_error("cannot open " + path);
    int n = 0;
    if (std::fscanf(f, "%d", &n) != 1 || n <= 0)
    {
        std::fclose(f);
        throw std::runtime_error("bad file " + path);
    }
    Matrix m(n);
    for (size_t i = 0; i < m.a.size(); ++i)
        if (std::fscanf(f, "%lld", &m.a[i]) != 1)
        {
            std::fclose(f);
            throw std::runtime_error("bad file " + path);
        }
    std::fclose(f);
    return m;
}

inline void write_matrix(const std::string &path, const Matrix &m)
{
    FILE *f = std::fopen(path.c_str(), "w");
    if (!f)
        throw std::runtime_error("cannot write " + path);
    std::fprintf(f, "%d\n", m.n);
    for (int i = 0; i < m.n; ++i)
        for (int j = 0; j < m.n; ++j)
            std::fprintf(f, "%lld%c", m.a[(size_t)i * m.n + j], j + 1 < m.n ? ' ' : '\n');
    std::fclose(f);
}

inline void multiply(const Value *A, const Value *B, Value *C, int n, int rows)
{
    for (int i = 0; i < rows; ++i)
    {
        Value *c = C + (size_t)i * n;
        for (int j = 0; j < n; ++j)
            c[j] = 0;
        for (int k = 0; k < n; ++k)
        {
            const Value aik = A[(size_t)i * n + k];
            const Value *b = B + (size_t)k * n;
            for (int j = 0; j < n; ++j)
                c[j] += aik * b[j];
        }
    }
}

inline double now_seconds()
{
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

inline void print_result(const char *lab, int n, int workers, double seconds)
{
    const double ops = 2.0 * n * n * n;
    std::printf("%s,%d,%d,%.6f,%.0f,%.3f\n", lab, n, workers, seconds, ops, ops / seconds * 1e-9);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank = 0, size = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (argc != 4)
    {
        if (rank == 0)
            std::fprintf(stderr, "usage: mpirun -np P %s A.txt B.txt C.txt\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    Matrix A, B;
    int n = 0;
    if (rank == 0)
    {
        try
        {
            A = read_matrix(argv[1]);
            B = read_matrix(argv[2]);
        }
        catch (const std::exception &e)
        {
            std::fprintf(stderr, "error: %s\n", e.what());
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (A.n != B.n)
        {
            std::fprintf(stderr, "error: sizes differ\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        n = A.n;
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0)
        B = Matrix(n);

    std::vector<int> counts(size), displs(size);
    for (int r = 0, row = 0; r < size; ++r)
    {
        const int rows = n / size + (r < n % size ? 1 : 0);
        counts[r] = rows * n;
        displs[r] = row * n;
        row += rows;
    }
    std::vector<Value> a(counts[rank]), c(counts[rank]);
    Matrix C(rank == 0 ? n : 0);

    MPI_Barrier(MPI_COMM_WORLD);
    const double t0 = MPI_Wtime();
    MPI_Scatterv(A.a.data(), counts.data(), displs.data(), MPI_LONG_LONG,
                 a.data(), counts[rank], MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(B.a.data(), n * n, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    multiply(a.data(), B.a.data(), c.data(), n, counts[rank] / n);
    MPI_Gatherv(c.data(), counts[rank], MPI_LONG_LONG,
                C.a.data(), counts.data(), displs.data(), MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    const double t = MPI_Wtime() - t0;

    if (rank == 0)
    {
        try
        {
            write_matrix(argv[3], C);
        }
        catch (const std::exception &e)
        {
            std::fprintf(stderr, "error: %s\n", e.what());
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        print_result("lab_03", n, size, t);
    }
    MPI_Finalize();
    return 0;
}