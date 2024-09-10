#include <cctype>
#include <charconv>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include "lib/sha1.hpp"
#include "lib/nlohmann/json.hpp"
using json = nlohmann::json;
int64_t read_int64_t(const char *begin, const char *end)
{
    int64_t number;
    auto [_, ec] = std::from_chars(begin, end, number);
    if (ec != std::errc())
    {
        std::cout << &begin << ":" << &end << std::endl;
        std::cout << begin << std::endl;
        throw std::runtime_error("Invalid number");
    }
    return number;
}
using decoded = std::pair<json, std::string_view>;
decoded decode_next(std::string_view sv);
// Example: "i52e" -> 52
// Example: "i-52e" -> -52
decoded decode_integer(std::string_view sv)
{
    auto e_index = sv.find('e');
    if (e_index != std::string::npos)
    {
        auto data = sv.data();
        sv.remove_prefix(e_index + 1);
        return std::make_pair(json(read_int64_t(data + 1, data + e_index)), sv);
    }
    else
    {
        throw std::runtime_error("Invalid encoded value: " + std::string(sv));
    }
}
// Example: "5:hello" -> "hello"
decoded decode_string(std::string_view sv)
{
    auto colon_index = sv.find(':');
    if (colon_index != std::string::npos)
    {
        auto data = sv.data();
        auto length = read_int64_t(data, data + colon_index);
        std::string_view value(data + colon_index + 1, length);
        sv.remove_prefix(colon_index + length + 1);
        return std::make_pair(json(std::string(value)), sv);
    }
    else
    {
        throw std::runtime_error("Invalid encoded value: " + std::string(sv));
    }
}
template <typename T>
concept Integral = std::integral<T>;
template <Integral T>
std::string encode(T val)
{
    return "i" + std::to_string(val) + "e";
}
std::string encode(const std::string &val)
{
    return std::to_string(val.length()) + ":" + val;
}
template <typename T>
std::string encode_pair(const std::string &key, T value)
{
    return encode(key) + encode(value);
}
#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
decoded decode_list(std::string_view sv)
{
    json arr = json::array();
    sv.remove_prefix(1);
    while (sv[0] != 'e')
    {
        auto [val, rest] = decode_next(sv);
        arr.push_back(val);
        sv = rest;
    }
    sv.remove_prefix(1);
    return std::make_pair(arr, sv);
}
decoded decode_dict(std::string_view sv)
{
    json obj = json::object();
    sv.remove_prefix(1);
    while (sv[0] != 'e')
    {
        auto [key, rest1] = decode_string(sv);
        auto [value, rest2] = decode_next(rest1);
        obj[key] = value;
        sv = rest2;
    }
    sv.remove_prefix(1);
    return std::make_pair(obj, sv);
}
decoded decode_next(std::string_view sv)
{
    if (std::isdigit(sv[0]))
    {
        return decode_string(sv);
    }
    else if (sv[0] == 'i')
    {
        return decode_integer(sv);
    }
    else if (sv[0] == 'l')
    {
        return decode_list(sv);
    }
    else if (sv[0] == 'd')
    {
        return decode_dict(sv);
    }
    else
    {
        throw std::runtime_error("Unhandled encoded value: " + std::string(sv));
    }
}
#pragma clang diagnostic pop
json decode_bencoded_value(const std::string &encoded_value)
{
    return decode_next(encoded_value).first;
}
struct Info
{
    size_t length;
    std::string name;
    size_t piece_length;
    std::string pieces;
    explicit Info(json obj) : length{obj["length"]}, name{obj["name"]}, piece_length{obj["piece length"]}, pieces{obj["pieces"]} {}
    std::string encode() const
    {
        return "d" + encode_pair("length", length) + encode_pair("name", name) + encode_pair("piece length", piece_length) + encode_pair("pieces", pieces) + "e";
    }
    SHA1 get_sha1() const
    {
        return sha1(encode());
    }
};
struct Torrent
{
    std::string announce;
    Info info;
    explicit Torrent(json obj) : announce{obj["announce"]}, info{obj["info"]} {}
};
void decode(const std::string &encoded_value)
{
    json decoded_value = decode_bencoded_value(encoded_value);
    std::cout << decoded_value.dump() << std::endl;
}
void info(const std::string &torrent_path)
{
    std::ifstream file(torrent_path, std::ios::binary);
    std::string str;
    file.seekg(0, std::ios::end);
    str.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    str.assign((std::istreambuf_iterator<char>(file)),
               std::istreambuf_iterator<char>());
    Torrent torrent(decode_bencoded_value(str));
    std::cout << "Tracker URL: " << torrent.announce << std::endl;
    std::cout << "Length: " << torrent.info.length << std::endl;
    std::cout << "Info Hash: " << torrent.info.get_sha1() << std::endl;
}

int main(int argc, char *argv[])
{
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
            std::cerr << "Usage: " << argv[0] << " decode <encoded_value>"
                      << std::endl;
            return 1;
        }
        // You can use print statements as follows for debugging, they'll be visible
        // when running tests.
        // std::cout << "Logs from your program will appear here!" << std::endl;
        // Uncomment this block to pass the first stage
        decode(argv[2]);
    }
    else if (command == "info")
    {
        if (argc < 3)
        {
            std::cerr << "Usage: " << argv[0] << " info <torrent_file>"
                      << std::endl;
            return 1;
        }
        info(argv[2]);
    }
    else
    {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }
    return 0;
}