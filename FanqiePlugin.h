// FanqiePlugin.h
#ifndef FANQIEPLUGIN_H
#define FANQIEPLUGIN_H

// #define SITE_NAME "Fanqie"
// #define SITE_ID "fanqie"
// #define SITE_URL "https://fanqienovel.com"
// #define PLUGIN_NAME "Fanqie-Plugin"
// #define PLUGIN_ID "fanqie-plugin"

#include "IPlugin.h"
#include "utils/utils.h"
#include <string>
#include <regex>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

using namespace std;
using json = nlohmann::json;

class FanqiePlugin : public IPlugin
{
public:
    string getAuthor() override
    {
        cpr::Response response = cpr::Get(cpr::Url{detail_api_url + this->id});
        json jsonResponse = json::parse(response.text);
        this->author = jsonResponse["data"][0]["author"].get<string>();
        return this->author;
    }

    void getCover() override
    {
        if (this->title.empty())
        {
            getTitle();
        }
        cpr::Response response = cpr::Get(cpr::Url{detail_api_url + this->id});
        json jsonResponse = json::parse(response.text);
        string cover_url = jsonResponse["data"][0]["expand_thumb_url"].get<string>();
        this->saveCover(cover_url);
    }

    string getTitle() override
    {
        cpr::Response response = cpr::Get(cpr::Url{detail_api_url + this->id});
        json jsonResponse = json::parse(response.text);
        this->title = jsonResponse["data"][0]["book_name"].get<string>();
        return this->title;
    }

    vector<unordered_map<string, string>> search(string keyword) override
    {
        return vector<unordered_map<string, string>>();
    }

protected:
    string detail_api_url = "https://fanqienovel.com/reading/bookapi/multi-detail/v/?aid=1967&iid=1&version_code=999&book_id=";
    string content_api_url = "https://fanqienovel.com/api/reader/directory/detail?bookId=";
    string chapter_api_url = "https://novel.snssdk.com/api/novel/reader/full/v1/?item_id=";

    vector<unordered_map<string, string>>
    getContent(bool force_update = false) override
    {
        if (this->title.empty())
        {
            getTitle();
        }
        if (db.isTableEmpty(this->title) == true || force_update == true)
        {
            db.createTable(this->title);
            cpr::Response detail_response = cpr::Get(cpr::Url{this->detail_api_url + this->id});
            json detail_json = json::parse(detail_response.text);
            string update_time = detail_json["data"][0]["last_chapter_update_time"].get<string>();

            cpr::Response response = cpr::Get(cpr::Url{this->content_api_url + this->id});
            json json_response = json::parse(response.text);
            for (const auto &volumes : json_response["data"]["chapterListWithVolume"])
            {
                for (const auto &chapter : volumes)
                {
                    string title = chapter["title"];
                    string url = fmt::format("https://fanqienovel.com/reader/{}", chapter["itemId"].get<string>());
                    string id = chapter["itemId"].get<string>();
                    string fetch_url = (this->chapter_api_url + id);
                    unordered_map<string, string> content = utils::initContentMap(title, url, id, fetch_url, update_time);
                    db.insertData(this->title, content);
                    this->content_data.push_back(content);
                }
            }
        }
        else
        {

            fmt::print("Checking novel update...\n");

            cpr::Response detail_response = cpr::Get(cpr::Url{this->detail_api_url + this->id});
            json detail_json = json::parse(detail_response.text);
            int remote_update_time = stoi(detail_json["data"][0]["last_chapter_update_time"].get<string>());

            int local_update_time = stoi(db.getLastUpdateTime(this->title));

            if (remote_update_time > local_update_time)
            {
                fmt::print("New chapters found, updating content...\n");
                getContent(true);
            }
            else
            {
                this->content_data = db.readData(this->title);
            }
        }
        return this->content_data;
    }

    void parseChapter(unordered_map<string, string> chapter_data) override
    {
        if (this->author.empty())
        {
            getAuthor();
        }

        int index = stoi(chapter_data["index"]);
        json chapter_json = json::parse(chapter_data["content"]);
        string main_chapter = chapter_json["data"]["content"];
        regex html_tag_regex("(<.*?>)+");
        main_chapter = regex_replace(main_chapter, html_tag_regex, "\n");
        string chapter_head = fmt::format("{}\n{}\n---\n", chapter_data["title"], this->author);
        string full_chapter = chapter_head + main_chapter;

        saveChapter(chapter_data["title"], full_chapter);
    }

    string getContentPage() override
    {
        return "";
    }

    // private:
    //     DB db = DB(this->db_path);
};
#endif // FANQIEPLUGIN_H