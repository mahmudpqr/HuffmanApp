
"use strict";

class Node {
    constructor(ch, freq, left = null, right = null) {
        this.ch = ch;
        this.freq = freq;
        this.left = left;
        this.right = right;
    }

    isLeaf() {
        return this.left === null && this.right === null;
    }
}

// State retained between encoding and decoding.
let root = null;
let frequencyMap = new Map();
let huffmanCodes = new Map();
let originalText = "";

// Calculate character frequencies.
function getFreqMap(text) {
    const freqMap = new Map();

    for (const ch of text) {
        freqMap.set(ch, (freqMap.get(ch) || 0) + 1);
    }

    return freqMap;
}

// Create and sort leaf nodes.
function generateNodes(freqMap) {
    const nodes = [];

    for (const [ch, freq] of freqMap) {
        nodes.push(new Node(ch, freq));
    }

    nodes.sort((a, b) => b.freq - a.freq);
    return nodes;
}

// Build the Huffman tree using the two least frequent nodes.
function buildTree(nodes) {
    if (nodes.length === 0) {
        return null;
    }

    while (nodes.length > 1) {
        const left = nodes.pop();
        const right = nodes.pop();

        const parent = new Node(
            null,
            left.freq + right.freq,
            left,
            right
        );

        nodes.push(parent);
        nodes.sort((a, b) => b.freq - a.freq);
    }

    return nodes[0];
}

// Generate a binary code for every leaf.
function generateCode(rootNode, code, codeMap) {
    if (!rootNode) {
        return;
    }

    if (rootNode.isLeaf()) {
        codeMap.set(rootNode.ch, code === "" ? "0" : code);
        return;
    }

    generateCode(rootNode.left, code + "0", codeMap);
    generateCode(rootNode.right, code + "1", codeMap);
}

// Encode the original text.
function encodeText(text, codeMap) {
    let encoded = "";

    for (const ch of text) {
        encoded += codeMap.get(ch);
    }

    return encoded;
}

// Decode using the existing Huffman tree.
function decodeText(rootNode, bits) {
    if (!rootNode) {
        throw new Error("No Huffman tree is available.");
    }

    if (bits.length === 0) {
        return "";
    }

    // Special case: only one distinct character.
    if (rootNode.isLeaf()) {
        for (const bit of bits) {
            if (bit !== "0") {
                throw new Error(
                    "Invalid bit for a single-character Huffman tree."
                );
            }
        }

        return rootNode.ch.repeat(bits.length);
    }

    let text = "";
    let current = rootNode;

    for (const bit of bits) {
        if (bit === "0") {
            current = current.left;
        } else if (bit === "1") {
            current = current.right;
        } else {
            throw new Error(
                "Encoded input must contain only 0 and 1."
            );
        }

        if (!current) {
            throw new Error("Invalid encoded bit sequence.");
        }

        if (current.isLeaf()) {
            text += current.ch;
            current = rootNode;
        }
    }

    // The bits must end at the root after a complete character.
    if (current !== rootNode) {
        throw new Error("Incomplete encoded bit sequence.");
    }

    return text;
}

// Display whitespace characters clearly.
function displayCharacter(ch) {
    const names = {
        " ": "[space]",
        "\n": "[newline]",
        "\r": "[carriage return]",
        "\t": "[tab]"
    };

    return names[ch] || ch;
}

// Populate a table safely using textContent.
function renderTable(tableBody, rows) {
    tableBody.replaceChildren();

    for (const values of rows) {
        const tr = document.createElement("tr");

        for (const value of values) {
            const td = document.createElement("td");
            td.textContent = value;
            tr.appendChild(td);
        }

        tableBody.appendChild(tr);
    }
}

// Update the frequency table.
function renderFrequencies(freqMap) {
    const rows = [...freqMap.entries()]
        .sort((a, b) => a[0].codePointAt(0) - b[0].codePointAt(0))
        .map(([ch, freq]) => [displayCharacter(ch), freq]);

    renderTable(
        document.getElementById("frequencyTable"),
        rows
    );
}

// Update the Huffman code table.
function renderCodes(codeMap) {
    const rows = [...codeMap.entries()]
        .sort((a, b) => a[0].codePointAt(0) - b[0].codePointAt(0))
        .map(([ch, code]) => [displayCharacter(ch), code]);

    renderTable(
        document.getElementById("codeTable"),
        rows
    );
}

function setStatus(message, type = "") {
    const status = document.getElementById("status");
    status.textContent = message;
    status.className = "status " + type;
}

// Encode button.
document.getElementById("encodeBtn").addEventListener("click", () => {
    const input = document.getElementById("inputText").value;

    if (input.length === 0) {
        setStatus("Please enter some text to encode.", "error");
        return;
    }

    try {
        originalText = input;
        frequencyMap = getFreqMap(originalText);

        const nodes = generateNodes(frequencyMap);
        root = buildTree(nodes);

        huffmanCodes = new Map();
        generateCode(root, "", huffmanCodes);

        const encoded = encodeText(originalText, huffmanCodes);

        renderFrequencies(frequencyMap);
        renderCodes(huffmanCodes);

        document.getElementById("encodedText").value = encoded;
        document.getElementById("decodedText").value = "";

        setStatus("Text encoded successfully.", "success");
    } catch (error) {
        setStatus(error.message, "error");
    }
});

// Decode button.
document.getElementById("decodeBtn").addEventListener("click", () => {
    if (!root) {
        setStatus("Encode some text first to build the Huffman tree.", "error");
        return;
    }

    const bits = document.getElementById("encodedText").value;

    try {
        const decoded = decodeText(root, bits);

        document.getElementById("decodedText").value = decoded;
        setStatus("Text decoded successfully.", "success");
    } catch (error) {
        document.getElementById("decodedText").value = "";
        setStatus(error.message, "error");
    }
});

// Copy buttons.
async function copyText(elementId) {
    const value = document.getElementById(elementId).value;

    try {
        await navigator.clipboard.writeText(value);
        setStatus("Copied to clipboard.", "success");
    } catch {
        setStatus(
            "Clipboard access failed. Select and copy the text manually.",
            "error"
        );
    }
}

document.getElementById("copyEncodedBtn").addEventListener("click", () => {
    copyText("encodedText");
});

document.getElementById("copyDecodedBtn").addEventListener("click", () => {
    copyText("decodedText");
});