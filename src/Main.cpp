#include <cctype>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include "lib/nlohmann/json.hpp"

using json = nlohmann::json;

json decode_bencoded_value(std::string& encoded_value) {
    if (std::isdigit(encoded_value[0])) {
        // Example: "5:hello" -> "hello"
        size_t colon_index = encoded_value.find(':');
        if (colon_index != std::string::npos) {
            std::string number_string = encoded_value.substr(0, colon_index);
            int64_t number = std::atoll(number_string.c_str());
            std::string str = encoded_value.substr(colon_index + 1, number);
            encoded_value = encoded_value.substr(colon_index+1+number, std::string::npos);
            return json(str);
        } else {
            throw std::runtime_error("Invalid encoded value: " + encoded_value);
        }
    } else if (encoded_value[0] == 'i') {
        size_t end_index = encoded_value.find('e');
        if (end_index != std::string::npos) {
            std::string number_string = encoded_value.substr(1, end_index);
            int64_t number = std::atoll(number_string.c_str());
            encoded_value = encoded_value.substr(end_index+1, std::string::npos);
            return json(number);
        } else {
            throw std::runtime_error("Invalid encoded value: " + encoded_value);
        }
    } else if (encoded_value[0] == 'l') {
        json list = json::array();
        encoded_value = encoded_value.substr(1, std::string::npos);
        while (true) {
            if (encoded_value[0] == 'e') break;
            json res = decode_bencoded_value(encoded_value);
            list.push_back(res);
        }
        encoded_value = encoded_value.substr(1, std::string::npos);
        return list;
    } else if (encoded_value[0] == 'd') {
        json dict = json::object();
        encoded_value = encoded_value.substr(1, std::string::npos);
        while (true) {
            if (encoded_value[0] == 'e') break;
            json key = decode_bencoded_value(encoded_value);
            if (not key.is_string()) {
                throw std::runtime_error("Invalid encoded value: " + encoded_value);
            }
            json val = decode_bencoded_value(encoded_value);
            dict[key] = val;
        }
        encoded_value = encoded_value.substr(1, std::string::npos);
        return dict;
    } else {
        throw std::runtime_error("Unhandled encoded value: " + encoded_value);
    }
}


json parse_torrent_file(const std::string &filename)
{
    std::ifstream ifs(filename);
    if (ifs)
    {
        ifs.seekg(0, ifs.end);
        int length = ifs.tellg();
        ifs.seekg(0, ifs.beg);
        char *buffer = new char[length];
        ifs.read(buffer, length);
        std::string torrent_data = buffer;
        return decode_bencoded_value(torrent_data);
    }
    throw std::runtime_error("Unable to find file: " + filename);
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
        // You can use print statements as follows for debugging, they'll be visible when running tests.
        // std::cout << "Logs from your program will appear here!" << std::endl;
        std::string encoded_value = argv[2];
        json decoded_value = decode_bencoded_value(encoded_value);
        std::cout << decoded_value.dump() << std::endl;
    } else if (command == "info") {
        std::string filename = argv[2];
        json decoded_data = parse_torrent_file(filename);
        std::string tracker_url;
        decoded_data["announce"].get_to(tracker_url);
        std::cout << "Tracker URL: " << tracker_url << std::endl;
        std::cout << "Length: " << decoded_data["info"]["length"] << std::endl;
    } else {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }
    return 0;
}