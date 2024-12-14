#include <iostream>
#include <string>
#include <cstdint>
#include <filesystem>
#include <vector>
#include <fstream>  // Добавлено для работы с файлами
#include <thread>  // Для задержки
#include <chrono>  // Для времени
#include <sstream>  // Для использования stringstream
#include <regex>  // Для регулярных выражений
#include <cstring>  // Для memset
#include "prepoccess.h"

#pragma once
namespace fs = std::filesystem;
class Data_base {
private:
    std::string main_file_name;
    std::fstream main_file;
    std::string shifts_file_name;
    std::fstream shifts_file;
    std::string hash_id_file_name;
    std::fstream hash_id_file;
    std::string hash_name_file_name;
    std::fstream hash_name_file;
    std::string hash_date_file_name;
    std::fstream hash_date_file;
    std::string hash_phone_file_name;
    std::fstream hash_phone_file;
    std::string hash_mail_file_name;
    std::fstream hash_mail_file;
    int total_lines;
public:
    // Конструктор
    Data_base(const std::string& name);
    // Открытие файла
    bool open_file();
    //очистить файлы
    void clear_file();
    //закрыть файл
    void close_file();
    // Добавление записи
    void add_line(const std::string& line);
    // Прочитать содержимое базы данных
    void read_base();

    std::string get_path_shifts(){
        return shifts_file_name;
    }

    std::string get_path_main(){
        return main_file_name;
    }

    void change_line_at_shifts_bin(const std::string& id, const std::string& fio, const std::string& date,
                                              const std::string& phone, const std::string& email, size_t offset);
    void edit_id(const std::string& old_id,const std::string& new_id);
    void edit_name(const std::string& id,const std::string& new_name);
    void edit_date(const std::string& id,const std::string& new_date);
    void edit_phone(const std::string& id,const std::string& new_phone);
    void edit_mail(const std::string& id,const std::string& new_mail);

    std::vector<std::string> read_fixed_line(size_t line_read); //чтение строки из shift файла

    void delete_line_by_id(const std::string& id);
    void delete_line_by_name(const std::string& name);
    void delete_line_by_date(const std::string& date);
    void delete_line_by_phone(const std::string& phone);
    void delete_line_by_mail(const std::string& mail);

    void delete_line_from_shifts(int number_line);
    void delete_line_from_hash_name(int number_line,const std::string& name);
    void delete_line_from_hash_date(int number_line,const std::string& date);
    void delete_line_from_hash_phone(int number_line,const std::string& phone);
    void delete_line_from_hash_mail(int number_line,const std::string& mail);

    std::pair<int, int> find_line_number_in_hash_id(const std::string& id);
    std::pair<std::vector<std::string>, int> find_line_numbers_for_name(const std::string& name);
    std::pair<std::vector<std::string>, int> find_line_numbers_for_date(const std::string& date);
    std::pair<std::vector<std::string>, int> find_line_numbers_for_phone(const std::string& phone);
    std::pair<std::vector<std::string>, int> find_line_numbers_for_mail(const std::string& mail);

    void add_line_at_hash_name_bin(const std::string& name, const std::string& number_of_line_in_shifts_bin);
    void add_line_at_hash_date_bin(const std::string& date, const std::string& number_of_line_in_shifts_bin);
    void add_line_at_hash_phone_bin(const std::string& phone, const std::string& number_of_line_in_shifts_bin);
    void add_line_at_hash_mail_bin(const std::string& mail, const std::string& number_of_line_in_shifts_bin);
    void add_line_at_hash_id_bin(const std::string& id, const std::string& number_of_line_in_shifts_bin);
    void add_line_at_shifts_bin(const std::string& id,const std::string& fio, const std::string& date, const std::string& phone, const std::string& email);
private:

};