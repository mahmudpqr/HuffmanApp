#include "httplib.h"
#include "huffman_backend.h"

#include <windows.h>
#include <shellapi.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

using namespace std;
namespace fs = std::filesystem;


mutex huffman_mutex;


fs::path get_executable_directory() {

    wchar_t buffer[MAX_PATH];

    DWORD length = GetModuleFileNameW(
        nullptr,
        buffer,
        MAX_PATH
    );

    if (length == 0)
        return fs::current_path();

    return fs::path(
        wstring(buffer, length)
    ).parent_path();
}


string read_file(const fs::path& path) {

    ifstream file(path, ios::binary);

    if (!file)
        return "";

    stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}


string json_escape(const string& input) {

    string output;

    for (unsigned char ch : input) {

        switch (ch) {

        case '\"':
            output += "\\\"";
            break;

        case '\\':
            output += "\\\\";
            break;

        case '\b':
            output += "\\b";
            break;

        case '\f':
            output += "\\f";
            break;

        case '\n':
            output += "\\n";
            break;

        case '\r':
            output += "\\r";
            break;

        case '\t':
            output += "\\t";
            break;

        default:

            if (ch < 0x20) {
                char buffer[7];

                snprintf(
                    buffer,
                    sizeof(buffer),
                    "\\u%04x",
                    ch
                );

                output += buffer;
            }
            else {
                output += static_cast<char>(ch);
            }
        }
    }

    return output;
}


string character_display(char ch) {

    switch (ch) {

    case ' ':
        return "SPACE";

    case '\n':
        return "NEWLINE";

    case '\r':
        return "CARRIAGE RETURN";

    case '\t':
        return "TAB";

    default:
        return string(1, ch);
    }
}


int main() {

    fs::path app_directory =
        get_executable_directory();

    fs::path web_directory =
        app_directory / "web";


    fs::path index_file =
        web_directory / "index.html";

    if (!fs::exists(index_file)) {

        cerr
            << "Error: Could not find frontend files.\n"
            << "Expected:\n"
            << index_file.string()
            << "\n";

        return 1;
    }


    httplib::Server server;


    server.Get("/", [&](const httplib::Request&,
                        httplib::Response& res) {

        string html =
            read_file(index_file);

        if (html.empty()) {

            res.status = 500;
            res.set_content(
                "Could not read index.html.",
                "text/plain"
            );

            return;
        }

        res.set_content(
            html,
            "text/html; charset=UTF-8"
        );
    });


    server.Get("/style.css",
        [&](const httplib::Request&,
            httplib::Response& res) {

        string css =
            read_file(web_directory / "style.css");

        if (css.empty()) {

            res.status = 404;
            return;
        }

        res.set_content(
            css,
            "text/css; charset=UTF-8"
        );
    });


    server.Get("/script.js",
        [&](const httplib::Request&,
            httplib::Response& res) {

        string js =
            read_file(web_directory / "script.js");

        if (js.empty()) {

            res.status = 404;
            return;
        }

        res.set_content(
            js,
            "application/javascript; charset=UTF-8"
        );
    });


    server.Post(
        "/api/encode",
        [&](const httplib::Request& req,
            httplib::Response& res) {

        if (!req.has_param("text")) {

            res.status = 400;

            res.set_content(
                R"({"success":false,"error":"Missing text."})",
                "application/json"
            );

            return;
        }

        string text =
            req.get_param_value("text");


        HuffmanResult result;

        {
            lock_guard<mutex> lock(huffman_mutex);

            if (!encode_text(text, result)) {

                res.status = 400;

                res.set_content(
                    R"({"success":false,"error":"Input text cannot be empty."})",
                    "application/json"
                );

                return;
            }
        }


        string json =
            R"({"success":true,"codes":[)";

        for (size_t i = 0;
             i < result.codes.size();
             ++i) {

            const auto& [ch, code] =
                result.codes[i];

            if (i > 0)
                json += ",";

            json +=
                "{\"character\":\"" +
                json_escape(
                    character_display(ch)
                ) +
                "\",\"code\":\"" +
                json_escape(code) +
                "\"}";
        }

        json +=
            "],\"encoded_bits\":\"" +
            json_escape(result.encoded_bits) +
            "\"}";


        res.set_content(
            json,
            "application/json; charset=UTF-8"
        );
    });


    server.Post(
        "/api/decode",
        [&](const httplib::Request& req,
            httplib::Response& res) {

        if (!req.has_param("bits")) {

            res.status = 400;

            res.set_content(
                R"({"success":false,"error":"Missing encoded bit string."})",
                "application/json"
            );

            return;
        }

        string bits =
            req.get_param_value("bits");

        string decoded;

        {
            lock_guard<mutex> lock(huffman_mutex);

            if (!decode_text(bits, decoded)) {

                res.status = 400;

                res.set_content(
                    R"({"success":false,"error":"Invalid bit string or no Huffman tree is available. Encode text first."})",
                    "application/json"
                );

                return;
            }
        }


        string json =
            "{\"success\":true,\"decoded_text\":\"" +
            json_escape(decoded) +
            "\"}";


        res.set_content(
            json,
            "application/json; charset=UTF-8"
        );
    });


    const int port = 8080;

    string url =
        "http://127.0.0.1:" +
        to_string(port);


    cout
        << "Huffman server starting...\n"
        << "Application directory: "
        << app_directory.string()
        << "\n"
        << "Web directory: "
        << web_directory.string()
        << "\n"
        << "URL: "
        << url
        << "\n";


    ShellExecuteW(
        nullptr,
        L"open",
        L"http://127.0.0.1:8080",
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    );


    if (!server.listen(
            "127.0.0.1",
            port)) {

        cerr
            << "Failed to start server on port "
            << port
            << ".\n";

        return 1;
    }


    clear_huffman_tree();

    return 0;
}