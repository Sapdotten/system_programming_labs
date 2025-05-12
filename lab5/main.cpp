#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <unistd.h>
#include <sys/types.h>

// Цвета для вывода
#define RESET   "\033[0m"
#define VIOLET  "\033[35m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define RED     "\033[31m"
#define BOLD    "\033[1m"

struct Account {
    uid_t uid;
    std::string name;
    std::string home;
    std::string hash;
    std::unordered_map<std::string, bool> memberships;  // true — админ
};

std::string extract_hash(const std::string& login, std::ifstream& shadow) {
    std::string entry;
    while (std::getline(shadow, entry)) {
        std::stringstream ss(entry);
        std::string uname, pass_hash;
        std::getline(ss, uname, ':');
        std::getline(ss, pass_hash, ':');
        if (uname == login)
            return pass_hash;
    }
    shadow.clear();
    shadow.seekg(0);
    return "Unavailable";
}

std::unordered_map<std::string, bool> extract_groups(const std::string& login, std::ifstream& gshadow) {
    std::unordered_map<std::string, bool> result;
    std::string entry;

    while (std::getline(gshadow, entry)) {
        std::stringstream ss(entry);
        std::string group, x, admins, members;
        std::getline(ss, group, ':');
        std::getline(ss, x, ':');
        std::getline(ss, admins, ':');
        std::getline(ss, members, ':');

        if (members.find(login) != std::string::npos)
            result[group] = false;
        if (admins.find(login) != std::string::npos)
            result[group] = true;
    }
    gshadow.clear();
    gshadow.seekg(0);
    return result;
}

std::vector<Account> collect_accounts() {
    std::ifstream passwd("/etc/passwd");
    std::ifstream shadow("/etc/shadow");
    std::ifstream gshadow("/etc/gshadow");
    setuid(getuid());  // чтобы не работать от root

    std::vector<Account> accounts;
    std::string record;

    while (std::getline(passwd, record)) {
        std::stringstream ss(record);
        std::string uname, x, uid_str, gid, misc, home, shell;
        std::getline(ss, uname, ':');
        std::getline(ss, x, ':');
        std::getline(ss, uid_str, ':');
        std::getline(ss, gid, ':');
        std::getline(ss, misc, ':');
        std::getline(ss, home, ':');
        std::getline(ss, shell, ':');

        Account acc;
        acc.name = uname;
        acc.uid = std::stoi(uid_str);
        acc.home = home;
        acc.hash = extract_hash(uname, shadow);
        acc.memberships = extract_groups(uname, gshadow);
        accounts.push_back(acc);
    }
    return accounts;
}

void print_accounts(const std::vector<Account>& users) {
    std::cout << BOLD
              << std::setw(6) << "UID"
              << std::setw(20) << "Username"
              << std::setw(25) << "Home Directory"
              << std::setw(20) << "Hash"
              << "Groups"
              << RESET << '\n';

    for (const auto& u : users) {
        std::cout << std::setw(6) << u.uid
                  << std::setw(20) << CYAN << u.name << RESET
                  << std::setw(25) << VIOLET << u.home << RESET
                  << std::setw(20) << u.hash << " ";

        for (const auto& [group, is_admin] : u.memberships) {
            std::cout << (is_admin ? RED : BLUE) << group << RESET << " ";
        }
        std::cout << '\n';
    }
}

int main() {
    auto users = collect_accounts();
    print_accounts(users);
    return 0;
}
