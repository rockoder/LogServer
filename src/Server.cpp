#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <thread>
#include <mutex>

#include "Queue.h"

using namespace std;

#define BACKLOG     10

Queue<string> msgQueue;

void writeToFile(string logfileName)
{
    ofstream logFile(logfileName);
    string msg;

    while (true)
    {
        msgQueue.waitAndPop(msg);

        logFile << msg;
    }
}

void handle(int sock)
{
    int len = 2048;

    int read_size;
    char client_message[len];
    string line;

    while ((read_size = recv(sock, client_message, len, 0)) > 0)
    {
        client_message[read_size] = '\0';

        line += client_message;

        size_t found = line.find_last_of("\n");

        if (found == string::npos)
        {
            continue;
        }
        else
        {
            msgQueue.push(line.substr(0, found + 1));

            line = line.substr(found + 1);
        }

        memset(client_message, 0, len);
    }

    if (read_size == 0)
    {
        cout << "Client got disconnected." << endl;
    }
    else if (read_size == -1)
    {
        cerr << "Error in receiving message." << endl;
    }
}

int main(int argc, char* argv[])
{
    string serverPort;
    string logfileName;

    if (argc != 3)
    {
        std::cout << "Usage: server server_port log_file_name" << endl;
        return -1;
    }
    else
    {
        serverPort = argv[1];
        logfileName = argv[2];
    }

    int sock;
    struct addrinfo hints;
    struct addrinfo* res;

    int reuseaddr = 1;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(NULL, serverPort.c_str(), &hints, &res) != 0)
    {
        cerr << "Error in getaddrinfo." << endl;
        return -1;
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (sock == -1)
    {
        cerr << "Could not create socket." << endl;
        return -1;
    }

    /* Enable the socket to reuse the address */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1)
    {
        cerr << "Error in setsockopt." << endl;
        return -1;
    }

    if (bind(sock, res->ai_addr, res->ai_addrlen) == -1)
    {
        cerr << "Error in bind." << endl;
        return -1;
    }

    freeaddrinfo(res);

    if (listen(sock, BACKLOG) == -1)
    {
        cerr << "Error in listen." << endl;
        return -1;
    }

    std::thread writerThread(writeToFile, logfileName);
    writerThread.detach();

    while (true)
    {
        unsigned int size = sizeof(struct sockaddr_in);
        struct sockaddr_in their_addr;
        int newsock = accept(sock, (struct sockaddr*)&their_addr, &size);

        if (newsock == -1)
        {
            cerr << "Error in accept." << endl;
            return -1;
        }
        else
        {
            cout << "Got a connection from " << inet_ntoa(their_addr.sin_addr) << " on port "
                 << htons(their_addr.sin_port) << endl;

            std::thread clientConnThread(handle, newsock);
            clientConnThread.detach();
        }
    }

    close(sock);

    return 0;
}
