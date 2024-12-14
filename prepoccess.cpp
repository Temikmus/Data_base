#include "prepoccess.h"
// Функция для предобработки файла с пустыми строками
void preprocess_file(const std::string& file_path, size_t num_lines, size_t line_size) {
    std::ofstream file(file_path, std::ios::binary);  // Открытие файла для записи в бинарном режиме
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }

    // Создаём пустую строку (все байты нулевые)
    char empty_line[line_size] = {0};

    // Записываем пустые строки в файл
    for (size_t i = 0; i < num_lines; ++i) {
        file.write(empty_line, line_size);
    }

    if (!file.good()) {
        std::cerr << "Ошибка записи в файл (prepocessor)!" << std::endl;
    }

    file.close();
}