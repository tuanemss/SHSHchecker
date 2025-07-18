#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>
#include <cstdlib>
#include <curl/curl.h>
#include "json.hpp"

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

// Convert ECID to decimal if it's in hex
std::string convert_ecid_to_decimal(const std::string& ecid_input) {
    if (ecid_input.find_first_not_of("0123456789") == std::string::npos) {
        return ecid_input;  // Already decimal
    }
    uint64_t dec_ecid;
    std::stringstream ss;
    ss << std::hex << ecid_input;
    ss >> dec_ecid;
    return std::to_string(dec_ecid);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./SHSHchecker <ECID> <MODEL>\n";
        return 1;
    }

    std::string raw_ecid = argv[1];
    std::string ecid = convert_ecid_to_decimal(raw_ecid);

    std::string url = "http://cydia.saurik.com/tss@home/api/check/" + ecid;
    std::string json_data = http_get(url);

    if (json_data.empty()) {
        std::cerr << "Error: Empty response from server. Check ECID/model or your network.\n";
        return 1;
    }

    json blobs = json::parse(json_data, nullptr, false);
    if (blobs.is_discarded() || !blobs.is_array()) {
        std::cerr << "Error: Invalid JSON response from server\n";
        return 1;
    }

    for (const auto& blob : blobs) {
        std::string model = json_str(blob, "model");
        std::string build = json_str(blob, "build");
        std::string firmware = json_str(blob, "firmware");
        std::string cpid = json_str(blob, "chip");
        std::string bdid = json_str(blob, "board");

        std::string subdir = "shsh/" + model + "-" + ecid;
        fs::create_directories(subdir);

        std::string filename = ecid + "-" + model + "-" + firmware + "-" + build + ".shsh";
        std::string path = subdir + "/" + filename;

        std::cout << "Fetching blob for " << model << " iOS " << firmware << "...\n";

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
