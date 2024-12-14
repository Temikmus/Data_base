#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <filesystem>
#include <vector>
#include <thread>  // Для задержки
#include <chrono>  // Для времени
#include <sstream>  // Для использования stringstream
#include <regex>  // Для регулярных выражений
#include "Data_base.h"

#pragma once

namespace fs = std::filesystem;

// Функция для отображения всех файлов в директории
void show_files_in_directory(const std::string& path) {
    std::cout << "Доступные файлы в директории \"" << path << "\":" << std::endl;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry)) {
            std::cout << entry.path().filename() << std::endl;
        }
    }
}

// Функция для выбора файла
std::string choose_file() {
    std::string path;
    std::cout << "Введите путь к директории, где находятся файлы базы данных: ";
    std::cin >> path;
    show_files_in_directory(path);

    std::string chosen_file;
    std::cout << "Введите имя файла для работы: ";
    std::cin >> chosen_file;
    return path + "/" + chosen_file;
}

// Функция для создания базы данных
void create_base() {
    std::string directory, file_name;

    // Вводим путь к директории и имя файла
    std::cout << "Введите путь к директории: ";
    std::getline(std::cin, directory);

    std::cout << "Введите имя файла: ";
    std::getline(std::cin, file_name);
    if (file_name.find(".bin", file_name.size()-4) == std::string::npos)
    {
        file_name += ".bin";
    }

    // Создаем полный путь к файлу базы данных
    std::string file_path = directory + "/" + file_name;

    // Проверяем, существует ли уже файл с таким именем
    if (fs::exists(file_path)) {
        std::cerr << "Ошибка: файл с таким именем уже существует!" << std::endl;
        // Добавляем небольшую задержку
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        return;
    }

    // Создание бинарного файла базы данных
    std::ofstream db_file(file_path, std::ios::binary);
    if (db_file.is_open()) {
        std::cout << "Файл базы данных успешно создан: " << file_path << std::endl;
        db_file.close();  // Закрываем файл
    } else {
        std::cerr << "Не удалось создать файл базы данных!" << std::endl;
        return;
    }

    // Создание папки с именем файла (без расширения) в директории
    std::string folder_name = file_name.substr(0, file_name.find_last_of('.')); // Получаем имя без расширения
    fs::path folder_path = fs::path(directory) / folder_name;

    if (!fs::exists(folder_path)) {
        fs::create_directory(folder_path);
        std::cout << "Папка " << folder_path << " успешно создана." << std::endl;
    } else {
        std::cout << "Папка уже существует: " << folder_path << std::endl;
    }

    // Список файлов, которые должны быть созданы в новой папке
    std::string file_names[] = {
            "hash_date.bin", "hash_id.bin", "hash_name.bin",
            "hash_phone.bin", "hash_mail.bin", "shifts.bin"
    };

    // Создаем каждый из этих файлов в новой папке
    for (const std::string& file : file_names) {
        std::ofstream new_file(folder_path / file, std::ios::binary);
        if (new_file.is_open()) {
            std::cout << "Файл " << file << " успешно создан в папке " << folder_path << std::endl;
            new_file.close();  // Закрываем файл
        } else {
            std::cerr << "Не удалось создать файл: " << file << std::endl;
        }
    }
    // Предобработка файлов
    preprocess_file(directory + "/" + folder_name + "/hash_id.bin", 20, 17);
    preprocess_file(directory + "/" + folder_name + "/hash_name.bin", 20, 100 + 8 * 10 - 1);
    preprocess_file(directory + "/" + folder_name + "/hash_date.bin", 20, 11 + 8 * 10 - 1);
    preprocess_file(directory + "/" + folder_name + "/hash_phone.bin", 20, 12 + 8 * 10 - 1);
    preprocess_file(directory + "/" + folder_name + "/hash_mail.bin", 20, 100 + 8 * 10 - 1);
}

int main() {


    // Создаём объект базы данных с выбранным файлом
    //create_base();
    std::string base_file = choose_file();
    Data_base base(base_file);
    if (base.open_file()) {
        base.add_line("123,Artemy Muslin Mich,24.07.2005,+79302854508,artemmuslin@gmail.com");
        //base.delete_line_by_name("Alexey Smirnov");
        //base.delete_line_by_mail("artemmuslin@gmail.com");
        base.edit_name("131", "Chuspan durak");
        base.read_base();
    } else {
        std::cout << "Не удалось открыть файл " << base_file << std::endl;
    }
    return 0;
}


//base.add_line("123,Artemy Muslin Mich,24.07.2005,+79302854508,artemmuslin@gmail.com");
//base.add_line("124,John Doe,15.06.1990,+79998887766,johndoe@mail.com");
//base.add_line("125,Jane Smith,10.02.1985,+79251234567,janesmith@domain.com");
//base.add_line("126,Mikhail Ivanov,01.12.1992,+79161234567,mikhail.ivanov@mail.ru");
//base.add_line("127,Olga Kuznetsova,19.09.1987,+79031234567,olgakuznetsova@yandex.ru");
//base.add_line("128,Sergey Petrov,23.03.1995,+79301234567,sergey.petrov@service.com");
//base.add_line("129,Alexey Smirnov,30.08.1980,+79123456789,alexey.smirnov@company.ru");
//base.add_line("129,Alexey Smirnov,30.08.1980,+79123456789,alexey.smirnov@company.ru");
//base.add_line("130,Alexey Smirnov,30.08.1980,+79123456789,alexey.smirnov@company.ru");
//base.add_line("131,Alexey Smirnov,30.08.1980,+79123456789,alexey.smirnov@company.ru");
//base.add_line("132,Alexey Smirnov,30.08.1980,+7913456789,alexey.smirnov@company.ru");
//base.add_line("133,Alexey Smirnov,32.08.1980,+79123456789,alexey.smirnov@company.ru");
//base.add_line("134,Alexey Smirnov,30.08.1980,+79123456789,alexey.smirnovcompany.ru");
//ids = base.find_line_numbers_for_name("Alexey Smirnov").first;
//for (std::string i: ids){
//std::cout<<i<<" ";
//}