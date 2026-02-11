#include "client.h"
#include "http_request.h"
#include "server.h"
#include "util.h"
#include <thread>

void handle_connection(client_socket client);

/**
 * A basic HTTP web server that returns a requested file or returns an error if not found. Uses
 * the thread-per-connection model to handle requests and is thus synchronous.
 */
int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::println("Usage: web [port]");
        std::exit(1);
    }

    // Check if CLI argument is valid
    int port = 80;
    if (argc == 2) {
        try {
            port = std::stoi(argv[1]);
        } catch (std::invalid_argument& e) {
            std::println(stderr, "Usage: web [port]");
            std::exit(1);
        }
    }

    try {
        auto socket = server_socket::listen("localhost", port);
        std::println("Listening on port {}...\n", port);
        while (true) {
            std::thread(handle_connection, socket.accept()).detach();
        }
    } catch (std::exception& e) {
        std::println(stderr, "{}", e.what());
    }
}

void handle_connection(client_socket client) {
    try {
        // Get HTTP request from browser
        auto message = client.recv().first;

        // Parse request
        auto request = http_request(message);

        // Log request
        std::println("request length: {}", message.length());
        std::println("request line: {}, {}, {}", request.method, request.target, request.version);
        std::print("{}", message);

        // This server only supports the GET method
        if (request.method != "GET") {
            client.send(create_http_response("501 Not Implemented"));
            return;
        }

        // Get file name from request
        // We assume `/` fetches `index.html`
        auto file = request.target == "/" ? "/index.html" : request.target;

        // Try to read file, else return an error
        try {
            auto content = read_file(std::format("www{}", file));
            client.send(create_http_response("200 OK", content));
        } catch (std::invalid_argument& e) {
            // The file does not exist, so we return 404
            client.send(create_http_response("404 Not Found"));
        } catch (std::ios_base::failure& e) {
            // There was an error reading the file, so we return 500
            client.send(create_http_response("500 Internal Server Error"));
        }
    } catch (std::exception& e) {
        std::println(stderr, "{}", e.what());
    }
}
