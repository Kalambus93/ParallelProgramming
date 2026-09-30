print("По какой таблице будет сделан отчет?")

size = int(input("Введите размер матрицы: "))
while size != 200 and size != 400 and size != 800 and size != 1200 and size != 1600 and size != 2000:
    size = int(input("Введите размер матрицы: "))

path_for_result = f"../matrices/result/result{size}.txt"
path_for_record = f"../matrices/record/record{size}.txt"

print(f"Выполняется написание отчета для матрицы размерностью {size}")

with open(path_for_result, "r", encoding='utf-8') as file_r:
    line1 = file_r.readline()

print(line1)

with open(path_for_record, "a", encoding="utf-8") as file_a:
    file_a.write(line1)