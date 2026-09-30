#include <mpi.h>
import std;

void VectorLoading(std::vector<int> &vec, const std::string &text)
{
    vec.clear();

    int number = 0;
    std::ifstream file(text);

    if (!file.is_open())
    {
        throw std::runtime_error("Error open file: " + text);
    }

    while (file >> number)
    {
        vec.push_back(number);
    }

    if (!file.eof())
    {
        throw std::runtime_error("Error reading file: " + text);
    }
}

void productMatrixLocal(const std::vector<int> &local_A,
                        const std::vector<int> &full_B,
                        std::vector<long long> &local_C,
                        std::size_t local_N,
                        std::size_t size)
{
    local_C.resize(local_N * size);
    std::fill(local_C.begin(), local_C.end(), 0LL);

    for (std::size_t i = 0; i < local_N; ++i)
    {
        for (std::size_t k = 0; k < size; ++k)
        {
            long long a_ik = local_A[i * size + k];

            for (std::size_t j = 0; j < size; ++j)
            {
                local_C[i * size + j] += a_ik * full_B[k * size + j];
            }
        }
    }
}

bool WriteResult(const std::vector<long long> &result,
                 const std::string &full_path,
                 std::size_t size,
                 double time_sec)
{
    std::ofstream file(full_path);

    if (!file)
    {
        return false;
    }

    file << "#Время выполнения: "
         << std::fixed << std::setprecision(4) << time_sec
         << " сек\n";

    file << "#Количество элементов матрицы: "
         << size * size
         << "\n\n";

    for (std::size_t i = 0; i < size; ++i)
    {
        for (std::size_t j = 0; j < size; ++j)
        {
            file << std::setw(12) << result[i * size + j];
        }

        file << "\n";
    }

    std::cout << "Матрица успешно записана в файл " << full_path << std::endl;

    return true;
}

bool WriteRecord(
    const std::string &path,
    int attempt_number,
    int processes,
    int matrix_size,
    double elapsed)
{
    // Пытаемся создать родительскую папку, если её нет.
    // Если не получилось — просто игнорируем, ofstream ниже вернёт false.
    try
    {
        std::filesystem::create_directories(
            std::filesystem::path(path).parent_path());
    }
    catch (...)
    {
    }

    // Режим std::ios::app означает append:
    // файл не стирается, а новые данные дописываются в конец.
    std::ofstream file(path, std::ios::app);

    if (!file)
    {
        return false;
    }

    file << "\n"
         << "====================================" << "\n"
         << "Замер номер: " << attempt_number << "\n"
         << "Количество процессов: " << processes << "\n"
         << "Матрица размером: " << matrix_size << "*" << matrix_size << "\n"
         << "Время измерения: "
         << std::fixed << std::setprecision(6) << elapsed
         << " сек" << "\n"
         << "====================================" << "\n";

    return file.good();
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank = 0;
    int procs = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    const std::vector<int> sizes = {200, 400, 800, 1200, 1600, 2000};

    const std::string data_dir = "../../../labs/matrices/data/";

    const std::string result_dir = "../../../labs/matrices/result/";

    const std::string path_record = "../../../labs/matrices/record/record.txt";

    for (std::size_t i = 0; i < 5; ++i)
    {
        for (int n : sizes)
        {
            if (rank == 0)
            {
                std::cout << "====================================" << std::endl;
                std::cout << "Обрабатываю матрицу размером " << n << "x" << n << std::endl;
            }

            // Если размер не делится на количество процессов, пропускаем.
            // Все процессы принимают одинаковое решение, поэтому MPI не сломается.
            if (n <= 0 || n % procs != 0)
            {
                if (rank == 0)
                {
                    std::cerr << "Пропускаю размер " << n
                              << ", потому что он не делится на количество процессов "
                              << procs << std::endl;
                }

                continue;
            }

            int local_rows = n / procs;
            int total = n * n;
            int local_total = local_rows * n;

            // Память выделяется на всех процессах
            std::vector<int> A(total);
            std::vector<int> B(total);

            std::vector<int> local_A(local_total);
            std::vector<long long> local_C;

            int load_ok = 1;

            // Читаем файлы только на процессе 0
            if (rank == 0)
            {
                try
                {
                    std::string path1 =
                        data_dir + "m" + std::to_string(n) + "_1.txt";

                    std::string path2 =
                        data_dir + "m" + std::to_string(n) + "_2.txt";

                    std::cout << "Загрузка файлов:" << std::endl;
                    std::cout << path1 << std::endl;
                    std::cout << path2 << std::endl;

                    VectorLoading(A, path1);
                    VectorLoading(B, path2);

                    if (static_cast<int>(A.size()) != total ||
                        static_cast<int>(B.size()) != total)
                    {
                        load_ok = 0;
                        std::cerr << "Ошибка: размер загруженных данных не совпадает с ожидаемым для n="
                                  << n << std::endl;
                    }
                }
                catch (const std::exception &e)
                {
                    load_ok = 0;
                    std::cerr << "Ошибка загрузки для n=" << n << ": " << e.what() << std::endl;
                }
            }

            // Все процессы должны одинаково узнать, удалось ли загрузить файлы.
            // Иначе можно получить зависание на коллективных MPI-операциях.
            MPI_Bcast(&load_ok, 1, MPI_INT, 0, MPI_COMM_WORLD);

            if (!load_ok)
            {
                if (rank == 0)
                {
                    std::cerr << "Пропускаю размер " << n
                              << " из-за ошибки загрузки файлов" << std::endl;
                }

                continue;
            }

            // Рассылаем матрицу A по строкам
            MPI_Scatter(A.data(), local_total, MPI_INT,
                        local_A.data(), local_total, MPI_INT,
                        0, MPI_COMM_WORLD);

            // Каждому процессу нужна полная матрица B
            MPI_Bcast(B.data(), total, MPI_INT,
                      0, MPI_COMM_WORLD);

            // Замер времени параллельного умножения
            MPI_Barrier(MPI_COMM_WORLD);
            double start = MPI_Wtime();

            productMatrixLocal(local_A,
                               B,
                               local_C,
                               static_cast<std::size_t>(local_rows),
                               static_cast<std::size_t>(n));

            MPI_Barrier(MPI_COMM_WORLD);
            double finish = MPI_Wtime();

            double elapsed = finish - start;

            if (rank == 0)
            {
                std::cout << "Выполнение заняло: "
                          << std::fixed << std::setprecision(6) << elapsed
                          << " сек" << std::endl;
            }

            // Собираем результат на процессе 0
            std::vector<long long> result;

            if (rank == 0)
            {
                result.resize(total);
            }

            MPI_Gather(local_C.data(), local_total, MPI_LONG_LONG_INT,
                       rank == 0 ? result.data() : local_C.data(),
                       local_total, MPI_LONG_LONG_INT,
                       0, MPI_COMM_WORLD);

            // Пишем результат только на процессе 0
            if (rank == 0)
            {
                std::string output_path =
                    result_dir + "result" + std::to_string(n) + ".txt";

                if (!WriteResult(result, output_path, static_cast<std::size_t>(n), elapsed))
                {
                    std::cerr << "Не удалось записать результат в файл: "
                              << output_path << std::endl;
                }

                int attempt_number = static_cast<int>(i) + 1;

                if (!WriteRecord(
                        path_record,
                        attempt_number,
                        procs,
                        n,
                        elapsed))
                {
                    std::cerr << "Не удалось записать замер в файл: "
                              << path_record << std::endl;
                }
                else
                {
                    std::cout << "Замер записан в файл: " << path_record << std::endl;
                }
            }
        }
    }

    if (rank == 0)
    {
        std::cout << "====================================" << std::endl;
        std::cout << "Все матрицы обработаны." << std::endl;
    }

    MPI_Finalize();
    return 0;
}