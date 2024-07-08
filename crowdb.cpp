#include <sqlite_modern_cpp.h>
#include <crow_all.h>
#include <expected>
#include <filesystem>

namespace fs = std::filesystem;

void display_image(crow::response& res, const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (file) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        res.add_header("Content-Length", std::to_string(buffer.str().size()));
        res.write(buffer.str());
        res.end();
    } else {
        res.code = 404;
        res.write("File not found");
        res.end();
    }
}

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
       "   weight REAL,"
       "   profile_picture TEXT" 
       ");";

    db << "INSERT INTO user (age, name, weight) VALUES (?, ?, ?);"
       << 20 << "Test" << 83.25;

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

    CROW_ROUTE(app, "/profile_pictures/<string>")
    ([](const crow::request& req, crow::response& res, std::string filename) {
        std::string filepath = "./profile_pictures/" + filename; 
        display_image(res, filepath);
    });

    CROW_ROUTE(app, "/add_user").methods(crow::HTTPMethod::Post)([&db](const crow::request& request) {
        crow::multipart::message file_message(request);

        std::string name;
        uint8_t age;
        float weight;
        std::string profile_picture_path;  

        for (const std::pair<const std::string, crow::multipart::part>& part : file_message.part_map) {
            const  std::string& part_name = part.first;
            const  crow::multipart::part& part_value = part.second;

            if (part_name == "file") {
                auto headers_it = part_value.headers.find("Content-Disposition");
                if (headers_it == part_value.headers.end()) {
                    return crow::response(400, "UPLOAD FAILED");
                }

                auto params_it = headers_it->second.params.find("filename");
                if (params_it == headers_it->second.params.end()) {
                    return crow::response(400);
                }

                std::string_view filename = params_it->second;

                std::string upload_dir = "./profile_pictures/";
                fs::create_directories(upload_dir);
                std::string unique_filename = upload_dir + std::string(filename);

                if (fs::exists(unique_filename)) {
                    return crow::response(400, "UPLOAD FAILED: File exists");
                }

                // JPEG, PNG, GIF, BMP
                if (part_value.body.starts_with("\xFF\xD8\xFF") || part_value.body.starts_with("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A") || part_value.body.starts_with("\x47\x49\x46\x38") || part_value.body.starts_with("\x42\x4D")) {
                    std::ofstream out_file(unique_filename, std::ios::out | std::ios::binary);
                    out_file.write(part_value.body.data(), part_value.body.length());
                    profile_picture_path = unique_filename;
                } else {
                    return crow::response(400, "UPLOAD FAILED: Not an image file");
                }


            } else if (part_name == "age") {
                std::expected<uint8_t, crow::response> check_age = u8_validator(part_value.body);
                if (check_age) {
                    age = *check_age;
                } else {
                    std::error_code error;
                    if (!profile_picture_path.empty() && fs::exists(profile_picture_path)) {
                        fs::remove(profile_picture_path, error);
                    } 
                    return crow::response(400, "INVALID AGE");
                }
            } else if (part_name == "name") {
                name = part_value.body;
            } else if (part_name == "weight") {
                try {
                    weight = std::stof(part_value.body);
                } catch (const std::exception& e) {
                    std::error_code error;
                    if (!profile_picture_path.empty() && fs::exists(profile_picture_path)) {
                        fs::remove(profile_picture_path, error);
                    } 
                    return crow::response(400, "INVALID WEIGHT");
                }
            }
        }

        db << "INSERT INTO user (age, name, weight, profile_picture) VALUES (?, ?, ?, ?);"
        << age << name << weight << profile_picture_path;

        return crow::response(200, "USER CREATED SUCCESSFULLY");
    });

    CROW_ROUTE(app, "/edit_user").methods(crow::HTTPMethod::Post)([&db](const crow::request& request) {
        crow::multipart::message file_message(request);

        uint32_t id = std::stoi(request.url_params.get("id"));
        std::string name;
        uint8_t age;
        float weight;
        std::string profile_picture_path;

        for (const std::pair<const std::string, crow::multipart::part>& part : file_message.part_map) {
            const  std::string& part_name = part.first;
            const  crow::multipart::part& part_value = part.second;

            if (part_name == "file") {
                auto headers_it = part_value.headers.find("Content-Disposition");
                if (headers_it == part_value.headers.end()) {
                    return crow::response(400, "UPLOAD FAILED");
                }

                auto params_it = headers_it->second.params.find("filename");
                if (params_it == headers_it->second.params.end()) {
                    return crow::response(400);
                }

                std::string_view filename = params_it->second;
                std::string upload_dir = "./profile_pictures/";
                if (!fs::exists(upload_dir)) {
                    fs::create_directories(upload_dir);
                }
                std::string unique_filename = upload_dir + std::string(filename);

                if (fs::exists(unique_filename)) {
                    return crow::response(400, "UPLOAD FAILED: FILE EXISTS");
                }

                // JPEG, PNG, GIF, BMP
                if (part_value.body.starts_with("\xFF\xD8\xFF") || part_value.body.starts_with("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A") || part_value.body.starts_with("\x47\x49\x46\x38") || part_value.body.starts_with("\x42\x4D")) {
                    std::ofstream out_file(unique_filename, std::ios::out | std::ios::binary);
                    out_file.write(part_value.body.data(), part_value.body.length());
                    profile_picture_path = unique_filename;
                } else {
                    return crow::response(400, "UPLOAD FAILED: Not an image file");
                }

            } else if (part_name == "age") {
                std::expected<uint8_t, crow::response> check_age = u8_validator(part_value.body);
                if (check_age) {
                    age = *check_age;
                } else {
                    std::error_code error;
                    if (!profile_picture_path.empty() && fs::exists(profile_picture_path)) {
                        fs::remove(profile_picture_path, error);
                    } 
                    return crow::response(400, "INVALID AGE");
                }
            } else if (part_name == "name") {
                name = part_value.body;
            } else if (part_name == "weight") {
                try {
                    weight = std::stof(part_value.body);
                } catch (const std::exception& e) {
                    std::error_code error;
                    if (!profile_picture_path.empty() && fs::exists(profile_picture_path)) {
                        fs::remove(profile_picture_path, error);
                    } 
                    return crow::response(400, "INVALID WEIGHT");
                }
            }
        }

        if (!profile_picture_path.empty()) {
            std::string profile_picture_delete;

            db << "SELECT profile_picture FROM user WHERE _id = ?;" << id
            >> [&](const std::string& profile_picture) {
                profile_picture_delete = profile_picture;
            };

            std::error_code error;
            if (!profile_picture_path.empty() && fs::exists(profile_picture_delete)) {
                fs::remove(profile_picture_delete, error);
            } 
            
            if (error) {
                return crow::response(500, "Error deleting profile picture");
            }

            db << "UPDATE user SET age = ?, name = ?, weight = ?, profile_picture = ? WHERE _id = ?;"
            << age << name << weight << profile_picture_path << id;
        } else {
            db << "UPDATE user SET age = ?, name = ?, weight = ? WHERE _id = ?;"
            << age << name << weight << id;
        }

        return crow::response(200, "USER UPDATED SUCCESSFULLY");
    });

    CROW_ROUTE(app, "/delete_user")
    ([&db](const crow::request& request) {
        uint32_t id = std::stoi(request.url_params.get("id"));
        std::string profile_picture_path;

        db << "SELECT profile_picture FROM user WHERE _id = ?;" << id
        >> [&](const std::string& profile_picture) {
            profile_picture_path = profile_picture;
        };

        db << "DELETE FROM user WHERE _id = ?;" << id;

        std::error_code error;
        if (!profile_picture_path.empty() && fs::exists(profile_picture_path)) {
            fs::remove(profile_picture_path, error);
        } 
        
        if (error) {
            return crow::response(500, "Error deleting profile picture");
        }

        return crow::response(200, "User and profile picture deleted successfully");
    });

    CROW_ROUTE(app, "/")
    ([&db]() {
        crow::mustache::template_t page = crow::mustache::load("users_template.html");
        crow::json::wvalue json_data;
        std::vector<crow::json::wvalue> users;

        db << "SELECT _id, age, name, weight, profile_picture FROM user;" >> [&users](const int32_t& id, const int32_t& age, const std::string& name, const float& weight, const std::string& profile_picture) {
            crow::json::wvalue user;
            user["id"] = id;
            user["age"] = age;
            user["name"] = name;
            user["weight"] = weight;
            user["profile_picture"] = profile_picture; 
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

    app.bindaddr("192.168.0.21").port(18080).run();
}
//clang++ -Wall -Wextra -fsanitize=address crowdb.cpp -o crowdb -std=c++23 -lwsock32 -lws2_32 -lsqlite3