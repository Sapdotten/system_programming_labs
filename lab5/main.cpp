#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <unistd.h>
#include <sys/types.h>

#define RESET     "\033[0m"
#define GREEN     "\033[32m"
#define RED       "\033[31m"
#define YELLOW    "\033[33m"
#define BLUE      "\033[34m"
#define PURPLE    "\033[35m"
#define CYAN      "\033[36m"
#define WHITE     "\033[37m"

struct User {
    uid_t uid;
    std::string username;
    std::string dir;
    std::string hash;
    std::unordered_map<std::string, bool> groups;
};

std::string get_password_hash(const std::string& username, std::ifstream& shadow_file) {
    if (!shadow_file.is_open()) return YELLOW "restricted" RESET;

    std::string line;
    while (std::getline(shadow_file, line)) {
        std::stringstream ss(line);
        std::string user, hash;
        std::getline(ss, user, ':');
        if (user == username) {
            std::getline(ss, hash, ':');
            return hash;
        }
    }
    shadow_file.clear();
    shadow_file.seekg(0);
    return YELLOW "no match" RESET;
}

std::unordered_map<std::string, bool> get_groups(const std::string& username, std::ifstream& gshadow_file) {
    std::unordered_map<std::string, bool> groups;
    if (!gshadow_file.is_open()) return groups;

    std::string line;
    while (std::getline(gshadow_file, line)) {
        std::stringstream ss(line);
        std::string group_name, x, admins, members;
        std::getline(ss, group_name, ':');
        std::getline(ss, x, ':');
        std::getline(ss, admins, ':');
        std::getline(ss, members, ':');

        if (admins.find(username) != std::string::npos) {
            groups[group_name] = true;
        } else if (members.find(username) != std::string::npos) {
            groups[group_name] = false;
        }
    }

    gshadow_file.clear();
    gshadow_file.seekg(0);
    return groups;
}

std::vector<User> get_users_info() {
    std::ifstream passwd_file("/etc/passwd");
    std::ifstream shadow_file("/etc/shadow");
    std::ifstream gshadow_file("/etc/gshadow");

    std::vector<User> users;

    if (!passwd_file.is_open()) {
        std::cerr << RED "Error: Unable to open /etc/passwd" RESET << std::endl;
        return users;
    }

    std::string line;
    while (std::getline(passwd_file, line)) {
        std::stringstream ss(line);
        std::string username, x, uid_str, gid_str, addons, dir, shell;

        std::getline(ss, username,  ':');
        std::getline(ss, x,          ':');
        std::getline(ss, uid_str,   ':');
        std::getline(ss, gid_str,   ':');
        std::getline(ss, addons,    ':');
        std::getline(ss, dir,       ':');
        std::getline(ss, shell,     ':');

        try {
            User user;
            user.username = username;
            user.uid      = std::stoi(uid_str);
            user.dir      = dir;
            user.hash     = get_password_hash(username, shadow_file);
            user.groups   = get_groups(username, gshadow_file);

            users.push_back(user);
        } catch (...) {
            std::cerr << YELLOW "Skipping invalid entry" RESET << std::endl;
        }
    }

    return users;
}

void print_users(const std::vector<User>& users) {
    std::cout << std::setw(6)  << std::left << BLUE "UID"      << RESET
              << std::setw(20) << std::left << BLUE "Username" << RESET
              << std::setw(25) << std::left << BLUE "Directory"<< RESET
              << std::setw(15) << std::left << BLUE "Hash"     << RESET
              << BLUE "Groups" RESET << std::endl;

    for (const auto& user : users) {
        std::cout << std::setw(6)  << std::left << CYAN << user.uid      << RESET
                  << std::setw(20) << std::left << GREEN << user.username << RESET
                  << std::setw(25) << std::left << PURPLE << user.dir    << RESET
                  << std::setw(15) << std::left << YELLOW << user.hash   << RESET;

        for (const auto& [group, is_admin] : user.groups) {
            if (is_admin)
                std::cout << RED "[" << WHITE << group << RED "]" RESET " ";
            else
                std::cout << WHITE << group << RESET " ";
        }

        std::cout << std::endl;
    }
}

int main() {
    std::cout << WHITE "User Information Tool v1.0" RESET << std::endl << std::endl;

    std::vector<User> users = get_users_info();
    print_users(users);

    std::cout << std::endl;
    return 0;
}