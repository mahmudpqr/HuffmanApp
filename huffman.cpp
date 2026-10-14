#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

using namespace std;

struct Node {
	char ch;
	int freq;
	Node *left, *right;

	Node(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
	Node(int f, Node* l, Node* r) : ch('\0'), freq(f), left(l), right(r) {}
};

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

vector<Node*> generate_nodes(const unordered_map<char, int>& freq_map) {
	vector<Node*> nodes;

	for (const auto& [ch, freq] : freq_map) {
		nodes.push_back(new Node(ch, freq));
	}

	sort(nodes.begin(), nodes.end(), compare_nodes);
	return nodes;
}

Node* build_tree(vector<Node*>& nodes) {
	if (nodes.empty()) return nullptr;

	while(nodes.size() > 1) {
		Node* left = nodes.back();
		nodes.pop_back();

		Node* right = nodes.back();
		nodes.pop_back();

		Node* parent = new Node(left->freq + right->freq, left, right);

		auto it = lower_bound(nodes.begin(), nodes.end(), parent, compare_nodes);
		nodes.insert(it, parent);
	}

	return nodes.back();
}

void delete_tree(Node* root) {
	if (!root) return;

	delete_tree(root->left);
	delete_tree(root->right);
	delete root;
}

void generate_code(Node* root, string code, unordered_map<char, string>& code_map) {
	if (!root) return;

	if (!root->left && !root->right) {
		code_map[root->ch] = code.empty() ? "0" : code;
		return;
	}

	generate_code(root->left, code + "0", code_map);
	generate_code(root->right, code + "1", code_map);
}

string decode_text(Node* root, const string& code) {
	if (!root || code.empty()) return "";

	if (!root->left && !root->right) {
    	return string(code.size(), root->ch);
	}

	string text = "";
	Node* current = root;

	for (char bit : code) {
		if (bit == '0') {
			current = current->left;
		} else if (bit == '1') {
			current = current->right;
		}

		if (!current->left && !current->right) {
			text += current->ch;
			current = root;
		}
	}

	return text;
}

int main() {
    string original_text = "hello world";
    cout << "original text: " << original_text << "\n\n";

    unordered_map<char, int> freq_map = get_freq_map(original_text);
    vector<Node*> nodes = generate_nodes(freq_map);

    Node* root = build_tree(nodes);

    unordered_map<char, string> huffman_codes;
    generate_code(root, "", huffman_codes);

    cout << "huffman codes:\n\n";
    for (const auto& [ch, code] : huffman_codes) {
        cout << ch << " : " << code << "\n";
    }
    cout << "\n";

    string encoded_bits = "";
    for (char ch : original_text) {
        encoded_bits += huffman_codes[ch];
    }
    cout << "encoded bits:  " << encoded_bits << "\n";

    string decoded_text = decode_text(root, encoded_bits);
    cout << "decoded text:  " << decoded_text << "\n";

    delete_tree(root);
    return 0;
}