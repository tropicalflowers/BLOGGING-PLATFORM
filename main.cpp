#define CROW_MAIN
#include "crow.h"
#include "crow/utility.h"
// filepath: d:\BLOGGING PLATFORM\main.cpp
#include <chrono>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <set>
#include "json/json.hpp"

using json = nlohmann::json;

struct blogpost_struct {
    int id;
    std::string title;
    std::string content;
    std::string timestamp;
    std::vector<std::string> tags;
    int likes = 0;
    std::vector<std::string> comments;

};

std::vector<blogpost_struct> blogpost_vector;

std::string get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_now = std::chrono::system_clock::to_time_t(now);
    return std::string(std::ctime(&time_now));
}

void save_posts_to_file(const std::string& filename = "posts.json") {
    json j;
    for (const auto& post : blogpost_vector) {
        j.push_back({
            {"id", post.id},
            {"title", post.title},
            {"content", post.content},
            {"timestamp", post.timestamp},
            {"tags", post.tags},
            {"likes", post.likes},
            {"comments", post.comments}
        });
    }
    std::ofstream file(filename);
    file << j.dump(4);
}

void load_posts_from_file(const std::string& filename = "posts.json") {
    std::ifstream file(filename);
    if (file) {
        json j;
        file >> j;
        blogpost_vector.clear();
        for (const auto& item : j) {
            blogpost_struct post;
            post.id = item["id"];
            post.title = item["title"];
            post.content = item["content"];
            post.timestamp = item["timestamp"];
            post.tags = item.value("tags", std::vector<std::string>());
            blogpost_vector.push_back(post);
            post.likes = item.value("likes", 0);
            post.comments = item.value("comments", std::vector<std::string>());

        }
    }
}
std::string extract_form_value(const std::string& body, const std::string& key) {
    std::string prefix = key + "=";
    size_t start = body.find(prefix);
    if (start == std::string::npos) return "";
    start += prefix.length();
    size_t end = body.find("&", start);
    std::string value = body.substr(start, end - start);
    return value;
}



std::string url_decode(const std::string& str) {
    std::string decoded;
    char a, b;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%') {
            if (i + 2 < str.length()) {
                a = str[i + 1];
                b = str[i + 2];
                a = (a >= 'A') ? (a & ~0x20) - 'A' + 10 : a - '0';
                b = (b >= 'A') ? (b & ~0x20) - 'A' + 10 : b - '0';
                decoded += static_cast<char>(a * 16 + b);
                i += 2;
            }
        } else if (str[i] == '+') {
            decoded += ' ';
        } else {
            decoded += str[i];
        }
    }
    return decoded;
}


int main() {
    load_posts_from_file();
    crow::SimpleApp app;


   CROW_ROUTE(app, "/")([] {
    std::ostringstream html;
    html << "<!DOCTYPE html><html><head><title>Blogging Platform</title>";

    // 💅 Include styles
    html << "<link rel='stylesheet' type='text/css' href='/static/styles.css'>";

    html << "</head><body>";
    html << "<h1>Welcome to Blogging Platform!</h1>";
    html << "<a href='/add'> Add New Post</a><br><br>";

    // 🌸 Search bar with tag suggestion hook
    html << "<form action='/search' method='GET' onsubmit='return handleSearchSubmit()'>";
    html << "Search: <input type='text' id='searchInput' name='q' autocomplete='off' oninput='showTagSuggestions()'>";
    html << "<input type='submit' value='Search'>";
    html << "</form>";

    // 🌼 Live tag suggestion output area
    html << "<div id='tagSuggestions'></div><br>";

    // 🌻 Collect all unique tags
    std::set<std::string> all_tags;
    for (const auto& post : blogpost_vector) {
        for (const auto& tag : post.tags) {
            all_tags.insert(tag);
        }
    }

    // 🌿 Inject all tag names into JS for autocomplete
    html << "<script>\nconst tags = [";
    bool first = true;
    for (const auto& tag : all_tags) {
        if (!first) html << ", ";
        html << "'" << tag << "'";
        first = false;
    }
    html << "];\n</script>\n";

    // ✨ Display blog posts
    for (const auto& post : blogpost_vector) {
        html << "<div style='margin-bottom: 15px;'>";
        html << "<strong>" << post.id << ".</strong> ";
        html << "<a href='/post/" << post.id << "'>" << post.title << "</a>";
        html << " - <a href='/delete/" << post.id << "' style='color:red;'>Delete</a>";
        html << " - <a href='/edit/" << post.id << "' style='color:orange;'>Edit</a> ";
        html << "<small style='color:gray;'> Posted on: " << post.timestamp << "</small>";
        html << "</div>";
    }

    // 💻 Link JS for tag suggestions
    html << "<script src='/static/scripts.js'></script>";
    html << "</body></html>";

    return html.str();
});



    CROW_ROUTE(app, "/add")([] {
        std::ostringstream html;
        html << "<h1>Add a New Blog Post</h1>";
        html << "<form action='/submit' method='GET'>";
        html << "Title: <input type='text' name='title'><br><br>";
        html << "Content:<br><textarea name='content' rows='5' cols='40'></textarea><br><br>";
        html << "Tags (comma separated): <input type='text' name='tags'><br><br>";
        html << "<input type='submit' value='Post'>";
        html << "</form><br><a href='/'>Back to Home</a>";
        return html.str();
    });

    CROW_ROUTE(app, "/submit")([](const crow::request& req) {
        auto title = req.url_params.get("title");
        auto content = req.url_params.get("content");
        auto tags_param = req.url_params.get("tags");

        if (!title || !content) {
            return crow::response(400, "Missing title or content!");
        }

        int nextId = blogpost_vector.empty() ? 1 : blogpost_vector.back().id + 1;
        std::string timestamp = get_current_timestamp();

        std::vector<std::string> tags;
        if (tags_param) {
            std::stringstream ss(tags_param);
            std::string tag;
            while (std::getline(ss, tag, ',')) {
                tag.erase(0, tag.find_first_not_of(" \t"));
                tag.erase(tag.find_last_not_of(" \t") + 1);
                if (!tag.empty()) tags.push_back(tag);
            }
        }

        blogpost_vector.push_back({nextId, title, content, timestamp, tags});
        save_posts_to_file();

        crow::response res;
        res.code = 302;
        res.set_header("Location", "/");
        return res;
    });

    CROW_ROUTE(app, "/delete/<int>")([](int postId) {
        for (auto it = blogpost_vector.begin(); it != blogpost_vector.end(); ++it) {
            if (it->id == postId) {
                blogpost_vector.erase(it);
                break;
            }
        }
        save_posts_to_file();
        crow::response res;
        res.code = 302;
        res.set_header("Location", "/");
        return res;
    });

CROW_ROUTE(app, "/post/<int>")([](int id) {
    for (const auto& post : blogpost_vector) {
        if (post.id == id) {
            std::ostringstream html;

            html << "<!DOCTYPE html><html><head><title>" << post.title << "</title>";
            html << "<link rel='stylesheet' type='text/css' href='/static/styles.css'>";
            html << "</head><body>";

            html << "<div style='max-width: 700px; margin: 40px auto; font-family: Arial, sans-serif;'>";

            html << "<h1 style='color:#333;'>" << post.title << "</h1>";
            html << "<p style='font-size: 18px;'>" << post.content << "</p>";
            html << "<small style='color: #777;'>Posted on: " << post.timestamp << "</small><br><br>";

            // 💗 Show likes and like button
            html << "<p><strong>Likes:</strong> " << post.likes << " ";
            html << "<a href='/like/" << post.id << "' style='color:hotpink;'>❤️ Like</a></p>";

            // 🌼 Tags
            if (!post.tags.empty()) {
                html << "<p>Tags: ";
                for (const auto& tag : post.tags) {
                    html << "<a href='/tag/" << tag << "' style='text-decoration:none;'>";
                    html << "<span style='background:#eef; color:#333; padding:4px 8px; margin:4px; border-radius:10px;'>#" << tag << "</span>";
                    html << "</a> ";
                }
                html << "</p>";
            }

            // 💬 Comments
            html << "<h3>Comments:</h3><ul>";
            for (const auto& comment : post.comments) {
                html << "<li>" << comment << "</li>";
            }
            html << "</ul>";

            // ✍️ Comment form
            html << "<form action='/comment/" << post.id << "' method='POST'>";
            html << "<input type='text' name='comment' placeholder='Add a comment...' required>";
            html << "<input type='submit' value='Post'>";
            html << "</form>";

            html << "<br><a href='/' style='color:#6b46c1; text-decoration:none;'>← Back to Home</a>";
            html << "</div></body></html>";

            return crow::response(html.str());
        }
    }
    return crow::response(404, "Post not found.");
});



   CROW_ROUTE(app, "/edit/<int>").methods("GET"_method)
([](const crow::request& req, crow::response& res, int id) {
    std::cout << "Requested to edit post with ID: " << id << std::endl;

    for (const auto& post : blogpost_vector) {
        if (post.id == id) {
            std::ostringstream html;
            html << "<h1>Edit Post</h1>";
            html << "<form action='/update/" << post.id << "' method='POST'>";
            html << "Title: <input type='text' name='title' value='" << post.title << "'><br><br>";
            html << "Content:<br><textarea name='content' rows='5' cols='40'>" << post.content << "</textarea><br><br>";

            std::ostringstream tagstream;
            for (size_t i = 0; i < post.tags.size(); ++i) {
                tagstream << post.tags[i];
                if (i != post.tags.size() - 1) tagstream << ", ";
            }

            html << "Tags: <input type='text' name='tags' value='" << tagstream.str() << "'><br><br>";
            html << "<input type='submit' value='Update'>";
            html << "</form><br><a href='/'>Back to Home</a>";

            res.write(html.str());
            res.end();
            return;
        }
    }

    std::cout << "Post with ID " << id << " not found!" << std::endl;
    res.code = 404;
    res.write("Post not found.");
    res.end();
});

   CROW_ROUTE(app, "/update/<int>").methods("POST"_method)
([](const crow::request& req, crow::response& res, int id) {
    auto title = req.url_params.get("title");
    auto content = req.url_params.get("content");
    auto tags_param = req.url_params.get("tags");

    for (auto& post : blogpost_vector) {
        if (post.id == id) {
            if (title) post.title = title;
            if (content) post.content = content;

            post.tags.clear();
            if (tags_param) {
                std::stringstream ss(tags_param);
                std::string tag;
                while (std::getline(ss, tag, ',')) {
                    tag.erase(0, tag.find_first_not_of(" \t"));
                    tag.erase(tag.find_last_not_of(" \t") + 1);
                    if (!tag.empty()) post.tags.push_back(tag);
                }
            }

            save_posts_to_file();
            res.code = 302;
            res.set_header("Location", "/");
            res.end();
            return;
        }
    }

    res.code = 404;
    res.write("Post not found");
    res.end();
});





    CROW_ROUTE(app, "/search")([](const crow::request& req) {
        auto query = req.url_params.get("q");
        std::ostringstream html;
        html << "<h1>Search Results</h1>";
        html << "<a href='/'>Back to Home</a><br><br>";

        std::set<std::string> all_tags;

        for (const auto& post : blogpost_vector) {
            for (const auto& tag : post.tags) {
                all_tags.insert(tag);
            }
        }
        html << "<p><strong>Tags these posts have used:</strong><br>";
        for (const auto& tag : all_tags) {
            html << "<a href='/tag/" << tag << "' style='text-decoration:none;'>";
            html << "<span style='background:#eef; color:#333; padding:4px 8px; margin:4px; border-radius:10px;'>#" << tag << "</span> ";
            html << "</a>";
       
        }
        html << "</p><hr>";

        if (!query) {
            html << "No search query provided!";
            return html.str();
        }

        std::string q = query;
        bool found = false;

        for (const auto& post : blogpost_vector) {
            if (post.title.find(q) != std::string::npos || post.content.find(q) != std::string::npos) {
                found = true;
                html << "<div style='margin-bottom: 15px;'>";
                html << "<strong>" << post.id << ".</strong> ";
                html << "<a href='/post/" << post.id << "'>" << post.title << "</a>";
                html << " - <small style='color:gray;'>Posted on: " << post.timestamp << "</small>";
                html << "</div>";
            }
        }

        if (!found) {
            html << "<p>No posts found matching <strong>" << q << "</strong>!</p>";
        }

        return html.str();
    });

    CROW_ROUTE(app, "/tag/<string>")([](const std::string& tagname) {
    std::ostringstream html;
    html << "<h1>Posts tagged with #" << tagname << "</h1>";
    html << "<a href='/'>Back to Home</a><br><br>";

    bool found = false;

    for (const auto& post : blogpost_vector) {
        // Search for tag match in the post's tag list
        for (const auto& tag : post.tags) {
            if (tag == tagname) {
                found = true;
                html << "<div style='margin-bottom: 15px;'>";
                html << "<strong>" << post.id << ".</strong> ";
                html << "<a href='/post/" << post.id << "'>" << post.title << "</a>";
                html << " - <small style='color:gray;'>Posted on: " << post.timestamp << "</small>";
                html << "</div>";
                break; // Stop after finding the tag match
            }
        }
    }

    if (!found) {
        html << "<p>No posts found with tag <strong>#" << tagname << "</strong></p>";
    }

    return html.str();
});

CROW_ROUTE(app, "/static/<string>")
([](const crow::request&, crow::response& res, std::string filename) {
    std::ifstream file("static/" + filename, std::ios::binary);
    if (file) {
        std::ostringstream contents;
        contents << file.rdbuf();
        res.write(contents.str());
    } else {
        res.code = 404;
        res.write("File not found");
    }
    res.end();
});
CROW_ROUTE(app, "/like/<int>")
([](int id) {
    for (auto& post : blogpost_vector) {
        if (post.id == id) {
            post.likes++;
            save_posts_to_file();
            crow::response res;
            res.code = 302;
            res.set_header("Location", "/");
            return res;
        }
    }
    return crow::response(404, "Post not found");
});
CROW_ROUTE(app, "/comment/<int>").methods("POST"_method)
([](const crow::request& req, crow::response& res, int id) {
    std::string raw_comment = extract_form_value(req.body, "comment");
    std::string comment = url_decode(raw_comment); // use our custom decode function!

    if (comment.empty()) {
        res.code = 400;
        res.write("Comment is empty!");
        res.end();
        return;
    }

    for (auto& post : blogpost_vector) {
        if (post.id == id) {
            post.comments.push_back(comment);
            save_posts_to_file();
            res.code = 302;
            res.set_header("Location", "/post/" + std::to_string(id));
            res.end();
            return;
        }
    }

    res.code = 404;
    res.write("Post not found");
    res.end();
});







    app.port(18080).multithreaded().run();
}
