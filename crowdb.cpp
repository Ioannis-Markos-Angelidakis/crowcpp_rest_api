#include <sqlite_modern_cpp.h>
#include <crow_all.h>
#include <expected>
#include <filesystem>
#include <random>
#include <expected>
#include <string>
#include <bitset>

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

bool is_symbol(const uint32_t codepoint) {
    return
        (codepoint >= 0x0021 && codepoint <= 0x002F) || // ! " # $ % & ' ( ) * + , - . /
        (codepoint >= 0x003A && codepoint <= 0x0040) || // : ; < = > ? @
        (codepoint >= 0x005B && codepoint <= 0x0060) || // [ \ ] ^ _ `
        (codepoint >= 0x007B && codepoint <= 0x007E) || // { | } ~
        (codepoint == 0x20AC) || // €
        (codepoint >= 0x2000 && codepoint <= 0x206F) || // General Punctuation
        (codepoint >= 0x2100 && codepoint <= 0x214F) || // Letterlike Symbols
        (codepoint >= 0x2200 && codepoint <= 0x22FF) || // Mathematical Operators
        (codepoint >= 0x2300 && codepoint <= 0x23FF) || // Miscellaneous Technical
        (codepoint >= 0x2400 && codepoint <= 0x243F) || // Control Pictures
        (codepoint >= 0x2440 && codepoint <= 0x245F) || // Optical Character Recognition
        (codepoint >= 0x2500 && codepoint <= 0x257F) || // Box Drawing
        (codepoint >= 0x2580 && codepoint <= 0x259F) || // Block Elements
        (codepoint >= 0x25A0 && codepoint <= 0x25FF) || // Geometric Shapes
        (codepoint >= 0x2600 && codepoint <= 0x26FF) || // Miscellaneous Symbols
        (codepoint >= 0x2700 && codepoint <= 0x27BF) || // Dingbats
        (codepoint >= 0x2B50 && codepoint <= 0x2B59) || // Miscellaneous Symbols and Pictographs
        (codepoint >= 0x1F300 && codepoint <= 0x1F5FF) || // Miscellaneous Symbols and Pictographs
        (codepoint >= 0x1F600 && codepoint <= 0x1F64F) || // Emoticons
        (codepoint >= 0x1F680 && codepoint <= 0x1F6FF) || // Transport and Map Symbols
        (codepoint >= 0x1F700 && codepoint <= 0x1F77F);   // Alchemical Symbols
}

bool is_uppercase(const uint32_t codepoint) {
    return
        (codepoint >= 0x0041 && codepoint <= 0x005A) || // Basic Latin A-Z
        (codepoint >= 0x00C0 && codepoint <= 0x00D6) || // Latin-1 Supplement À-Ö
        (codepoint >= 0x00D8 && codepoint <= 0x00DE) || // Latin-1 Supplement Ø-Þ
        (codepoint >= 0x0100 && codepoint <= 0x017F) || // Latin Extended-A
        (codepoint >= 0x0180 && codepoint <= 0x024F) || // Latin Extended-B
        (codepoint >= 0x0410 && codepoint <= 0x042F) || // Cyrillic А-Я
        (codepoint >= 0x0391 && codepoint <= 0x03A9) || // Greek and Coptic Α-Ω
        (codepoint >= 0x0531 && codepoint <= 0x0556) || // Armenian
        (codepoint >= 0x05D0 && codepoint <= 0x05EA) || // Hebrew
        (codepoint >= 0x0600 && codepoint <= 0x06C0) || // Arabic (some uppercase)
        (codepoint >= 0x0780 && codepoint <= 0x07A5) || // Thaana
        (codepoint >= 0x0905 && codepoint <= 0x0939) || // Devanagari
        (codepoint >= 0x0985 && codepoint <= 0x0995) || // Bengali
        (codepoint >= 0x0A05 && codepoint <= 0x0A0A) || // Gurmukhi
        (codepoint >= 0x0A85 && codepoint <= 0x0A8D) || // Gujarati
        (codepoint >= 0x0B05 && codepoint <= 0x0B0C) || // Oriya
        (codepoint >= 0x0B85 && codepoint <= 0x0B9A) || // Tamil
        (codepoint >= 0x0C05 && codepoint <= 0x0C0C) || // Telugu
        (codepoint >= 0x0C85 && codepoint <= 0x0C8C) || // Kannada
        (codepoint >= 0x0D05 && codepoint <= 0x0D0C) || // Malayalam
        (codepoint >= 0x1000 && codepoint <= 0x102A) || // Myanmar
        (codepoint >= 0x10A0 && codepoint <= 0x10C5) || // Georgian
        (codepoint >= 0x1100 && codepoint <= 0x1159) || // Hangul Jamo
        (codepoint >= 0xAC00 && codepoint <= 0xD7A3);   // Hangul Syllables
}

bool is_pass_ok(const std::string& str) {
    bool has_upper = false;
    bool has_symbols = false;
    bool has_num = false;
    
    for (size_t i = 0; i < str.length(); ) {
        uint32_t codepoint = 0;
        
        if ((str.at(i) & 0x80) == 0x00) { // 1-byte UTF-8
            codepoint = str.at(i);
            ++i;
        } else if ((str.at(i) & 0xE0) == 0xC0) { // 2-byte UTF-8
            codepoint = ((str.at(i) & 0x1F) << 6) | (str.at(i + 1) & 0x3F);
            i += 2;
        } else if ((str.at(i) & 0xF0) == 0xE0) { // 3-byte UTF-8
            codepoint = ((str.at(i) & 0x0F) << 12) | ((str.at(i + 1) & 0x3F) << 6) | (str.at(i + 2) & 0x3F);
            i += 3;
        } else if ((str.at(i) & 0xF8) == 0xF0) { // 4-byte UTF-8
            codepoint = ((str.at(i) & 0x07) << 18) | ((str.at(i + 1) & 0x3F) << 12) | ((str.at(i + 2) & 0x3F) << 6) | (str.at(i + 3) & 0x3F);
            i += 4;
        } else {
            return false;
        }
        
        if (is_uppercase(codepoint)) {
            has_upper = true;
        } else if (is_symbol(codepoint)) {
            has_symbols = true;
        } else if (codepoint >= 0x0030 && codepoint <= 0x0039) {
            has_num = true;
        }
        
        if (has_upper && has_num && has_symbols) {
            return true;
        }        
    }
    
    return has_upper && has_num && has_symbols;
}

std::bitset<32> get_codepoint(const std::string& str, size_t& i) {
    char32_t codepoint = 0;
    unsigned char c = str.at(i);
    if (c <= 0x7F) { // 1-byte UTF-8
        codepoint = c;
        ++i;
    } else if (c <= 0xDF) { // 2-byte UTF-8
        codepoint = ((c & 0x1F) << 6) | (str.at(i + 1) & 0x3F);
        i += 2;
    } else if (c <= 0xEF) { // 3-byte UTF-8
        codepoint = ((c & 0x0F) << 12) | ((str.at(i + 1) & 0x3F) << 6) | (str.at(i + 2) & 0x3F);
        i += 3;
    } else { // 4-byte UTF-8
        codepoint = ((c & 0x07) << 18) | ((str.at(i + 1) & 0x3F) << 12) | ((str.at(i + 2) & 0x3F) << 6) | (str.at(i + 3) & 0x3F);
        i += 4;
    }
    return std::bitset<32>(codepoint);
}

std::string codepoint_to_utf8(char32_t codepoint) {
    std::string result;
    if (codepoint <= 0x7F) {
        result += static_cast<char>(codepoint);
    } else if (codepoint <= 0x7FF) {
        result += static_cast<char>(0xC0 | (codepoint >> 6));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
        result += static_cast<char>(0xE0 | (codepoint >> 12));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else {
        result += static_cast<char>(0xF0 | (codepoint >> 18));
        result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
    return result;
}

std::string encrypt(const std::string& message, const std::string& key) {
    size_t key_length = key.length();
    std::string output;

    size_t message_index = 0;
    size_t key_index = 0;

    while (message_index < message.length()) {
        std::bitset<32> message_cp = get_codepoint(message, message_index);
        std::bitset<32> key_cp = get_codepoint(key, key_index);

        std::bitset<32> encrypted_cp = message_cp ^ key_cp;
        char32_t encrypted_codepoint = static_cast<char32_t>(encrypted_cp.to_ulong());

        output += codepoint_to_utf8(encrypted_codepoint);

        key_index = (key_index + 1) % key_length;
    }

    return key + output;
}

std::string decrypt(const std::string& password) {
    std::string key = password.substr(0, 20);
    std::string encrypted_part = password.substr(20);
    size_t key_length = key.length();
    std::string output;

    size_t message_index = 0;
    size_t key_index = 0;

    while (message_index < encrypted_part.length()) {
        std::bitset<32> encrypted_cp = get_codepoint(encrypted_part, message_index);
        std::bitset<32> key_cp = get_codepoint(key, key_index);

        std::bitset<32> decrypted_cp = encrypted_cp ^ key_cp;
        char32_t decrypted_codepoint = static_cast<char32_t>(decrypted_cp.to_ulong());

        output += codepoint_to_utf8(decrypted_codepoint);

        key_index = (key_index + 1) % key_length;
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
    std::string cookie = request.get_header_value("Cookie");

    if (cookie.empty()) {
        return {false, 0, "", "ERROR: Missing ID or NOT auth"};
    }

    std::string cookie_name = "user_id=";
    size_t start = cookie.find(cookie_name);
    if (start == std::string::npos) {
        return {false, 0, "", "ERROR: Missing id in cookie"};
    }
    std::string id_from_cookie{cookie, start + cookie_name.size(), 4};

    std::expected<uint8_t, std::string> id = u32_validator(id_from_cookie);
    std::string retrieved_session_token;
    uint32_t user_id;

    if (id) {
        user_id = id.value();
    } else {
        return {false, 0, "", "ERROR: Wrong ID format"};
    }

    cookie_name = "session_token=";
    start = cookie.find(cookie_name);
    if (start == std::string::npos) {
        return {false, 0, "", "ERROR: Missing session token in cookie"};
    }

    std::string session_token{cookie, start + cookie_name.size(), 20};
    db << "SELECT session_token FROM sessions WHERE session_token = ? AND user_id = ?;"
       << session_token << user_id
       >> [&retrieved_session_token](const std::string& token) { retrieved_session_token = token; };



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

    db << "CREATE TABLE IF NOT EXISTS posts ("
          "   _id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
          "   user_id INTEGER NOT NULL,"
          "   content TEXT NOT NULL,"
          "   timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
          "   FOREIGN KEY(user_id) REFERENCES user(_id)"
          ");";
}

int32_t main() {
    crow::SimpleApp app;
    sqlite::database db("dbfile.db");
    create_database(db);

    CROW_ROUTE(app, "/load_posts").methods(crow::HTTPMethod::Get)([&db](const crow::request& request) {
        std::string page_param = (request.url_params.get("page"))? request.url_params.get("page") : "";
        current_user user = is_authorized(request, db);

        if (page_param.empty() || !user.logged_in) {
            crow::mustache::template_t page = crow::mustache::load("error.html");
            crow::json::wvalue context;
            return page.render(context); 
        }

        std::expected<uint32_t, std::string> page_num_exp = u32_validator(page_param);
        uint32_t page_num = 0;

        if (page_num_exp) {
            page_num = page_num_exp.value();
        } else {
            crow::mustache::template_t page = crow::mustache::load("error.html");
            crow::json::wvalue context;
            return page.render(context);  
        }

        crow::mustache::template_t page = crow::mustache::load("posts.html");
        crow::json::wvalue json_data;
        std::vector<crow::json::wvalue> posts;

        db << "SELECT content, timestamp FROM posts WHERE user_id = ? ORDER BY timestamp DESC LIMIT 10 OFFSET ?;" 
           << user.id << page_num * 10
           >> [&](std::string content, std::string timestamp) {
                crow::json::wvalue post;
                post["content"] = content;
                post["timestamp"] = timestamp;
                posts.emplace_back(post);
            };

        json_data["posts"] = std::move(posts);
        return page.render(json_data);
    });

    CROW_ROUTE(app, "/home").methods(crow::HTTPMethod::Get)([&db](const crow::request& request) {
        std::string page_param = (request.url_params.get("page"))? request.url_params.get("page") : "";
        current_user user = is_authorized(request, db);

        if (page_param.empty() || !user.logged_in) {
            crow::mustache::template_t page = crow::mustache::load("error.html");
            crow::json::wvalue context;
            return page.render(context); 
        }

        std::expected<uint32_t, std::string> page_num_exp = u32_validator(page_param);
        uint32_t page_num = 0;

        if (page_num_exp) {
            page_num = page_num_exp.value();
        } else {
            crow::mustache::template_t page = crow::mustache::load("error.html");
            crow::json::wvalue context;
            return page.render(context);  
        }

        crow::mustache::template_t page = crow::mustache::load("posts.html");
        crow::json::wvalue json_data;
        std::vector<crow::json::wvalue> posts;

        db << "SELECT content, timestamp FROM posts ORDER BY timestamp DESC LIMIT 10 OFFSET ?;" 
           << page_num * 10
           >> [&](std::string content, std::string timestamp) {
                crow::json::wvalue post;
                post["content"] = content;
                post["timestamp"] = timestamp;
                posts.emplace_back(post);
            };

        json_data["posts"] = std::move(posts);
        return page.render(json_data);
    });

    CROW_ROUTE(app, "/submit_post").methods(crow::HTTPMethod::Post)([&db](const crow::request& request) {
        std::string content = crow::json::load(request.body)["content"].s();
        current_user user = is_authorized(request, db);

        if (!user.logged_in) {
            return crow::response(400, "Not authorized");
        }

        if (content.empty()) {
            return crow::response(400, "Content cannot be empty");
        }

        db << "INSERT INTO posts (user_id, content) VALUES (?, ?);" 
           << user.id << content;
        return crow::response(200, "Post created successfully");
    });

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
                    if (is_pass_ok(part_value.body) && part_value.body.length() >= 8) {
                        std::string key = session_token();
                        password = encrypt(part_value.body, key);
                    } else {
                        return crow::response(400, "ERROR: Password needs to be at least 8 chars with a symbol, number, capital letter");
                    }
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
        response.add_header("Set-Cookie", "user_id=" + std::to_string(user_id) + "; Path=/; HttpOnly");
        response.add_header("Location", "/");
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
        
        if (decrypt(db_password) != password) {
            return crow::response(400, "Wrong password");
        }

        db << "INSERT INTO sessions (user_id, session_token) VALUES (?, ?);" 
           << user_id << token;

        crow::response response(301, "/logged_in"); 
        response.add_header("Set-Cookie", "session_token=" + token + "; Path=/; HttpOnly");
        response.add_header("Set-Cookie", "user_id=" + std::to_string(user_id) + "; Path=/; HttpOnly");
        response.add_header("Location", "/");
        return response;
    });

    CROW_ROUTE(app, "/profile") ([&db](const crow::request& request) {
        current_user user = is_authorized(request, db);

        if (user.logged_in) {
            crow::mustache::template_t page = crow::mustache::load("profile.html");
            crow::json::wvalue json_data;
            std::vector<crow::json::wvalue> users;

            db << "SELECT _id, name, email, password, profile_picture FROM user WHERE _id = ?;" 
            << user.id 
            >> [&users](const int32_t& id, const std::string& name, const std::string& email, const std::string& password, const std::string& profile_picture) {
                crow::json::wvalue user;
                user["id"] = id;
                user["name"] = name; 
                user["email"] = email;
                user["password"] = decrypt(password);
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
                    if (is_pass_ok(part_value.body) && part_value.body.length() >= 8) {
                        std::string key = session_token();
                        password = encrypt(part_value.body, key);
                    } else {
                        return crow::response(400, "ERROR: Password needs to be at least 8 chars with a symbol, number, capital letter");
                    }
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
        current_user user = is_authorized(request, db);
        std::string profile_picture_path;

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

    CROW_ROUTE(app, "/") ([&db](const crow::request& request) {
       current_user user = is_authorized(request, db);
        crow::mustache::template_t page = (user.logged_in)? crow::mustache::load("home.html") : crow::mustache::load("index.html");
        crow::json::wvalue json_data;

        return page.render(json_data);
    });

    app.bindaddr("ENTER HOSTING IPV4").port(18080).run();
}
