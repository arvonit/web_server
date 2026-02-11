#include "client.h"
#include "util.h"
#include <netdb.h>
#include <unistd.h>

client_socket::client_socket(int fd) : fd(fd) {
}

client_socket::client_socket(client_socket&& rhs) : fd(rhs.fd) {
    // Invalidate previous socket's fd so that it does not close when destroyed
    rhs.fd = -1;
}

client_socket& client_socket::operator=(client_socket&& rhs) {
    // Check against self-assignment
    if (&rhs == this) {
        return *this;
    }

    // Set file descriptor from rvalue and invalidate old one
    fd = rhs.fd;
    rhs.fd = -1;

    return *this;
}

client_socket client_socket::connect(std::string address, int port) {
    int fd;
    struct addrinfo hints, *server_info, *p;

    // Create a socket
    memset(&hints, 0, sizeof(hints)); // Initialize the struct to all zeros
    hints.ai_family = AF_UNSPEC;      // We don't care if it's IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP socket

    // Call getaddrinfo to populate results into `server_info`
    if (int status =
            getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints, &server_info);
        status != 0) {
        throw std::runtime_error(std::format("gai error: {}", gai_strerror(status)));
    }

    // Search for correct server in linked list
    for (p = server_info; p != NULL; p = p->ai_next) {
        if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            std::println(stderr, "[warning] client socket error: {}", strerror(errno));
            continue;
        }

        if (::connect(fd, p->ai_addr, p->ai_addrlen) < 0) {
            close(fd);
            throw std::runtime_error(std::format("connect error: {}", strerror(errno)));
        }

        break;
    }

    // If we're at the end of the list, we failed to connect
    if (p == NULL) {
        throw std::runtime_error("Failed to connect");
    }

    freeaddrinfo(server_info); // We're done with this struct

    return client_socket(fd);
}

client_socket client_socket::from_fd(int fd) {
    if (!is_valid_fd(fd)) {
        throw std::runtime_error("File descriptor does not correspond to a valid socket");
    }
    return client_socket(fd);
}

// TODO: Finish implementing
// void send_all(std::string message) {
//     std::println("message length: {} bytes", message.length());

//     auto len = message.length();
//     int bytes_sent = 0;
//     int bytes_left = len;
//     auto message_c = message.c_str();

//     while (bytes_sent < len) {
//         int n = ::send(fd, message_c + bytes_sent, bytes_left, 0);
//         if (n < 0 && errno == EINTR) { // Continue if send() was interrupted
//             continue;
//         } else if (n < 0) {
//             throw std::runtime_error(std::format("send error: {}", strerror(errno)));
//         }
//         bytes_sent += n;
//         bytes_left -= n;
//     }

//     std::println("sent: {} bytes", bytes_sent);
// }

int client_socket::send(std::string message) {
    int bytes = ::send(fd, message.c_str(), message.length(), 0);
    if (bytes < 0) {
        throw std::runtime_error(std::format("send error: {}", strerror(errno)));
    }
    return bytes;
}

std::pair<std::string, int> client_socket::recv(int size) {
    std::string message(size, 0);

    int bytes = ::recv(fd, message.data(), size, 0);
    if (bytes < 0) {
        throw std::runtime_error(std::format("recv error: {}", strerror(errno)));
    }
    // std::println("message size: {}", size);

    // Remove trailing null terminators from string
    // This reduces the size of the string from 8192 bytes to its actual content
    message.erase(std::find(message.begin(), message.end(), '\0'), message.end());

    return std::make_pair(message, bytes);
}

// TODO: Finish implementing
// std::string recv_all(int size) {
//     std::string message(size, 0);
//     int bytes_left = size;

//     // Actually this won't work, size is way bigger than actually message lol
//     while (bytes_left > 0) {
//         int bytes = ::recv(fd, message.data(), bytes, 0);
//         if (bytes == 0) {
//             // TODO: Handle other end disconnecting
//             break;
//         } else if (bytes < 0 && errno == EINTR) { // System call interrupt
//             continue;
//         } else if (bytes < 0) {
//             throw std::runtime_error(std::format("recv error: {}", strerror(errno)));
//             break;
//         }
//         bytes_left -= bytes;
//     }

//     return message;
// }

client_socket::~client_socket() {
    if (is_valid_fd(fd)) {
        close(fd);
        // std::println("socket (fd: {}) destroyed", fd);
    }
}
