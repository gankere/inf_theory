#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdint>

std::string to_hex_byte(int n) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << n;
    std::string s = ss.str();
    if (s.length() == 1) s = "0" + s;
    return s;
}

int hex_byte_to_int(const std::string& s) {
    return std::stoi(s, nullptr, 16);
}

void Packing(std::string& str) {
    std::string result;
    int n = str.length();

    for (int i = 0; i < n; i += 2) {
        std::string current = str.substr(i, 2);
        int count = 1;

        while (i + 2 * count < n && str.substr(i + 2 * count, 2) == current) {
            count++;
        }

        int current_val = hex_byte_to_int(current);

        if (count == 1) {
            if (current_val >= 0xC0) {
                result += "C1";
                result += current;
            } else {
                result += current;
            }
        } else {
            if (count > 63) count = 63;
            int counter_byte = 0xC0 | count;
            result += to_hex_byte(counter_byte);
            result += current;
        }

        i += (count - 1) * 2;
    }

    str = result;
}

void Unpacking(std::string& str) {
    std::string result;
    int n = str.length();

    for (int i = 0; i < n; i += 2) {
        std::string current_byte = str.substr(i, 2);
        int val = hex_byte_to_int(current_byte);

        if (val >= 0xC0) {
            int count = val & 0x3F;
            if (i + 2 >= n) break;
            std::string value_byte = str.substr(i + 2, 2);

            for (int j = 0; j < count; j++) {
                result += value_byte;
            }

            i += 2;
        } else {
            result += current_byte;
        }
    }

    str = result;
}

std::vector<uint8_t> rle4_encode_row(const std::vector<uint8_t>& pixels) {
    std::vector<uint8_t> output;
    size_t n = pixels.size();
    size_t i = 0;

    while (i < n) {
        uint8_t current = pixels[i];
        size_t count = 1;
        
        //Считаем одинаковые пиксели
        while (i + count < n && pixels[i + count] == current && count < 255) {
            count++;
        }
        // Если нашли 3 или более одинаковых пикселей подряд - используем закодированный режим
        if (count >= 3) {
            output.push_back(static_cast<uint8_t>(count));
            output.push_back((current << 4) | current);
            i += count; // Перемещаемся вперед на длину обработанной последовательности
        } else {
            // Режим абсолютного кодирования для разноцветных последовательностей
            size_t abs_count = 1;
            // Определяем, сколько пикселей взять в абсолютный режим
            while (i + abs_count < n && abs_count < 255) {
                // Проверяем, не началась ли последовательность из 3 одинаковых
                if (abs_count >= 2 && 
                    pixels[i + abs_count] == pixels[i + abs_count - 1] &&
                    pixels[i + abs_count] == pixels[i + abs_count - 2]) {
                    break;
                }
                abs_count++;
            }

            output.push_back(0);
            output.push_back(static_cast<uint8_t>(abs_count));
            
            //пиксели парами в байты
            for (size_t j = 0; j < abs_count; j += 2) {
                uint8_t byte_val;
                if (j + 1 < abs_count) {
                    byte_val = (pixels[i + j] << 4) | pixels[i + j + 1];
                } else {
                    byte_val = pixels[i + j] << 4;
                }
                output.push_back(byte_val);
            }
            
            // Выравнивание до четного количества байтов
            if (((abs_count + 1) / 2) % 2 == 1) {
                output.push_back(0);
            }
            
            i += abs_count;
        }
    }

    output.push_back(0);
    output.push_back(0);
    return output;
}

#pragma pack(push, 1)
struct BMPHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BMPInfoHeader {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};

#pragma pack(pop)

bool compress_bmp_rle_simple(const std::string& input, const std::string& output) {
    std::cout << "Reading BMP file..." << std::endl;
    
    std::ifstream file(input, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open input file: " << input << std::endl;
        return false;
    }

    BMPHeader header;
    BMPInfoHeader info;

    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    file.read(reinterpret_cast<char*>(&info), sizeof(info));

    if (header.bfType != 0x4D42) {
        std::cerr << "Not a valid BMP file" << std::endl;
        return false;
    }

    if (info.biBitCount != 4) {
        std::cerr << "Only 4-bit BMP files are supported" << std::endl;
        return false;
    }

    std::cout << "Original BMP: " << info.biWidth << "x" << std::abs(info.biHeight) 
              << ", " << info.biBitCount << " bits" << std::endl;

    int width = info.biWidth;
    int height = std::abs(info.biHeight);
    bool isTopDown = (info.biHeight < 0);
    
    // Правильный расчет размера строки
    int row_size = ((width + 1) / 2 + 3) & ~3;

    std::vector<uint32_t> palette(16);// Read palette
    size_t palette_offset = sizeof(BMPHeader) + info.biSize;
    file.seekg(palette_offset);
    file.read(reinterpret_cast<char*>(palette.data()), 16 * 4);

    std::vector<uint8_t> image_data(row_size * height);
    file.seekg(header.bfOffBits);
    file.read(reinterpret_cast<char*>(image_data.data()), row_size * height);

    std::vector<uint8_t> compressed_data;

    std::cout << "Compressing " << height << " rows..." << std::endl;
    
    for (int y = 0; y < height; y++) {
        if (y % 50 == 0) {
            std::cout << "Row " << y << "/" << height << std::endl;
        }
        
        int row_index = y;
        
        std::vector<uint8_t> row_pixels;
        for (int x = 0; x < width; x++) {
            uint8_t byte = image_data[row_index * row_size + x / 2];
            uint8_t pixel;
            if (x % 2 == 0) {
                pixel = (byte >> 4) & 0x0F;
            } else {
                pixel = byte & 0x0F;
            }
            row_pixels.push_back(pixel);
        }
        
        auto encoded_row = rle4_encode_row(row_pixels);
        compressed_data.insert(compressed_data.end(), encoded_row.begin(), encoded_row.end());
    }

    std::cout << "Compression completed!" << std::endl;

    compressed_data.push_back(0);
    compressed_data.push_back(1);

    info.biCompression = 2; // BI_RLE4
    info.biSizeImage = compressed_data.size();
    
    info.biHeight = isTopDown ? -height : height; 
    info.biBitCount = 4;

    header.bfSize = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + 16 * 4 + compressed_data.size();
    header.bfOffBits = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + 16 * 4;

    //Запись сжатого BMP
    std::ofstream out(output, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot create output file: " << output << std::endl;
        return false;
    }

    out.write(reinterpret_cast<const char*>(&header), sizeof(header));
    out.write(reinterpret_cast<const char*>(&info), sizeof(info));
    out.write(reinterpret_cast<const char*>(palette.data()), 16 * 4);
    out.write(reinterpret_cast<const char*>(compressed_data.data()), compressed_data.size());

    std::cout << "RLE4 BMP saved as: " << output << std::endl;

    // --- Статистика ---
    int original_data_size = row_size * height;
    int compressed_data_size = compressed_data.size();
    int original_file_size = header.bfSize;
    int compressed_file_size = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + 16 * 4 + compressed_data.size();

    std::cout << "\n=== COMPRESSION RESULTS ===" << std::endl;
    std::cout << "Original pixel data: " << original_data_size << " bytes" << std::endl;
    std::cout << "Compressed data: " << compressed_data_size << " bytes" << std::endl;
    std::cout << "Original file size: " << original_file_size << " bytes" << std::endl;
    std::cout << "Compressed file size: " << compressed_file_size << " bytes" << std::endl;

    if (original_data_size > 0) {
        double data_ratio = (1.0 - (double)compressed_data_size / original_data_size) * 100.0;
        double file_ratio = (1.0 - (double)compressed_file_size / original_file_size) * 100.0;
        std::cout << "Data compression ratio: " << data_ratio << "%" << std::endl;
        std::cout << "File compression ratio: " << file_ratio << "%" << std::endl;
    }

    return true;
}

int main() {
    std::cout << "=== BMP RLE4 Compressor ===" << std::endl;

    std::string input_file, output_file;

    std::cout << "Enter input BMP file name: ";
    std::cin >> input_file;

    std::cout << "Enter output BMP file name: ";
    std::cin >> output_file;
    
    if (!output_file.empty() && output_file.find('.') == std::string::npos) {
        output_file += ".bmp";
    }

    std::cout << std::endl;
    
    if (!compress_bmp_rle_simple(input_file, output_file)) {
        std::cerr << "Compression failed!" << std::endl;
        return 1;
    }

    // Демонстрация упаковки/распаковки
    std::cout << "\n" << std::string(50, '=') << std::endl;
    
    std::cout << "\nPacking: " << std::endl;
    std::string str1 = "01BD2F154FDBE01015FDE01ED1001A0A101C61EF01D0113333333333333334";
    std::cout << "Before packing: " << str1 << std::endl;
    Packing(str1);
    std::cout << "After packing:  " << str1 << std::endl;

    std::cout << "\nUnpacking: " << std::endl;
    std::string str2 = "D0DEA0C1D0FF363523C6C1C5AA";
    std::cout << "Before unpacking: " << str2 << std::endl;
    Unpacking(str2);
    std::cout << "After unpacking:  " << str2 << std::endl;

    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Program completed successfully!" << std::endl;

    return 0;
}