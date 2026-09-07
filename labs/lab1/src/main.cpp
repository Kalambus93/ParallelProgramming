import std;

void VectorLoading(std::vector<int> &vec, const std::string &text)
{
    int number = 0;

    std::ifstream file(text);

    if (!file.is_open())
    {
        throw std::logic_error("Error open file");
    }

    while (file >> number)
    {
        vec.push_back(number);
    }
}

void productMatrix(const std::vector<int> &multiplier_one,
                   const std::vector<int> &multiplier_two,
                   std::vector<long long> &result,
                   std::size_t size)
{
    result.resize(size * size);
    std::fill(result.begin(), result.end(), 0);

    for (std::size_t i = 0; i < size; ++i)
    {
        for (std::size_t k = 0; k < size; ++k)
        {
            long long a_ik = multiplier_one[i * size + k];

            for (std::size_t j = 0; j < size; ++j)
            {
                result[i * size + j] += a_ik * multiplier_two[k * size + j];
            }
        }
    }
}

void WriteResult(const std::vector<long long> &result, std::string_view filename, std::size_t size)
{
    std::string full_path = "../../../labs/matrices/result/" + std::string(filename);
    std::ofstream file(full_path);

    if (!file)
    {
        throw std::logic_error("Файл не открылся");
    }

    for (std::size_t i = 0; i < size; ++i)
    {
        for (std::size_t j = 0; j < size; ++j)
        {
            std::print(file, "{:12}", result[i * size + j]);
        }
        std::println(file, "");
    }

    std::println("Матрица успешно записана в файл {}", filename);
}

int main()
{
    std::vector<int> matrix_one;
    std::vector<int> matrix_two;
    std::vector<long long> matrix_result;

    std::string m200_1 = "../../../labs/matrices/data/m200_1.txt";
    std::string m200_2 = "../../../labs/matrices/data/m200_2.txt";
    std::string m400_1 = "../../../labs/matrices/data/m400_1.txt";
    std::string m400_2 = "../../../labs/matrices/data/m400_2.txt";
    std::string m800_1 = "../../../labs/matrices/data/m800_1.txt";
    std::string m800_2 = "../../../labs/matrices/data/m800_2.txt";
    std::string m1200_1 = "../../../labs/matrices/data/m1200_1.txt";
    std::string m1200_2 = "../../../labs/matrices/data/m1200_2.txt";
    std::string m1600_1 = "../../../labs/matrices/data/m1600_1.txt";
    std::string m1600_2 = "../../../labs/matrices/data/m1600_2.txt";
    std::string m2000_1 = "../../../labs/matrices/data/m2000_1.txt";
    std::string m2000_2 = "../../../labs/matrices/data/m2000_2.txt";

    std::println("Выберите какую матрицу вы хотите проверить: 200*200, 400*400, 800*800, 1200*1200, 1600*1600, 2000*2000");

    int size_matrix = 0;

    while (size_matrix != 200 && size_matrix != 400 && size_matrix != 800 && size_matrix != 1200 && size_matrix != 1600 && size_matrix != 2000)
    {
        std::cin >> size_matrix;
        if (size_matrix != 200 && size_matrix != 400 && size_matrix != 800 && size_matrix != 1200 && size_matrix != 1600 && size_matrix != 2000)
        {
            std::println("Введите корректный размер матрицы из представленных");
        }
    }

    switch (size_matrix)
    {
    case 200:
    {
        VectorLoading(matrix_one, m200_1);
        VectorLoading(matrix_two, m200_2);
        break;
    }
    case 400:
    {
        VectorLoading(matrix_one, m400_1);
        VectorLoading(matrix_two, m400_2);
        break;
    }
    case 800:
    {
        VectorLoading(matrix_one, m800_1);
        VectorLoading(matrix_two, m800_2);
        break;
    }
    case 1200:
    {
        VectorLoading(matrix_one, m1200_1);
        VectorLoading(matrix_two, m1200_2);
        break;
    }
    case 1600:
    {
        VectorLoading(matrix_one, m1600_1);
        VectorLoading(matrix_two, m1600_2);
        break;
    }
    case 2000:
    {
        VectorLoading(matrix_one, m2000_1);
        VectorLoading(matrix_two, m2000_2);
        break;
    }
    }

    auto start_time = std::chrono::steady_clock::now();

    productMatrix(matrix_one, matrix_two, matrix_result, size_matrix);

    auto end_time = std::chrono::steady_clock::now();

    auto time = end_time - start_time;

    switch (size_matrix)
    {
    case 200:
    {
        std::string path = "result200.txt";
        WriteResult(matrix_result, path, size_matrix);
        break;
    }
    case 400:
    {
        std::string path = "result400.txt";
        WriteResult(matrix_result, path, size_matrix);
        break;
    }
    case 800:
    {
        std::string path = "result800.txt";
        WriteResult(matrix_result, path, size_matrix);
        break;
    }
    case 1200:
    {
        std::string path = "result1200.txt";
        WriteResult(matrix_result, path, size_matrix);
        break;
    }
    case 1600:
    {
        std::string path = "result1600.txt";
        WriteResult(matrix_result, path, size_matrix);
        break;
    }
    case 2000:
    {
        std::string path = "result2000.txt";
        WriteResult(matrix_result, path, size_matrix);
        break;
    }
    }

    std::println("Время выполнения умножения: {}", time);

    return 0;
}