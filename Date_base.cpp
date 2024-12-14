#include "Data_base.h"


#pragma once


// хеш-функция для строк (DJBX33A)
uint64_t hash_str(const std::string& str) {
    uint64_t hash = 5381;
    for (char c : str) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    if (hash%20==0){
        return 1;
    }
    return hash;
}

// хеш-функция для числа
uint64_t hash_int(uint64_t number) {
    number ^= (number >> 16);  // Шаг 1: XOR со сдвигом
    number *= 0x85ebca6b;      // Шаг 2: Умножение на "магическое" число
    number ^= (number >> 13);  // Шаг 3: Ещё один XOR-сдвиг
    number *= 0xc2b2ae35;      // Шаг 4: Умножение на второе "магическое" число
    number ^= (number >> 16);  // Шаг 5: Финальный XOR-сдвиг
    if (number%20==0){
        return 1;
    }
    return number;             // Возврат хеш-значения
}

// хеш-функция для даты рождения
uint64_t hash_date(int year, int month, int day) {
    return hash_int(year * 10000 + month * 100 + day);
}

Data_base::Data_base(const std::string& name) {
    main_file_name = name;
    std::string new_name = name;
    new_name = new_name.erase(new_name.rfind("."));
    shifts_file_name = new_name + "/shifts.bin";
    hash_id_file_name = new_name + "/hash_id.bin";
    hash_name_file_name = new_name + "/hash_name.bin";
    hash_date_file_name = new_name + "/hash_date.bin";
    hash_phone_file_name = new_name + "/hash_phone.bin";
    hash_mail_file_name = new_name + "/hash_mail.bin";
    total_lines = 20;
}

// Открытие файлов в бинарном режиме
bool Data_base::open_file() {
    main_file.open(main_file_name, std::ios::in | std::ios::out | std::ios::binary);  // Открытие файла в бинарном режиме
    shifts_file.open(shifts_file_name, std::ios::in | std::ios::out | std::ios::binary);
    hash_id_file.open(hash_id_file_name, std::ios::in | std::ios::out | std::ios::binary);
    hash_name_file.open(hash_name_file_name, std::ios::in | std::ios::out | std::ios::binary);
    hash_date_file.open(hash_date_file_name, std::ios::in | std::ios::out | std::ios::binary);
    hash_phone_file.open(hash_phone_file_name, std::ios::in | std::ios::out | std::ios::binary);
    hash_mail_file.open(hash_mail_file_name, std::ios::in | std::ios::out | std::ios::binary);
    return main_file.is_open();
}

void Data_base::clear_file() {
    main_file.close(); // Закрыть файл, если он открыт
    main_file.open(main_file_name, std::ios::out | std::ios::trunc | std::ios::binary); // Открыть для записи и очистить содержимое

    // Очищаем shifts_file
    shifts_file.close();  // Закрываем файл, если он открыт
    shifts_file.open(shifts_file_name, std::ios::out | std::ios::trunc | std::ios::binary);  // Открываем для записи и очищаем содержимое

    hash_id_file.close(); // Закрыть файл, если он открыт
    preprocess_file(hash_id_file_name, total_lines, 17);

    hash_name_file.close(); // Закрыть файл, если он открыт
    preprocess_file(hash_name_file_name, total_lines, 100 + 8 * 10 - 1);

    hash_date_file.close(); // Закрыть файл, если он открыт
    preprocess_file(hash_date_file_name, total_lines, 11 + 8 * 10 - 1);

    hash_phone_file.close(); // Закрыть файл, если он открыт
    preprocess_file(hash_phone_file_name, total_lines, 12 + 8 * 10 - 1);

    hash_mail_file.close(); // Закрыть файл, если он открыт
    preprocess_file(hash_mail_file_name, total_lines, 100 + 8 * 10 - 1);
}

void Data_base::close_file() {
    if (main_file.is_open()) {
        main_file.close();
    }
}

// Функция для разделения строки на части
std::vector<std::string> split(const std::string& str, char sep) {
    std::vector<std::string> result;
    std::string token;
    std::stringstream ss(str);

    // Разделение строки по разделителю
    while (std::getline(ss, token, sep)) {
        result.push_back(token);
    }

    return result;
}



bool isLeapYear(int year) {
    return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

int check_validity_line(const std::vector<std::string>& tokens){

    if (tokens.size()!=5){
        std::cerr<<"Введено недостаточно полей.";
        return 0;
    }

    //проверка id
    for (char c: tokens[0]){
        if(!std::isdigit(c)){
            std::cerr<<"Id должен быть целым числом.";
            return 0;
        }
    }

    //проверка name
    std::regex fio_regex("^[а-яА-ЯёЁ]+\\s+[а-яА-ЯёЁ]+(\\s+[а-яА-ЯёЁ]+)?$|^[a-zA-Z]+\\s+[a-zA-Z]+(\\s+[a-zA-Z]+)?$");
    if(!std::regex_match(tokens[1], fio_regex)){
        std::cerr<<"ФИО записано некорректно.";
        return 0;
    }

    //проверка date
    // Регулярное выражение для формата DD.MM.YYYY
    std::regex date_pattern(R"(\d{2}\.\d{2}\.\d{4}$)");

    // Проверяем, соответствует ли строка формату даты
    if (!std::regex_match(tokens[2], date_pattern)) {
        std::cerr<<"Дата должна быть записана в формате DD.MM.YYYY";
        return 0;
    }

    std::vector<std::string> date = split(tokens[2], '.');
    int day = std::stoi(date[0]);
    int month = std::stoi(date[1]);
    int year = std::stoi(date[2]);

    // Массив с количеством дней в месяце для обычного года
    std::vector<int> days_in_month = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Проверка на високосный год для февраля
    if (isLeapYear(year)) {
        days_in_month[1] = 29;  // Февраль имеет 29 дней в високосный год
    }

    // Проверка корректности месяца и дня
    if (month < 1 || month > 12) {
        std::cerr<<"Месяц должен быть от 1 до 12.";
        return 0;
    }

    if (day < 1 || day > days_in_month[month - 1]) {
        std::cerr<<"Неправильное кол-во дней в меясце.";
        return 0;
    }

    //проверка phone
    std::regex phone_regex(R"(^(\+7|8)\d{10}$)");
    if(!std::regex_match(tokens[3], phone_regex)){
        std::cerr<<"Такого номера не существует.";
        return 0;
    }

    std::regex mail_regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if(!std::regex_match(tokens[4], mail_regex)){
        std::cerr<<"Такой почты не существует";
        return 0;
    }

    return 1;
}

// Добавление записи
void Data_base::add_line(const std::string& line) {
    std::vector<std::string> tokens = split(line, ',');
    if (check_validity_line(tokens)){
        std::string str = "";
        str+=tokens[0];
        std::string result;
        std::istringstream iss(tokens[1]);
        std::string word;
        // Перебираем слова в строке
        while (iss >> word) {
            // Делаем первую букву заглавной, остальные - строчными
            if (!word.empty()) {
                word[0] = std::toupper(word[0]); // Первая буква - заглавная
                for (size_t i = 1; i < word.size(); ++i) {
                    word[i] = std::tolower(word[i]); // Остальные - строчные
                }
                result += word + " "; // Добавляем слово и пробел
            }
        }

        // Убираем последний пробел
        if (!result.empty()) {
            result.pop_back();
        }
        str=str + ","+result+","+tokens[2];
        if (tokens[3][0]=='+'){
            tokens[3] = "8"+tokens[3].substr(2);
        }
        str+=(","+tokens[3]+","+tokens[4]);
        std::vector<std::string> elements = split(str, ',');
        if (find_line_number_in_hash_id(elements[0]).first!=-1){
            std::cout<<"Невозможно добавить, так как такой id уже существует.";
            return;
        }
        const std::size_t line_size = 231; // Размер одной строки в байтах
        // Открываем файл в бинарном режиме для проверки его размера
        std::ifstream file(shifts_file_name, std::ios::binary | std::ios::ate);
        if (!file) {
            std::cerr << "Ошибка открытия файла!" << std::endl;
            return;
        }
        // Получаем размер файла в байтах
        std::streampos file_size = file.tellg();
        file.close();
        // Вычисляем номер следующей строки
        std::size_t line_count = static_cast<std::size_t>(file_size) / line_size +1;
        std::string line_shift = std::to_string(line_count);
        add_line_at_hash_id_bin(elements[0], line_shift);
        add_line_at_hash_name_bin(elements[1], line_shift);
        add_line_at_hash_date_bin(elements[2], line_shift);
        add_line_at_hash_phone_bin(elements[3], line_shift);
        add_line_at_hash_mail_bin(elements[4], line_shift);
        add_line_at_shifts_bin(elements[0], elements[1], elements[2], elements[3], elements[4]);

        // Открытие main_file для записи в бинарном формате
        main_file.close();  // Закрытие файла, если он был открыт
        main_file.open(main_file_name, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);  // Открытие файла в бинарном режиме для записи

        // Создаем буфер для записи строки в файл
        char buffer[line_size] = {0};
        std::memcpy(buffer, elements[0].c_str(), std::min(elements[0].size(), size_t(10)));  // Заполняем id
        std::memcpy(buffer + 10, elements[1].c_str(), std::min(elements[1].size(), size_t(100)));  // Заполняем name
        std::memcpy(buffer + 110, elements[2].c_str(), std::min(elements[2].size(), size_t(10)));  // Заполняем date
        std::memcpy(buffer + 120, elements[3].c_str(), std::min(elements[3].size(), size_t(11)));  // Заполняем phone
        std::memcpy(buffer + 131, elements[4].c_str(), std::min(elements[4].size(), size_t(100)));  // Заполняем email

        // Записываем строку в бинарный файл
        main_file.write(buffer, line_size);
        if (!main_file.good()) {
            std::cerr << "Ошибка записи в файл!" << std::endl;
        }
    }
}

void Data_base::read_base() {
    std::fstream file(shifts_file_name, std::ios::binary | std::ios::in | std::ios::out);
    char buffer[231] = {0};  // Буфер для чтения строки
    if (file.is_open()) {
        file.seekg(0);  // Перемещаем указатель на начало файла
        std::cout << "Содержимое файла:\n";
        while (file.read(buffer, 231)) {  // Читаем строки по 231 байт
            std::string id(buffer, 10);
            if (id[0]!='\0'){
                std::cout << "ID: " << id << ", Name: " << std::string(buffer + 10, 100)
                          << ", Date: " << std::string(buffer + 110, 10) << ", Phone: " << std::string(buffer + 120, 11)
                          << ", Email: " << std::string(buffer + 131, 100) << std::endl;
            }
        }
    }
}

// Функция для записи строки в конец бинарного файла
void Data_base::add_line_at_shifts_bin(const std::string& id, const std::string& fio, const std::string& date,
                                       const std::string& phone, const std::string& email) {
    std::ofstream file(shifts_file_name, std::ios::binary | std::ios::app);  // Открытие файла в режиме добавления в конец
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }

    // Размер строки (231 байт данных)
    size_t line_size = 231;

    // Буфер для строки
    char buffer[line_size] = {0};  // Инициализируем нулями

    // Заполняем id (10 байт)
    std::memcpy(buffer, id.c_str(), std::min(id.size(), size_t(10)));

    // Заполняем fio (100 байт)
    std::memcpy(buffer + 10, fio.c_str(), std::min(fio.size(), size_t(100)));

    // Заполняем date (10 байт)
    std::memcpy(buffer + 10 + 100, date.c_str(), std::min(date.size(), size_t(10)));

    // Заполняем phone (11 байт)
    std::memcpy(buffer + 10 + 100 + 10, phone.c_str(), std::min(phone.size(), size_t(11)));

    // Заполняем email (100 байт)
    std::memcpy(buffer + 10 + 100 + 10 + 11, email.c_str(), std::min(email.size(), size_t(100)));

    // Записываем строку в файл
    file.write(buffer, line_size);  // Запись в бинарный файл
    if (!file.good()) {
        std::cerr << "Ошибка записи в файл!" << std::endl;
    }

    file.close();
}

void Data_base::add_line_at_hash_id_bin(const std::string& id, const std::string& number_of_line_in_shifts_bin) {
    std::fstream file(hash_id_file_name, std::ios::binary | std::ios::in | std::ios::out);  // Открытие файла в режиме чтения/записи
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }

    std::cout << "add_line_at_hash_id_bin\n";

    // Размер строки (17 байт данных: 10 байт на id и 7 байт на номер строки)
    size_t line_size = 17;

    // Вычисляем номер строки в хеш-файле
    int line_number = hash_int(std::stoi(id)) % total_lines;

    // Начальное смещение для записи
    size_t offset = (line_number - 1) * line_size;
    size_t start_offset = offset;  // Запоминаем начальную строку для проверки

    // Буфер для чтения строки
    char buffer[line_size] = {0};  // Инициализируем буфер нулями

    // Поиск пустой строки или строки с таким же id
    while (true) {
        // Перемещаем указатель чтения на текущую строку
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "Ошибка перемещения указателя чтения!" << std::endl;
            file.close();
            return;
        }

        // Читаем текущую строку
        file.read(buffer, line_size);

        // Проверяем, является ли строка пустой (вся строка равна нулю)
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {  // Если найдены непустые символы, строка занята
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            // Если строка пустая, записываем id и номер строки
            std::memset(buffer, 0, line_size);  // Инициализируем нулями
            std::memcpy(buffer, id.c_str(), std::min(id.size(), size_t(10)));  // Заполняем id (10 байт)
            std::memcpy(buffer + 10, number_of_line_in_shifts_bin.c_str(),
                        std::min(number_of_line_in_shifts_bin.size(), size_t(7)));  // Заполняем номер строки (7 байт)

            // Записываем строку
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "Ошибка записи в файл!" << std::endl;
            }
            file.close();
            return;
        }

        // Если строка не пустая, переходим к следующей строке
        offset += line_size;

        // Проверка на выход за пределы файла
        if (offset >= total_lines * line_size) {  // Если достигнут конец файла
            offset = 0;  // Возвращаемся в начало файла
        }

        // Если мы вернулись к стартовой строке, значит в файле нет пустых строк
        if (offset == start_offset) {
            std::cerr << "Не удалось найти пустую строку для записи!" << std::endl;
            file.close();
            return;
        }
    }
}


void Data_base::add_line_at_hash_name_bin(const std::string& name, const std::string& number_of_line_in_shifts_bin) {
    std::fstream file(hash_name_file_name, std::ios::binary | std::ios::in | std::ios::out);  // Открытие файла в режиме чтения/записи
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }
    std::cout<<" add_line_at_hash_name_bin \n";
    int line_number = hash_str(name)%total_lines;
    // Размер строки
    size_t line_size = 100 + 8 * 10 - 1;

    // Начальное смещение
    size_t offset = (line_number - 1) * line_size;
    size_t start_offset = offset;  // Запоминаем начальную строку

    // Буфер для чтения строки
    char buffer[line_size] = {0};

    // Поиск пустой строки
    while (true) {
        // Перемещаем указатель чтения на текущую строку
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "Ошибка перемещения указателя чтения!(2)" << std::endl;
            file.close();
            return;
        }

        // Читаем текущую строку
        file.read(buffer, line_size);

        // Проверяем, является ли строка пустой
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {  // Если найдены непустые символы, строка занята
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            // Если строка пустая, записываем имя и номер строки
            std::memset(buffer, 0, line_size);  // Инициализируем нулями
            std::memcpy(buffer, name.c_str(), std::min(name.size(), size_t(100)));  // Заполняем имя
            std::memcpy(buffer + 100, number_of_line_in_shifts_bin.c_str(),
                        std::min(number_of_line_in_shifts_bin.size(), size_t(7)));  // Заполняем первый номер строки

            // Записываем строку
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "Ошибка записи в файл!" << std::endl;
            }
            file.close();
            return;
        }

        // Если строка не пустая, проверяем имя
        char existing_name[100] = {0};  // 100 байт для имени
        std::memcpy(existing_name, buffer, 100);

        if (name == existing_name) {
            // Если имя совпадает, добавляем номер строки
            char* numbers_start = buffer + 100;  // Начало списка номеров строк
            size_t numbers_length = line_size - 100;  // Длина блока с номерами строк

            // Поиск конца списка номеров строк
            char* numbers_end = numbers_start;
            while (*numbers_end != '\0') {
                ++numbers_end;
            }

            // Проверяем, есть ли место для нового номера строки
            if ((numbers_end - numbers_start) + 8 > numbers_length) {
                std::cerr << "Ошибка: превышено количество номеров строк для имени!" << std::endl;
                file.close();
                return;
            }

            // Добавляем запятую и новый номер строки
            std::strncat(numbers_start, ",", 1);
            std::strncat(numbers_start, number_of_line_in_shifts_bin.c_str(), 7);

            // Записываем обновленную строку
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "Ошибка записи в файл!" << std::endl;
            }
            file.close();
            return;
        }

        // Если имя не совпадает, переходим к следующей строке
        offset += line_size;

        // Проверка на выход за пределы файла
        if (offset >= total_lines * line_size) { // Если достигнут конец файла
            // Возвращаемся в начало файла
            offset = 0;
        }

        // Если мы вернулись к стартовой строке, значит в файле нет пустых строк
        if (offset == start_offset) {
            std::cerr << "Не удалось найти пустую строку для записи!" << std::endl;
            file.close();
            return;
        }
    }
}

void Data_base::add_line_at_hash_date_bin(const std::string& date, const std::string& number_of_line_in_shifts_bin) {
    std::fstream file(hash_date_file_name, std::ios::binary | std::ios::in | std::ios::out);  // Открытие файла в режиме чтения/записи
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }
    std::cout<<" add_line_at_hash_date_bin \n";
    std::vector<std::string> splited_date = split(date, '.');
    int line_number = hash_date(std::stoi(splited_date[2]), std::stoi(splited_date[1]), std::stoi(splited_date[0]))%total_lines;
    // Размер строки
    size_t line_size = 11 + 8 * 10 - 1;

    // Начальное смещение
    size_t offset = (line_number - 1) * line_size;
    size_t start_offset = offset;  // Запоминаем начальную строку

    // Буфер для чтения строки
    char buffer[line_size] = {0};

    // Поиск пустой строки
    while (true) {
        // Перемещаем указатель чтения на текущую строку
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "Ошибка перемещения указателя чтения!" << std::endl;
            file.close();
            return;
        }

        // Читаем текущую строку
        file.read(buffer, line_size);

        // Проверяем, является ли строка пустой
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {  // Если найдены непустые символы, строка занята
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            // Если строка пустая, записываем имя и номер строки
            std::memset(buffer, 0, line_size);  // Инициализируем нулями
            std::memcpy(buffer, date.c_str(), std::min(date.size(), size_t(11)));  // Заполняем имя
            std::memcpy(buffer + 11, number_of_line_in_shifts_bin.c_str(),
                        std::min(number_of_line_in_shifts_bin.size(), size_t(7)));  // Заполняем первый номер строки

            // Записываем строку
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "Ошибка записи в файл!" << std::endl;
            }
            file.close();
            return;
        }

        // Если строка не пустая, проверяем имя
        char existing_date[11] = {0};  // 10 байт для даты
        std::memcpy(existing_date, buffer, 11);

        if (date == existing_date) {
            // Если имя совпадает, добавляем номер строки
            char* numbers_start = buffer + 11;  // Начало списка номеров строк
            size_t numbers_length = line_size - 11;  // Длина блока с номерами строк

            // Поиск конца списка номеров строк
            char* numbers_end = numbers_start;
            while (*numbers_end != '\0') {
                ++numbers_end;
            }

            // Проверяем, есть ли место для нового номера строки
            if ((numbers_end - numbers_start) + 8 > numbers_length) {
                std::cerr << "Ошибка: превышено количество номеров строк для имени!" << std::endl;
                file.close();
                return;
            }

            // Добавляем запятую и новый номер строки
            std::strncat(numbers_start, ",", 1);
            std::strncat(numbers_start, number_of_line_in_shifts_bin.c_str(), 7);

            // Записываем обновленную строку
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "Ошибка записи в файл!" << std::endl;
            }
            file.close();
            return;
        }

        // Если имя не совпадает, переходим к следующей строке
        offset += line_size;

        // Проверка на выход за пределы файла
        if (offset >= total_lines * line_size) { // Если достигнут конец файла
            // Возвращаемся в начало файла
            offset = 0;
        }

        // Если мы вернулись к стартовой строке, значит в файле нет пустых строк
        if (offset == start_offset) {
            std::cerr << "Не удалось найти пустую строку для записи!" << std::endl;
            file.close();
            return;
        }
    }
}

void Data_base::add_line_at_hash_phone_bin(const std::string& phone, const std::string& number_of_line_in_shifts_bin) {
    std::fstream file(hash_phone_file_name, std::ios::binary | std::ios::in | std::ios::out);  // Открытие файла в режиме чтения/записи
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }
    std::cout<<" add_line_at_hash_phone_bin \n";
    int line_number = hash_int(std::stoll(phone))%total_lines;
    // Размер строки
    size_t line_size = 12 + 8 * 10 - 1;

    // Начальное смещение
    size_t offset = (line_number - 1) * line_size;
    size_t start_offset = offset;  // Запоминаем начальную строку

    // Буфер для чтения строки
    char buffer[line_size] = {0};

    // Поиск пустой строки
    while (true) {
        // Перемещаем указатель чтения на текущую строку
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "Ошибка перемещения указателя чтения!" << std::endl;
            file.close();
            return;
        }

        // Читаем текущую строку
        file.read(buffer, line_size);

        // Проверяем, является ли строка пустой
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {  // Если найдены непустые символы, строка занята
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            // Если строка пустая, записываем имя и номер строки
            std::memset(buffer, 0, line_size);  // Инициализируем нулями
            std::memcpy(buffer, phone.c_str(), std::min(phone.size(), size_t(12)));  // Заполняем имя
            std::memcpy(buffer + 12, number_of_line_in_shifts_bin.c_str(),
                        std::min(number_of_line_in_shifts_bin.size(), size_t(7)));  // Заполняем первый номер строки

            // Записываем строку
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "Ошибка записи в файл!" << std::endl;
            }
            file.close();
            return;
        }

        // Если строка не пустая, проверяем имя
        char existing_name[12] = {0};  // 100 байт для имени
        std::memcpy(existing_name, buffer, 12);

        if (phone == existing_name) {
            // Если имя совпадает, добавляем номер строки
            char* numbers_start = buffer + 12;  // Начало списка номеров строк
            size_t numbers_length = line_size - 12;  // Длина блока с номерами строк

            // Поиск конца списка номеров строк
            char* numbers_end = numbers_start;
            while (*numbers_end != '\0') {
                ++numbers_end;
            }

            // Проверяем, есть ли место для нового номера строки
            if ((numbers_end - numbers_start) + 8 > numbers_length) {
                std::cerr << "Ошибка: превышено количество номеров строк для имени!" << std::endl;
                file.close();
                return;
            }

            // Добавляем запятую и новый номер строки
            std::strncat(numbers_start, ",", 1);
            std::strncat(numbers_start, number_of_line_in_shifts_bin.c_str(), 7);

            // Записываем обновленную строку
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "Ошибка записи в файл!" << std::endl;
            }
            file.close();
            return;
        }

        // Если имя не совпадает, переходим к следующей строке
        offset += line_size;

        // Проверка на выход за пределы файла
        if (offset >= total_lines * line_size) { // Если достигнут конец файла
            // Возвращаемся в начало файла
            offset = 0;
        }

        // Если мы вернулись к стартовой строке, значит в файле нет пустых строк
        if (offset == start_offset) {
            std::cerr << "Не удалось найти пустую строку для записи!" << std::endl;
            file.close();
            return;
        }
    }
}

void Data_base::add_line_at_hash_mail_bin(const std::string& mail, const std::string& number_of_line_in_shifts_bin) {
    std::fstream file(hash_mail_file_name, std::ios::binary | std::ios::in | std::ios::out);  // Открытие файла в режиме чтения/записи
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }
    int line_number = hash_str(mail)%total_lines;
    // Размер строки
    size_t line_size = 100 + 8 * 10 - 1;
    std::cout<<" add_line_at_hash_mail_bin \n";
    // Начальное смещение
    size_t offset = (line_number - 1) * line_size;
    size_t start_offset = offset;  // Запоминаем начальную строку

    // Буфер для чтения строки
    char buffer[line_size] = {0};

    // Поиск пустой строки
    while (true) {
        // Перемещаем указатель чтения на текущую строку
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "Ошибка перемещения указателя чтения!" << std::endl;
            file.close();
            return;
        }

        // Читаем текущую строку
        file.read(buffer, line_size);

        // Проверяем, является ли строка пустой
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {  // Если найдены непустые символы, строка занята
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            // Если строка пустая, записываем имя и номер строки
            std::memset(buffer, 0, line_size);  // Инициализируем нулями
            std::memcpy(buffer, mail.c_str(), std::min(mail.size(), size_t(100)));  // Заполняем имя
            std::memcpy(buffer + 100, number_of_line_in_shifts_bin.c_str(),
                        std::min(number_of_line_in_shifts_bin.size(), size_t(7)));  // Заполняем первый номер строки

            // Записываем строку
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "Ошибка записи в файл!" << std::endl;
            }
            file.close();
            return;
        }

        // Если строка не пустая, проверяем имя
        char existing_mail[100] = {0};  // 100 байт для имени
        std::memcpy(existing_mail, buffer, 100);

        if (mail == existing_mail) {
            // Если имя совпадает, добавляем номер строки
            char* numbers_start = buffer + 100;  // Начало списка номеров строк
            size_t numbers_length = line_size - 100;  // Длина блока с номерами строк

            // Поиск конца списка номеров строк
            char* numbers_end = numbers_start;
            while (*numbers_end != '\0') {
                ++numbers_end;
            }

            // Проверяем, есть ли место для нового номера строки
            if ((numbers_end - numbers_start) + 8 > numbers_length) {
                std::cerr << "Ошибка: превышено количество номеров строк для имени!" << std::endl;
                file.close();
                return;
            }

            // Добавляем запятую и новый номер строки
            std::strncat(numbers_start, ",", 1);
            std::strncat(numbers_start, number_of_line_in_shifts_bin.c_str(), 7);

            // Записываем обновленную строку
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "Ошибка записи в файл!" << std::endl;
            }
            file.close();
            return;
        }

        // Если имя не совпадает, переходим к следующей строке
        offset += line_size;

        // Проверка на выход за пределы файла
        if (offset >= total_lines * line_size) { // Если достигнут конец файла
            // Возвращаемся в начало файла
            offset = 0;
        }

        // Если мы вернулись к стартовой строке, значит в файле нет пустых строк
        if (offset == start_offset) {
            std::cerr << "Не удалось найти пустую строку для записи!" << std::endl;
            file.close();
            return;
        }
    }
}

// Функция для поиска номеров строк по имени, начиная с определенной строки
std::pair<int, int> Data_base::find_line_number_in_hash_id(const std::string& id) {
    std::fstream file(hash_id_file_name, std::ios::binary | std::ios::in);  // Открытие файла для чтения
    int line_number=-1;  // Вектор для хранения номеров строк

    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return {line_number,-1};
    }
    int start_line = hash_int(std::stoi(id))%total_lines;
    // Размер строки
    size_t line_size = 10 + 7;

    // Буфер для чтения строки
    char buffer[line_size] = {0};

    // Начальное смещение
    size_t offset = (start_line - 1) * line_size;
    size_t start_offset = offset;  // Запоминаем начальную строку

    // Поиск номеров строк по имени
    while (true) {
        // Перемещаем указатель чтения на текущую строку
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "Ошибка перемещения указателя чтения! (3)" << std::endl;
            file.close();
            return {line_number,-1};
        }

        // Читаем текущую строку
        file.read(buffer, line_size);

        // Проверяем, является ли строка пустой
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            break;  // Мы достигли конца файла
        }

        // Проверяем имя
        char existing_id[10] = {0};  // 10 байт для id
        std::memcpy(existing_id, buffer, 10);

        if (id == existing_id) {
            // Если id совпадает, извлекаем номер строки (он находится в следующих 7 байтах после id)
            char number_str[7] = {0};  // Массив для хранения 7 байт + 1 для завершающего нуля
            std::memcpy(number_str, buffer + 10, 7);  // Копируем 7 байт с номера строки

            // Преобразуем в строку
            std::string number(number_str);
            line_number = std::stoi(number);
            return {line_number,offset};
        }

        // Переходим к следующей строке
        offset += line_size;

        // Проверка на выход за пределы файла
        if (offset >= total_lines * line_size) { // Если достигнут конец файла
            // Возвращаемся в начало файла
            offset = 0;
        }

        // Если мы вернулись к стартовой строке, значит в файле нет пустых строк
        if (offset == start_offset) {
            std::cerr << "Не удалось найти все строки для имени!" << std::endl;
            file.close();
            return {line_number,-1};
        }
    }

    file.close();
    return {line_number,-1};
}

// Функция для поиска номеров строк по имени, начиная с определенной строки
std::pair<std::vector<std::string>, int> Data_base::find_line_numbers_for_name(const std::string& name) {
    std::fstream file(hash_name_file_name, std::ios::binary | std::ios::in);  // Открытие файла для чтения
    std::vector<std::string> line_numbers;  // Вектор для хранения номеров строк

    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return {line_numbers, -1};
    }

    // Размер строки
    size_t line_size = 100 + 8 * 10 - 1;

    // Буфер для чтения строки
    char buffer[line_size] = {0};

    int start_line = hash_str(name)%total_lines;
    // Начальное смещение
    size_t offset = (start_line - 1) * line_size;
    size_t start_offset = offset;  // Запоминаем начальную строку

    // Поиск номеров строк по имени
    while (true) {
        // Перемещаем указатель чтения на текущую строку
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "Ошибка перемещения указателя чтения!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }

        // Читаем текущую строку
        file.read(buffer, line_size);

        // Проверяем, является ли строка пустой
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            break;  // Мы достигли конца файла
        }

        // Проверяем имя
        char existing_name[100] = {0};  // 100 байт для имени
        std::memcpy(existing_name, buffer, 100);

        if (name == existing_name) {
            // Если имя совпадает, извлекаем номера строк
            char* numbers_start = buffer + 100;  // Начало списка номеров строк
            std::string numbers_str(numbers_start);  // Преобразуем в строку для удобства обработки

            // Проверяем, пустая строка или нет
            if (!numbers_str.empty()) {
                // Разделяем строку с номерами по запятой
                std::stringstream ss(numbers_str);
                std::string number;
                while (std::getline(ss, number, ',')) {
                    line_numbers.push_back(number);  // Добавляем каждый номер в результат
                }

                // Если в строке не было запятых, то весь `numbers_str` уже содержит номер
                if (line_numbers.empty()) {
                    line_numbers.push_back(numbers_str);
                }
            }

            return {line_numbers, offset};
        }

        // Переходим к следующей строке
        offset += line_size;

        // Проверка на выход за пределы файла
        if (offset >= total_lines * line_size) { // Если достигнут конец файла
            // Возвращаемся в начало файла
            offset = 0;
        }

        // Если мы вернулись к стартовой строке, значит в файле нет пустых строк
        if (offset == start_offset) {
            std::cerr << "Не удалось найти все строки для имени!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }
    }
    file.close();
    return {line_numbers, -1};
}

// Функция для поиска номеров строк по имени, начиная с определенной строки
std::pair<std::vector<std::string>, int> Data_base::find_line_numbers_for_date(const std::string& date) {
    std::fstream file(hash_date_file_name, std::ios::binary | std::ios::in);  // Открытие файла для чтения
    std::vector<std::string> line_numbers;  // Вектор для хранения номеров строк

    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return {line_numbers, -1};
    }

    // Размер строки
    size_t line_size = 11 + 8 * 10 - 1;

    // Буфер для чтения строки
    char buffer[line_size] = {0};
    std::vector<std::string> splited_date = split(date, '.');
    int start_line = hash_date(std::stoi(splited_date[2]), std::stoi(splited_date[1]), std::stoi(splited_date[0]))%total_lines;

    // Начальное смещение
    size_t offset = (start_line - 1) * line_size;
    size_t start_offset = offset;  // Запоминаем начальную строку

    // Поиск номеров строк по имени
    while (true) {
        // Перемещаем указатель чтения на текущую строку
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "Ошибка перемещения указателя чтения!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }

        // Читаем текущую строку
        file.read(buffer, line_size);

        // Проверяем, является ли строка пустой
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            break;  // Мы достигли конца файла
        }

        // Проверяем имя
        char existing_name[11] = {0};  // 100 байт для имени
        std::memcpy(existing_name, buffer, 11);

        if (date == existing_name) {
            // Если имя совпадает, извлекаем номера строк
            char* numbers_start = buffer + 11;  // Начало списка номеров строк
            std::string numbers_str(numbers_start);  // Преобразуем в строку для удобства обработки

            // Проверяем, пустая строка или нет
            if (!numbers_str.empty()) {
                // Разделяем строку с номерами по запятой
                std::stringstream ss(numbers_str);
                std::string number;
                while (std::getline(ss, number, ',')) {
                    line_numbers.push_back(number);  // Добавляем каждый номер в результат
                }

                // Если в строке не было запятых, то весь `numbers_str` уже содержит номер
                if (line_numbers.empty()) {
                    line_numbers.push_back(numbers_str);
                }
            }

            return {line_numbers, offset};
        }

        // Переходим к следующей строке
        offset += line_size;

        // Проверка на выход за пределы файла
        if (offset >= total_lines * line_size) { // Если достигнут конец файла
            // Возвращаемся в начало файла
            offset = 0;
        }

        // Если мы вернулись к стартовой строке, значит в файле нет пустых строк
        if (offset == start_offset) {
            std::cerr << "Не удалось найти все строки для имени!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }
    }
    file.close();
    return {line_numbers, -1};
}


// Функция для поиска номеров строк по имени, начиная с определенной строки
std::pair<std::vector<std::string>, int> Data_base::find_line_numbers_for_phone(const std::string& phone) {
    std::fstream file(hash_phone_file_name, std::ios::binary | std::ios::in);  // Открытие файла для чтения
    std::vector<std::string> line_numbers;  // Вектор для хранения номеров строк

    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return {line_numbers, -1};
    }

    // Размер строки
    size_t line_size = 12 + 8 * 10 - 1;

    // Буфер для чтения строки
    char buffer[line_size] = {0};
    int start_line = hash_int(std::stoll(phone))%total_lines;

    // Начальное смещение
    size_t offset = (start_line - 1) * line_size;
    size_t start_offset = offset;  // Запоминаем начальную строку

    // Поиск номеров строк по имени
    while (true) {
        // Перемещаем указатель чтения на текущую строку
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "Ошибка перемещения указателя чтения!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }

        // Читаем текущую строку
        file.read(buffer, line_size);

        // Проверяем, является ли строка пустой
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            std::cout<<"smth wrong with hash";
            break;  // Мы достигли конца файла
        }

        // Проверяем имя
        char existing_name[12] = {0};  // 100 байт для имени
        std::memcpy(existing_name, buffer, 12);

        if (phone == existing_name) {
            // Если имя совпадает, извлекаем номера строк
            char* numbers_start = buffer + 12;  // Начало списка номеров строк
            std::string numbers_str(numbers_start);  // Преобразуем в строку для удобства обработки

            // Проверяем, пустая строка или нет
            if (!numbers_str.empty()) {
                // Разделяем строку с номерами по запятой
                std::stringstream ss(numbers_str);
                std::string number;
                while (std::getline(ss, number, ',')) {
                    line_numbers.push_back(number);  // Добавляем каждый номер в результат
                }

                // Если в строке не было запятых, то весь `numbers_str` уже содержит номер
                if (line_numbers.empty()) {
                    line_numbers.push_back(numbers_str);
                }
            }

            return {line_numbers, offset};
        }

        // Переходим к следующей строке
        offset += line_size;

        // Проверка на выход за пределы файла
        if (offset >= total_lines * line_size) { // Если достигнут конец файла
            // Возвращаемся в начало файла
            offset = 0;
        }

        // Если мы вернулись к стартовой строке, значит в файле нет пустых строк
        if (offset == start_offset) {
            std::cerr << "Не удалось найти все строки для имени!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }
    }
    file.close();
    return {line_numbers, -1};
}

// Функция для поиска номеров строк по имени, начиная с определенной строки
std::pair<std::vector<std::string>, int> Data_base::find_line_numbers_for_mail(const std::string& mail) {
    std::fstream file(hash_mail_file_name, std::ios::binary | std::ios::in);  // Открытие файла для чтения
    std::vector<std::string> line_numbers;  // Вектор для хранения номеров строк

    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return {line_numbers, -1};
    }

    // Размер строки
    size_t line_size = 100 + 8 * 10 - 1;

    // Буфер для чтения строки
    char buffer[line_size] = {0};
    int start_line = hash_str(mail)%total_lines;

    // Начальное смещение
    size_t offset = (start_line - 1) * line_size;
    size_t start_offset = offset;  // Запоминаем начальную строку

    // Поиск номеров строк по имени
    while (true) {
        // Перемещаем указатель чтения на текущую строку
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "Ошибка перемещения указателя чтения!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }

        // Читаем текущую строку
        file.read(buffer, line_size);

        // Проверяем, является ли строка пустой
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            break;  // Мы достигли конца файла
        }

        // Проверяем имя
        char existing_name[100] = {0};  // 100 байт для имени
        std::memcpy(existing_name, buffer, 100);

        if (mail == existing_name) {
            // Если имя совпадает, извлекаем номера строк
            char* numbers_start = buffer + 100;  // Начало списка номеров строк
            std::string numbers_str(numbers_start);  // Преобразуем в строку для удобства обработки

            // Проверяем, пустая строка или нет
            if (!numbers_str.empty()) {
                // Разделяем строку с номерами по запятой
                std::stringstream ss(numbers_str);
                std::string number;
                while (std::getline(ss, number, ',')) {
                    line_numbers.push_back(number);  // Добавляем каждый номер в результат
                }

                // Если в строке не было запятых, то весь `numbers_str` уже содержит номер
                if (line_numbers.empty()) {
                    line_numbers.push_back(numbers_str);
                }
            }

            return {line_numbers, offset};
        }

        // Переходим к следующей строке
        offset += line_size;

        // Проверка на выход за пределы файла
        if (offset >= total_lines * line_size) { // Если достигнут конец файла
            // Возвращаемся в начало файла
            offset = 0;
        }

        // Если мы вернулись к стартовой строке, значит в файле нет пустых строк
        if (offset == start_offset) {
            std::cerr << "Не удалось найти все строки для имени!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }
    }
    file.close();
    return {line_numbers, -1};
}

// Функция для чтения строки из бинарного файла
std::vector<std::string> Data_base::read_fixed_line(size_t line_read) {
    std::ifstream file(shifts_file_name, std::ios::binary);  // Открытие файла в бинарном режиме
    std::vector<std::string> elements;
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return elements;
    }
    size_t line_size = 231;
    char buffer[line_size] = {0};  // Буфер для строки (231 байт данных + 1 байт для '\0')
    size_t offset = (line_read - 1) * line_size;
    // Перемещаем указатель чтения на текущую строку
    file.seekg(offset);
    if (!file.good()) {
        std::cerr << "Ошибка перемещения указателя чтения!" << std::endl;
        file.close();
    }
    // Читаем текущую строку
    file.read(buffer, line_size);
    std::string id,name,date,phone, mail;
    // Преобразуем байты в строку и выводим
    for (size_t i = 0; i < 10; ++i) {
        if (buffer[i] != '\0') {  // Пропускаем нулевые байты
            id+=buffer[i];
        }
        else{
            break;
        }
    }
    for (size_t i = 0+10; i < 10+100; ++i) {
        if (buffer[i] != '\0') {  // Пропускаем нулевые байты
            name+=buffer[i];
        }
        else{
            break;
        }
    }
    for (size_t i = 0+10+100; i < 10+100+10; ++i) {
        if (buffer[i] != '\0') {  // Пропускаем нулевые байты
            date+=buffer[i];
        }
        else{
            break;
        }
    }
    for (size_t i = 0+10+100+10; i < 10+100+10+11; ++i) {
        if (buffer[i] != '\0') {  // Пропускаем нулевые байты
            phone+=buffer[i];
        }
        else{
            break;
        }
    }
    for (size_t i = 0+10+100+10+11; i < 10+100+10+11+100; ++i) {
        if (buffer[i] != '\0') {  // Пропускаем нулевые байты
            mail+=buffer[i];
        }
        else{
            break;
        }
    }
    std::cout<<id<<" "<<name<<" "<<date<<" "<<phone<<" "<<mail<<"\n";
    elements.push_back(id);
    elements.push_back(name);
    elements.push_back(date);
    elements.push_back(phone);
    elements.push_back(mail);
    return elements;
    file.close();
}

// Функция для удаления строки с указанным id
void Data_base::delete_line_by_id(const std::string& id) {
    std::fstream file(hash_id_file_name, std::ios::binary | std::ios::in | std::ios::out);  // Открытие файла в режиме чтения/записи
    std::pair<int, int> info = find_line_number_in_hash_id(id);
    if (info.first==-1){
        std::cout<<"Нет такого id.";
        return;
    }
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }

    size_t line_size = 17;
    // Буфер для чтения строки
    char buffer[line_size] = {0};
    // Начальное смещение
    size_t offset = info.second;
    size_t start_offset = offset;  // Запоминаем начальную строку
    // Если id совпадает, заменяем строку на пустую
    std::memset(buffer, 0, line_size);  // Заменяем строку на пустую

    // Записываем строку обратно в файл
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "Ошибка записи в файл!" << std::endl;
    }
    file.close();
    std::cout << "Строка с id " << id << " была удалена." << std::endl;
    std::vector<std::string> elements = read_fixed_line(info.first);
    delete_line_from_shifts(info.first);
    delete_line_from_hash_name(info.first, elements[1]);
    delete_line_from_hash_date(info.first, elements[2]);
    delete_line_from_hash_phone(info.first, elements[3]);
    delete_line_from_hash_mail(info.first, elements[4]);
}

void Data_base::delete_line_from_shifts(int number_line){
    std::fstream file(shifts_file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }
    size_t line_size = 231;
    // Буфер для чтения строки
    char buffer[line_size] = {0};
    // Начальное смещение
    size_t offset = (number_line - 1) * line_size;

    // Если id совпадает, заменяем строку на пустую
    std::memset(buffer, 0, line_size);  // Заменяем строку на пустую

    // Записываем строку обратно в файл
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "Ошибка записи в файл!" << std::endl;
    }

    file.close();
}

void Data_base::delete_line_from_hash_name(int number_line,const std::string& name){
    std::pair<std::vector<std::string>, int> info = find_line_numbers_for_name(name);
    info.first.erase(std::remove(info.first.begin(), info.first.end(), std::to_string(number_line)), info.first.end());
    std::fstream file(hash_name_file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }
    size_t line_size = 100 + 8 * 10 - 1;
// Буфер для чтения строки
    char buffer[line_size] = {0};
// Начальное смещение
    size_t offset = info.second;

    if (info.first.size() == 0) {
        // Если список строк пуст, заменяем всю строку на пустую
        std::memset(buffer, 0, line_size);  // Заменяем строку на пустую
    } else {
        // Если есть номера строк, собираем их в строку с запятыми
        std::string numbers_str = "";
        for (size_t i = 0; i < info.first.size(); ++i) {
            numbers_str += info.first[i];
            if (i != info.first.size() - 1) {
                numbers_str += ",";  // Добавляем запятую между номерами
            }
        }

        // Заполняем буфер
        std::memset(buffer, 0, line_size);  // Инициализируем буфер нулями
        std::memcpy(buffer, name.c_str(), std::min(name.size(), size_t(100)));  // Копируем имя в первые 100 байт
        std::memcpy(buffer + 100, numbers_str.c_str(), std::min(numbers_str.size(), line_size - 100));  // Копируем номера строк
    }

// Записываем строку обратно в файл
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "Ошибка записи в файл!" << std::endl;
    }

    file.close();

}

void Data_base::delete_line_from_hash_date(int number_line,const std::string& date){
    std::pair<std::vector<std::string>, int> info = find_line_numbers_for_date(date);
    info.first.erase(std::remove(info.first.begin(), info.first.end(), std::to_string(number_line)), info.first.end());
    std::fstream file(hash_date_file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }
    size_t line_size = 11 + 8 * 10 - 1;
// Буфер для чтения строки
    char buffer[line_size] = {0};
// Начальное смещение
    size_t offset = info.second;

    if (info.first.size() == 0) {
        // Если список строк пуст, заменяем всю строку на пустую
        std::memset(buffer, 0, line_size);  // Заменяем строку на пустую
    } else {
        // Если есть номера строк, собираем их в строку с запятыми
        std::string numbers_str = "";
        for (size_t i = 0; i < info.first.size(); ++i) {
            numbers_str += info.first[i];
            if (i != info.first.size() - 1) {
                numbers_str += ",";  // Добавляем запятую между номерами
            }
        }

        // Заполняем буфер
        std::memset(buffer, 0, line_size);  // Инициализируем буфер нулями
        std::memcpy(buffer, date.c_str(), std::min(date.size(), size_t(11)));  // Копируем имя в первые 100 байт
        std::memcpy(buffer + 11, numbers_str.c_str(), std::min(numbers_str.size(), line_size - 11));  // Копируем номера строк
    }

// Записываем строку обратно в файл
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "Ошибка записи в файл!" << std::endl;
    }

    file.close();

}

void Data_base::delete_line_from_hash_phone(int number_line,const std::string& phone){
    std::pair<std::vector<std::string>, int> info = find_line_numbers_for_phone(phone);
    info.first.erase(std::remove(info.first.begin(), info.first.end(), std::to_string(number_line)), info.first.end());
    std::fstream file(hash_phone_file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }
    size_t line_size = 12 + 8 * 10 - 1;
// Буфер для чтения строки
    char buffer[line_size] = {0};
// Начальное смещение
    size_t offset = info.second;

    if (info.first.size() == 0) {
        // Если список строк пуст, заменяем всю строку на пустую
        std::memset(buffer, 0, line_size);  // Заменяем строку на пустую
    } else {
        // Если есть номера строк, собираем их в строку с запятыми
        std::string numbers_str = "";
        for (size_t i = 0; i < info.first.size(); ++i) {
            numbers_str += info.first[i];
            if (i != info.first.size() - 1) {
                numbers_str += ",";  // Добавляем запятую между номерами
            }
        }

        // Заполняем буфер
        std::memset(buffer, 0, line_size);  // Инициализируем буфер нулями
        std::memcpy(buffer, phone.c_str(), std::min(phone.size(), size_t(12)));  // Копируем имя в первые 100 байт
        std::memcpy(buffer + 12, numbers_str.c_str(), std::min(numbers_str.size(), line_size - 12));  // Копируем номера строк
    }

// Записываем строку обратно в файл
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "Ошибка записи в файл!" << std::endl;
    }

    file.close();

}

void Data_base::delete_line_from_hash_mail(int number_line,const std::string& mail){
    std::pair<std::vector<std::string>, int> info = find_line_numbers_for_mail(mail);
    info.first.erase(std::remove(info.first.begin(), info.first.end(), std::to_string(number_line)), info.first.end());
    std::fstream file(hash_mail_file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }
    size_t line_size = 100 + 8 * 10 - 1;
// Буфер для чтения строки
    char buffer[line_size] = {0};
// Начальное смещение
    size_t offset = info.second;

    if (info.first.size() == 0) {
        // Если список строк пуст, заменяем всю строку на пустую
        std::memset(buffer, 0, line_size);  // Заменяем строку на пустую
    } else {
        // Если есть номера строк, собираем их в строку с запятыми
        std::string numbers_str = "";
        for (size_t i = 0; i < info.first.size(); ++i) {
            numbers_str += info.first[i];
            if (i != info.first.size() - 1) {
                numbers_str += ",";  // Добавляем запятую между номерами
            }
        }

        // Заполняем буфер
        std::memset(buffer, 0, line_size);  // Инициализируем буфер нулями
        std::memcpy(buffer, mail.c_str(), std::min(mail.size(), size_t(100)));  // Копируем имя в первые 100 байт
        std::memcpy(buffer + 100, numbers_str.c_str(), std::min(numbers_str.size(), line_size - 100));  // Копируем номера строк
    }

// Записываем строку обратно в файл
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "Ошибка записи в файл!" << std::endl;
    }

    file.close();
}

void Data_base::delete_line_by_name(const std::string &name) {
    std::pair<std::vector<std::string>, int> info = find_line_numbers_for_name(name);
    for (std::string i: info.first){
        delete_line_by_id(read_fixed_line(std::stoi(i))[0]);
    }
}

void Data_base::delete_line_by_date(const std::string &date) {
    std::pair<std::vector<std::string>, int> info = find_line_numbers_for_date(date);
    for (std::string i: info.first){
        delete_line_by_id(read_fixed_line(std::stoi(i))[0]);
    }
}

void Data_base::delete_line_by_phone(const std::string &phone) {
    std::pair<std::vector<std::string>, int> info = find_line_numbers_for_phone(phone);
    for (std::string i: info.first){
        delete_line_by_id(read_fixed_line(std::stoi(i))[0]);
    }
}

void Data_base::delete_line_by_mail(const std::string &mail) {
    std::pair<std::vector<std::string>, int> info = find_line_numbers_for_mail(mail);
    for (std::string i: info.first){
        delete_line_by_id(read_fixed_line(std::stoi(i))[0]);
    }
}

// Функция для замены строки  бинарного файла
void Data_base::change_line_at_shifts_bin(const std::string& id, const std::string& fio, const std::string& date,
                                       const std::string& phone, const std::string& email, size_t number_line) {
    std::fstream file(shifts_file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }

    // Размер строки (231 байт данных)
    size_t line_size = 231;

    // Буфер для строки
    char buffer[line_size] = {0};  // Инициализируем нулями

    // Заполняем id (10 байт)
    std::memcpy(buffer, id.c_str(), std::min(id.size(), size_t(10)));

    // Заполняем fio (100 байт)
    std::memcpy(buffer + 10, fio.c_str(), std::min(fio.size(), size_t(100)));

    // Заполняем date (10 байт)
    std::memcpy(buffer + 10 + 100, date.c_str(), std::min(date.size(), size_t(10)));

    // Заполняем phone (11 байт)
    std::memcpy(buffer + 10 + 100 + 10, phone.c_str(), std::min(phone.size(), size_t(11)));

    // Заполняем email (100 байт)
    std::memcpy(buffer + 10 + 100 + 10 + 11, email.c_str(), std::min(email.size(), size_t(100)));
    size_t offset = (number_line-1)*line_size;
    file.seekp(offset);
    // Записываем строку в файл
    file.write(buffer, line_size);  // Запись в бинарный файл
    if (!file.good()) {
        std::cerr << "Ошибка записи в файл!" << std::endl;
    }

    file.close();
}

void Data_base::edit_id(const std::string& old_id,const std::string& new_id){
    for (char c: new_id){
        if(!std::isdigit(c)){
            std::cerr<<"Id должен быть целым числом.";
            return;
        }
    }
    if (find_line_number_in_hash_id(new_id).first!=-1){
        std::cout<<"Такой id уже есть в таблице.";
        return;
    }
    std::pair<int, int> info = find_line_number_in_hash_id(old_id);
    std::vector<std::string> elements = read_fixed_line(info.first);
    change_line_at_shifts_bin(new_id, elements[1], elements[2], elements[3], elements[4],info.first);

    std::fstream file(hash_id_file_name, std::ios::binary | std::ios::in | std::ios::out);  // Открытие файла в режиме чтения/записи
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
    }
    size_t line_size = 17;
    // Буфер для чтения строки
    char buffer[line_size] = {0};
    // Начальное смещение
    size_t offset = info.second;
    std::memset(buffer, 0, line_size);  // Заменяем строку на пустую
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "Ошибка записи в файл!" << std::endl;
    }
    file.close();
    add_line_at_hash_id_bin(new_id, std::to_string(info.first));
}

void Data_base::edit_name(const std::string& id,const std::string& new_name){
    std::regex fio_regex("^[а-яА-ЯёЁ]+\\s+[а-яА-ЯёЁ]+(\\s+[а-яА-ЯёЁ]+)?$|^[a-zA-Z]+\\s+[a-zA-Z]+(\\s+[a-zA-Z]+)?$");
    if(!std::regex_match(new_name, fio_regex)){
        std::cerr<<"ФИО записано некорректно.";
        return;
    }
    std::pair<int,int> info_about_hash_id = find_line_number_in_hash_id(id);
    if (info_about_hash_id.first==-1){
        std::cout<<"Такого id нет в таблице.";
        return;
    }
    std::vector<std::string> elements = read_fixed_line(info_about_hash_id.first);

    change_line_at_shifts_bin(elements[0], new_name, elements[2], elements[3], elements[4],info_about_hash_id.first);
    delete_line_from_hash_name(info_about_hash_id.first, elements[1]);
    add_line_at_hash_name_bin(new_name, std::to_string(info_about_hash_id.first));
}

void Data_base::edit_date(const std::string &id, const std::string &new_date) {
    //проверка date
    // Регулярное выражение для формата DD.MM.YYYY
    std::regex date_pattern(R"(\d{2}\.\d{2}\.\d{4}$)");

    // Проверяем, соответствует ли строка формату даты
    if (!std::regex_match(new_date, date_pattern)) {
        std::cerr<<"Дата должна быть записана в формате DD.MM.YYYY";
        return;
    }

    std::vector<std::string> date = split(new_date, '.');
    int day = std::stoi(date[0]);
    int month = std::stoi(date[1]);
    int year = std::stoi(date[2]);

    // Массив с количеством дней в месяце для обычного года
    std::vector<int> days_in_month = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Проверка на високосный год для февраля
    if (isLeapYear(year)) {
        days_in_month[1] = 29;  // Февраль имеет 29 дней в високосный год
    }

    // Проверка корректности месяца и дня
    if (month < 1 || month > 12) {
        std::cerr<<"Месяц должен быть от 1 до 12.";
        return;
    }

    if (day < 1 || day > days_in_month[month - 1]) {
        std::cerr<<"Неправильное кол-во дней в меясце.";
        return;
    }
    std::pair<int,int> info_about_hash_id = find_line_number_in_hash_id(id);
    if (info_about_hash_id.first==-1){
        std::cout<<"Такого id нет в таблице.";
        return;
    }
    std::vector<std::string> elements = read_fixed_line(info_about_hash_id.first);

    change_line_at_shifts_bin(elements[0], elements[1], new_date, elements[3], elements[4],info_about_hash_id.first);
    delete_line_from_hash_date(info_about_hash_id.first, elements[2]);
    add_line_at_hash_date_bin(new_date, std::to_string(info_about_hash_id.first));
}

void Data_base::edit_phone(const std::string &id, const std::string &new_phone) {
    std::regex phone_regex(R"(^(\+7|8)\d{10}$)");
    if(!std::regex_match(new_phone, phone_regex)){
        std::cerr<<"Такого номера не существует.";
        return;
    }
    std::pair<int,int> info_about_hash_id = find_line_number_in_hash_id(id);
    if (info_about_hash_id.first==-1){
        std::cout<<"Такого id нет в таблице.";
        return;
    }
    std::vector<std::string> elements = read_fixed_line(info_about_hash_id.first);

    change_line_at_shifts_bin(elements[0], elements[1], elements[2], new_phone, elements[4],info_about_hash_id.first);
    delete_line_from_hash_phone(info_about_hash_id.first, elements[3]);
    add_line_at_hash_phone_bin(new_phone, std::to_string(info_about_hash_id.first));
}

void Data_base::edit_mail(const std::string &id, const std::string &new_mail) {
    std::regex mail_regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if(!std::regex_match(new_mail, mail_regex)){
        std::cerr<<"Такой почты не существует";
        return;
    }
    std::pair<int,int> info_about_hash_id = find_line_number_in_hash_id(id);
    if (info_about_hash_id.first==-1){
        std::cout<<"Такого id нет в таблице.";
        return;
    }
    std::vector<std::string> elements = read_fixed_line(info_about_hash_id.first);

    change_line_at_shifts_bin(elements[0], elements[1], elements[2], elements[3], new_mail,info_about_hash_id.first);
    delete_line_from_hash_mail(info_about_hash_id.first, elements[4]);
    add_line_at_hash_mail_bin(new_mail, std::to_string(info_about_hash_id.first));
}


