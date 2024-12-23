#include <sqlite_modern_cpp.h>
#include <crow_all.h>
#include <expected>
#include <filesystem>
#include <random>
#include <expected>
#include <string>
#include <bitset>

namespace fs = std::filesystem;

const std::string IP = "";
const std::string PORT = "";
const std::string SMPT_EMAIL = "";
const std::string SMPT_URL = "";
const std::string SMPT_AND_PASS = "";

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
    std::string name;
    bool verified;
    std::string error_message;
};

std::string escape_string_decode(const std::string& str) {
    std::string decoded_string;
    
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str.at(i);

        switch (c) {
            case '%': {
                if (i + 2 < str.length()) { 
                    std::istringstream hex_stream(str.substr(i + 1, 2));
                    int32_t hex_value;
                    if (hex_stream >> std::hex >> hex_value) {
                        decoded_string += static_cast<char>(hex_value);
                        i += 2; 
                    } else {
                        decoded_string += c; 
                    }
                }
                break;
            }
            case '+':
                decoded_string += ' ';
                break;
            default:
                decoded_string += c; 
                break;
        }
    }

    return decoded_string;
}

std::string escape_string_encode(const std::string& str) {
    std::ostringstream encoded_string;

    for (unsigned char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded_string << c;
        } else if (c == ' ') {
            encoded_string << '+';
        } else {
            encoded_string << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        }
    }

    return encoded_string.str();
}

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
        return {false, 0, "", "", false, "ERROR: Missing ID or NOT auth"};
    }

    std::string cookie_name = "user_id=";
    size_t start = cookie.find(cookie_name);
    if (start == std::string::npos) {
        return {false, 0, "", "", false, "ERROR: Missing id in cookie"};
    }
    std::string id_from_cookie{cookie, start + cookie_name.size(), 4};

    std::expected<uint8_t, std::string> id = u32_validator(id_from_cookie);
    std::string retrieved_session_token;
    std::string retrieved_user_name;
    bool retrieved_verified;
    uint32_t user_id;

    if (id) {
        user_id = id.value();
    } else {
        return {false, 0, "", "", false,  "ERROR: Wrong ID format"};
    }
 
    cookie_name = "session_token=";
    start = cookie.find(cookie_name);
    if (start == std::string::npos) {
        return {false, 0, "",  "", false, "ERROR: Missing session token in cookie"};
    }

    std::string session_token{cookie, start + cookie_name.size(), 20};
    db << "SELECT session_token FROM sessions WHERE session_token = ? AND user_id = ?;"
       << session_token << user_id
       >> [&retrieved_session_token](const std::string& token) { retrieved_session_token = token; };

    db << "SELECT name, verified FROM user WHERE _id = ?;"
       << user_id
       >> [&](const std::string& name, bool verified) { retrieved_user_name = name; retrieved_verified = verified; };

    if ((retrieved_session_token != session_token) || retrieved_session_token.empty()) {
        return {false, 0, "",  "", false, "ERROR: Not authorized"};
    }

    return {true, user_id,  retrieved_session_token, retrieved_user_name, retrieved_verified, ""};
}

std::string session_token(size_t size) {
    const std::string chars_range = 
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789"
    "!@#$%^&*()-_=+[]{}|:,.<>?/";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, chars_range.size() - 1);

    std::string random_string;

    for (uint32_t i = 0; i < size; ++i) {
        random_string += chars_range[distribution(generator)];
    }

    return random_string;
}

int32_t send_verification_email(const std::string& email_to, const std::string& key) {
    std::string link = "http://" + IP + ":" + PORT + "/verify/" + email_to + "/" + escape_string_encode(key);

    std::ostringstream email_content;
    email_content << "from: " << SMPT_EMAIL << "\n"
                  << "To: " << email_to << "\n"
                  << "Subject: Verify your email\n"
                  << "Content-Type: text/html; charset=UTF-8\n"
                  << "<html>\n"
                  << "<body>\n"
                  << "<h2><a href=\"" << link << "\">Click here to verify your email</a></h2>\n"
                  << "</body>\n"
                  << "</html>";

    std::string command = R"(curl --url ")" + SMPT_URL + R"(" --ssl-reqd )"
                          R"(--mail-from ")" + SMPT_EMAIL + R"(" )"
                          R"(--mail-rcpt ")" + email_to + R"(" )"
                          R"(--upload-file - )"  // Upload content from stdin
                          R"(--user ")" + SMPT_AND_PASS + R"(" )";

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.data(), "w"), pclose);
    if (!pipe) {
        throw std::runtime_error("Failed to open pipe to curl.");
    }

    std::string content = email_content.str();
    size_t written = fwrite(content.data(), sizeof(char), content.size(), pipe.get());
    if (written != content.size()) {
        throw std::runtime_error("Failed to write entire email content.");
    }

    int32_t result = pclose(pipe.release());
    return result;
}

int32_t send_reset_email(const std::string& email_to, const std::string& key) {
    std::string link = "http://" + IP + ":" + PORT + "/reset_password/" + email_to + "/" + escape_string_encode(key);
    
    std::ostringstream email_content;
    email_content << "from: " << SMPT_EMAIL << "\n"
                  << "To: " << email_to << "\n"
                  << "Subject: Reset password email\n"
                  << "Content-Type: text/html; charset=UTF-8\n"
                  << "<html>\n"
                  << "<body>\n"
                  << "<h2><a href=\"" << link << "\">Click here to reset</a></h2>\n"
                  << "<p>Click the link to set your new password.</p>\n"
                  << "</body>\n"
                  << "</html>";

    std::string command = R"(curl --url ")" + SMPT_URL + R"(" --ssl-reqd )"
                          R"(--mail-from ")" + SMPT_EMAIL + R"(" )"
                          R"(--mail-rcpt ")" + email_to + R"(" )"
                          R"(--upload-file - )"  // Upload content from stdin
                          R"(--user ")" + SMPT_AND_PASS + R"(" )";

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.data(), "w"), pclose);
    if (!pipe) {
        throw std::runtime_error("Failed to open pipe to curl.");
    }

    std::string content = email_content.str();
    size_t written = fwrite(content.data(), sizeof(char), content.size(), pipe.get());
    if (written != content.size()) {
        throw std::runtime_error("Failed to write entire email content.");
    }

    int32_t result = pclose(pipe.release());
    return result;
}

void create_database(sqlite::database& db) {
	db << R"(CREATE TABLE IF NOT EXISTS user (
		_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
		name VARCHAR(25) NOT NULL,
		email VARCHAR(25) UNIQUE NOT NULL,
		password TEXT NOT NULL,
		profile_picture TEXT,
        verified BOOLEAN DEFAULT FALSE
	);)";

	db << R"(CREATE TABLE IF NOT EXISTS to_verify (
		email VARCHAR(25) PRIMARY KEY NOT NULL,
		key VARCHAR(25) NOT NULL
	);)";

    db << R"(
        CREATE TABLE IF NOT EXISTS password_resets (
        email TEXT PRIMARY KEY NOT NULL,
        token TEXT NOT NULL,
        expires_at DATETIME
        );)";

	db << R"(CREATE TABLE IF NOT EXISTS sessions (
		user_id INTEGER NOT NULL,
		session_token CHAR(20) NOT NULL,
		PRIMARY KEY (user_id, session_token),
		FOREIGN KEY(user_id) REFERENCES user(_id)
	);)";

	db << R"(CREATE TABLE IF NOT EXISTS posts (
		_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
		user_id INTEGER NOT NULL,
		content TEXT NOT NULL,
        post_pic_url TEXT,
		timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
        time_edited DATETIME,
		FOREIGN KEY(user_id) REFERENCES user(_id)
	);)";

	db << R"(CREATE TABLE IF NOT EXISTS replies (
		_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
		post_id INTEGER NOT NULL,
		user_id INTEGER NOT NULL,
		content TEXT NOT NULL,
        author TEXT NOT NULL,
		timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
        time_edited DATETIME,
		parent_reply_id INTEGER,
		FOREIGN KEY(post_id) REFERENCES posts(_id),
		FOREIGN KEY(user_id) REFERENCES user(_id),
		FOREIGN KEY(parent_reply_id) REFERENCES replies(_id)
	);)";

	db << R"(CREATE TABLE IF NOT EXISTS post_likes (
		post_id INTEGER NOT NULL,
		user_id INTEGER NOT NULL,
		PRIMARY KEY (post_id, user_id),
		FOREIGN KEY(post_id) REFERENCES posts(_id),
		FOREIGN KEY(user_id) REFERENCES user(_id)
	);)";

	db << R"(CREATE TABLE IF NOT EXISTS reply_likes (
		reply_id INTEGER NOT NULL,
		user_id INTEGER NOT NULL,
		PRIMARY KEY (reply_id, user_id),
		FOREIGN KEY(reply_id) REFERENCES replies(_id),
		FOREIGN KEY(user_id) REFERENCES user(_id)
	);)";
}

int32_t main() {
    crow::SimpleApp app;
    sqlite::database db("dbfile.db");
    create_database(db);

    //email/token
    CROW_ROUTE(app, "/reset_password/<string>/<string>")([&db](std::string email, const std::string& token) {
        email = escape_string_decode(email);
        uint32_t db_token = 0;
        db << "SELECT COUNT(*) FROM password_resets WHERE (email = ? AND token = ?);" 
           << email << token 
           >> db_token; 

        crow::mustache::template_t page = crow::mustache::load("error.html");
        crow::json::wvalue json_data;
        std::vector<crow::json::wvalue> details;

        if (db_token <= 0) {
            json_data["error"] = "ERROR: Invalid details.";
            return page.render(json_data);
        }

        page = crow::mustache::load("reset_password.html");
        json_data["email"] = email;
        json_data["token"] = token;
        return page.render(json_data);
    });

    //email/token/new_password
    CROW_ROUTE(app, "/reset_password/<string>/<string>/<string>").methods(crow::HTTPMethod::Post)([&db](std::string email, const std::string& token, std::string new_password) {
        email = escape_string_decode(email);
        new_password = escape_string_decode(new_password);
        uint32_t db_token = 0;
        db << "SELECT COUNT(*) FROM password_resets WHERE (email = ? AND token = ?);" 
           << email << token 
           >> db_token;

        if (db_token <= 0) {
            return crow::response(400, "Invalid token");
        }

        if (!is_pass_ok(new_password) || new_password.size() < 8 || new_password.size() > 25) {
            return crow::response(400, "ERROR: Password needs to be  >= 8 <= 25 chars with a symbol, number, capital letter");
        }

        std::string key = session_token(20);
        std::string sanitized_password;
        std::ranges::for_each(new_password, [&sanitized_password](const char ch){ ((ch >= 0 && ch <= 31) || ch == 127)? "" :  sanitized_password += ch ; });
        std::string password = encrypt(sanitized_password, key);
        db << "UPDATE user SET password = ? WHERE email = ?;"
           << password << email;

        db << "DELETE FROM password_resets WHERE email = ?;" 
           << email;

        crow::response response(302, "/"); 
        response.add_header("Location", "/");
        return response;
    });

    CROW_ROUTE(app, "/forgot_password/<string>").methods(crow::HTTPMethod::Post)([&db](std::string email) {
        email = escape_string_decode(email);
        uint32_t user_email = 0;
        db << "SELECT COUNT(*) FROM user WHERE email = ?;" 
           << email 
           >> user_email;

        if (user_email <= 0) {
            return crow::response(400, "Email not found");
        }

        std::string reset_token = session_token(20);

        db << "INSERT OR REPLACE INTO password_resets (email, token, expires_at) VALUES (?, ?, datetime('now', 'localtime'));"
           << email << escape_string_encode(reset_token);

        send_reset_email(email, reset_token);

        return crow::response(200, "Password reset email sent");
    });

    CROW_ROUTE(app, "/like").methods(crow::HTTPMethod::Post)([&db](const crow::request& request) {
        crow::json::rvalue body = crow::json::load(request.body);

        if (!body || !body.has("post_id")) {
            return crow::response(400, "ERROR: Invalid JSON");
        }

        std::string post_id = body["post_id"].s();
        if(post_id.empty()) {
            return crow::response(400, "ERROR: Missing post_id");
        }

        current_user user = is_authorized(request, db);
        if (!user.logged_in || !user.verified) {
            return crow::response(400, "ERROR: Not authorized");
        }

        bool post_exists = false;
        db << "SELECT 1 FROM posts WHERE _id = ?;"
            << post_id 
            >> [&post_exists]() { post_exists = true; };

        if (post_id.empty() || !post_exists) {
            return crow::response(400, "ERROR: Invalid post ID");
        }

        bool like_exists = false;
        db << "SELECT 1 FROM post_likes WHERE post_id = ? AND user_id = ?;"
            << post_id << user.id
            >> [&like_exists]() { like_exists = true; };

        if (like_exists) {
            db << "DELETE FROM post_likes WHERE post_id = ? AND user_id = ?;"
               << post_id << user.id;
            return crow::response(200, "Like removed successfully");
        } else {
            db << "INSERT INTO post_likes (post_id, user_id) VALUES (?, ?);"
               << post_id << user.id;
            return crow::response(200, "Like added successfully");
        }
    });

    CROW_ROUTE(app, "/like_reply/<int>").methods(crow::HTTPMethod::Post)([&db](const crow::request& request, const int32_t reply_id) {
        current_user user = is_authorized(request, db);
        if (!user.logged_in || !user.verified) {
            return crow::response(400, "ERROR: Not authorized");
        }

        bool reply_exists = false;
        db << "SELECT 1 FROM replies WHERE _id = ?;"
            << reply_id
            >> [&reply_exists]() { reply_exists = true; };

        if (!reply_exists) {
            return crow::response(400, "ERROR: Invalid reply ID");
        }

        bool like_exists = false;
        db << "SELECT 1 FROM reply_likes WHERE reply_id = ? AND user_id = ?;"
            << reply_id << user.id
            >> [&like_exists]() { like_exists = true; };

        if (like_exists) {
            db << "DELETE FROM reply_likes WHERE reply_id = ? AND user_id = ?;"
               << reply_id << user.id;
            return crow::response(200, "Like removed successfully");
        } else {
            db << "INSERT INTO reply_likes (reply_id, user_id) VALUES (?, ?);"
               << reply_id << user.id;
            return crow::response(200, "Like added successfully");
        }
    });

    CROW_ROUTE(app, "/load_my_posts").methods(crow::HTTPMethod::Get)([&db](const crow::request& request) {
        std::string page_param = (request.url_params.get("page"))? request.url_params.get("page") : "";
        current_user user = is_authorized(request, db);

        if (page_param.empty() || !user.logged_in) {
            crow::mustache::template_t page = crow::mustache::load("error.html");
            crow::json::wvalue context;
            context["error"] = "Please log in!";
            return page.render(context); 
        } else if (!user.verified) {
            crow::mustache::template_t page = crow::mustache::load("error.html");
            crow::json::wvalue context;
            context["error"] = "Please verify your email first!";
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

        db << R"(
            SELECT 
                posts._id AS post_id, 
                posts.user_id AS author_id, 
                user.name AS author_name, 
                posts.content,
                posts.post_pic_url, 
                posts.timestamp, 
                posts.time_edited,
                COUNT(post_likes.user_id) AS like_count
            FROM posts
            LEFT JOIN post_likes ON posts._id = post_likes.post_id
            LEFT JOIN user ON posts.user_id = user._id
            WHERE user._id = ?
            GROUP BY posts._id
            ORDER BY posts.timestamp DESC 
            LIMIT 10 OFFSET ?;
        )" << user.id << page_num * 10 
           >>[&](const uint32_t post_id, uint32_t user_id,
                    const std::string &author_name, const std::string &content,
                    const std::string &post_pic_url, const std::string &timestamp,
                    const std::string &time_edited, const uint32_t like_count) {
                crow::json::wvalue post;
                post["post_id"] = post_id;
                post["user_id"] = user_id;
                if (user_id == user.id) {
                    post["editable"] = post_id;
                }
                post["author_name"] = author_name;
                post["content"] = content;
                if (!post_pic_url.empty()) {
                    post["post_pic_url"] = post_pic_url;
                }
                post["timestamp"] = timestamp;
                if (!time_edited.empty()) {
                    post["edited"] = time_edited;
                }
                post["like_count"] = like_count;
                posts.emplace_back(post);
            };

        json_data["posts"] = std::move(posts);
        return page.render(json_data);
    });

    CROW_ROUTE(app, "/load_users_posts/<int>").methods(crow::HTTPMethod::Get)([&db](const crow::request& request, const int32_t user_id) {
        std::string page_param = (request.url_params.get("page"))? request.url_params.get("page") : "";

        if (page_param.empty()) {
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

        if (user_id < 0) {
            crow::mustache::template_t page = crow::mustache::load("error.html");
            crow::json::wvalue context;
            return page.render(context); 
        }

        crow::mustache::template_t page = crow::mustache::load("posts.html");
        crow::json::wvalue json_data;
        std::vector<crow::json::wvalue> posts;
        current_user user = is_authorized(request, db);

        db << R"(
            SELECT 
                posts._id AS post_id, 
                posts.user_id AS author_id, 
                user.name AS author_name, 
                posts.content,
                posts.post_pic_url, 
                posts.timestamp, 
                posts.time_edited,
                COUNT(post_likes.user_id) AS like_count
            FROM posts
            LEFT JOIN post_likes ON posts._id = post_likes.post_id
            LEFT JOIN user ON posts.user_id = user._id
            WHERE author_id = ?
            GROUP BY posts._id
            ORDER BY posts.timestamp DESC 
            LIMIT 10 OFFSET ?;
        )" << user_id << page_num * 10 
           >> [&](const uint32_t post_id, uint32_t user_id,
                    const std::string &author_name, const std::string &content,
                    const std::string &post_pic_url, const std::string &timestamp,
                    const std::string &time_edited, const uint32_t like_count) {
                crow::json::wvalue post;
                post["post_id"] = post_id;
                post["user_id"] = user_id;
                if (user_id == user.id) {
                    post["editable"] = post_id;
                }
                post["author_name"] = author_name;
                post["content"] = content;
                if (!post_pic_url.empty()) {
                    post["post_pic_url"] = post_pic_url;
                }
                post["timestamp"] = timestamp;
                if (!time_edited.empty()) {
                    post["edited"] = time_edited;
                }
                post["like_count"] = like_count;
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
            context["error"] = "Please log in!";
            return page.render(context); 
        } else if (!user.verified) {
            crow::mustache::template_t page = crow::mustache::load("error.html");
            crow::json::wvalue context;
            context["error"] = "Please verify your email first!";
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

        db << R"(
            SELECT 
                posts._id AS post_id, 
                posts.user_id AS author_id, 
                user.name AS author_name, 
                posts.content,
                posts.post_pic_url, 
                posts.timestamp, 
                posts.time_edited,
                COUNT(post_likes.user_id) AS like_count
            FROM posts
            LEFT JOIN post_likes ON posts._id = post_likes.post_id
            LEFT JOIN user ON posts.user_id = user._id
            GROUP BY posts._id
            ORDER BY posts.timestamp DESC 
            LIMIT 10 OFFSET ?;
        )" << page_num * 10 >>
            [&](const uint32_t post_id, uint32_t user_id,
                    const std::string &author_name, const std::string &content,
                    const std::string &post_pic_url, const std::string &timestamp,
                    const std::string &time_edited, const uint32_t like_count) {
                crow::json::wvalue post;
                post["post_id"] = post_id;
                post["user_id"] = user_id;
                if (user_id == user.id) {
                    post["editable"] = post_id;
                }
                post["author_name"] = author_name;
                post["content"] = content;
                if (!post_pic_url.empty()) {
                    post["post_pic_url"] = post_pic_url;
                }
                post["timestamp"] = timestamp;
                if (!time_edited.empty()) {
                    post["edited"] = time_edited;
                }
                post["like_count"] = like_count;
                posts.emplace_back(post);
            };

        json_data["posts"] = std::move(posts);
        return page.render(json_data);
    });

    CROW_ROUTE(app, "/submit_post").methods(crow::HTTPMethod::Post)([&db](const crow::request& request) {
        current_user user = is_authorized(request, db);

        if (!user.logged_in || !user.verified) {
            return crow::response(400, "Not authorized");
        }
        crow::multipart::message file_message(request);
        fs::path profile_picture_path;
        std::string content = file_message.get_part_by_name("content").body;
        std::string post_pic = file_message.get_part_by_name("post_pic").body;

        if (!post_pic.empty()) {
               const crow::multipart::part &part_value = file_message.get_part_by_name("post_pic");

            if (part_value.body.empty()) {
                return crow::response(400, "No file part found");
             }

            const crow::multipart::header &header = part_value.get_header_object("Content-Disposition");
            if (header.value.empty()) {
                   return crow::response(400, "UPLOAD FAILED");
            }

            std::string prof_name = header.params.at("filename");

            if (prof_name.empty()) {
                return crow::response(400, "Missing filename");
            }

            fs::path upload_dir = fs::path("user_data") / fs::path(std::to_string(user.id)) / fs::path("post_pic");
            std::error_code error;

            if (!fs::exists(upload_dir, error)) {
                 fs::create_directories(upload_dir, error);
            }

            if (error) {
                return crow::response(500, "Error creating user directory");
            }

            bool starts_img = false;

            if (post_pic.starts_with(image.jpg)) {
                starts_img = true;
                if (!prof_name.ends_with(".jpg"))
                  prof_name += ".jpg";
            } else if (post_pic.starts_with(image.png)) {
                starts_img = true;
                if (!prof_name.ends_with(".png"))
                prof_name += ".png";
            } else if (post_pic.starts_with(image.gif)) {
                starts_img = true;
                if (!prof_name.ends_with(".gif"))
                prof_name += ".gif";
            } else if (post_pic.starts_with(image.bmp)) {
                starts_img = true;
                  if (!prof_name.ends_with(".bmp"))
                prof_name += ".bmp";
            }
                
             if (starts_img) {
                std::ofstream out_file(upload_dir / prof_name, std::ofstream::out | std::ios::binary);
                out_file.write(post_pic.data(), post_pic.length());
                profile_picture_path = upload_dir / prof_name;
            } else {
                return crow::response(400, "UPLOAD FAILED: Not an image file");
            }
        }

        if (content.empty() && post_pic.empty()) {
            return crow::response(400, "Post cannot be empty");
        }

        if (!post_pic.empty()) {
            db << "INSERT INTO posts (user_id, content, post_pic_url) VALUES (?, ?, ?);" 
               << user.id << content << profile_picture_path.string();
        } else {
            db << "INSERT INTO posts (user_id, content) VALUES (?, ?);" 
               << user.id << content;
        }

        uint32_t post_id = 0;
        db << "SELECT last_insert_rowid();" 
           >> [&post_id](const uint32_t id) {
            post_id = id;
        };
 
        crow::json::wvalue response;
        response["post_id"] = post_id;
        response["user_id"] = user.id;
        if(!post_pic.empty()) {
            response["post_pic_url"] = profile_picture_path.string();
        }
        response["user_name"] = user.name;

        return crow::response{response};
    });

    CROW_ROUTE(app, "/edit_post/<int>").methods(crow::HTTPMethod::Post)([&db](const crow::request& request, const int32_t post_id) {
        current_user user = is_authorized(request, db);

        if (!user.logged_in || !user.verified) {
            return crow::response(400, "ERROR: Not authorized");
        }

        // Check if the post exists and if the user is the owner
        bool is_owner = false;
        db << "SELECT COUNT(*) FROM posts WHERE _id = ? AND user_id = ?;"
           << post_id << user.id
           >> [&is_owner](const int32_t count) {
                is_owner = (count > 0);
            };

        if (!is_owner) {
            return crow::response(403, "ERROR: You are not allowed to edit this post");
        }

        crow::multipart::message file_message(request);
        fs::path profile_picture_path;
        std::string content = file_message.get_part_by_name("content").body;
        std::string post_pic = file_message.get_part_by_name("post_pic").body;
        std::string remove_pic = file_message.get_part_by_name("remove_picture").body;

        if (!post_pic.empty()) {
            const crow::multipart::part &part_value = file_message.get_part_by_name("post_pic");
            std::error_code error; 

            std::string old_post_pic;
            db << "SELECT post_pic_url FROM posts WHERE _id = ? AND user_id = ?;"
               << post_id << user.id
               >> old_post_pic;

            if (!old_post_pic.empty() && !post_pic.empty()) {
                fs::remove(fs::path(old_post_pic), error);
            }

            if (error) {
                return crow::response(500, "Error removing old post pic");
            }

            if (part_value.body.empty()) {
                return crow::response(400, "No file part found");
             }

            const crow::multipart::header &header = part_value.get_header_object("Content-Disposition");
            if (header.value.empty()) {
                   return crow::response(400, "UPLOAD FAILED");
            }

            std::string prof_name = "post_" + std::to_string(post_id) + header.params.at("filename");

            if (prof_name.empty()) {
                return crow::response(400, "Missing filename");
            }

            fs::path upload_dir = fs::path("user_data") / fs::path(std::to_string(user.id)) / fs::path("post_pic");
            

            if (!fs::exists(upload_dir, error)) {
                 fs::create_directories(upload_dir, error);
            }

            if (error) {
                return crow::response(500, "Error creating user directory");
            }

            bool starts_img = false;

            if (post_pic.starts_with(image.jpg)) {
                starts_img = true;
                if (!prof_name.ends_with(".jpg"))
                  prof_name += ".jpg";
            } else if (post_pic.starts_with(image.png)) {
                starts_img = true;
                if (!prof_name.ends_with(".png"))
                prof_name += ".png";
            } else if (post_pic.starts_with(image.gif)) {
                starts_img = true;
                if (!prof_name.ends_with(".gif"))
                prof_name += ".gif";
            } else if (post_pic.starts_with(image.bmp)) {
                starts_img = true;
                  if (!prof_name.ends_with(".bmp"))
                prof_name += ".bmp";
            }
                
             if (starts_img) {
                std::ofstream out_file(upload_dir / prof_name, std::ofstream::out | std::ios::binary);
                out_file.write(post_pic.data(), post_pic.length());
                profile_picture_path = upload_dir / prof_name;
            } else {
                return crow::response(400, "UPLOAD FAILED: Not an image file");
            }
        }

        if (content.empty() && post_pic.empty()) {
            return crow::response(400, "Post cannot be empty");
        }

        if (!post_pic.empty()) {
            db << "UPDATE posts SET content = ?, post_pic_url = ?, time_edited = datetime('now', 'localtime') WHERE (user_id = ? AND _id = ?);" 
               << content << profile_picture_path.string() << user.id << post_id;
        } else if (!remove_pic.empty()) {
            std::error_code error; 

            std::string old_post_pic;
            db << "SELECT post_pic_url FROM posts WHERE _id = ? AND user_id = ?;"
               << post_id << user.id
               >> old_post_pic;

            fs::remove(fs::path(old_post_pic), error);

            if (error) {
                return crow::response(500, "Error removing old post pic");
            }

            db << "UPDATE posts SET content = ?, post_pic_url = ?, time_edited = datetime('now', 'localtime') WHERE (user_id = ? AND _id = ?);"
               << content << nullptr << user.id << post_id;
        } else {
            db << "UPDATE posts SET content = ?, time_edited = datetime('now', 'localtime') WHERE (user_id = ? AND _id = ?);"
               << content << user.id << post_id;
        }

        return crow::response{200, "Post updated successfully!"};
    });

    CROW_ROUTE(app, "/edit_reply/<int>").methods(crow::HTTPMethod::Put)([&db](const crow::request& request, const uint32_t reply_id) {
        crow::json::rvalue body = crow::json::load(request.body);

        if (!body || !body.has("content")) {
            return crow::response(400, "ERROR: Invalid JSON");
        }
        
        std::string content = crow::json::load(request.body)["content"].s();
        current_user user = is_authorized(request, db);

        if (!user.logged_in || !user.verified) {
            return crow::response(400, "ERROR: Not authorized");
        }

        if (content.empty()) {
            return crow::response(400, "ERROR: Content cannot be empty");
        }

        bool is_owner = false;
        db << "SELECT COUNT(*) FROM replies WHERE _id = ? AND user_id = ?;"
           << reply_id << user.id
           >> [&is_owner](int32_t count) {
                is_owner = (count > 0);
        };

        if (!is_owner) {
            return crow::response(403, "ERROR: You are not allowed to edit this reply");
        }
        db << "UPDATE replies SET content = ?, time_edited = datetime('now', 'localtime') WHERE (user_id = ? AND _id = ?);" 
           << content << user.id << reply_id;

        return crow::response{200, "Reply updated successfully!"};
    });

    CROW_ROUTE(app, "/delete_post/<int>").methods(crow::HTTPMethod::Delete)([&db](const crow::request& request, int32_t post_id) {
        current_user user = is_authorized(request, db);

        if (!user.logged_in || !user.verified) {
            return crow::response(400, "Not authorized");
        }

        // Check if the post exists and if the user is the owner
        bool is_owner = false;
        db << "SELECT COUNT(*) FROM posts WHERE _id = ? AND user_id = ?;"
           << post_id << user.id
           >> [&is_owner](int32_t count) {
            is_owner = (count > 0);
        };

        if (!is_owner) {
            return crow::response(403, "You are not allowed to delete this post");
        }
        
        std::error_code error; 

        std::string old_post_pic;
        db << "SELECT post_pic_url FROM posts WHERE _id = ? AND user_id = ?;"
           << post_id << user.id
           >> old_post_pic;

        fs::remove(fs::path(old_post_pic), error);

        if (error) {
            return crow::response(500, "Error removing old post pic");
        }

        db << "DELETE FROM post_likes WHERE post_id = ?;"
           << post_id;

        db << "DELETE FROM posts WHERE _id = ? AND user_id = ?;"
           << post_id << user.id;

        db << "DELETE FROM replies WHERE post_id = ? AND user_id = ?;"
           << post_id << user.id;

        return crow::response(200, "Post and associated likes deleted successfully");
    });

    CROW_ROUTE(app, "/delete_reply/<int>").methods(crow::HTTPMethod::Delete)([&db](const crow::request& request, const uint32_t reply_id) {
        current_user user = is_authorized(request, db);

        if (!user.logged_in || !user.verified) {
            return crow::response(400, "ERROR: Not authorized");
        }

        bool is_owner = false;
        db << "SELECT COUNT(*) FROM replies WHERE _id = ? AND user_id = ?;"
           << reply_id << user.id
           >> [&is_owner](int32_t count) {
                is_owner = (count > 0);
            };

        if (!is_owner) {
            return crow::response(403, "ERROR: You are not allowed to delete this reply");
        }

        db << "DELETE FROM replies WHERE _id = ? AND user_id = ?;"
           << reply_id << user.id;

        db << "DELETE FROM reply_likes WHERE reply_id = ? AND user_id = ?;"
           << reply_id << user.id;

        return crow::response(200, "Reply and it's associated likes deleted successfully!");
    });

    CROW_ROUTE(app, "/reply/<int>").methods(crow::HTTPMethod::Post)([&db](const crow::request& request, int32_t post_id) {
        crow::json::rvalue body = crow::json::load(request.body);
        std::string content = body["content"].s();
        int32_t parent_reply_id = body.has("parent_reply_id") ? body["parent_reply_id"].i() : 0;

        current_user user = is_authorized(request, db);

        if (!user.logged_in || !user.verified) {
            return crow::response(400, "Not authorized");
        }

        if (content.empty()) {
            return crow::response(400, "Reply content cannot be empty");
        }

        std::string author;
        if (parent_reply_id == 0) {
            try {
                db << "SELECT user.name FROM posts JOIN user ON posts.user_id = user._id WHERE posts._id = ?;"
                << post_id
                >> author;
            }  catch (...) { 
                return crow::response{"No such a post!"};
            } 
        } else {
            try {
                db << "SELECT user.name FROM replies JOIN user ON replies.user_id = user._id WHERE replies._id = ?;"
                << parent_reply_id
                >> author;
            }  catch (...) { 
                return crow::response{"No such a post!"};
            } 
        }

        db << "INSERT INTO replies (post_id, user_id, content, parent_reply_id, author, timestamp) VALUES (?, ?, ?, ?, ?, datetime('now', 'localtime'));"
           << post_id << user.id << content << parent_reply_id << author;

        uint32_t reply_id = 0;
        db << "SELECT last_insert_rowid();" 
           >> reply_id;

        crow::json::wvalue response;
        response["reply_id"] = reply_id;
        response["user_id"] = user.id;
        response["user_name"] = user.name;
        response["author"] = author;
        response["content"] = content;
        response["parent_reply_id"] = parent_reply_id;

        return crow::response{response};
    });

    CROW_ROUTE(app, "/load_replies/<int>").methods(crow::HTTPMethod::Get)([&db](const crow::request& req, const int32_t post_id) {
        std::string offset_check = req.url_params.get("offset") ? req.url_params.get("offset") : "";
        std::string parent_reply_id_check = req.url_params.get("parent_reply_id") ? req.url_params.get("parent_reply_id") : "";

        std::expected<uint32_t, std::string> offset_exp = u32_validator(offset_check);
        uint32_t offset = 0;
        std::expected<uint32_t, std::string> parent_reply_id_exp = u32_validator(parent_reply_id_check);
        uint32_t parent_reply_id = 0;

        if (offset_exp && parent_reply_id_exp) {
            offset = offset_exp.value();
            parent_reply_id = parent_reply_id_exp.value();
        } else {
            crow::mustache::template_t page = crow::mustache::load("error.html");
            crow::json::wvalue context;
            return page.render(context);  
        }

        crow::json::wvalue response;
        std::vector<crow::json::wvalue> replies_list;
        current_user user = is_authorized(req, db);
        db << R"(
            SELECT replies._id, 
                replies.user_id, 
                replies.content, 
                replies.timestamp, 
                replies.time_edited,
                replies.author, 
                user.name, 
                replies.post_id, 
                COUNT(reply_likes.reply_id) AS total_reply_likes
            FROM replies
            JOIN user ON replies.user_id = user._id 
            LEFT JOIN reply_likes ON replies._id = reply_likes.reply_id
            WHERE replies.post_id = ? AND (replies.parent_reply_id = ? OR ? = 0)
            GROUP BY replies._id, replies.user_id, replies.content, replies.timestamp, replies.author, user.name, replies.post_id
            ORDER BY replies.timestamp DESC 
            LIMIT 5 OFFSET ?;
            )"
            << post_id << parent_reply_id << parent_reply_id << offset
            >> [&](const uint32_t reply_id, const uint32_t user_id, const std::string& content, const std::string& timestamp, const std::string& time_edited, const std::string& user_name, const std::string& author, const uint32_t post_id, const uint32_t reply_like_count) {
                crow::json::wvalue reply;
                reply["reply_id"] = reply_id;
                reply["post_id"] = post_id;
                reply["user_id"] = user_id;
                if (user_id == user.id) {
                    reply["editable"] = "editable";
                }
                reply["content"] = content;
                reply["timestamp"] = timestamp;
                if (!time_edited.empty()) {
                    reply["edited"] = time_edited;
                }
                reply["author"] = author;
                reply["user_name"] = user_name;
                reply["reply_like_count"] = reply_like_count;
                replies_list.emplace_back(reply);
        };

        crow::mustache::template_t page = crow::mustache::load("replies.html");
        response["replies"] = std::move(replies_list);
        return page.render(response);
    });

    //id/filename/
    CROW_ROUTE(app, "/user_data/<int>/prof_pic/<string>") ([](crow::response& res, const uint32_t& user_id, const std::string& path) {
        fs::path filepath = fs::path("user_data") / fs::path(std::to_string(user_id)) / fs::path("prof_pic") / fs::path(escape_string_decode(path));
        std::ifstream file(filepath, std::ifstream::in | std::ios::binary);

        if (file) {
          std::ostringstream buffer;
          buffer << file.rdbuf();

          if (buffer.str().starts_with(image.jpg) ||
              buffer.str().starts_with(image.png) ||
              buffer.str().starts_with(image.gif) ||
              buffer.str().starts_with(image.bmp)) {
                res.set_static_file_info(filepath.string());
                res.end();
          } else {
            res.code = 400;
            res.write("Invalid image format");
            res.end();
          }
        } else {
          res.code = 400;
          res.write("File doesn't exist");
          res.end();
        }

    });

    CROW_ROUTE(app, "/user_data/<int>/post_pic/<string>") ([](crow::response& res, const uint32_t& user_id, const std::string& path) {
        fs::path filepath = fs::path("user_data") / fs::path(std::to_string(user_id)) / fs::path("post_pic") / fs::path(escape_string_decode(path));
        std::ifstream file(filepath, std::ifstream::in | std::ios::binary);

        if (file) {
          std::ostringstream buffer;
          buffer << file.rdbuf();

          if (buffer.str().starts_with(image.jpg) ||
              buffer.str().starts_with(image.png) ||
              buffer.str().starts_with(image.gif) ||
              buffer.str().starts_with(image.bmp)) {
                res.set_static_file_info(filepath.string());
                res.end();
          } else {
            res.code = 400;
            res.write("Invalid image format");
            res.end();
          }
        } else {
          res.code = 400;
          res.write("File doesn't exist");
          res.end();
        }

    });

    CROW_ROUTE(app, "/register").methods(crow::HTTPMethod::Post)([&db](const crow::request& request) {
        crow::multipart::message file_message(request);
        std::string name = file_message.get_part_by_name("name").body;
        std::string email = file_message.get_part_by_name("email").body;
        std::string password = file_message.get_part_by_name("password").body;

        if (name.empty()) {
            return crow::response(400, "Name is EMPTY");
        }

        if (email.empty()) {
            return crow::response(400, "Email is EMPTY");
        } else {
          std::string db_email;

          db << "SELECT email FROM user WHERE email = ? AND verified = true;"
             << email >>
              [&db_email](const std::string _email) { db_email = _email; };

          if (!db_email.empty()) {
            return crow::response(400, "Email already exists");
          }
        }

        if (password.empty()) {
            return crow::response(400, "Password is EMPTY");
        } else {
          if (is_pass_ok(password) && (password.length() >= 8 && password.length() <= 25)) {
                std::string key = session_token(20);
                std::string sanitized_password;
                std::ranges::for_each(password, [&sanitized_password](const char ch){ ((ch >= 0 && ch <= 31) || ch == 127)? "" :  sanitized_password += ch ; });
                password = encrypt(sanitized_password, key);
          } else {
                return crow::response(400, "ERROR: Password needs to be at least 8 chars with a symbol, number, capital letter");
          }
        }

        std::string verify_key = session_token(20);
        send_verification_email(email, verify_key);
        std::string token = session_token(20);

        db << "INSERT OR REPLACE INTO to_verify (email, key) VALUES (?, ?);"
           << email << verify_key;

        int32_t user_id = -1;
 
        try {
            db << "INSERT INTO user (name, email, password) VALUES (?, ?, ?);"
               << name << email << password;

            user_id = db.last_insert_rowid();
        } catch (const sqlite::sqlite_exception& error) {
            db << "SELECT _id FROM user WHERE email = ?;"
               << email 
               >> [&user_id](const int32_t id) { user_id = id; };
        }

        db << "INSERT OR REPLACE INTO sessions (user_id, session_token) VALUES (?, ?);" 
           << user_id << token;

        fs::path profile_picture_path;
        fs::path upload_dir = fs::path("user_data") / fs::path(std::to_string(user_id)) / fs::path("prof_pic");   
        std::error_code sys_error;

        if (fs::exists(upload_dir, sys_error)) {
            fs::remove_all(upload_dir, sys_error); 
            db << "UPDATE user SET profile_picture = NULL WHERE _id = ?;"
               << user_id;
            if (sys_error) {
                return crow::response(500, "Error removing user directory");
            }
        } 

        fs::create_directories(upload_dir, sys_error);
        if (sys_error) {
          return crow::response(500, "Error creating user directory");
        }

        std::string prof_pic = file_message.get_part_by_name("file").body;

        if (!prof_pic.empty()) {
            const crow::multipart::part& part_value = file_message.get_part_by_name("file");
            if (part_value.body.empty()) {
                return crow::response(400, "No file part found");
            }
            
            const crow::multipart::header& header = part_value.get_header_object("Content-Disposition");
            if (header.value.empty()) {
                return crow::response(400, "UPLOAD FAILED");
            }

            std::string prof_name = header.params.at("filename");

            if (prof_name.empty()) {
                return crow::response(400, "Missing filename");
            }

            bool starts_img = false;

            if (prof_pic.starts_with(image.jpg)) {
                starts_img = true;
                if (!prof_name.ends_with(".jpg"))
                    prof_name += ".jpg";
            } else if (prof_pic.starts_with(image.png)) {
                    starts_img = true;
                if (!prof_name.ends_with(".png"))
                    prof_name += ".png";
            } else if (prof_pic.starts_with(image.gif)) {
                    starts_img = true;
                if (!prof_name.ends_with(".gif"))
                    prof_name += ".gif";
            } else if (prof_pic.starts_with(image.bmp)) {
                    starts_img = true;
                if (!prof_name.ends_with(".bmp"))
                    prof_name += ".bmp";
            }

            if (starts_img) {
                std::ofstream out_file(upload_dir / prof_name, std::ofstream::out | std::ios::binary);
                out_file.write(prof_pic.data(), prof_pic.length());
                profile_picture_path = upload_dir / prof_name;

                db << "UPDATE user SET profile_picture = ? WHERE _id = ?;"
                   << profile_picture_path.string() << user_id;
            } else {
                return crow::response(400, "UPLOAD FAILED: Not an image file");
            }
        }

        crow::response response(301, "/logged_in"); 
        response.add_header("Set-Cookie", "session_token=" + token + "; Path=/; HttpOnly");
        response.add_header("Set-Cookie", "user_id=" + std::to_string(user_id) + "; Path=/; HttpOnly");
        response.add_header("Location", "/");
        return response;
    });

    //email/key
    CROW_ROUTE(app, "/verify/<string>/<string>") ([&db](const crow::request& req, std::string email, std::string key) {
        current_user user = is_authorized(req, db);

        if (!user.logged_in) {
            crow::response response(400, "ERROR: Please login to verify."); 
            return response;
        }

        email = escape_string_decode(email);
        key = escape_string_decode(key);
        std::string db_email;
        std::string db_key;
        
        db << "SELECT * FROM to_verify;"
           >> [&](std::string curr_email, std::string curr_key){ db_email = curr_email, db_key = curr_key; };

        if (email == db_email && key == db_key) {
            db << "UPDATE user SET verified = TRUE WHERE email = ?;"
               << email;

            db << "DELETE FROM to_verify WHERE email = ?;"
               << email;
            crow::response response(301, "/"); 
            response.add_header("Location", "/");
            return response;
        } 

        crow::response response(404, "ERROR:INVALID"); 
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
            } else if (part_name == "password" && part_value.body.empty()) {
                return crow::response(400, "Password is EMPTY");
            } else if (part_name == "password") {
                std::string sanitized_password;
                std::ranges::for_each(part_value.body, [&sanitized_password](const char ch){ ((ch >= 0 && ch <= 31) || ch == 127)? "" :  sanitized_password += ch ; });
                password = sanitized_password;
            }
        }

        std::string token = session_token(20);
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
            >> [&users](const int32_t id, const std::string& name, const std::string& email, const std::string& password, const std::string& profile_picture) {
                crow::json::wvalue user;
                user["id"] = id;
                user["name"] = name; 
                user["email"] = email;
                user["password"] = decrypt(password);
                if (!profile_picture.empty()) {
                    user["profile_picture"] = profile_picture; 
                }
                users.emplace_back(user);
            };

            json_data["users"] = std::move(users);
            return page.render(json_data);
        }

        crow::mustache::template_t page = crow::mustache::load("error.html");
        crow::json::wvalue context;
        return page.render(context);  
    }); 
 
    CROW_ROUTE(app, "/prof/<int>") ([&db](const int32_t user_id) {
        crow::mustache::template_t page = crow::mustache::load("user_profile.html");
        crow::json::wvalue json_data;
        std::vector<crow::json::wvalue> users;

        db << "SELECT _id, name, profile_picture FROM user WHERE _id = ?;" 
            << user_id
            >> [&users](const int32_t& id, const std::string& name, const std::string& profile_picture) {
                crow::json::wvalue user;
                user["id"] = id;
                user["name"] = name; 
                if (!profile_picture.empty()) {
                    user["profile_picture"] = profile_picture; 
                }
                users.emplace_back(user);
            };

            json_data["users"] = std::move(users);
        return page.render(json_data);
    });

    CROW_ROUTE(app, "/edit_user").methods(crow::HTTPMethod::Post)([&db](const crow::request& request) {
        crow::multipart::message file_message(request);
        current_user user = is_authorized(request, db);

        if (user.logged_in) {
            std::string name = file_message.get_part_by_name("name").body;
            std::string new_email = file_message.get_part_by_name("email").body;
            std::string password = file_message.get_part_by_name("password").body;
            std::string remove_profile_pic = file_message.get_part_by_name("remove_picture").body;

            bool found = false;
            std::string old_email;

            db << "SELECT email FROM user WHERE _id = ?;" 
               << user.id 
               >> old_email;

            db << "SELECT COUNT(*) FROM user WHERE email = ?;" 
               << new_email 
               >> found;

            if (found && old_email != new_email) {
                return crow::response(400, "ERROR: Email already exists!");
            } else if (old_email != new_email) {
                std::string key = session_token(20);
                send_verification_email(new_email, key);

                db << "INSERT INTO to_verify (email, key) VALUES(?, ?);"
                    << new_email << key;

                db << "UPDATE user SET verified = FALSE WHERE email = ?;"
                    << old_email;

                db << "DELETE FROM sessions WHERE (user_id = ? AND session_token "
                        "!= ?);"
                 << user.id << user.session_token;
            }

            if (is_pass_ok(password) && password.length() >= 8) {
                std::string key = session_token(20);
                std::string sanitized_password;
                std::ranges::for_each(password, [&sanitized_password](const char ch) { 
                    ((ch >= 0 && ch <= 31) || ch == 127) ? "" : sanitized_password += ch;
                });
                password = encrypt(sanitized_password, key);
            } else {
              return crow::response(400, "ERROR: Password needs to be at least 8 chars with a symbol, number, capital letter");
            }

            if (name.empty() || new_email.empty() || password.empty()) {
                return crow::response(400, "ERROR: Empty field");
            }

            fs::path profile_picture_path;
            std::string prof_pic = file_message.get_part_by_name("file").body;

            if (!prof_pic.empty()) {
                const crow::multipart::part &part_value = file_message.get_part_by_name("file");

                if (part_value.body.empty()) {
                    return crow::response(400, "No file part found");
                }

                const crow::multipart::header &header = part_value.get_header_object("Content-Disposition");
                if (header.value.empty()) {
                    return crow::response(400, "UPLOAD FAILED");
                }

                std::string prof_name = header.params.at("filename");

                if (prof_name.empty()) {
                    return crow::response(400, "Missing filename");
                }

                fs::path upload_dir = fs::path("user_data") / fs::path(std::to_string(user.id)) / fs::path("prof_pic");
                std::error_code error;

                if (!fs::exists(upload_dir, error)) {
                    fs::create_directories(upload_dir, error);
                }

                if (error) {
                    return crow::response(500, "Error creating user directory");
                }

                bool starts_img = false;

                if (prof_pic.starts_with(image.jpg)) {
                    starts_img = true;
                    if (!prof_name.ends_with(".jpg"))
                    prof_name += ".jpg";
                } else if (prof_pic.starts_with(image.png)) {
                    starts_img = true;
                    if (!prof_name.ends_with(".png"))
                    prof_name += ".png";
                } else if (prof_pic.starts_with(image.gif)) {
                    starts_img = true;
                    if (!prof_name.ends_with(".gif"))
                    prof_name += ".gif";
                } else if (prof_pic.starts_with(image.bmp)) {
                    starts_img = true;
                    if (!prof_name.ends_with(".bmp"))
                    prof_name += ".bmp";
                }
                
                if (starts_img) {
                    std::ofstream out_file(upload_dir / prof_name, std::ofstream::out | std::ios::binary);
                    out_file.write(prof_pic.data(), prof_pic.length());
                    profile_picture_path = upload_dir / prof_name;
                } else {
                    return crow::response(400, "UPLOAD FAILED: Not an image file");
                }
            }

            if (!profile_picture_path.empty() || !remove_profile_pic.empty()) {
                fs::path profile_picture_delete;

                db << "SELECT profile_picture FROM user WHERE _id = ?;"
                    << user.id 
                    >> [&profile_picture_delete](const std::string &profile_picture) { profile_picture_delete = fs::path(profile_picture); };

                std::error_code error;
                if ((!profile_picture_path.empty() || !remove_profile_pic.empty()) && fs::exists(profile_picture_delete) && profile_picture_delete != profile_picture_path) {
                    fs::remove(profile_picture_delete, error);
                }

                if (error) {
                    return crow::response(500, "Error deleting profile picture");
                }

                db << "UPDATE user SET name = ?, email = ?, password = ?, profile_picture = ? WHERE _id = ?;"
                    << name << new_email << password << profile_picture_path.string() << user.id;
                } else {
                db << "UPDATE user SET name = ?, email = ?, password = ? WHERE _id = ?;"
                    << name << new_email << password << user.id;
                }
                return crow::response(200, "USER UPDATED SUCCESSFULLY");
        }

        return crow::response(400, user.error_message);
    });

    CROW_ROUTE(app, "/delete_user/<string>")([&db](const crow::request& request, std::string password) {
        current_user user = is_authorized(request, db);

        if (user.logged_in) {
            std::string db_password;
            password = escape_string_decode(password);

            db << "SELECT password FROM user WHERE _id = ?;"
            << user.id
            >> [&](const std::string& pass) {
                db_password = decrypt(pass);
            };

            if (db_password != password) {
                return crow::response(400, "Wrong password");
            }

            db << "DELETE FROM posts WHERE user_id = ?;" << user.id;

            db << R"(
                DELETE FROM replies 
                WHERE user_id = ? OR parent_reply_id IN (
                    SELECT _id FROM replies WHERE user_id = ?
                );
            )" << user.id << user.id;

            db << "DELETE FROM post_likes WHERE user_id = ?;" << user.id;

            db << "DELETE FROM reply_likes WHERE user_id = ?;" << user.id;

            db << "DELETE FROM sessions WHERE user_id = ?;" << user.id;

            db << "DELETE FROM user WHERE _id = ?;" << user.id;

            std::error_code error;
            if (fs::exists(fs::path("user_data") / fs::path(std::to_string(user.id)), error)) {
                fs::remove_all(fs::path("user_data") / fs::path(std::to_string(user.id)), error);
            }

            if (error) {
                return crow::response(500, "Error deleting user directory");
            }

            return crow::response(200, "User and all related data deleted successfully");
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

    std::future<void> _a = app.bindaddr(IP).port(std::stoi(PORT)).multithreaded().run_async();
}
