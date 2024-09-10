#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>

#include "lib/nlohmann/json.hpp"

using json = nlohmann::json;

// Forward declarations
json decode_bencoded_value(const std::string &encoded_value, size_t &position);

json decode_bencoded_string(const std::string &encoded_string, size_t &position)
{
    // Example: "5:hello" -> "hello"
    size_t length_prefix = encoded_string.find(':', position);
    if (length_prefix != std::string::npos)
    {
        std::string string_size_str = encoded_string.substr(position, length_prefix - position);
        int64_t string_size_int = std::atoll(string_size_str.c_str());
        position = length_prefix + 1 + string_size_int; // Update position
        std::string str = encoded_string.substr(length_prefix + 1, string_size_int);
        return json(str);
    }
    else
    {
        throw std::runtime_error("Invalid encoded value: " + encoded_string);
    }
}

json decode_bencoded_integer(const std::string &encoded_value, size_t &position)
{
    position++; // Skip 'i'
    size_t end = encoded_value.find('e', position);
    if (end == std::string::npos)
    {
        throw std::invalid_argument("Invalid bencoded integer");
    }
    std::string integer_str = encoded_value.substr(position, end - position);
    position = end + 1; // Move past 'e'
    return std::stoll(integer_str);
}

json decode_bencoded_list(const std::string &encoded_value, size_t &position)
{
    position++; // Skip 'l'
    json list = json::array();
    while (encoded_value[position] != 'e')
    {
        list.push_back(decode_bencoded_value(encoded_value, position));
    }
    position++; // Skip 'e'
    return list;
}

json decode_bencoded_value(const std::string &encoded_value, size_t &position)
{
    if (std::isdigit(encoded_value[position]))
    {
        return decode_bencoded_string(encoded_value, position);
    }
    else if (encoded_value[position] == 'i')
    {
        return decode_bencoded_integer(encoded_value, position);
    }
    else if (encoded_value[position] == 'l')
    {
        return decode_bencoded_list(encoded_value, position);
    }
    else
    {
        throw std::runtime_error("Unhandled encoded value: " + encoded_value);
    }
}

json decode_bencoded_value(const std::string &encoded_value)
{
    size_t position = 0;
    return decode_bencoded_value(encoded_value, position);
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
