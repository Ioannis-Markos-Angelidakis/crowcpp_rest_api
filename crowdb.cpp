#include <sqlite_modern_cpp.h>
#include <crow_all.h>
#include <expected>
#include <filesystem>
#include <random>
#include <expected>
#include <string>

namespace fs = std::filesystem;

struct image_format {
    std::string_view jpg = "\xFF\xD8\xFF";
    std::string_view png = "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A";
    std::string_view gif = "\x47\x49\x46\x38";
    std::string_view bmp = "\x42\x4D";
}image;
 
struct current_user {
    bool logged_in;
    uint32_t id;
    std::string session_token;
    std::string error_message;
};

std::string encrypt_decrypt(const std::string& message, const std::string& key) { //Simple XOR encrypt
    size_t key_length = key.length();
    std::string output = message;

    for (size_t i = 0; i < message.length(); ++i) {
        output.at(i) = message.at(i) ^ key.at(i % key_length);
    }

    return output;
}

std::expected<uint32_t, std::string> u32_validator(const std::string& is_num) {
    try {
        uintmax_t num = std::stoul(is_num);
        if (num <= UINT32_MAX) {
            return static_cast<uint32_t>(num);
        }
    } catch (std::invalid_argument const& ex) {
        return std::unexpected("NOT VALID U32");
    }
    return std::unexpected("NOT VALID U32");
}

current_user is_authorized(const crow::request& request, sqlite::database& db) {
    std::string id_from_request = request.url_params.get("id") ? request.url_params.get("id") : "";
    std::string cookie = request.get_header_value("Cookie");

    if (id_from_request.empty() || cookie.empty()) {
        return {false, 0, "", "ERROR: Missing ID or NOT auth"};
    }

    std::expected<uint8_t, std::string> id = u32_validator(id_from_request);
    std::string retrieved_session_token;
    uint32_t user_id;

    if (id) {
        user_id = id.value();
    } else {
        return {false, 0, "", "ERROR: Wrong ID format"};
    }

    db << "SELECT session_token FROM sessions WHERE user_id = ?;"
       << user_id 
       >> [&retrieved_session_token](const std::string& token) { retrieved_session_token = token; };

    std::string cookie_name = "session_token=";
    size_t start = cookie.find(cookie_name);
    if (start == std::string::npos) {
        return {false, 0, "", "ERROR: Missing session token in cookie"};
    }
    std::string session_token{cookie, start + cookie_name.size(), retrieved_session_token.size()};

    if ((retrieved_session_token != session_token) || retrieved_session_token.empty()) {
        return {false, 0, "", "ERROR: Not authorized"};
    }

    return {true, user_id, retrieved_session_token, ""};
}

std::string session_token() {
    const std::string characters = 
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "!@#$%^&*()-_=+[]{}|:,.<>?/";

    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, characters.size() - 1);

    std::string random_string;

    for (uint32_t i = 0; i < 20; ++i) {
        random_string += characters[distribution(generator)];
    }

    return random_string;
}

void display_image(crow::response& res, const std::string& filepath) {
    std::ifstream file(filepath, std::ifstream::in | std::ios::binary);

    if (file) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.add_header("Content-Length", std::to_string(buffer.str().size()));
        res.write(buffer.str());
        res.end();
    } else {
        res.code = 404;
        res.write("File not found");
        res.end();
    }
}

void create_database(sqlite::database& db) {
    db << "CREATE TABLE IF NOT EXISTS user ("
          "   _id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
          "   name VARCHAR(25) NOT NULL,"
          "   email VARCHAR(25) NOT NULL,"
          "   password TEXT NOT NULL,"
          "   profile_picture TEXT" 
          ");";

    db << "CREATE TABLE IF NOT EXISTS sessions ("
          "   user_id INTEGER NOT NULL,"
          "   session_token CHAR(20) NOT NULL"
          ");";
}

int32_t main() {
    crow::SimpleApp app;
    sqlite::database db("dbfile.db");
    create_database(db);

    CROW_ROUTE(app, "/profile_pictures/<string>") ([](const crow::request& req, crow::response& res, std::string filename) {
        std::string filepath = "./profile_pictures/" + filename; 
        display_image(res, filepath);
    });

    CROW_ROUTE(app, "/register").methods(crow::HTTPMethod::Post)([&db](const crow::request& request) {
        crow::multipart::message file_message(request);
        std::string name;
        std::string email;
        std::string password;
        std::string profile_picture_path; 

        for (const auto& [part_name, part_value] : file_message.part_map) {
            if (part_name == "name" && part_value.body.empty()) {
                return crow::response(400, "Name is EMPTY");
            } else if (part_name == "name") {
                name = part_value.body;
            } else if (part_name == "email" && part_value.body.empty()) {
                return crow::response(400, "Email is EMPTY");
            } else if (part_name == "email") {
                std::string db_email;
                email = part_value.body;
                db << "SELECT email FROM user WHERE email = ?;"
                   << email
                   >> [&db_email](const std::string _email){ db_email = _email; };

                if (!db_email.empty()) {
                    return crow::response(400, "Email already exists");
                }
            } else if (part_name == "password" && part_value.body.empty()) {
                return crow::response(400, "Password is EMPTY");
            } else if (part_name == "password") {
                std::string key = session_token();
                password = key + encrypt_decrypt(part_value.body, key);
            }
        }

        for (const auto& [part_name, part_value] : file_message.part_map) {
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

                if (part_value.body.starts_with(image.jpg) || part_value.body.starts_with(image.png) || part_value.body.starts_with(image.gif) || part_value.body.starts_with(image.bmp)) {
                    std::ofstream out_file(unique_filename, std::ofstream::out | std::ios::binary);
                    out_file.write(part_value.body.data(), part_value.body.length());
                    profile_picture_path = unique_filename;
                } else {
                    return crow::response(400, "UPLOAD FAILED: Not an image file");
                }
            }
        }

        std::string token = session_token();
        db << "INSERT INTO user (name, email, password, profile_picture) VALUES (?, ?, ?, ?);"
           << name << email << password << profile_picture_path;

        int32_t user_id;
        db << "SELECT last_insert_rowid();" 
           >> user_id;

        db << "INSERT INTO sessions (user_id, session_token) VALUES (?, ?);" 
           << user_id << token;

        crow::response response(301, "/logged_in"); 
        response.add_header("Set-Cookie", "session_token=" + token + "; Path=/; HttpOnly");
        response.add_header("Location", "/logged_in?id=" + std::to_string(user_id));
        return response;
    });

    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::Post)([&db](const crow::request& request) {
        crow::multipart::message file_message(request);
        std::string email;
        std::string password;

        for (const auto& [part_name, part_value] : file_message.part_map) {
            if (part_name == "email" && part_value.body.empty()) {
                return crow::response(400, "Email is EMPTY");
            } else if (part_name == "email") {
                email = part_value.body;
            } else if (part_name == "password" &&part_value.body.empty()) {
                return crow::response(400, "Password is EMPTY");
            } else if (part_name == "password") {
                password = part_value.body;
            }
        }

        std::string token = session_token();
        int32_t user_id;
        bool id_exists = false;

        db << "SELECT _id FROM user WHERE email = ?;" 
           << email
           >> [&user_id, &id_exists](const int32_t id){ id_exists = true; user_id = id; };

        if (!id_exists) {
            return crow::response(400, "Wrong email");
        } 

        std::string db_password;
        db << "SELECT password FROM user WHERE _id = ?;"
           << user_id
           >> [&db_password](const std::string& pass){ db_password = pass ;};
        
        if (encrypt_decrypt(db_password.substr(20), db_password.substr(0, 20)) != password) {
            return crow::response(400, "Wrong password");
        }

        db << "INSERT INTO sessions (user_id, session_token) VALUES (?, ?);" 
           << user_id << token;

        crow::response response(301, "/logged_in"); 
        response.add_header("Set-Cookie", "session_token=" + token + "; Path=/; HttpOnly");
        response.add_header("Location", "/logged_in?id=" + std::to_string(user_id));
        return response;
    });

    CROW_ROUTE(app, "/logged_in") ([&db](const crow::request& request) {
        current_user user = is_authorized(request, db);

        if (user.logged_in) {
            crow::mustache::template_t page = crow::mustache::load("logged_in.html");
            crow::json::wvalue json_data;
            std::vector<crow::json::wvalue> users;

            db << "SELECT _id, name, email, password, profile_picture FROM user WHERE _id = ?;" 
            << user.id 
            >> [&users](const int32_t& id, const std::string& name, const std::string& email, const std::string& password, const std::string& profile_picture) {
                crow::json::wvalue user;
                user["id"] = id;
                user["name"] = name; 
                user["email"] = email;
                user["password"] = encrypt_decrypt(password.substr(20), password.substr(0, 20));
                user["profile_picture"] = profile_picture; 
                users.emplace_back(user);
            };

            json_data["users"] = std::move(users);
            return page.render(json_data);
        }

        crow::mustache::template_t page = crow::mustache::load("error.html");
        crow::json::wvalue context;
        return page.render(context);  
    }); 

    CROW_ROUTE(app, "/edit_user").methods(crow::HTTPMethod::Post)([&db](const crow::request& request) {
        crow::multipart::message file_message(request);
        current_user user = is_authorized(request, db);

        if (user.logged_in) {
            std::string name;
            std::string email;
            std::string password;
            std::string remove_profile_pic;

            for (const auto& [part_name, part_value] : file_message.part_map) {
                if (part_name == "name") {
                    name = part_value.body;
                } else if (part_name == "email") {
                    email = part_value.body;
                } else if (part_name == "password") {
                    std::string key = session_token();
                    password = key + encrypt_decrypt(part_value.body, key);
                } else if (part_name == "remove_picture") {
                    remove_profile_pic = part_value.body;
                }
            }

            if (name.empty() || email.empty() || password.empty()) {
                return crow::response(400, "ERROR: Empty field");
            }

            std::string profile_picture_path;
            for (const auto& [part_name, part_value] : file_message.part_map) {
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

                    if (part_value.body.starts_with(image.jpg) || part_value.body.starts_with(image.png) || part_value.body.starts_with(image.gif) || part_value.body.starts_with(image.bmp)) {
                        std::ofstream out_file(unique_filename, std::ofstream::out | std::ios::binary);
                        out_file.write(part_value.body.data(), part_value.body.length());
                        profile_picture_path = unique_filename;
                    } else {
                        return crow::response(400, "UPLOAD FAILED: Not an image file");
                    }
                }
            }

            if (!profile_picture_path.empty() || !remove_profile_pic.empty()) {
                std::string profile_picture_delete;

                db << "SELECT profile_picture FROM user WHERE _id = ?;" << user.id
                >> [&profile_picture_delete](const std::string& profile_picture) {
                    profile_picture_delete = profile_picture;
                };

                std::error_code error;
                if ((!profile_picture_path.empty() || !remove_profile_pic.empty()) && fs::exists(profile_picture_delete)) {
                    fs::remove(profile_picture_delete, error);
                } 
                
                if (error) {
                    return crow::response(500, "Error deleting profile picture");
                }

                db << "UPDATE user SET name = ?, email = ?, password = ?, profile_picture = ? WHERE _id = ?;"
                << name << email << password << profile_picture_path << user.id;
            } else {
                db << "UPDATE user SET name = ?, email = ?, password = ? WHERE _id = ?;"
                << name << email << password << user.id;
            }

            return crow::response(200, "USER UPDATED SUCCESSFULLY");
        }
        return crow::response(400, user.error_message);
    });

    CROW_ROUTE(app, "/delete_user") ([&db](const crow::request& request) {
        current_user user = is_authorized(request, db);std::string profile_picture_path;

        if (user.logged_in) {
            db << "SELECT profile_picture FROM user WHERE _id = ?;" << user.id
            >> [&](const std::string& profile_picture) {
                profile_picture_path = profile_picture;
            };

            db << "DELETE FROM user WHERE _id = ?;" << user.id;
            
            db << "DELETE FROM sessions WHERE user_id = ?;"
               << user.id;

            std::error_code error;
            if (!profile_picture_path.empty() && fs::exists(profile_picture_path)) {
                fs::remove(profile_picture_path, error);
            } 
            
            if (error) {
                return crow::response(500, "Error deleting profile picture");
            }

            return crow::response(200, "User and profile picture deleted successfully");
        }
        return crow::response(400, user.error_message);
    });

    CROW_ROUTE(app, "/logout") ([&db](const crow::request& request) {
        current_user user = is_authorized(request, db);

        if (user.logged_in) {
            db << "SELECT session_token FROM sessions WHERE user_id = ?;"
            << user.id;

            db << "DELETE FROM sessions WHERE session_token = ?;"
            << user.session_token;

            crow::response response(301, "/"); 
            response.add_header("Location", "/");
            return response;
        }
        return crow::response(400, user.error_message);
    });

    CROW_ROUTE(app, "/") ([&db]() {
        crow::mustache::template_t page = crow::mustache::load("index.html");
        crow::json::wvalue json_data;

        return page.render(json_data);
    });

    app.bindaddr("YOUR IP").port(18080).run();
}
