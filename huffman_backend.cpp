#include "huffman_backend.h"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>

using namespace std;


struct Node {
    char ch;
    int freq;
    Node *left, *right;

    Node(char c, int f)
        : ch(c), freq(f), left(nullptr), right(nullptr) {}

    Node(int f, Node* l, Node* r)
        : ch('\0'), freq(f), left(l), right(r) {}
};


static Node* huffman_root = nullptr;


unordered_map<char, int> get_freq_map(const string& str) {
    unordered_map<char, int> freq_map;

    for (char ch : str) {
        freq_map[ch]++;
    }

    return freq_map;
}


bool compare_nodes(const Node* x, const Node* y) {
    return x->freq > y->freq;
}


vector<Node*> generate_nodes(
    const unordered_map<char, int>& freq_map
) {
    vector<Node*> nodes;

    for (const auto& [ch, freq] : freq_map) {
        nodes.push_back(new Node(ch, freq));
    }

    sort(nodes.begin(), nodes.end(), compare_nodes);

    return nodes;
}


Node* build_tree(vector<Node*>& nodes) {
    if (nodes.empty())
        return nullptr;

    while (nodes.size() > 1) {
        Node* left = nodes.back();
        nodes.pop_back();

        Node* right = nodes.back();
        nodes.pop_back();

        Node* parent = new Node(
            left->freq + right->freq,
            left,
            right
        );

        auto it = lower_bound(
            nodes.begin(),
            nodes.end(),
            parent,
            compare_nodes
        );

        nodes.insert(it, parent);
    }

    return nodes.back();
}


void delete_tree(Node* root) {
    if (!root)
        return;

    delete_tree(root->left);
    delete_tree(root->right);

    delete root;
}


void generate_code(
    Node* root,
    string code,
    unordered_map<char, string>& code_map
) {
    if (!root)
        return;

    if (!root->left && !root->right) {
        code_map[root->ch] =
            code.empty() ? "0" : code;

        return;
    }

    generate_code(
        root->left,
        code + "0",
        code_map
    );

    generate_code(
        root->right,
        code + "1",
        code_map
    );
}


void clear_huffman_tree() {
    if (huffman_root) {
        delete_tree(huffman_root);
        huffman_root = nullptr;
    }
}


bool encode_text(
    const string& text,
    HuffmanResult& result
) {
    result.codes.clear();
    result.encoded_bits.clear();

    if (text.empty())
        return false;

    clear_huffman_tree();

    unordered_map<char, int> freq_map =
        get_freq_map(text);

    vector<Node*> nodes =
        generate_nodes(freq_map);

    huffman_root =
        build_tree(nodes);

    if (!huffman_root)
        return false;

    unordered_map<char, string> code_map;

    generate_code(
        huffman_root,
        "",
        code_map
    );

    for (const auto& [ch, code] : code_map) {
        result.codes.push_back({ch, code});
    }

    sort(
        result.codes.begin(),
        result.codes.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        }
    );

    for (char ch : text) {
        result.encoded_bits += code_map[ch];
    }

    return true;
}


bool decode_text(
    const string& encoded_bits,
    string& decoded_text
) {
    decoded_text.clear();

    if (!huffman_root)
        return false;

    if (encoded_bits.empty())
        return false;

    if (!huffman_root->left &&
        !huffman_root->right) {

        for (char bit : encoded_bits) {
            if (bit != '0')
                return false;

            decoded_text += huffman_root->ch;
        }

        return true;
    }

    Node* current = huffman_root;

    for (char bit : encoded_bits) {

        if (bit == '0') {
            current = current->left;
        }
        else if (bit == '1') {
            current = current->right;
        }
        else {
            decoded_text.clear();
            return false;
        }

        if (!current) {
            decoded_text.clear();
            return false;
        }

        if (!current->left &&
            !current->right) {

            decoded_text += current->ch;
            current = huffman_root;
        }
    }

    if (current != huffman_root) {
        decoded_text.clear();
        return false;
    }

    return true;
}