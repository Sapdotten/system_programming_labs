#include "common.h"
#include "signal.h"
#include "sys/wait.h"
#include <cstring>
#define RESET "\033[0m"
#define PURPLE "\033[35m"
#define GREEN "\033[32m"

int RANGE_START = 0;
int RANGE_END = 10;

int pipe_fd[2];

void handle_client(int client_fd, sockaddr_in client_addr)
{
    std::string log_entry;
    std::string client_ip = inet_ntoa(client_addr.sin_addr);

    log_entry = GREEN + client_ip + RESET + ": connected\n";
    write(pipe_fd[1], log_entry.c_str(), log_entry.size());

    srand(time(nullptr) + getpid());

    while (true)
    {
        log_entry = GREEN + client_ip + RESET + ": new session started\n";
        write(pipe_fd[1], log_entry.c_str(), log_entry.size());

        int secret = RANGE_START + rand() % (RANGE_END - RANGE_START + 1);
        int32_t range_data[2] = {htonl(RANGE_START), htonl(RANGE_END)};
        send(client_fd, range_data, sizeof(range_data), MSG_WAITALL);

        while (true)
        {
            int32_t received_guess = 0;
            int recv_status = recv(client_fd, &received_guess, sizeof(received_guess), MSG_WAITALL);
            received_guess = ntohl(received_guess);

            if (recv_status <= 0)
                break;

            log_entry = GREEN + client_ip + RESET + ": guess received (" + PURPLE + std::to_string(received_guess) + RESET + ")\n";
            write(pipe_fd[1], log_entry.c_str(), log_entry.size());

            int32_t result = 0;
            if (received_guess == secret)
                result = CORRECT;
            else if (received_guess < secret)
                result = TOO_HIGH;
            else
                result = TOO_LOW;

            int32_t response = htonl(result);
            send(client_fd, &response, sizeof(response), MSG_WAITALL);

            if (ntohl(response) == 1)
                break;
        }

        int32_t play_again_flag = 0;
        int status = recv(client_fd, &play_again_flag, sizeof(play_again_flag), MSG_WAITALL);
        play_again_flag = ntohl(play_again_flag);
        if (status <= 0 || play_again_flag == 0)
            break;
    }

    log_entry = client_ip + ": disconnected\n";
    write(pipe_fd[1], log_entry.c_str(), log_entry.size());
    close(client_fd);
}

void clean_zombie_processes(int)
{
    while (waitpid(-1, nullptr, WNOHANG) > 0)
        ;
}

int main(int argc, char *argv[])
{
    if (argc < 7)
    {
        std::cerr << "Usage:\n ./client --port <port> --start <start_num> --end <end_num>\n";
        return 1;
    }
    int port = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "--port"))
            port = std::stoi(argv[++i]);
        else if (!strcmp(argv[i], "--start"))
            RANGE_START = std::stoi(argv[++i]);
        else if (!strcmp(argv[i], "--end"))
            RANGE_END = std::stoi(argv[++i]);
    }
    if (port < 1024 || port > 65535)
        return 1;

    signal(SIGCHLD, clean_zombie_processes);

    pipe(pipe_fd);

    pid_t logger = fork();
    if (logger == 0)
    {
        // Процесс логирования
        close(pipe_fd[1]);
        char buffer[128];
        while (true)
        {
            ssize_t count = read(pipe_fd[0], buffer, sizeof(buffer) - 1);
            if (count <= 0)
                break;
            buffer[count] = '\0';
            std::cout << buffer;
        }
        exit(0);
    }

    close(pipe_fd[0]);

    int server_socket = make_socket(SOCK_STREAM);
    sockaddr_in server_addr = local_addr(port);
    bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 16);

    while (true)
    {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr *)&client_addr, &addr_len);
        pid_t pid = fork();
        if (pid == 0)
        {
            close(server_socket);
            handle_client(client_socket, client_addr);
            exit(0);
        }
        else
        {
            close(client_socket);
        }
    }
}
