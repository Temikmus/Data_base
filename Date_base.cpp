#include "Data_base.h"


#pragma once


// ��-�㭪�� ��� ��ப (DJBX33A)
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

// ��-�㭪�� ��� �᫠
uint64_t hash_int(uint64_t number) {
    number ^= (number >> 16);  // ��� 1: XOR � ᤢ����
    number *= 0x85ebca6b;      // ��� 2: ��������� �� "�����᪮�" �᫮
    number ^= (number >> 13);  // ��� 3: ��� ���� XOR-ᤢ��
    number *= 0xc2b2ae35;      // ��� 4: ��������� �� ��஥ "�����᪮�" �᫮
    number ^= (number >> 16);  // ��� 5: ������� XOR-ᤢ��
    if (number%20==0){
        return 1;
    }
    return number;             // ������ ��-���祭��
}

// ��-�㭪�� ��� ���� ஦�����
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

// ����⨥ 䠩��� � ����୮� ०���
bool Data_base::open_file() {
    main_file.open(main_file_name, std::ios::in | std::ios::out | std::ios::binary);  // ����⨥ 䠩�� � ����୮� ०���
    shifts_file.open(shifts_file_name, std::ios::in | std::ios::out | std::ios::binary);
    hash_id_file.open(hash_id_file_name, std::ios::in | std::ios::out | std::ios::binary);
    hash_name_file.open(hash_name_file_name, std::ios::in | std::ios::out | std::ios::binary);
    hash_date_file.open(hash_date_file_name, std::ios::in | std::ios::out | std::ios::binary);
    hash_phone_file.open(hash_phone_file_name, std::ios::in | std::ios::out | std::ios::binary);
    hash_mail_file.open(hash_mail_file_name, std::ios::in | std::ios::out | std::ios::binary);
    return main_file.is_open();
}

void Data_base::clear_file() {
    main_file.close(); // ������� 䠩�, �᫨ �� �����
    main_file.open(main_file_name, std::ios::out | std::ios::trunc | std::ios::binary); // ������ ��� ����� � ������ ᮤ�ন���

    // ��頥� shifts_file
    shifts_file.close();  // ����뢠�� 䠩�, �᫨ �� �����
    shifts_file.open(shifts_file_name, std::ios::out | std::ios::trunc | std::ios::binary);  // ���뢠�� ��� ����� � ��頥� ᮤ�ন���

    hash_id_file.close(); // ������� 䠩�, �᫨ �� �����
    preprocess_file(hash_id_file_name, total_lines, 17);

    hash_name_file.close(); // ������� 䠩�, �᫨ �� �����
    preprocess_file(hash_name_file_name, total_lines, 100 + 8 * 10 - 1);

    hash_date_file.close(); // ������� 䠩�, �᫨ �� �����
    preprocess_file(hash_date_file_name, total_lines, 11 + 8 * 10 - 1);

    hash_phone_file.close(); // ������� 䠩�, �᫨ �� �����
    preprocess_file(hash_phone_file_name, total_lines, 12 + 8 * 10 - 1);

    hash_mail_file.close(); // ������� 䠩�, �᫨ �� �����
    preprocess_file(hash_mail_file_name, total_lines, 100 + 8 * 10 - 1);
}

void Data_base::close_file() {
    if (main_file.is_open()) {
        main_file.close();
    }
}

// �㭪�� ��� ࠧ������� ��ப� �� ���
std::vector<std::string> split(const std::string& str, char sep) {
    std::vector<std::string> result;
    std::string token;
    std::stringstream ss(str);

    // ���������� ��ப� �� ࠧ����⥫�
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
        std::cerr<<"������� �������筮 �����.";
        return 0;
    }

    //�஢�ઠ id
    for (char c: tokens[0]){
        if(!std::isdigit(c)){
            std::cerr<<"Id ������ ���� 楫� �᫮�.";
            return 0;
        }
    }

    //�஢�ઠ name
    std::regex fio_regex("^[�-�-���]+\\s+[�-�-���]+(\\s+[�-�-���]+)?$|^[a-zA-Z]+\\s+[a-zA-Z]+(\\s+[a-zA-Z]+)?$");
    if(!std::regex_match(tokens[1], fio_regex)){
        std::cerr<<"��� ����ᠭ� �����४⭮.";
        return 0;
    }

    //�஢�ઠ date
    // �����୮� ��ࠦ���� ��� �ଠ� DD.MM.YYYY
    std::regex date_pattern(R"(\d{2}\.\d{2}\.\d{4}$)");

    // �஢��塞, ᮮ⢥����� �� ��ப� �ଠ�� ����
    if (!std::regex_match(tokens[2], date_pattern)) {
        std::cerr<<"��� ������ ���� ����ᠭ� � �ଠ� DD.MM.YYYY";
        return 0;
    }

    std::vector<std::string> date = split(tokens[2], '.');
    int day = std::stoi(date[0]);
    int month = std::stoi(date[1]);
    int year = std::stoi(date[2]);

    // ���ᨢ � ������⢮� ���� � ����� ��� ���筮�� ����
    std::vector<int> days_in_month = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // �஢�ઠ �� ��᮪��� ��� ��� 䥢ࠫ�
    if (isLeapYear(year)) {
        days_in_month[1] = 29;  // ���ࠫ� ����� 29 ���� � ��᮪��� ���
    }

    // �஢�ઠ ���४⭮�� ����� � ���
    if (month < 1 || month > 12) {
        std::cerr<<"����� ������ ���� �� 1 �� 12.";
        return 0;
    }

    if (day < 1 || day > days_in_month[month - 1]) {
        std::cerr<<"���ࠢ��쭮� ���-�� ���� � �����.";
        return 0;
    }

    //�஢�ઠ phone
    std::regex phone_regex(R"(^(\+7|8)\d{10}$)");
    if(!std::regex_match(tokens[3], phone_regex)){
        std::cerr<<"������ ����� �� �������.";
        return 0;
    }

    std::regex mail_regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if(!std::regex_match(tokens[4], mail_regex)){
        std::cerr<<"����� ����� �� �������";
        return 0;
    }

    return 1;
}

// ���������� �����
void Data_base::add_line(const std::string& line) {
    std::vector<std::string> tokens = split(line, ',');
    if (check_validity_line(tokens)){
        std::string str = "";
        str+=tokens[0];
        std::string result;
        std::istringstream iss(tokens[1]);
        std::string word;
        // ��ॡ�ࠥ� ᫮�� � ��ப�
        while (iss >> word) {
            // ������ ����� �㪢� ���������, ��⠫�� - ����묨
            if (!word.empty()) {
                word[0] = std::toupper(word[0]); // ��ࢠ� �㪢� - ���������
                for (size_t i = 1; i < word.size(); ++i) {
                    word[i] = std::tolower(word[i]); // ��⠫�� - �����
                }
                result += word + " "; // ������塞 ᫮�� � �஡��
            }
        }

        // ���ࠥ� ��᫥���� �஡��
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
            std::cout<<"���������� ��������, ⠪ ��� ⠪�� id 㦥 �������.";
            return;
        }
        const std::size_t line_size = 231; // ������ ����� ��ப� � �����
        // ���뢠�� 䠩� � ����୮� ०��� ��� �஢�ન ��� ࠧ���
        std::ifstream file(shifts_file_name, std::ios::binary | std::ios::ate);
        if (!file) {
            std::cerr << "�訡�� ������ 䠩��!" << std::endl;
            return;
        }
        // ����砥� ࠧ��� 䠩�� � �����
        std::streampos file_size = file.tellg();
        file.close();
        // ����塞 ����� ᫥���饩 ��ப�
        std::size_t line_count = static_cast<std::size_t>(file_size) / line_size +1;
        std::string line_shift = std::to_string(line_count);
        add_line_at_hash_id_bin(elements[0], line_shift);
        add_line_at_hash_name_bin(elements[1], line_shift);
        add_line_at_hash_date_bin(elements[2], line_shift);
        add_line_at_hash_phone_bin(elements[3], line_shift);
        add_line_at_hash_mail_bin(elements[4], line_shift);
        add_line_at_shifts_bin(elements[0], elements[1], elements[2], elements[3], elements[4]);

        // ����⨥ main_file ��� ����� � ����୮� �ଠ�
        main_file.close();  // �����⨥ 䠩��, �᫨ �� �� �����
        main_file.open(main_file_name, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);  // ����⨥ 䠩�� � ����୮� ०��� ��� �����

        // ������� ���� ��� ����� ��ப� � 䠩�
        char buffer[line_size] = {0};
        std::memcpy(buffer, elements[0].c_str(), std::min(elements[0].size(), size_t(10)));  // ������塞 id
        std::memcpy(buffer + 10, elements[1].c_str(), std::min(elements[1].size(), size_t(100)));  // ������塞 name
        std::memcpy(buffer + 110, elements[2].c_str(), std::min(elements[2].size(), size_t(10)));  // ������塞 date
        std::memcpy(buffer + 120, elements[3].c_str(), std::min(elements[3].size(), size_t(11)));  // ������塞 phone
        std::memcpy(buffer + 131, elements[4].c_str(), std::min(elements[4].size(), size_t(100)));  // ������塞 email

        // �����뢠�� ��ப� � ������ 䠩�
        main_file.write(buffer, line_size);
        if (!main_file.good()) {
            std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
        }
    }
}

void Data_base::read_base() {
    std::fstream file(shifts_file_name, std::ios::binary | std::ios::in | std::ios::out);
    char buffer[231] = {0};  // ���� ��� �⥭�� ��ப�
    if (file.is_open()) {
        file.seekg(0);  // ��६�頥� 㪠��⥫� �� ��砫� 䠩��
        std::cout << "����ন��� 䠩��:\n";
        while (file.read(buffer, 231)) {  // ��⠥� ��ப� �� 231 ����
            std::string id(buffer, 10);
            if (id[0]!='\0'){
                std::cout << "ID: " << id << ", Name: " << std::string(buffer + 10, 100)
                          << ", Date: " << std::string(buffer + 110, 10) << ", Phone: " << std::string(buffer + 120, 11)
                          << ", Email: " << std::string(buffer + 131, 100) << std::endl;
            }
        }
    }
}

// �㭪�� ��� ����� ��ப� � ����� ����୮�� 䠩��
void Data_base::add_line_at_shifts_bin(const std::string& id, const std::string& fio, const std::string& date,
                                       const std::string& phone, const std::string& email) {
    std::ofstream file(shifts_file_name, std::ios::binary | std::ios::app);  // ����⨥ 䠩�� � ०��� ���������� � �����
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }

    // ������ ��ப� (231 ���� ������)
    size_t line_size = 231;

    // ���� ��� ��ப�
    char buffer[line_size] = {0};  // ���樠�����㥬 ��ﬨ

    // ������塞 id (10 ����)
    std::memcpy(buffer, id.c_str(), std::min(id.size(), size_t(10)));

    // ������塞 fio (100 ����)
    std::memcpy(buffer + 10, fio.c_str(), std::min(fio.size(), size_t(100)));

    // ������塞 date (10 ����)
    std::memcpy(buffer + 10 + 100, date.c_str(), std::min(date.size(), size_t(10)));

    // ������塞 phone (11 ����)
    std::memcpy(buffer + 10 + 100 + 10, phone.c_str(), std::min(phone.size(), size_t(11)));

    // ������塞 email (100 ����)
    std::memcpy(buffer + 10 + 100 + 10 + 11, email.c_str(), std::min(email.size(), size_t(100)));

    // �����뢠�� ��ப� � 䠩�
    file.write(buffer, line_size);  // ������ � ������ 䠩�
    if (!file.good()) {
        std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
    }

    file.close();
}

void Data_base::add_line_at_hash_id_bin(const std::string& id, const std::string& number_of_line_in_shifts_bin) {
    std::fstream file(hash_id_file_name, std::ios::binary | std::ios::in | std::ios::out);  // ����⨥ 䠩�� � ०��� �⥭��/�����
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }

    std::cout << "add_line_at_hash_id_bin\n";

    // ������ ��ப� (17 ���� ������: 10 ���� �� id � 7 ���� �� ����� ��ப�)
    size_t line_size = 17;

    // ����塞 ����� ��ப� � ��-䠩��
    int line_number = hash_int(std::stoi(id)) % total_lines;

    // ��砫쭮� ᬥ饭�� ��� �����
    size_t offset = (line_number - 1) * line_size;
    size_t start_offset = offset;  // ���������� ��砫��� ��ப� ��� �஢�ન

    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};  // ���樠�����㥬 ���� ��ﬨ

    // ���� ���⮩ ��ப� ��� ��ப� � ⠪�� �� id
    while (true) {
        // ��६�頥� 㪠��⥫� �⥭�� �� ⥪���� ��ப�
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "�訡�� ��६�饭�� 㪠��⥫� �⥭��!" << std::endl;
            file.close();
            return;
        }

        // ��⠥� ⥪���� ��ப�
        file.read(buffer, line_size);

        // �஢��塞, ���� �� ��ப� ���⮩ (��� ��ப� ࠢ�� ���)
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {  // �᫨ ������� ������� ᨬ����, ��ப� �����
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            // �᫨ ��ப� �����, �����뢠�� id � ����� ��ப�
            std::memset(buffer, 0, line_size);  // ���樠�����㥬 ��ﬨ
            std::memcpy(buffer, id.c_str(), std::min(id.size(), size_t(10)));  // ������塞 id (10 ����)
            std::memcpy(buffer + 10, number_of_line_in_shifts_bin.c_str(),
                        std::min(number_of_line_in_shifts_bin.size(), size_t(7)));  // ������塞 ����� ��ப� (7 ����)

            // �����뢠�� ��ப�
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
            }
            file.close();
            return;
        }

        // �᫨ ��ப� �� �����, ���室�� � ᫥���饩 ��ப�
        offset += line_size;

        // �஢�ઠ �� ��室 �� �।��� 䠩��
        if (offset >= total_lines * line_size) {  // �᫨ ���⨣��� ����� 䠩��
            offset = 0;  // �����頥��� � ��砫� 䠩��
        }

        // �᫨ �� ���㫨�� � ���⮢�� ��ப�, ����� � 䠩�� ��� ������ ��ப
        if (offset == start_offset) {
            std::cerr << "�� 㤠���� ���� ������ ��ப� ��� �����!" << std::endl;
            file.close();
            return;
        }
    }
}


void Data_base::add_line_at_hash_name_bin(const std::string& name, const std::string& number_of_line_in_shifts_bin) {
    std::fstream file(hash_name_file_name, std::ios::binary | std::ios::in | std::ios::out);  // ����⨥ 䠩�� � ०��� �⥭��/�����
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }
    std::cout<<" add_line_at_hash_name_bin \n";
    int line_number = hash_str(name)%total_lines;
    // ������ ��ப�
    size_t line_size = 100 + 8 * 10 - 1;

    // ��砫쭮� ᬥ饭��
    size_t offset = (line_number - 1) * line_size;
    size_t start_offset = offset;  // ���������� ��砫��� ��ப�

    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};

    // ���� ���⮩ ��ப�
    while (true) {
        // ��६�頥� 㪠��⥫� �⥭�� �� ⥪���� ��ப�
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "�訡�� ��६�饭�� 㪠��⥫� �⥭��!(2)" << std::endl;
            file.close();
            return;
        }

        // ��⠥� ⥪���� ��ப�
        file.read(buffer, line_size);

        // �஢��塞, ���� �� ��ப� ���⮩
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {  // �᫨ ������� ������� ᨬ����, ��ப� �����
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            // �᫨ ��ப� �����, �����뢠�� ��� � ����� ��ப�
            std::memset(buffer, 0, line_size);  // ���樠�����㥬 ��ﬨ
            std::memcpy(buffer, name.c_str(), std::min(name.size(), size_t(100)));  // ������塞 ���
            std::memcpy(buffer + 100, number_of_line_in_shifts_bin.c_str(),
                        std::min(number_of_line_in_shifts_bin.size(), size_t(7)));  // ������塞 ���� ����� ��ப�

            // �����뢠�� ��ப�
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
            }
            file.close();
            return;
        }

        // �᫨ ��ப� �� �����, �஢��塞 ���
        char existing_name[100] = {0};  // 100 ���� ��� �����
        std::memcpy(existing_name, buffer, 100);

        if (name == existing_name) {
            // �᫨ ��� ᮢ������, ������塞 ����� ��ப�
            char* numbers_start = buffer + 100;  // ��砫� ᯨ᪠ ����஢ ��ப
            size_t numbers_length = line_size - 100;  // ����� ����� � ����ࠬ� ��ப

            // ���� ���� ᯨ᪠ ����஢ ��ப
            char* numbers_end = numbers_start;
            while (*numbers_end != '\0') {
                ++numbers_end;
            }

            // �஢��塞, ���� �� ���� ��� ������ ����� ��ப�
            if ((numbers_end - numbers_start) + 8 > numbers_length) {
                std::cerr << "�訡��: �ॢ�襭� ������⢮ ����஢ ��ப ��� �����!" << std::endl;
                file.close();
                return;
            }

            // ������塞 ������� � ���� ����� ��ப�
            std::strncat(numbers_start, ",", 1);
            std::strncat(numbers_start, number_of_line_in_shifts_bin.c_str(), 7);

            // �����뢠�� ����������� ��ப�
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
            }
            file.close();
            return;
        }

        // �᫨ ��� �� ᮢ������, ���室�� � ᫥���饩 ��ப�
        offset += line_size;

        // �஢�ઠ �� ��室 �� �।��� 䠩��
        if (offset >= total_lines * line_size) { // �᫨ ���⨣��� ����� 䠩��
            // �����頥��� � ��砫� 䠩��
            offset = 0;
        }

        // �᫨ �� ���㫨�� � ���⮢�� ��ப�, ����� � 䠩�� ��� ������ ��ப
        if (offset == start_offset) {
            std::cerr << "�� 㤠���� ���� ������ ��ப� ��� �����!" << std::endl;
            file.close();
            return;
        }
    }
}

void Data_base::add_line_at_hash_date_bin(const std::string& date, const std::string& number_of_line_in_shifts_bin) {
    std::fstream file(hash_date_file_name, std::ios::binary | std::ios::in | std::ios::out);  // ����⨥ 䠩�� � ०��� �⥭��/�����
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }
    std::cout<<" add_line_at_hash_date_bin \n";
    std::vector<std::string> splited_date = split(date, '.');
    int line_number = hash_date(std::stoi(splited_date[2]), std::stoi(splited_date[1]), std::stoi(splited_date[0]))%total_lines;
    // ������ ��ப�
    size_t line_size = 11 + 8 * 10 - 1;

    // ��砫쭮� ᬥ饭��
    size_t offset = (line_number - 1) * line_size;
    size_t start_offset = offset;  // ���������� ��砫��� ��ப�

    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};

    // ���� ���⮩ ��ப�
    while (true) {
        // ��६�頥� 㪠��⥫� �⥭�� �� ⥪���� ��ப�
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "�訡�� ��६�饭�� 㪠��⥫� �⥭��!" << std::endl;
            file.close();
            return;
        }

        // ��⠥� ⥪���� ��ப�
        file.read(buffer, line_size);

        // �஢��塞, ���� �� ��ப� ���⮩
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {  // �᫨ ������� ������� ᨬ����, ��ப� �����
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            // �᫨ ��ப� �����, �����뢠�� ��� � ����� ��ப�
            std::memset(buffer, 0, line_size);  // ���樠�����㥬 ��ﬨ
            std::memcpy(buffer, date.c_str(), std::min(date.size(), size_t(11)));  // ������塞 ���
            std::memcpy(buffer + 11, number_of_line_in_shifts_bin.c_str(),
                        std::min(number_of_line_in_shifts_bin.size(), size_t(7)));  // ������塞 ���� ����� ��ப�

            // �����뢠�� ��ப�
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
            }
            file.close();
            return;
        }

        // �᫨ ��ப� �� �����, �஢��塞 ���
        char existing_date[11] = {0};  // 10 ���� ��� ����
        std::memcpy(existing_date, buffer, 11);

        if (date == existing_date) {
            // �᫨ ��� ᮢ������, ������塞 ����� ��ப�
            char* numbers_start = buffer + 11;  // ��砫� ᯨ᪠ ����஢ ��ப
            size_t numbers_length = line_size - 11;  // ����� ����� � ����ࠬ� ��ப

            // ���� ���� ᯨ᪠ ����஢ ��ப
            char* numbers_end = numbers_start;
            while (*numbers_end != '\0') {
                ++numbers_end;
            }

            // �஢��塞, ���� �� ���� ��� ������ ����� ��ப�
            if ((numbers_end - numbers_start) + 8 > numbers_length) {
                std::cerr << "�訡��: �ॢ�襭� ������⢮ ����஢ ��ப ��� �����!" << std::endl;
                file.close();
                return;
            }

            // ������塞 ������� � ���� ����� ��ப�
            std::strncat(numbers_start, ",", 1);
            std::strncat(numbers_start, number_of_line_in_shifts_bin.c_str(), 7);

            // �����뢠�� ����������� ��ப�
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
            }
            file.close();
            return;
        }

        // �᫨ ��� �� ᮢ������, ���室�� � ᫥���饩 ��ப�
        offset += line_size;

        // �஢�ઠ �� ��室 �� �।��� 䠩��
        if (offset >= total_lines * line_size) { // �᫨ ���⨣��� ����� 䠩��
            // �����頥��� � ��砫� 䠩��
            offset = 0;
        }

        // �᫨ �� ���㫨�� � ���⮢�� ��ப�, ����� � 䠩�� ��� ������ ��ப
        if (offset == start_offset) {
            std::cerr << "�� 㤠���� ���� ������ ��ப� ��� �����!" << std::endl;
            file.close();
            return;
        }
    }
}

void Data_base::add_line_at_hash_phone_bin(const std::string& phone, const std::string& number_of_line_in_shifts_bin) {
    std::fstream file(hash_phone_file_name, std::ios::binary | std::ios::in | std::ios::out);  // ����⨥ 䠩�� � ०��� �⥭��/�����
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }
    std::cout<<" add_line_at_hash_phone_bin \n";
    int line_number = hash_int(std::stoll(phone))%total_lines;
    // ������ ��ப�
    size_t line_size = 12 + 8 * 10 - 1;

    // ��砫쭮� ᬥ饭��
    size_t offset = (line_number - 1) * line_size;
    size_t start_offset = offset;  // ���������� ��砫��� ��ப�

    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};

    // ���� ���⮩ ��ப�
    while (true) {
        // ��६�頥� 㪠��⥫� �⥭�� �� ⥪���� ��ப�
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "�訡�� ��६�饭�� 㪠��⥫� �⥭��!" << std::endl;
            file.close();
            return;
        }

        // ��⠥� ⥪���� ��ப�
        file.read(buffer, line_size);

        // �஢��塞, ���� �� ��ப� ���⮩
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {  // �᫨ ������� ������� ᨬ����, ��ப� �����
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            // �᫨ ��ப� �����, �����뢠�� ��� � ����� ��ப�
            std::memset(buffer, 0, line_size);  // ���樠�����㥬 ��ﬨ
            std::memcpy(buffer, phone.c_str(), std::min(phone.size(), size_t(12)));  // ������塞 ���
            std::memcpy(buffer + 12, number_of_line_in_shifts_bin.c_str(),
                        std::min(number_of_line_in_shifts_bin.size(), size_t(7)));  // ������塞 ���� ����� ��ப�

            // �����뢠�� ��ப�
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
            }
            file.close();
            return;
        }

        // �᫨ ��ப� �� �����, �஢��塞 ���
        char existing_name[12] = {0};  // 100 ���� ��� �����
        std::memcpy(existing_name, buffer, 12);

        if (phone == existing_name) {
            // �᫨ ��� ᮢ������, ������塞 ����� ��ப�
            char* numbers_start = buffer + 12;  // ��砫� ᯨ᪠ ����஢ ��ப
            size_t numbers_length = line_size - 12;  // ����� ����� � ����ࠬ� ��ப

            // ���� ���� ᯨ᪠ ����஢ ��ப
            char* numbers_end = numbers_start;
            while (*numbers_end != '\0') {
                ++numbers_end;
            }

            // �஢��塞, ���� �� ���� ��� ������ ����� ��ப�
            if ((numbers_end - numbers_start) + 8 > numbers_length) {
                std::cerr << "�訡��: �ॢ�襭� ������⢮ ����஢ ��ப ��� �����!" << std::endl;
                file.close();
                return;
            }

            // ������塞 ������� � ���� ����� ��ப�
            std::strncat(numbers_start, ",", 1);
            std::strncat(numbers_start, number_of_line_in_shifts_bin.c_str(), 7);

            // �����뢠�� ����������� ��ப�
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
            }
            file.close();
            return;
        }

        // �᫨ ��� �� ᮢ������, ���室�� � ᫥���饩 ��ப�
        offset += line_size;

        // �஢�ઠ �� ��室 �� �।��� 䠩��
        if (offset >= total_lines * line_size) { // �᫨ ���⨣��� ����� 䠩��
            // �����頥��� � ��砫� 䠩��
            offset = 0;
        }

        // �᫨ �� ���㫨�� � ���⮢�� ��ப�, ����� � 䠩�� ��� ������ ��ப
        if (offset == start_offset) {
            std::cerr << "�� 㤠���� ���� ������ ��ப� ��� �����!" << std::endl;
            file.close();
            return;
        }
    }
}

void Data_base::add_line_at_hash_mail_bin(const std::string& mail, const std::string& number_of_line_in_shifts_bin) {
    std::fstream file(hash_mail_file_name, std::ios::binary | std::ios::in | std::ios::out);  // ����⨥ 䠩�� � ०��� �⥭��/�����
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }
    int line_number = hash_str(mail)%total_lines;
    // ������ ��ப�
    size_t line_size = 100 + 8 * 10 - 1;
    std::cout<<" add_line_at_hash_mail_bin \n";
    // ��砫쭮� ᬥ饭��
    size_t offset = (line_number - 1) * line_size;
    size_t start_offset = offset;  // ���������� ��砫��� ��ப�

    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};

    // ���� ���⮩ ��ப�
    while (true) {
        // ��६�頥� 㪠��⥫� �⥭�� �� ⥪���� ��ப�
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "�訡�� ��६�饭�� 㪠��⥫� �⥭��!" << std::endl;
            file.close();
            return;
        }

        // ��⠥� ⥪���� ��ப�
        file.read(buffer, line_size);

        // �஢��塞, ���� �� ��ப� ���⮩
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {  // �᫨ ������� ������� ᨬ����, ��ப� �����
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            // �᫨ ��ப� �����, �����뢠�� ��� � ����� ��ப�
            std::memset(buffer, 0, line_size);  // ���樠�����㥬 ��ﬨ
            std::memcpy(buffer, mail.c_str(), std::min(mail.size(), size_t(100)));  // ������塞 ���
            std::memcpy(buffer + 100, number_of_line_in_shifts_bin.c_str(),
                        std::min(number_of_line_in_shifts_bin.size(), size_t(7)));  // ������塞 ���� ����� ��ப�

            // �����뢠�� ��ப�
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
            }
            file.close();
            return;
        }

        // �᫨ ��ப� �� �����, �஢��塞 ���
        char existing_mail[100] = {0};  // 100 ���� ��� �����
        std::memcpy(existing_mail, buffer, 100);

        if (mail == existing_mail) {
            // �᫨ ��� ᮢ������, ������塞 ����� ��ப�
            char* numbers_start = buffer + 100;  // ��砫� ᯨ᪠ ����஢ ��ப
            size_t numbers_length = line_size - 100;  // ����� ����� � ����ࠬ� ��ப

            // ���� ���� ᯨ᪠ ����஢ ��ப
            char* numbers_end = numbers_start;
            while (*numbers_end != '\0') {
                ++numbers_end;
            }

            // �஢��塞, ���� �� ���� ��� ������ ����� ��ப�
            if ((numbers_end - numbers_start) + 8 > numbers_length) {
                std::cerr << "�訡��: �ॢ�襭� ������⢮ ����஢ ��ப ��� �����!" << std::endl;
                file.close();
                return;
            }

            // ������塞 ������� � ���� ����� ��ப�
            std::strncat(numbers_start, ",", 1);
            std::strncat(numbers_start, number_of_line_in_shifts_bin.c_str(), 7);

            // �����뢠�� ����������� ��ப�
            file.seekp(offset);
            file.write(buffer, line_size);
            if (!file.good()) {
                std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
            }
            file.close();
            return;
        }

        // �᫨ ��� �� ᮢ������, ���室�� � ᫥���饩 ��ப�
        offset += line_size;

        // �஢�ઠ �� ��室 �� �।��� 䠩��
        if (offset >= total_lines * line_size) { // �᫨ ���⨣��� ����� 䠩��
            // �����頥��� � ��砫� 䠩��
            offset = 0;
        }

        // �᫨ �� ���㫨�� � ���⮢�� ��ப�, ����� � 䠩�� ��� ������ ��ப
        if (offset == start_offset) {
            std::cerr << "�� 㤠���� ���� ������ ��ப� ��� �����!" << std::endl;
            file.close();
            return;
        }
    }
}

// �㭪�� ��� ���᪠ ����஢ ��ப �� �����, ��稭�� � ��।������� ��ப�
std::pair<int, int> Data_base::find_line_number_in_hash_id(const std::string& id) {
    std::fstream file(hash_id_file_name, std::ios::binary | std::ios::in);  // ����⨥ 䠩�� ��� �⥭��
    int line_number=-1;  // ����� ��� �࠭���� ����஢ ��ப

    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return {line_number,-1};
    }
    int start_line = hash_int(std::stoi(id))%total_lines;
    // ������ ��ப�
    size_t line_size = 10 + 7;

    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};

    // ��砫쭮� ᬥ饭��
    size_t offset = (start_line - 1) * line_size;
    size_t start_offset = offset;  // ���������� ��砫��� ��ப�

    // ���� ����஢ ��ப �� �����
    while (true) {
        // ��६�頥� 㪠��⥫� �⥭�� �� ⥪���� ��ப�
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "�訡�� ��६�饭�� 㪠��⥫� �⥭��! (3)" << std::endl;
            file.close();
            return {line_number,-1};
        }

        // ��⠥� ⥪���� ��ப�
        file.read(buffer, line_size);

        // �஢��塞, ���� �� ��ப� ���⮩
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            break;  // �� ���⨣�� ���� 䠩��
        }

        // �஢��塞 ���
        char existing_id[10] = {0};  // 10 ���� ��� id
        std::memcpy(existing_id, buffer, 10);

        if (id == existing_id) {
            // �᫨ id ᮢ������, ��������� ����� ��ப� (�� ��室���� � ᫥����� 7 ����� ��᫥ id)
            char number_str[7] = {0};  // ���ᨢ ��� �࠭���� 7 ���� + 1 ��� �������饣� ���
            std::memcpy(number_str, buffer + 10, 7);  // �����㥬 7 ���� � ����� ��ப�

            // �८�ࠧ㥬 � ��ப�
            std::string number(number_str);
            line_number = std::stoi(number);
            return {line_number,offset};
        }

        // ���室�� � ᫥���饩 ��ப�
        offset += line_size;

        // �஢�ઠ �� ��室 �� �।��� 䠩��
        if (offset >= total_lines * line_size) { // �᫨ ���⨣��� ����� 䠩��
            // �����頥��� � ��砫� 䠩��
            offset = 0;
        }

        // �᫨ �� ���㫨�� � ���⮢�� ��ப�, ����� � 䠩�� ��� ������ ��ப
        if (offset == start_offset) {
            std::cerr << "�� 㤠���� ���� �� ��ப� ��� �����!" << std::endl;
            file.close();
            return {line_number,-1};
        }
    }

    file.close();
    return {line_number,-1};
}

// �㭪�� ��� ���᪠ ����஢ ��ப �� �����, ��稭�� � ��।������� ��ப�
std::pair<std::vector<std::string>, int> Data_base::find_line_numbers_for_name(const std::string& name) {
    std::fstream file(hash_name_file_name, std::ios::binary | std::ios::in);  // ����⨥ 䠩�� ��� �⥭��
    std::vector<std::string> line_numbers;  // ����� ��� �࠭���� ����஢ ��ப

    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return {line_numbers, -1};
    }

    // ������ ��ப�
    size_t line_size = 100 + 8 * 10 - 1;

    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};

    int start_line = hash_str(name)%total_lines;
    // ��砫쭮� ᬥ饭��
    size_t offset = (start_line - 1) * line_size;
    size_t start_offset = offset;  // ���������� ��砫��� ��ப�

    // ���� ����஢ ��ப �� �����
    while (true) {
        // ��६�頥� 㪠��⥫� �⥭�� �� ⥪���� ��ப�
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "�訡�� ��६�饭�� 㪠��⥫� �⥭��!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }

        // ��⠥� ⥪���� ��ப�
        file.read(buffer, line_size);

        // �஢��塞, ���� �� ��ப� ���⮩
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            break;  // �� ���⨣�� ���� 䠩��
        }

        // �஢��塞 ���
        char existing_name[100] = {0};  // 100 ���� ��� �����
        std::memcpy(existing_name, buffer, 100);

        if (name == existing_name) {
            // �᫨ ��� ᮢ������, ��������� ����� ��ப
            char* numbers_start = buffer + 100;  // ��砫� ᯨ᪠ ����஢ ��ப
            std::string numbers_str(numbers_start);  // �८�ࠧ㥬 � ��ப� ��� 㤮��⢠ ��ࠡ�⪨

            // �஢��塞, ����� ��ப� ��� ���
            if (!numbers_str.empty()) {
                // ������塞 ��ப� � ����ࠬ� �� ����⮩
                std::stringstream ss(numbers_str);
                std::string number;
                while (std::getline(ss, number, ',')) {
                    line_numbers.push_back(number);  // ������塞 ����� ����� � १����
                }

                // �᫨ � ��ப� �� �뫮 �������, � ���� `numbers_str` 㦥 ᮤ�ন� �����
                if (line_numbers.empty()) {
                    line_numbers.push_back(numbers_str);
                }
            }

            return {line_numbers, offset};
        }

        // ���室�� � ᫥���饩 ��ப�
        offset += line_size;

        // �஢�ઠ �� ��室 �� �।��� 䠩��
        if (offset >= total_lines * line_size) { // �᫨ ���⨣��� ����� 䠩��
            // �����頥��� � ��砫� 䠩��
            offset = 0;
        }

        // �᫨ �� ���㫨�� � ���⮢�� ��ப�, ����� � 䠩�� ��� ������ ��ப
        if (offset == start_offset) {
            std::cerr << "�� 㤠���� ���� �� ��ப� ��� �����!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }
    }
    file.close();
    return {line_numbers, -1};
}

// �㭪�� ��� ���᪠ ����஢ ��ப �� �����, ��稭�� � ��।������� ��ப�
std::pair<std::vector<std::string>, int> Data_base::find_line_numbers_for_date(const std::string& date) {
    std::fstream file(hash_date_file_name, std::ios::binary | std::ios::in);  // ����⨥ 䠩�� ��� �⥭��
    std::vector<std::string> line_numbers;  // ����� ��� �࠭���� ����஢ ��ப

    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return {line_numbers, -1};
    }

    // ������ ��ப�
    size_t line_size = 11 + 8 * 10 - 1;

    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};
    std::vector<std::string> splited_date = split(date, '.');
    int start_line = hash_date(std::stoi(splited_date[2]), std::stoi(splited_date[1]), std::stoi(splited_date[0]))%total_lines;

    // ��砫쭮� ᬥ饭��
    size_t offset = (start_line - 1) * line_size;
    size_t start_offset = offset;  // ���������� ��砫��� ��ப�

    // ���� ����஢ ��ப �� �����
    while (true) {
        // ��६�頥� 㪠��⥫� �⥭�� �� ⥪���� ��ப�
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "�訡�� ��६�饭�� 㪠��⥫� �⥭��!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }

        // ��⠥� ⥪���� ��ப�
        file.read(buffer, line_size);

        // �஢��塞, ���� �� ��ப� ���⮩
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            break;  // �� ���⨣�� ���� 䠩��
        }

        // �஢��塞 ���
        char existing_name[11] = {0};  // 100 ���� ��� �����
        std::memcpy(existing_name, buffer, 11);

        if (date == existing_name) {
            // �᫨ ��� ᮢ������, ��������� ����� ��ப
            char* numbers_start = buffer + 11;  // ��砫� ᯨ᪠ ����஢ ��ப
            std::string numbers_str(numbers_start);  // �८�ࠧ㥬 � ��ப� ��� 㤮��⢠ ��ࠡ�⪨

            // �஢��塞, ����� ��ப� ��� ���
            if (!numbers_str.empty()) {
                // ������塞 ��ப� � ����ࠬ� �� ����⮩
                std::stringstream ss(numbers_str);
                std::string number;
                while (std::getline(ss, number, ',')) {
                    line_numbers.push_back(number);  // ������塞 ����� ����� � १����
                }

                // �᫨ � ��ப� �� �뫮 �������, � ���� `numbers_str` 㦥 ᮤ�ন� �����
                if (line_numbers.empty()) {
                    line_numbers.push_back(numbers_str);
                }
            }

            return {line_numbers, offset};
        }

        // ���室�� � ᫥���饩 ��ப�
        offset += line_size;

        // �஢�ઠ �� ��室 �� �।��� 䠩��
        if (offset >= total_lines * line_size) { // �᫨ ���⨣��� ����� 䠩��
            // �����頥��� � ��砫� 䠩��
            offset = 0;
        }

        // �᫨ �� ���㫨�� � ���⮢�� ��ப�, ����� � 䠩�� ��� ������ ��ப
        if (offset == start_offset) {
            std::cerr << "�� 㤠���� ���� �� ��ப� ��� �����!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }
    }
    file.close();
    return {line_numbers, -1};
}


// �㭪�� ��� ���᪠ ����஢ ��ப �� �����, ��稭�� � ��।������� ��ப�
std::pair<std::vector<std::string>, int> Data_base::find_line_numbers_for_phone(const std::string& phone) {
    std::fstream file(hash_phone_file_name, std::ios::binary | std::ios::in);  // ����⨥ 䠩�� ��� �⥭��
    std::vector<std::string> line_numbers;  // ����� ��� �࠭���� ����஢ ��ப

    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return {line_numbers, -1};
    }

    // ������ ��ப�
    size_t line_size = 12 + 8 * 10 - 1;

    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};
    int start_line = hash_int(std::stoll(phone))%total_lines;

    // ��砫쭮� ᬥ饭��
    size_t offset = (start_line - 1) * line_size;
    size_t start_offset = offset;  // ���������� ��砫��� ��ப�

    // ���� ����஢ ��ப �� �����
    while (true) {
        // ��६�頥� 㪠��⥫� �⥭�� �� ⥪���� ��ப�
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "�訡�� ��६�饭�� 㪠��⥫� �⥭��!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }

        // ��⠥� ⥪���� ��ப�
        file.read(buffer, line_size);

        // �஢��塞, ���� �� ��ப� ���⮩
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            std::cout<<"smth wrong with hash";
            break;  // �� ���⨣�� ���� 䠩��
        }

        // �஢��塞 ���
        char existing_name[12] = {0};  // 100 ���� ��� �����
        std::memcpy(existing_name, buffer, 12);

        if (phone == existing_name) {
            // �᫨ ��� ᮢ������, ��������� ����� ��ப
            char* numbers_start = buffer + 12;  // ��砫� ᯨ᪠ ����஢ ��ப
            std::string numbers_str(numbers_start);  // �८�ࠧ㥬 � ��ப� ��� 㤮��⢠ ��ࠡ�⪨

            // �஢��塞, ����� ��ப� ��� ���
            if (!numbers_str.empty()) {
                // ������塞 ��ப� � ����ࠬ� �� ����⮩
                std::stringstream ss(numbers_str);
                std::string number;
                while (std::getline(ss, number, ',')) {
                    line_numbers.push_back(number);  // ������塞 ����� ����� � १����
                }

                // �᫨ � ��ப� �� �뫮 �������, � ���� `numbers_str` 㦥 ᮤ�ন� �����
                if (line_numbers.empty()) {
                    line_numbers.push_back(numbers_str);
                }
            }

            return {line_numbers, offset};
        }

        // ���室�� � ᫥���饩 ��ப�
        offset += line_size;

        // �஢�ઠ �� ��室 �� �।��� 䠩��
        if (offset >= total_lines * line_size) { // �᫨ ���⨣��� ����� 䠩��
            // �����頥��� � ��砫� 䠩��
            offset = 0;
        }

        // �᫨ �� ���㫨�� � ���⮢�� ��ப�, ����� � 䠩�� ��� ������ ��ப
        if (offset == start_offset) {
            std::cerr << "�� 㤠���� ���� �� ��ப� ��� �����!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }
    }
    file.close();
    return {line_numbers, -1};
}

// �㭪�� ��� ���᪠ ����஢ ��ப �� �����, ��稭�� � ��।������� ��ப�
std::pair<std::vector<std::string>, int> Data_base::find_line_numbers_for_mail(const std::string& mail) {
    std::fstream file(hash_mail_file_name, std::ios::binary | std::ios::in);  // ����⨥ 䠩�� ��� �⥭��
    std::vector<std::string> line_numbers;  // ����� ��� �࠭���� ����஢ ��ப

    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return {line_numbers, -1};
    }

    // ������ ��ப�
    size_t line_size = 100 + 8 * 10 - 1;

    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};
    int start_line = hash_str(mail)%total_lines;

    // ��砫쭮� ᬥ饭��
    size_t offset = (start_line - 1) * line_size;
    size_t start_offset = offset;  // ���������� ��砫��� ��ப�

    // ���� ����஢ ��ப �� �����
    while (true) {
        // ��६�頥� 㪠��⥫� �⥭�� �� ⥪���� ��ப�
        file.seekg(offset);
        if (!file.good()) {
            std::cerr << "�訡�� ��६�饭�� 㪠��⥫� �⥭��!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }

        // ��⠥� ⥪���� ��ப�
        file.read(buffer, line_size);

        // �஢��塞, ���� �� ��ப� ���⮩
        bool is_empty = true;
        for (size_t i = 0; i < line_size; ++i) {
            if (buffer[i] != '\0') {
                is_empty = false;
                break;
            }
        }

        if (is_empty) {
            break;  // �� ���⨣�� ���� 䠩��
        }

        // �஢��塞 ���
        char existing_name[100] = {0};  // 100 ���� ��� �����
        std::memcpy(existing_name, buffer, 100);

        if (mail == existing_name) {
            // �᫨ ��� ᮢ������, ��������� ����� ��ப
            char* numbers_start = buffer + 100;  // ��砫� ᯨ᪠ ����஢ ��ப
            std::string numbers_str(numbers_start);  // �८�ࠧ㥬 � ��ப� ��� 㤮��⢠ ��ࠡ�⪨

            // �஢��塞, ����� ��ப� ��� ���
            if (!numbers_str.empty()) {
                // ������塞 ��ப� � ����ࠬ� �� ����⮩
                std::stringstream ss(numbers_str);
                std::string number;
                while (std::getline(ss, number, ',')) {
                    line_numbers.push_back(number);  // ������塞 ����� ����� � १����
                }

                // �᫨ � ��ப� �� �뫮 �������, � ���� `numbers_str` 㦥 ᮤ�ন� �����
                if (line_numbers.empty()) {
                    line_numbers.push_back(numbers_str);
                }
            }

            return {line_numbers, offset};
        }

        // ���室�� � ᫥���饩 ��ப�
        offset += line_size;

        // �஢�ઠ �� ��室 �� �।��� 䠩��
        if (offset >= total_lines * line_size) { // �᫨ ���⨣��� ����� 䠩��
            // �����頥��� � ��砫� 䠩��
            offset = 0;
        }

        // �᫨ �� ���㫨�� � ���⮢�� ��ப�, ����� � 䠩�� ��� ������ ��ப
        if (offset == start_offset) {
            std::cerr << "�� 㤠���� ���� �� ��ப� ��� �����!" << std::endl;
            file.close();
            return {line_numbers, -1};
        }
    }
    file.close();
    return {line_numbers, -1};
}

// �㭪�� ��� �⥭�� ��ப� �� ����୮�� 䠩��
std::vector<std::string> Data_base::read_fixed_line(size_t line_read) {
    std::ifstream file(shifts_file_name, std::ios::binary);  // ����⨥ 䠩�� � ����୮� ०���
    std::vector<std::string> elements;
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return elements;
    }
    size_t line_size = 231;
    char buffer[line_size] = {0};  // ���� ��� ��ப� (231 ���� ������ + 1 ���� ��� '\0')
    size_t offset = (line_read - 1) * line_size;
    // ��६�頥� 㪠��⥫� �⥭�� �� ⥪���� ��ப�
    file.seekg(offset);
    if (!file.good()) {
        std::cerr << "�訡�� ��६�饭�� 㪠��⥫� �⥭��!" << std::endl;
        file.close();
    }
    // ��⠥� ⥪���� ��ப�
    file.read(buffer, line_size);
    std::string id,name,date,phone, mail;
    // �८�ࠧ㥬 ����� � ��ப� � �뢮���
    for (size_t i = 0; i < 10; ++i) {
        if (buffer[i] != '\0') {  // �ய�᪠�� �㫥�� �����
            id+=buffer[i];
        }
        else{
            break;
        }
    }
    for (size_t i = 0+10; i < 10+100; ++i) {
        if (buffer[i] != '\0') {  // �ய�᪠�� �㫥�� �����
            name+=buffer[i];
        }
        else{
            break;
        }
    }
    for (size_t i = 0+10+100; i < 10+100+10; ++i) {
        if (buffer[i] != '\0') {  // �ய�᪠�� �㫥�� �����
            date+=buffer[i];
        }
        else{
            break;
        }
    }
    for (size_t i = 0+10+100+10; i < 10+100+10+11; ++i) {
        if (buffer[i] != '\0') {  // �ய�᪠�� �㫥�� �����
            phone+=buffer[i];
        }
        else{
            break;
        }
    }
    for (size_t i = 0+10+100+10+11; i < 10+100+10+11+100; ++i) {
        if (buffer[i] != '\0') {  // �ய�᪠�� �㫥�� �����
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

// �㭪�� ��� 㤠����� ��ப� � 㪠����� id
void Data_base::delete_line_by_id(const std::string& id) {
    std::fstream file(hash_id_file_name, std::ios::binary | std::ios::in | std::ios::out);  // ����⨥ 䠩�� � ०��� �⥭��/�����
    std::pair<int, int> info = find_line_number_in_hash_id(id);
    if (info.first==-1){
        std::cout<<"��� ⠪��� id.";
        return;
    }
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }

    size_t line_size = 17;
    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};
    // ��砫쭮� ᬥ饭��
    size_t offset = info.second;
    size_t start_offset = offset;  // ���������� ��砫��� ��ப�
    // �᫨ id ᮢ������, �����塞 ��ப� �� ������
    std::memset(buffer, 0, line_size);  // �����塞 ��ப� �� ������

    // �����뢠�� ��ப� ���⭮ � 䠩�
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
    }
    file.close();
    std::cout << "��ப� � id " << id << " �뫠 㤠����." << std::endl;
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
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }
    size_t line_size = 231;
    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};
    // ��砫쭮� ᬥ饭��
    size_t offset = (number_line - 1) * line_size;

    // �᫨ id ᮢ������, �����塞 ��ப� �� ������
    std::memset(buffer, 0, line_size);  // �����塞 ��ப� �� ������

    // �����뢠�� ��ப� ���⭮ � 䠩�
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
    }

    file.close();
}

void Data_base::delete_line_from_hash_name(int number_line,const std::string& name){
    std::pair<std::vector<std::string>, int> info = find_line_numbers_for_name(name);
    info.first.erase(std::remove(info.first.begin(), info.first.end(), std::to_string(number_line)), info.first.end());
    std::fstream file(hash_name_file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }
    size_t line_size = 100 + 8 * 10 - 1;
// ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};
// ��砫쭮� ᬥ饭��
    size_t offset = info.second;

    if (info.first.size() == 0) {
        // �᫨ ᯨ᮪ ��ப ����, �����塞 ��� ��ப� �� ������
        std::memset(buffer, 0, line_size);  // �����塞 ��ப� �� ������
    } else {
        // �᫨ ���� ����� ��ப, ᮡ�ࠥ� �� � ��ப� � �����묨
        std::string numbers_str = "";
        for (size_t i = 0; i < info.first.size(); ++i) {
            numbers_str += info.first[i];
            if (i != info.first.size() - 1) {
                numbers_str += ",";  // ������塞 ������� ����� ����ࠬ�
            }
        }

        // ������塞 ����
        std::memset(buffer, 0, line_size);  // ���樠�����㥬 ���� ��ﬨ
        std::memcpy(buffer, name.c_str(), std::min(name.size(), size_t(100)));  // �����㥬 ��� � ���� 100 ����
        std::memcpy(buffer + 100, numbers_str.c_str(), std::min(numbers_str.size(), line_size - 100));  // �����㥬 ����� ��ப
    }

// �����뢠�� ��ப� ���⭮ � 䠩�
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
    }

    file.close();

}

void Data_base::delete_line_from_hash_date(int number_line,const std::string& date){
    std::pair<std::vector<std::string>, int> info = find_line_numbers_for_date(date);
    info.first.erase(std::remove(info.first.begin(), info.first.end(), std::to_string(number_line)), info.first.end());
    std::fstream file(hash_date_file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }
    size_t line_size = 11 + 8 * 10 - 1;
// ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};
// ��砫쭮� ᬥ饭��
    size_t offset = info.second;

    if (info.first.size() == 0) {
        // �᫨ ᯨ᮪ ��ப ����, �����塞 ��� ��ப� �� ������
        std::memset(buffer, 0, line_size);  // �����塞 ��ப� �� ������
    } else {
        // �᫨ ���� ����� ��ப, ᮡ�ࠥ� �� � ��ப� � �����묨
        std::string numbers_str = "";
        for (size_t i = 0; i < info.first.size(); ++i) {
            numbers_str += info.first[i];
            if (i != info.first.size() - 1) {
                numbers_str += ",";  // ������塞 ������� ����� ����ࠬ�
            }
        }

        // ������塞 ����
        std::memset(buffer, 0, line_size);  // ���樠�����㥬 ���� ��ﬨ
        std::memcpy(buffer, date.c_str(), std::min(date.size(), size_t(11)));  // �����㥬 ��� � ���� 100 ����
        std::memcpy(buffer + 11, numbers_str.c_str(), std::min(numbers_str.size(), line_size - 11));  // �����㥬 ����� ��ப
    }

// �����뢠�� ��ப� ���⭮ � 䠩�
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
    }

    file.close();

}

void Data_base::delete_line_from_hash_phone(int number_line,const std::string& phone){
    std::pair<std::vector<std::string>, int> info = find_line_numbers_for_phone(phone);
    info.first.erase(std::remove(info.first.begin(), info.first.end(), std::to_string(number_line)), info.first.end());
    std::fstream file(hash_phone_file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }
    size_t line_size = 12 + 8 * 10 - 1;
// ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};
// ��砫쭮� ᬥ饭��
    size_t offset = info.second;

    if (info.first.size() == 0) {
        // �᫨ ᯨ᮪ ��ப ����, �����塞 ��� ��ப� �� ������
        std::memset(buffer, 0, line_size);  // �����塞 ��ப� �� ������
    } else {
        // �᫨ ���� ����� ��ப, ᮡ�ࠥ� �� � ��ப� � �����묨
        std::string numbers_str = "";
        for (size_t i = 0; i < info.first.size(); ++i) {
            numbers_str += info.first[i];
            if (i != info.first.size() - 1) {
                numbers_str += ",";  // ������塞 ������� ����� ����ࠬ�
            }
        }

        // ������塞 ����
        std::memset(buffer, 0, line_size);  // ���樠�����㥬 ���� ��ﬨ
        std::memcpy(buffer, phone.c_str(), std::min(phone.size(), size_t(12)));  // �����㥬 ��� � ���� 100 ����
        std::memcpy(buffer + 12, numbers_str.c_str(), std::min(numbers_str.size(), line_size - 12));  // �����㥬 ����� ��ப
    }

// �����뢠�� ��ப� ���⭮ � 䠩�
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
    }

    file.close();

}

void Data_base::delete_line_from_hash_mail(int number_line,const std::string& mail){
    std::pair<std::vector<std::string>, int> info = find_line_numbers_for_mail(mail);
    info.first.erase(std::remove(info.first.begin(), info.first.end(), std::to_string(number_line)), info.first.end());
    std::fstream file(hash_mail_file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }
    size_t line_size = 100 + 8 * 10 - 1;
// ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};
// ��砫쭮� ᬥ饭��
    size_t offset = info.second;

    if (info.first.size() == 0) {
        // �᫨ ᯨ᮪ ��ப ����, �����塞 ��� ��ப� �� ������
        std::memset(buffer, 0, line_size);  // �����塞 ��ப� �� ������
    } else {
        // �᫨ ���� ����� ��ப, ᮡ�ࠥ� �� � ��ப� � �����묨
        std::string numbers_str = "";
        for (size_t i = 0; i < info.first.size(); ++i) {
            numbers_str += info.first[i];
            if (i != info.first.size() - 1) {
                numbers_str += ",";  // ������塞 ������� ����� ����ࠬ�
            }
        }

        // ������塞 ����
        std::memset(buffer, 0, line_size);  // ���樠�����㥬 ���� ��ﬨ
        std::memcpy(buffer, mail.c_str(), std::min(mail.size(), size_t(100)));  // �����㥬 ��� � ���� 100 ����
        std::memcpy(buffer + 100, numbers_str.c_str(), std::min(numbers_str.size(), line_size - 100));  // �����㥬 ����� ��ப
    }

// �����뢠�� ��ப� ���⭮ � 䠩�
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
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

// �㭪�� ��� ������ ��ப�  ����୮�� 䠩��
void Data_base::change_line_at_shifts_bin(const std::string& id, const std::string& fio, const std::string& date,
                                       const std::string& phone, const std::string& email, size_t number_line) {
    std::fstream file(shifts_file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }

    // ������ ��ப� (231 ���� ������)
    size_t line_size = 231;

    // ���� ��� ��ப�
    char buffer[line_size] = {0};  // ���樠�����㥬 ��ﬨ

    // ������塞 id (10 ����)
    std::memcpy(buffer, id.c_str(), std::min(id.size(), size_t(10)));

    // ������塞 fio (100 ����)
    std::memcpy(buffer + 10, fio.c_str(), std::min(fio.size(), size_t(100)));

    // ������塞 date (10 ����)
    std::memcpy(buffer + 10 + 100, date.c_str(), std::min(date.size(), size_t(10)));

    // ������塞 phone (11 ����)
    std::memcpy(buffer + 10 + 100 + 10, phone.c_str(), std::min(phone.size(), size_t(11)));

    // ������塞 email (100 ����)
    std::memcpy(buffer + 10 + 100 + 10 + 11, email.c_str(), std::min(email.size(), size_t(100)));
    size_t offset = (number_line-1)*line_size;
    file.seekp(offset);
    // �����뢠�� ��ப� � 䠩�
    file.write(buffer, line_size);  // ������ � ������ 䠩�
    if (!file.good()) {
        std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
    }

    file.close();
}

void Data_base::edit_id(const std::string& old_id,const std::string& new_id){
    for (char c: new_id){
        if(!std::isdigit(c)){
            std::cerr<<"Id ������ ���� 楫� �᫮�.";
            return;
        }
    }
    if (find_line_number_in_hash_id(new_id).first!=-1){
        std::cout<<"����� id 㦥 ���� � ⠡���.";
        return;
    }
    std::pair<int, int> info = find_line_number_in_hash_id(old_id);
    std::vector<std::string> elements = read_fixed_line(info.first);
    change_line_at_shifts_bin(new_id, elements[1], elements[2], elements[3], elements[4],info.first);

    std::fstream file(hash_id_file_name, std::ios::binary | std::ios::in | std::ios::out);  // ����⨥ 䠩�� � ०��� �⥭��/�����
    if (!file.is_open()) {
        std::cerr << "�� 㤠���� ������ 䠩�!" << std::endl;
        return;
    }
    size_t line_size = 17;
    // ���� ��� �⥭�� ��ப�
    char buffer[line_size] = {0};
    // ��砫쭮� ᬥ饭��
    size_t offset = info.second;
    std::memset(buffer, 0, line_size);  // �����塞 ��ப� �� ������
    file.seekp(offset);
    file.write(buffer, line_size);
    if (!file.good()) {
        std::cerr << "�訡�� ����� � 䠩�!" << std::endl;
    }
    file.close();
    add_line_at_hash_id_bin(new_id, std::to_string(info.first));
}

void Data_base::edit_name(const std::string& id,const std::string& new_name){
    std::regex fio_regex("^[�-�-���]+\\s+[�-�-���]+(\\s+[�-�-���]+)?$|^[a-zA-Z]+\\s+[a-zA-Z]+(\\s+[a-zA-Z]+)?$");
    if(!std::regex_match(new_name, fio_regex)){
        std::cerr<<"��� ����ᠭ� �����४⭮.";
        return;
    }
    std::pair<int,int> info_about_hash_id = find_line_number_in_hash_id(id);
    if (info_about_hash_id.first==-1){
        std::cout<<"������ id ��� � ⠡���.";
        return;
    }
    std::vector<std::string> elements = read_fixed_line(info_about_hash_id.first);

    change_line_at_shifts_bin(elements[0], new_name, elements[2], elements[3], elements[4],info_about_hash_id.first);
    delete_line_from_hash_name(info_about_hash_id.first, elements[1]);
    add_line_at_hash_name_bin(new_name, std::to_string(info_about_hash_id.first));
}

void Data_base::edit_date(const std::string &id, const std::string &new_date) {
    //�஢�ઠ date
    // �����୮� ��ࠦ���� ��� �ଠ� DD.MM.YYYY
    std::regex date_pattern(R"(\d{2}\.\d{2}\.\d{4}$)");

    // �஢��塞, ᮮ⢥����� �� ��ப� �ଠ�� ����
    if (!std::regex_match(new_date, date_pattern)) {
        std::cerr<<"��� ������ ���� ����ᠭ� � �ଠ� DD.MM.YYYY";
        return;
    }

    std::vector<std::string> date = split(new_date, '.');
    int day = std::stoi(date[0]);
    int month = std::stoi(date[1]);
    int year = std::stoi(date[2]);

    // ���ᨢ � ������⢮� ���� � ����� ��� ���筮�� ����
    std::vector<int> days_in_month = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // �஢�ઠ �� ��᮪��� ��� ��� 䥢ࠫ�
    if (isLeapYear(year)) {
        days_in_month[1] = 29;  // ���ࠫ� ����� 29 ���� � ��᮪��� ���
    }

    // �஢�ઠ ���४⭮�� ����� � ���
    if (month < 1 || month > 12) {
        std::cerr<<"����� ������ ���� �� 1 �� 12.";
        return;
    }

    if (day < 1 || day > days_in_month[month - 1]) {
        std::cerr<<"���ࠢ��쭮� ���-�� ���� � �����.";
        return;
    }
    std::pair<int,int> info_about_hash_id = find_line_number_in_hash_id(id);
    if (info_about_hash_id.first==-1){
        std::cout<<"������ id ��� � ⠡���.";
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
        std::cerr<<"������ ����� �� �������.";
        return;
    }
    std::pair<int,int> info_about_hash_id = find_line_number_in_hash_id(id);
    if (info_about_hash_id.first==-1){
        std::cout<<"������ id ��� � ⠡���.";
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
        std::cerr<<"����� ����� �� �������";
        return;
    }
    std::pair<int,int> info_about_hash_id = find_line_number_in_hash_id(id);
    if (info_about_hash_id.first==-1){
        std::cout<<"������ id ��� � ⠡���.";
        return;
    }
    std::vector<std::string> elements = read_fixed_line(info_about_hash_id.first);

    change_line_at_shifts_bin(elements[0], elements[1], elements[2], elements[3], new_mail,info_about_hash_id.first);
    delete_line_from_hash_mail(info_about_hash_id.first, elements[4]);
    add_line_at_hash_mail_bin(new_mail, std::to_string(info_about_hash_id.first));
}


