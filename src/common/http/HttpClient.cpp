/* Copyright (c) 2019 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */
#include "common/http/HttpClient.h"
#include "common/process/ProcessUtils.h"
#include "common/base/Cord.h"
#include <curl/curl.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

namespace nebula {
namespace http {
struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    reinterpret_cast<std::string*>(userp)->append(reinterpret_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}
//  static size_t WriteMemory(void *contents, size_t size, size_t nmemb, void *userp) {
//     size_t realsize = size * nmemb;
//     struct MemoryStruct *mem = (struct MemoryStruct *)userp;
//     char *ptr = reinterpret_cast<char *> (realloc(mem->memory, mem->size + realsize + 1));
//     mem->memory = ptr;
//     memcpy(&(mem->memory[mem->size]), contents, realsize);
//     mem->size += realsize;
//     mem->memory[mem->size] = 0;
//     return realsize;
// }
 StatusOr<std::string> HttpClient::get(const std::string& path, const std::string& /*options*/) {
    // CURL *curl;
    // CURLcode res;
    // curl = curl_easy_init();
    // struct MemoryStruct chunk;
    // chunk.memory = reinterpret_cast<char *> (malloc(1));
    // chunk.size = 0;
    // if (curl) {
    //     curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
    //     curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    //     curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void *> (&chunk));
    //     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemory);
    //     res = curl_easy_perform(curl);
    //     StatusOr<std::string> result;
    //     if (chunk.size != 0) result=chunk.memory;
    //     if (chunk.size == 0) result = "";
    //     free(chunk.memory);
    //     curl_easy_cleanup(curl);
    //     LOG(INFO) << "HTTP return Code: " << res;
    //     if (result.ok()) {
    //         return result.value();
    //     } else {
    //         return Status::Error(folly::stringPrintf("Http Get Failed: %s", path.c_str()));
    //     }
    // }
    // free(chunk.memory);
    // return Status::Error(folly::stringPrintf("Libcurl Failed: %s", path.c_str()));
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string return_string;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &return_string);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        res = curl_easy_perform(curl);
        StatusOr<std::string> result = return_string;
        curl_easy_cleanup(curl);
        LOG(INFO) << "HTTP return Code: " << res;
        if (result.ok()) {
            return result.value();
        } else {
            return Status::Error(folly::stringPrintf("Http Get Failed: %s", path.c_str()));
        }
    }
    return Status::Error(folly::stringPrintf("Libcurl Failed: %s", path.c_str()));
}

StatusOr<std::string> HttpClient::post(const std::string& path, const std::string& header) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *my_curl_list = nullptr;
    int len = header.length();
    int now_total = 0;
    std::string newString = "";
    for ( int i = 0 ; i < len ; i++ ) {
        if (header[i] == '\"') {
            now_total++;
            if (now_total%2 == 1) {
                newString = "";
                continue;
            }
            if (now_total%2 == 0) {
                my_curl_list = curl_slist_append(my_curl_list, newString.c_str());
            }
            continue;
        }
            newString+=header[i];
    }
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, my_curl_list);
        res = curl_easy_perform(curl);
        curl_slist_free_all(my_curl_list);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return Status::Error(folly::stringPrintf("Http Get Failed: %s", path.c_str()));
        }
        curl_easy_cleanup(curl);
    }
    return Status::Error(folly::stringPrintf("Libcurl Failed: %s", path.c_str()));
}

StatusOr<std::string> HttpClient::post(const std::string& path,
                                       const std::unordered_map<std::string, std::string>& header) {
    folly::dynamic mapData = folly::dynamic::object;
    for (auto const& it : header) {
        mapData[it.first] = it.second;
    }
    return post(path, mapData);
}

StatusOr<std::string> HttpClient::post(const std::string& path, const folly::dynamic& data) {
    return sendRequest(path, data, "POST");
}

StatusOr<std::string> HttpClient::put(const std::string& path,
                                       const std::unordered_map<std::string, std::string>& header) {
    folly::dynamic mapData = folly::dynamic::object;
    for (auto const& it : header) {
        mapData[it.first] = it.second;
    }
    return put(path, mapData);
}

StatusOr<std::string> HttpClient::put(const std::string& path, const std::string& header) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *my_curl_list = nullptr;
    int len = header.length();
    int now_total = 0;
    std::string newString = "";
    for ( int i = 0 ; i < len ; i++ ) {
        if (header[i] == '\"') {
            now_total++;
            if (now_total%2 == 1) {
                newString = "";
                continue;
            }
            if (now_total%2 == 0) {
                my_curl_list = curl_slist_append(my_curl_list, newString.c_str());
            }
            continue;
        }
        newString+=header[i];
    }
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, my_curl_list);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "put");
        res = curl_easy_perform(curl);
        curl_slist_free_all(my_curl_list);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return "HttpClientHandler failed";
        }
         curl_easy_cleanup(curl);
    }
    return Status::Error(folly::stringPrintf("Libcurl Failed: %s", path.c_str()));
}

StatusOr<std::string> HttpClient::put(const std::string& path, const folly::dynamic& data) {
    return sendRequest(path, data, "PUT");
}

StatusOr<std::string> HttpClient::sendRequest(const std::string& path,
                                              const folly::dynamic& data,
                                              const std::string& reqType) {
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    std::string return_string;
    if (curl) {
        struct curl_slist *headers = nullptr;
        std::string mydata = folly::toJson(data);
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, reqType.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &return_string);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mydata.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);
        LOG(INFO) << "HTTP return Code: " << res;
        StatusOr<std::string> result = return_string;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        if (result.ok()) {
            return result.value();
        } else {
            return Status::Error(folly::stringPrintf("Http Get Failed: %s", path.c_str()));
        }
    }
    return Status::Error(folly::stringPrintf("Libcurl Failed: %s", path.c_str()));
}
}   // namespace http
}   // namespace nebula
