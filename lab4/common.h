#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>

inline std::ostream &operator<<(std::ostream &s, const sockaddr_in &addr)
{
    union
    {
        in_addr_t x;
        char c[sizeof(in_addr)];
    } t{};
    t.x = addr.sin_addr.s_addr;
    return s << std::to_string(int(t.c[0]))
             << "." << std::to_string(int(t.c[1]))
             << "." << std::to_string(int(t.c[2]))
             << "." << std::to_string(int(t.c[3]))
             << ":" << std::to_string(ntohs(addr.sin_port));
}

inline int make_socket(int type)
{
    switch (type)
    {
    case SOCK_STREAM:
        return socket(AF_INET, SOCK_STREAM, 0);
    default:
        errno = EINVAL;
        return -1;
    }
}

inline sockaddr_in local_addr(unsigned short port)
{
    sockaddr_in addr{};
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    return addr;
}

enum GuessResult
{
    CORRECT = 1,
    TOO_LOW = -1,
    TOO_HIGH = 0
};