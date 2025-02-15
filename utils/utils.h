#ifndef UTILS_H
#define UTILS_H

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1 // 启用弱命名空间

#include <string>
#include <unordered_map>

using namespace std;

namespace utils
{

    unordered_map<string, string> initContentMap(string title, string url, string id, string fetch_url, string update_time)
    {
        unordered_map<string, string> content_map;
        content_map["fetch_url"] = fetch_url;
        content_map["id"] = id;
        content_map["url"] = url;
        content_map["title"] = title;
        content_map["update_time"] = update_time;
        return content_map;
    }

} // namespace utils

#endif // UTILS_H