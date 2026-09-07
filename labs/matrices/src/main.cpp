import std;

int main()
{
    int n = 0;
    std::string filename;

    std::println("Введите размер квадратной матрицы");
    std::cin >> n;

    std::println("Введите название файла");
    std::cin >> filename;

    std::vector<int> vector_for_using(n * n);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    for (std::size_t i = 0; i < vector_for_using.size(); ++i)
    {
        vector_for_using[i] = dis(gen);
    }

    std::string full_path = "../../../labs/matrices/data/" + filename;
    std::ofstream file(full_path, std::ios::app);

    if (!file)
    {
        throw std::logic_error("Файл не открылся");
    }

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            std::print(file, "{:4}", vector_for_using[i * n + j]);
        }
        std::println(file, "");
    }

    std::println("Матрица успешно записана в файл {}", filename);

    return 0;
}