#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>

void shannonFanoEncode(std::vector<std::string>& alphabet, std::vector<double>& probs, std::vector<std::string>& codes, int start, int end){
    if (start >= end) return;

    double total = 0;
    for (int i = start; i <= end; i++) {
        total += probs[i];
    }

    double left_sum = 0;
    double min_diff = total;
    int split_index = start;

    for (int i = start; i <= end; i++) {
        left_sum += probs[i];
        double right_sum = total - left_sum;
        double difference = std::abs(left_sum - right_sum);

        if (difference < min_diff) {
            min_diff = difference;
            split_index = i;
        } else { // если разница начала увеличиваться - найден оптимальный вариант
            break;
        }
    }

    // присваиваем коды: левой (старшей) части "1", правой (младшей) "0"
    for (int i = start; i <= split_index; i++) {
        codes[i] += "1";
    }
    for (int i = split_index + 1; i <= end; i++) {
        codes[i] += "0";
    }

    //обработка обеих частей
    shannonFanoEncode(alphabet, probs, codes, start, split_index);
    shannonFanoEncode(alphabet, probs, codes, split_index + 1, end);
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    std::string content;
    if (file) {
        content = std::string((std::istreambuf_iterator<char>(file)), 
                             std::istreambuf_iterator<char>());
    }
    return content;
}

int main(){
    //Задание 1

    std::vector<std::string> alphabet {"z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8"};
    std::vector<double> probs {0.26 , 0.19, 0.14, 0.11, 0.1, 0.08, 0.07, 0.05};
    std::vector<std::string> codes(8, "");

    shannonFanoEncode(alphabet, probs, codes, 0, probs.size() - 1);
    double H = 0.0;
    double L_avg = 0.0;

    std::cout << "Задание 1\n";
    for (size_t i = 0; i < alphabet.size(); i++) {
        double p = probs[i];
        int l = codes[i].length();
        
        H += -p * log2(p);
        L_avg += p * l;
        
        std::cout << alphabet[i] << "  " << p << "  "<< codes[i] << "\n";
    }
    double R = 1 - H/3;

    std::cout << "Энтропия = " << H <<"\n";
    std::cout << "Средняя длина = " << L_avg << "\n";
    std::cout << "Избыточность = " << R << "\n";

    alphabet.clear();
    probs.clear();
    codes.clear();

    //Задание 2
    alphabet = {"z1z1z1", "z1z1z2", "z1z2z1", "z2z1z1", "z1z2z2", "z2z1z2", "z2z2z1", "z2z2z2"};
    probs = {0.729, 0.081, 0.081, 0.081, 0.009, 0.009, 0.009, 0.001};
    codes = std::vector<std::string>(8, "");
    L_avg = 0.0;

    std::cout << "\nЗадание 2\n";
    shannonFanoEncode(alphabet, probs, codes, 0, probs.size() - 1);
    for (size_t i = 0; i < alphabet.size(); i++) {
        double p = probs[i];
        int l = codes[i].length();

        L_avg += p * l;
        
        std::cout << alphabet[i] << "  " << p << "  "<< codes[i] << "\n";
    }
    
    std::cout << "Средняя длина = " << L_avg << "\n";

    // Задание 3 
    std::cout << "\nЗадание 3\n";
    std::string text = readFile("ex3.txt");

    std::cout << "Текст: " << text << "\n";
    std::cout << "Длина: " << text.length() << " символов\n\n";
    
    //анализ блоков
    for (int blockSize = 1; blockSize <= 3; blockSize++) {
        std::cout << "=== Блоки по " << blockSize << " символа ===\n";
        
        // Считаем частоты блоков
        std::map<std::string, int> blockCounts;
        int totalBlocks = 0;
        
        for (size_t i = 0; i <= text.length() - blockSize; i++) {
            std::string block = text.substr(i, blockSize);
            blockCounts[block]++;
            totalBlocks++;
        }
        
        std::cout << "  Всего блоков: " << totalBlocks << "\n";
        std::cout << "  Уникальных: " << blockCounts.size() << "\n";

        // Подготавливаем векторы для кодирования
        alphabet.clear();
        probs.clear();
        codes.clear();
        codes = std::vector<std::string>(8, "");

        for (const auto& pair : blockCounts) {
            alphabet.push_back(pair.first);
            probs.push_back((double)pair.second / totalBlocks);
            codes.push_back("");
        }

        //сортировка по убыванию
        std::vector<size_t> indices(alphabet.size());
        for (size_t i = 0; i < indices.size(); ++i) {
            indices[i] = i;
        }
        std::sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
            return probs[i] > probs[j]; // по убыванию
        });

        // Переставляем элементы в соответствии с сортировкой
        std::vector<std::string> sorted_alphabet, sorted_codes;
        std::vector<double> sorted_probs;
        for (size_t i : indices) {
            sorted_alphabet.push_back(alphabet[i]);
            sorted_probs.push_back(probs[i]);
            sorted_codes.push_back("");
        }

        shannonFanoEncode(sorted_alphabet, sorted_probs, sorted_codes, 0, sorted_alphabet.size() - 1);

        double H = 0.0;
        double L_avg = 0.0;

        for (size_t i = 0; i < sorted_probs.size(); i++) {
            double p = sorted_probs[i];
            int l = sorted_codes[i].length();
            if (p > 0) {
                H += -p * log2(p);
                L_avg += p * l;
            }
        }

        //избыточность ансамбля
        int uniqueBlocks = sorted_alphabet.size();
        double H_max = (uniqueBlocks > 1) ? log2(uniqueBlocks) : 0.0;
        double redundancy = (H_max > 0) ? (1.0 - H / H_max) : 0.0;

        //вывод таблицы кодов
        std::cout << "\n  Коды Шеннона-Фано:\n";
        std::cout << "  Блок\t\tВероятность\tКод\n";
        for (size_t i = 0; i < sorted_alphabet.size(); i++) {
            std::cout << "  " << sorted_alphabet[i] 
                      << "\t\t" << sorted_probs[i] 
                      << "\t\t" << sorted_codes[i] << "\n";
        }

        std::cout << "\n  Энтропия: " << H << "\n";
        std::cout << "  Избыточность: " << redundancy << "\n\n";
    }
}