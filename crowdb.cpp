#include <sqlite_modern_cpp.h>
#include <crow_all.h>
#include <expected>

std::expected<uint8_t, std::string> u8_validator(const std::string& age) {
    uintmax_t num;

    if ((num = std::stoi(age))) {
        if (num <= UINT8_MAX) {
            return static_cast<uint8_t>(num);
        }
        return std::unexpected("NOT VALID U8");
    }
    return std::unexpected("NOT VALID U8");
}

void initialize_database(sqlite::database& db) {
    db << "CREATE TABLE IF NOT EXISTS user ("
       "   _id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
       "   age INT,"
       "   name TEXT,"
       "   weight REAL"
       ");";

    db << "INSERT INTO user (age, name, weight) VALUES (?, ?, ?);"
       << 20 << "Μαλαπάππας" << 83.25;

    uint8_t age = 21;
    float weight = 68.5;
    std::string name = "jack";
    db << u"INSERT INTO user (age, name, weight) VALUES (?,?,?);" 
       << age << name << weight;
}

int32_t main() {
    crow::SimpleApp app;
    sqlite::database db("dbfile.db");
    initialize_database(db);

    CROW_ROUTE(app, "/add_user")
    ([&db](const crow::request& request) {
        std::expected<uint8_t, crow::response> check_age = u8_validator(request.url_params.get("age"));
        uint8_t age;
        if (check_age) {
            age = *check_age;
        } else {
            return crow::response(400, "INVALID AGE");
        }

        std::string name = request.url_params.get("name");
        float weight = std::stof(request.url_params.get("weight"));

        db << u"INSERT INTO user (age, name, weight) VALUES (?,?,?);"
           << age << name << weight;

        return crow::response(200, "OK");
    });

    CROW_ROUTE(app, "/edit_user")
    ([&db](const crow::request& request) {
        uint32_t id = std::stoi(request.url_params.get("id"));
        std::expected<uint8_t, crow::response> check_age = u8_validator(request.url_params.get("age"));
        uint8_t age;
        if (check_age) {
            age = *check_age;
        } else {
            return crow::response(400, "INVALID AGE");
        }
        
        std::string name = request.url_params.get("name");
        float weight = std::stof(request.url_params.get("weight"));

        db << u"UPDATE user SET age = ?, name = ?, weight = ? WHERE _id = ?;"
           << age << name << weight << id;

        return crow::response(200, "OK");
    });

    CROW_ROUTE(app, "/delete_user")
    ([&db](const crow::request& request) {
        uint32_t id = std::stoi(request.url_params.get("id"));

        db << u"DELETE FROM user WHERE _id = ?;" << id;

        return crow::response(200, "OK");
    });

    CROW_ROUTE(app, "/")
    ([&db]() {
        crow::mustache::template_t page = crow::mustache::load("users_template.html");
        crow::json::wvalue json_data;
        std::vector<crow::json::wvalue> users;

        db << "SELECT _id, age, name, weight FROM user;" >> [&users](const int32_t& id, const int32_t& age, const std::string& name, const float& weight) {
            crow::json::wvalue user;
            user["id"] = id;
            user["age"] = age;
            user["name"] = name;
            user["weight"] = weight;
            users.emplace_back(user);
        };

        json_data["users"] = std::move(users);

        return page.render(json_data);
    });

    CROW_ROUTE(app, "/search_user")
    ([&db](const crow::request& request) {
        std::string query = request.url_params.get("query");
        crow::json::wvalue json_data;
        std::vector<crow::json::wvalue> users;

        db << "SELECT _id, age, name, weight FROM user WHERE name LIKE ?;" << ('%' + query + '%') >> [&users](const int32_t& id, const int32_t& age, const std::string& name, const float& weight) {
            crow::json::wvalue user;
            user["id"] = id;
            user["age"] = age;
            user["name"] = name;
            user["weight"] = weight;
            users.emplace_back(user);
        };

        json_data["users"] = std::move(users);

        return crow::response(json_data);
    });

    app.bindaddr("192.168.2.7").port(18080).run();
}
//clang++ -Wall -Wextra -fsanitize=address crowdb.cpp -o crowdb -std=c++23 -lwsock32 -lws2_32 -lsqlite3