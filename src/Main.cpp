#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>

#include "lib/nlohmann/json.hpp"

using json = nlohmann::json;

std::pair<std::string, std::string> extract_string(const std::string& data) {
    size_t colon_index = data.find(':');
    if (colon_index == std::string::npos) {
        throw std::runtime_error("Invalid bencoded value: " + data);
    }

    int64_t length = std::atoll(data.substr(0, colon_index).c_str());
    std::string extracted_str = data.substr(colon_index + 1, length);
    std::string remaining_data = data.substr(colon_index + 1 + length);

    return {extracted_str, remaining_data};  // Return the extracted string and remaining data
}

// Decode function that handles strings and integers from bencoded data
std::pair<json, std::string> decode(const std::string& data) {
    if (std::isdigit(data[0])) {  // Byte string like "5:hello"
        auto extracted = extract_string(data);
        std::string decoded_str = extracted.first;  // Extracted string part
        std::string remaining_data = extracted.second;  // Remaining data after extraction
        return {json(decoded_str), remaining_data};  // Return JSON string and remaining data
    } else if (data[0] == 'i' && data[data.size() - 1] == 'e') {  // Integer like "i123e"
        std::string int_str = data.substr(1, data.size() - 2);
        return {json(std::atoll(int_str.c_str())), ""};  // Return JSON integer and empty string for remaining data
    } else {
        throw std::runtime_error("Unhandled bencoded value: " + data);
    }
}

// Main function that decodes the bencoded value
json decode_bencoded_value(const std::string& encoded_value) {
    auto result = decode(encoded_value);
    json decoded_json = result.first;  // The decoded value in JSON format
    std::string remaining_data = result.second;  // Remaining data after decoding (not used further in this case)
    return decoded_json;
}

int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

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

        // Uncomment this block to pass the first stage

        std::string encoded_value = argv[2]; // this is the change
        json decoded_value = decode_bencoded_value(encoded_value);
        std::cout << decoded_value.dump() << std::endl;
    } else {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}
