#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include "lib/nlohmann/json.hpp"
using json = nlohmann::json;
std::pair<json, size_t> decode_bencoded_value(const std::string& encoded_value);
std::string sha1(const std::string& input) {
    uint32_t h0 = 0x67452301;
    uint32_t h1 = 0xEFCDAB89;
    uint32_t h2 = 0x98BADCFE;
    uint32_t h3 = 0x10325476;
    uint32_t h4 = 0xC3D2E1F0;
    uint64_t bit_length = input.size() * 8;
    std::vector<uint8_t> data(input.begin(), input.end());
    data.push_back(0x80);
    while (data.size() % 64 != 56) {
        data.push_back(0);
    }
    for (int i = 7; i >= 0; --i) {
        data.push_back(bit_length >> (i * 8));
    }
    for (std::size_t i = 0; i < data.size(); i += 64) {
        uint32_t w[80];
        for (int j = 0; j < 16; ++j) {
            w[j] = (data[i + j * 4] << 24) | (data[i + j * 4 + 1] << 16) | (data[i + j * 4 + 2] << 8) | data[i + j * 4 + 3];
        }
        for (int j = 16; j < 80; ++j) {
            w[j] = (w[j - 3] ^ w[j - 8] ^ w[j - 14] ^ w[j - 16]);
            w[j] = (w[j] << 1) | (w[j] >> 31);
        }
        uint32_t a = h0;
        uint32_t b = h1;
        uint32_t c = h2;
        uint32_t d = h3;
        uint32_t e = h4;
        for (int j = 0; j < 80; ++j) {
            uint32_t f, k;
            if (j < 20) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            } else if (j < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (j < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[j];
            e = d;
            d = c;
            c = (b << 30) | (b >> 2);
            b = a;
            a = temp;
        }
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(8) << h0
        << std::setw(8) << h1
        << std::setw(8) << h2
        << std::setw(8) << h3
        << std::setw(8) << h4;
    return oss.str();
}
std::pair<json, size_t> decode_bencoded_string(const std::string& encoded_value) {
    size_t colon_index = encoded_value.find(':');
    if (colon_index != std::string::npos) {
        std::string number_string = encoded_value.substr(0, colon_index);
        int64_t number = std::atoll(number_string.c_str());
        std::string str = encoded_value.substr(colon_index + 1, number);
        return std::pair(json(str), colon_index + 1 + number);
    } else {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }
}
std::pair<json, size_t> decode_bencoded_integer(const std::string& encoded_value) {
    size_t number_end = encoded_value.find('e');
    return std::pair(json(std::atoll(encoded_value.substr(1, number_end).c_str())), number_end + 1);
}
std::pair<json, size_t> decode_bencoded_list(const std::string& encoded_value) {
    std::vector<json> list;
    size_t index = 1;
    while (encoded_value[index] != 'e') {
        auto [value, offset] = decode_bencoded_value(encoded_value.substr(index));
        index += offset;
        list.push_back(value);
    }
    return std::pair(json(list), index + 1);
}
std::pair<json, size_t> decode_bencoded_dictionary(const std::string& encoded_value) {
    std::map<json, json> map;
    size_t index = 1;
    while (encoded_value[index] != 'e') {
        auto [key, offset] = decode_bencoded_value(encoded_value.substr(index));
        index += offset;
        auto [value, another] = decode_bencoded_value(encoded_value.substr(index));
        index += another;
        map.insert(std::pair(key, value));
    }
    return std::pair(json(map), index + 1);
}
std::pair<json, size_t> decode_bencoded_value(const std::string& encoded_value) {
    if (std::isdigit(encoded_value[0])) {
        return decode_bencoded_string(encoded_value);
    }
    else if (encoded_value[0] == 'i') {
        return decode_bencoded_integer(encoded_value);
    }
    else if (encoded_value[0] == 'l') {
        return decode_bencoded_list(encoded_value);
    }
    else if (encoded_value[0] == 'd') {
        return decode_bencoded_dictionary(encoded_value);
    }
    else {
        throw std::runtime_error("Unhandled encoded value: " + encoded_value);
    }
}
json decode_bencoded(const std::string& encoded_value) {
    return decode_bencoded_value(encoded_value).first;
}
std::string to_bencode(json j) {
    std::string result;
    if (j.is_object()) {
        std::map<json, json> map = j;
        result.push_back('d');
        for (auto& [key, value] : map) {
            result += to_bencode(key);
            result += to_bencode(value);
        }
        result.push_back('e');
    }
    else if (j.is_array()) {
        std::vector<json> jsonV = j;
        result.push_back('l');
        for (auto value : jsonV)
        {
            result += to_bencode(value);
        }
        result.push_back('e');
    }
    else if (j.is_number()) {
        result.push_back('i');
        long long num = j;
        std::string chars = std::to_string(num);
        for (auto c : chars)
        {
            result.push_back(c);
        }
        result.push_back('e');
    }
    else if (j.is_string()) {
        std::string word = j;
        long long length = word.length();
        std::string chars = std::to_string(length);
        for (auto c : chars) {
            result.push_back(c);
        }
        result.push_back(':');
        for (auto c : word) {
            result.push_back(c);
        }
    }
    else {
        throw std::runtime_error("JSON type not handled");
    }
    return std::string(result.begin(), result.end());
}
json read_torrent_file(const std::string& path) {
    std::ifstream file(path, std::ios::in);
    if(!file.is_open()) {
        std::cout << "Error opening file" << std::endl;
        return json();
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return json(decode_bencoded(ss.str()));
}
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
return 1;
    }
    std::string command = argv[1];
    if (command == "decode") {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
            return 1;
        }
        std::string encoded_value = argv[2];
        json decoded_value = decode_bencoded(encoded_value);
        std::cout << decoded_value.dump() << std::endl;
    }
    else if (command == "info") {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " info <torrent_file>" << std::endl;
            return 1;
        }
        auto torrent_info = read_torrent_file(argv[2]);
        std::cout << "Tracker URL: " + torrent_info.at("announce").get<std::string>() << std::endl;
        std::cout << "Length: " + std::to_string(torrent_info.at("info").at("length").get<int>()) << std::endl;
        std::cout << "Info Hash: " << sha1(to_bencode(torrent_info.at("info")));
        std::cout << "Piece Length: " << std::to_string(torrent_info.at("info").at("piece length").get<int>()) << std::endl;
        std::cout << "Piece Hashes:" << std::endl;
        for (std::size_t i = 0; i < torrent_info.at("info").at("pieces").get<std::string>().length(); i += 20) {
            std::string piece = torrent_info.at("info").at("pieces").get<std::string>().substr(i, 20);
            std::stringstream ss;
            for (unsigned char byte : piece) {
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
            }
            std::cout << ss.str() << std::endl;
        }
        return 0;
    }
    else {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }
    return 0;
}