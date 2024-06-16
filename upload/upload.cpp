#include <crow_all.h>

int32_t main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        std::ifstream file("index.html");
        if (!file.is_open()) {
            return crow::response(404);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        return crow::response(buffer.str());
    });

    CROW_ROUTE(app, "/uploadfile").methods(crow::HTTPMethod::Post)([](const crow::request& request) {
        crow::multipart::message file_message(request);

        for (const std::pair<const std::string, crow::multipart::part>& part : file_message.part_map) {
            const  std::string& part_name = part.first;
            const  crow::multipart::part& part_value = part.second;
            
            if ("file" == part_name) { 
                auto headers_it = part_value.headers.find("Content-Disposition");
                if (headers_it == part_value.headers.end()) {
                    return crow::response(400, "UPLOAD FAILED");
                }
                
                auto params_it = headers_it->second.params.find("filename");
                if (params_it == headers_it->second.params.end()) {
                    return crow::response(400);
                }
                const std::string& filename = params_it->second;

                std::ofstream out_file(filename, std::ios::binary | std::ios::out);
                out_file << part_value.body;
                out_file.close();
            }
        }
        return crow::response(200, "UPLOAD SUCCESSFUL");
    });

    app.bindaddr("192.168.2.7").port(18080).run();
}
//clang++ -Wall -Wextra -fsanitize=address upload.cpp -o upload -std=c++23 -lwsock32 -lws2_32