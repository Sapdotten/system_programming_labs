#include "common.h"
#include "string.h"
#define RESET "\033[0m"
#define PURPLE "\033[35m"
#define GREEN "\033[32m"
constexpr int GUESS_RANGE_SIZE = 2;

int32_t server_port;

void play_game(int sock_fd, bool interactive)
{
    int32_t range_data[GUESS_RANGE_SIZE];
    int32_t guess, result;

    if (recv(sock_fd, range_data, sizeof(range_data), MSG_WAITALL) != sizeof(range_data))
        return;

    int32_t min = ntohl(range_data[0]);
    int32_t max = ntohl(range_data[1]);

    std::cout << "Guess the number between " << PURPLE << min << RESET << " and " << PURPLE << max << RESET << std::endl;

    while (true)
    {
        if (interactive)
        {
            std::cout << "Enter your guess: ";
            std::cin >> guess;
        }
        else
        {
            guess = (max + min) / 2;
            std::cout << "Robot guessed: " << guess << std::endl;
        }

        int32_t net_guess = htonl(guess);
        send(sock_fd, &net_guess, sizeof(net_guess), MSG_WAITALL);

        if (recv(sock_fd, &result, sizeof(result), MSG_WAITALL) != sizeof(result))
            break;
        result = ntohl(result);

        if (result == CORRECT)
        {
            std::cout << GREEN << "Correct guess!" << RESET << std::endl;
            break;
        }
        else if (result == TOO_HIGH)
        {
            std::cout << "The number is " << PURPLE << "higher." << RESET << std::endl;
            min = guess + 1;
        }
        else if (result == TOO_LOW)
        {
            std::cout << "The number is " << PURPLE << "lower." << RESET << std::endl;
            max = guess - 1;
        }
    }
}

void run_client(bool interactive, int iterations = 1)
{
    sockaddr_in server_addr = local_addr(server_port);
    int sock_fd = make_socket(SOCK_STREAM);
    int32_t continue_game = 1;

    if (connect(sock_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) != 0)
    {
        perror("Connect failed");
        exit(1);
    }

    while (continue_game and iterations)
    {
        play_game(sock_fd, interactive);
        if (interactive)
        {
            std::cout << "Play again? (1 = yes, 0 = no): ";
            std::cin >> continue_game;
        }
        else
            --iterations;

        int32_t continue_flag = htonl(continue_game);
        send(sock_fd, &continue_flag, sizeof(continue_flag), MSG_WAITALL);

        if (continue_flag == 0)
            break;
    }

    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);
}

int main(int argc, char *argv[])
{
    bool is_auto = false;
    int iters = 1;
    server_port = -1;
    if (argc < 3)
    {
        std::cerr << "Usage:\n ./client --port <port>\nor\n./client --port <port> --auto --iters <iters>\n";
        return 1;
    }
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "--port"))
            server_port = std::stoi(argv[++i]);
        else if (!strcmp(argv[i], "--auto"))
            is_auto = true;
        else if (!strcmp(argv[i], "--iters"))
            iters = std::stoi(argv[++i]);
    }

    if (server_port < 1024 || server_port > 65535)
    {
        std::cerr << "Invalid port number.\n";
        return 1;
    }
    if (is_auto)
        run_client(false, iters);
    else
        run_client(true);
}
