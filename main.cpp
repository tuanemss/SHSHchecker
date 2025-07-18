#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>
#include <cstdlib>
#include <curl/curl.h>
#include <regex>
#include "json.hpp"
#include <set>

using json = nlohmann::json;
namespace fs = std::filesystem;

static std::string USER_AGENT = "savethemblobs/2.1";

size_t write_to_string(void* ptr, size_t size, size_t nmemb, void* stream) {
    ((std::string*)stream)->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

std::string http_get(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            std::cerr << "GET failed: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
    }
    return response;
}

std::string http_post(const std::string& url, const std::string& data) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: text/xml");
        headers = curl_slist_append(headers, ("User-Agent: " + USER_AGENT).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            std::cerr << "POST failed: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return response;
}

std::string json_str(const json& j, const std::string& key) {
    if (!j.contains(key)) return "";
    if (j[key].is_string()) return j[key];
    if (j[key].is_number()) return std::to_string((uint64_t)j[key]);
    return "";
}

bool is_hex(const std::string& str) {
    return str.rfind("0x", 0) == 0 || std::regex_match(str, std::regex("^[0-9a-fA-F]+$"));
}

std::string hex_to_dec(const std::string& hex) {
    unsigned long long dec;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> dec;
    return std::to_string(dec);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./SHSHchecker <ECID> <MODEL>\n";
        return 1;
    }

    std::string input_ecid = argv[1];
    std::string ecid = is_hex(input_ecid) ? hex_to_dec(input_ecid) : input_ecid;

    std::string url = "http://cydia.saurik.com/tss@home/api/check/" + ecid;
    std::string json_data = http_get(url);

    if (json_data.empty()) {
        std::cerr << "Error: Empty response from server.\n";
        return 1;
    }

    json blobs = json::parse(json_data, nullptr, false);
    if (blobs.is_discarded() || !blobs.is_array()) {
        std::cerr << "Error: Invalid JSON response from server\n";
        return 1;
    }

    // Sort blobs by firmware
    std::vector<json> sorted_blobs = blobs.get<std::vector<json>>();
    std::sort(sorted_blobs.begin(), sorted_blobs.end(), [](const json& a, const json& b) {
        return json_str(a, "firmware") < json_str(b, "firmware");
    });

    std::cout << "Found " << sorted_blobs.size() << " SHSH blobs for ECID " << ecid << ":\n";
    for (size_t i = 0; i < sorted_blobs.size(); ++i) {
        std::string fw = json_str(sorted_blobs[i], "firmware");
        std::string build = json_str(sorted_blobs[i], "build");
        std::string model = json_str(sorted_blobs[i], "model");
        std::cout << "[" << (i + 1) << "] " << model << " - iOS " << fw << " (" << build << ")\n";
    }

    std::cout << "Enter blobs numbers to download (e.g. 1 3 5 or 1-5 ): ";
    std::string input;
    std::getline(std::cin, input);

    std::set<int> selected_indices;
    std::stringstream ss(input);
    std::string token;
    while (ss >> token) {
        if (token.find('-') != std::string::npos) {
            int start = std::stoi(token.substr(0, token.find('-')));
            int end = std::stoi(token.substr(token.find('-') + 1));
            for (int i = start; i <= end; ++i) selected_indices.insert(i - 1);
        } else {
            selected_indices.insert(std::stoi(token) - 1);
        }
    }

    for (int idx : selected_indices) {
        if (idx < 0 || idx >= (int)sorted_blobs.size()) continue;

        const auto& blob = sorted_blobs[idx];
        std::string model = json_str(blob, "model");
        std::string build = json_str(blob, "build");
        std::string firmware = json_str(blob, "firmware");
        std::string cpid = json_str(blob, "chip");
        std::string bdid = json_str(blob, "board");

        std::string subdir = "shsh/" + model + "-" + ecid;
        fs::create_directories(subdir);

        std::string filename = ecid + "-" + model + "-" + firmware + "-" + build + ".shsh";
        std::string path = subdir + "/" + filename;

        std::cout << "Downloading blob for " << model << " iOS " << firmware << "...\n";

        std::string manifest_url = "http://cydia.saurik.com/tss@home/api/manifest.xml/" + build + "/" + cpid + "/" + bdid;
        std::string manifest = http_get(manifest_url);

        if (manifest.find("$ecid") != std::string::npos) {
            std::string target = "<string>$ecid</string>";
            std::string replacement = "<integer>" + ecid + "</integer>";
            size_t pos = manifest.find(target);
            manifest.replace(pos, target.length(), replacement);
        }

        std::string blob_data = http_post("http://cydia.saurik.com/TSS/controller?action=2", manifest);

        if (blob_data.empty()) {
            std::cerr << "Failed to download blob for " << build << "\n";
            continue;
        }

        std::ofstream f(path);
        f << blob_data;
        f.close();

        std::cout << "Saved: " << path << "\n";
    }

    return 0;
}
