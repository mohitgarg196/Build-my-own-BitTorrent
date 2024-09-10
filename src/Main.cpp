#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>
#include <tuple>
#include "lib/nlohmann/json.hpp"
using json = nlohmann::json;
std::tuple<json, std::string> decode_bencoded_value(const std::string &encoded_value) {
    if (std::isdigit(encoded_value[0])) {
        // Example: "5:hello" -> "hello"
        size_t colon_index = encoded_value.find(':');
        if (colon_index != std::string::npos) {
            std::string number_string = encoded_value.substr(0, colon_index);
//            int64_t number = std::atoll(number_string.c_str()); // atoll is used to convert string to long long
            int64_t number = std::strtoll(number_string.c_str(), nullptr, 10);
            std::string str = encoded_value.substr(colon_index + 1, number);
            return std::make_tuple(json(str), encoded_value.substr(colon_index + 1 + number));
        } else {
            throw std::runtime_error("Invalid encoded value: " + encoded_value);
        }
    } else if (encoded_value[0] == 'i') {
        // Example: "i42e" -> 42
        size_t e_index = encoded_value.find('e');
        if (e_index == std::string::npos) {
            throw std::runtime_error("Invalid encoded value: " + encoded_value);
        }
        std::string number_string = encoded_value.substr(1, e_index - 1);
        int64_t number = std::strtoll(number_string.c_str(), nullptr, 10);
        return std::make_tuple(json(number), encoded_value.substr(e_index + 1));
    } else if (encoded_value[0] == 'l') {
        // Example: "l5:helloe" -> ["hello"]
        json list = json::array();
        // remove the 'l' from the beginning
        std::string rest = encoded_value.substr(1);
        while (rest[0] != 'e') {
            json value;
            std::tie(value, rest) = decode_bencoded_value(rest);
            list.push_back(value);
        }
        return std::make_tuple(list, rest.substr(1));
    } else if (encoded_value[0] == 'd') {
        // Example: "d3:cow3:moo4:spam4:eggse" -> {"cow": "moo", "spam": "eggs"}
        json dict = json::object();
        std::string rest = encoded_value.substr(1);
        while (rest[0] != 'e') {
            json key, value;
            std::tie(key, rest) = decode_bencoded_value(rest);
            std::tie(value, rest) = decode_bencoded_value(rest);
            dict[key.get<std::string>()] = value;
        }
        return std::make_tuple(dict, rest.substr(1));
    } else {
        throw std::runtime_error("Unhandled encoded value: " + encoded_value);
    }
}

int main(int argc, char *argv[])
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
        return 1;
    }

    std::string command = argv[1];

    if (command == "decode")
    {
        if (argc < 3)
        {
            std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
            return 1;
        }
        // You can use print statements as follows for debugging, they'll be visible when running tests.
        // std::cout << "Logs from your program will appear here!" << std::endl;

        // Uncomment this block to pass the first stage

        std::string encoded_value = argv[2]; // this is the change
        json decoded_value = decode_bencoded_value(encoded_value);
        std::cout << decoded_value.dump() << std::endl;
    }
    else
    {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}
