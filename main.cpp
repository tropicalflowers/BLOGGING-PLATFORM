#define CROW_MAIN
#include "crow.h"

#include <vector>
#include <string>
#include <sstream>

struct blogpost_struct {
    int id;
    std::string title;
    std::string content;
};

// Initial dummy posts
std::vector<blogpost_struct> blogpost_vector = {
    {1, "First Post", "This is the content of the first post."},
    {2, "Second Post", "This is the content of the second post."},
    {3, "Third Post", "This is the content of the third post."}
};

int main()
{
    crow::SimpleApp app;

    // Home Page
    CROW_ROUTE(app, "/")([] {
        std::ostringstream html;
        html << "<h1>Welcome to Blogging Platform!</h1>";
        html << "<a href='/add'>➕ Add New Post</a><br><br>";
        html << "<ul>";

        for (const auto& post : blogpost_vector) {
            html << "<li><strong>" << post.id << ".</strong> ";
            html << "<a href=\"/post/" << post.id << "\">" << post.title << "</a>";
            //html << " - <a href='/delete/" << post.id << "' style='color:red;'>Delete</a></li><br>";
            html << " - <a href='/delete/" << post.id << "' style='color:red;'>Delete</a>";
            html << " - <a href='/edit/" << post.id << "' style='color:orange;'>Edit</a></li><br>";

        }

        html << "</ul>";
        return html.str();
    });

    // Add Post Page
    CROW_ROUTE(app, "/add")([] {
        std::ostringstream html;
        html << "<h1>Add a New Blog Post</h1>";
        html << "<form action='/submit' method='GET'>";
        html << "Title: <input type='text' name='title'><br><br>";
        html << "Content:<br><textarea name='content' rows='5' cols='40'></textarea><br><br>";
        html << "<input type='submit' value='Post'>";
        html << "</form>";
        html << "<br><a href='/'>Back to Home</a>";
        return html.str();
    });

/*
    CROW_ROUTE(app, "/edit/<int>")
([](int id){
    for (const auto& post : blogpost_vector) {
        if (post.id == id) {
            std::ostringstream html;
            html << "<h1>Edit Post</h1>";
            html << "<form action='/update' method='GET'>";
            html << "<input type='hidden' name='id' value='" << post.id << "'>";
            html << "Title: <input type='text' name='title' value='" << post.title << "'><br><br>";
            html << "Content:<br><textarea name='content' rows='5' cols='40'>" << post.content << "</textarea><br><br>";
            html << "<input type='submit' value='Update'>";
            html << "</form><br><a href='/'>Back to Home</a>";
            return html.str();
        }
    }
    return crow::response(404, "Post not found.");
});*/

CROW_ROUTE(app, "/edit/<int>")
([](int id){
    for (const auto& post : blogpost_vector) {
        if (post.id == id) {
            std::ostringstream html;
            html << "<h1>Edit Post</h1>";
            html << "<form action='/update' method='GET'>";
            html << "<input type='hidden' name='id' value='" << post.id << "'>";
            html << "Title: <input type='text' name='title' value='" << post.title << "'><br><br>";
            html << "Content:<br><textarea name='content' rows='5' cols='40'>" << post.content << "</textarea><br><br>";
            html << "<input type='submit' value='Update'>";
            html << "</form><br><a href='/'>Back to Home</a>";
            return crow::response{html.str()}; // ✅ FIXED
        }
    }
    return crow::response(404, "Post not found.");
});

CROW_ROUTE(app, "/update")
([](const crow::request& req){
    auto id_param = req.url_params.get("id");
    auto title = req.url_params.get("title");
    auto content = req.url_params.get("content");

    if (!id_param || !title || !content) {
        return crow::response(400, "Missing parameters");
    }

    int id = std::stoi(id_param);
    for (auto& post : blogpost_vector) {
        if (post.id == id) {
            post.title = title;
            post.content = content;
            break;
        }
    }

    crow::response res;
    res.code = 302;
    res.set_header("Location", "/");
    return res;
});


    // Submit Form
    CROW_ROUTE(app, "/submit")([](const crow::request& req) {
        auto title = req.url_params.get("title");
        auto content = req.url_params.get("content");

        if (!title || !content) {
            return crow::response(400, "Missing title or content!");
        }

        int nextId = blogpost_vector.empty() ? 1 : blogpost_vector.back().id + 1;
        blogpost_vector.push_back({nextId, title, content});

        crow::response res;
        res.code = 302;
        res.set_header("Location", "/");
        return res;
    });

    // Delete Post
    CROW_ROUTE(app, "/delete/<int>")([](int postId) {
        for (auto it = blogpost_vector.begin(); it != blogpost_vector.end(); ++it) {
            if (it->id == postId) {
                blogpost_vector.erase(it);
                break;
            }
        }

        crow::response res;
        res.code = 302;
        res.set_header("Location", "/");
        return res;
    });

    // View Individual Post
    CROW_ROUTE(app, "/post/<int>")([](int id) {
        for (const auto& post : blogpost_vector) {
            if (post.id == id) {
                std::ostringstream html;
                html << "<h2>" << post.title << "</h2>";
                html << "<p>" << post.content << "</p>";
                html << "<br><a href='/'>Back to Home</a>";
                return crow::response(html.str());
            }
        }
        return crow::response(404, "Post not found.");
    });

    // Get All Posts in JSON
    CROW_ROUTE(app, "/posts")([] {
        crow::json::wvalue result;
        std::vector<crow::json::wvalue> post_list;

        for (const auto& post : blogpost_vector) {
            crow::json::wvalue item;
            item["id"] = post.id;
            item["title"] = post.title;
            item["content"] = post.content;
            post_list.push_back(item);
        }

        result["posts"] = std::move(post_list);
        return crow::response(result);
    });

    // Run Server
    app.port(18080).multithreaded().run();
}
