#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <sys/time.h>
#include <cstring>
#include <iomanip>

#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

using std::cerr;
using std::endl;
using std::string;
using std::stringstream;

string get_current_time()
{
    char buffer[30];
    struct timeval tv;
    time_t curtime;

    gettimeofday(&tv, NULL); 

    curtime = tv.tv_sec;

    strftime(buffer,30,"[%d/%m/%Y] [%T:",gmtime(&curtime));
    stringstream ss;

    ss << buffer << std::setfill('0') << std::setw(6) << tv.tv_usec << "]";

    return ss.str();
}

int main(int argc , char* argv[])
{
    string serverIp;
    string serverPort;
    string clientId;
    int serverPortInt;

    if (argc != 4)
    {
        std::cout << "Usage: client server_ip server_port client_id" << endl;
        return -1;
    }
    else
    {
        serverIp = argv[1];
        serverPort = argv[2];
        clientId = argv[3];
    
        stringstream ss(serverPort);
        ss >> serverPortInt;
    }

    struct addrinfo hints;
    struct addrinfo* res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(NULL, serverPort.c_str(), &hints, &res) != 0)
    {
        cerr << "Error in getaddrinfo." << endl;
        return -1;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1)
    {
        cerr << "Could not create socket." << endl;
        return -1;
    }

    struct sockaddr_in server;

    server.sin_addr.s_addr = inet_addr(serverIp.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(serverPortInt);
 
    int ret = connect(sock, (struct sockaddr *)&server, sizeof(server));

    if (ret < 0)
    {
        cerr << "Error in connecting." << ret << endl;
        return 1;
    }

    while (true)
    {
        string message = get_current_time() + " [client_" + clientId + "] "
                           + "Some log message.\n";

        if (send(sock, message.c_str(), message.size(), 0) < 0)
        {
            cerr << "Error in sending the message." << endl;
            return 1;
        }

        // Represents some work done between two log statements.
         usleep(5000);
    }

    return 0;
}
