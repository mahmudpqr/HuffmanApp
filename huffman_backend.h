#pragma once

#include <string>
#include <vector>
#include <utility>

struct HuffmanResult {
    std::vector<std::pair<char, std::string>> codes;
    std::string encoded_bits;
};

bool encode_text(const std::string& text, HuffmanResult& result);

bool decode_text(const std::string& encoded_bits, std::string& decoded_text);

void clear_huffman_tree();