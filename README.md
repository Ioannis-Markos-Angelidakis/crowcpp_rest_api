<img width="584" alt="200416-rest-api" src="https://github.com/Mark-AAAA/crowcpp_rest_api/assets/52075807/290c389a-1c52-4b43-bcf6-4219ca5a44f5">
<p>Experimenting with REST API using <a href="https://github.com/CrowCpp/Crow">CrowCpp</a> and <a href="https://github.com/SqliteModernCpp/sqlite_modern_cpp">SqliteModernCpp</a> with <a href="https://github.com/mustache/spec">Mustache</a> for SSR HTML.</p>
<h1>CrowCpp Web Project</h1>

<p>This project is a web application built using <a href="https://crowcpp.org/">CrowCpp</a> for handling HTTP requests and responses. The project uses a combination of CrowCpp for server-side routing, SQLite for database management, and Mustache for rendering HTML templates.</p>

<h2>Features</h2>
<ul>
  <li>User authentication and password management</li>
  <li>RESTful API design</li>
  <li>SQLite database integration</li>
  <li>Mustache templating for dynamic HTML rendering</li>
  <li>Email notifications using <code>curl</code> for password reset functionality</li>
</ul>

<h2>Routes</h2>
<ul>
  <li><code>/forgot_password/&lt;string&gt;</code> - Initiates a password reset process for the user with the provided email address.</li>
  <li><code>/reset_password</code> - Displays the password reset form, with token and email validation against the database.</li>
  <li><code>/</code> - Displays the home page of the application.</li>
  <li><code>/login</code> - Renders the login page and handles user authentication.</li>
  <li><code>/register</code> - Renders the registration page and handles user signup process.</li>
  <li><code>/forgot_password/&lt;string&gt;</code> - Initiates a password reset process for the user with the provided email address.</li>
  <li><code>/reset_password</code> - Displays the password reset form. Token and email are validated against the database.</li>
  <li><code>/post</code> - Allows authenticated users to create a new post.</li>
  <li><code>/post/&lt;id&gt;</code> - Displays a specific post based on its ID and allows users to view or reply.</li>
  <li><code>/reply</code> - Allows users to reply to posts or comments.</li>
  <li><code>/user/&lt;id&gt;</code> - Displays user profiles and their posts.</li>
</ul>

<h2>Technologies Used</h2>
<ul>
  <li><strong>C++</strong> - Core language used for developing the application</li>
  <li><strong>CrowCpp</strong> - Lightweight HTTP server framework</li>
  <li><strong>SQLite</strong> - Database for storing user data and tokens</li>
  <li><strong>Mustache</strong> - HTML templating engine for rendering views</li>
  <li><strong>curl</strong> - Command-line tool for sending emails for password recovery</li>
</ul>

<h2>Installation</h2>
<p>To build the project, make sure you have <code>CMake</code> installed. Follow these steps:</p>

<pre><code>git clone https://github.com/Ioannis-Markos-Angelidakis/crowcpp_rest_api.git
cd crowcpp_rest_api
mkdir build
cd build
cmake ..
make
./server
</code></pre>

<p>This project uses <code>FetchContent</code> to download dependencies (CrowCpp and SqliteModernCpp) as part of the CMake configuration. Below is the relevant CMake file:</p>

<pre><code>cmake_minimum_required(VERSION 3.20)

project(MyCrowProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FetchContent module to download dependencies
include(FetchContent)

# CrowCpp
FetchContent_Declare(
  CrowCpp
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG master 
)
FetchContent_MakeAvailable(CrowCpp)

# SqliteModernCpp
FetchContent_Declare(
  SqliteModernCpp
  GIT_REPOSITORY https://github.com/SqliteModernCpp/sqlite_modern_cpp.git
  GIT_TAG master  
)
FetchContent_MakeAvailable(SqliteModernCpp)

add_executable(${PROJECT_NAME} account.cpp)

# Link CrowCpp and SqliteModernCpp
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Crow::Crow
    sqlite_modern_cpp
)

# Include directories for SqliteModernCpp (if needed)
target_include_directories(${PROJECT_NAME} PRIVATE ${SqliteModernCpp_SOURCE_DIR})
</code></pre>

<p>After running CMake, the necessary dependencies will be automatically downloaded and linked to the project.</p>

<h2>Usage</h2>
<p>Once the server is running, you can access the web application through <code>localhost:8080</code> and interact with the various routes.</p>
