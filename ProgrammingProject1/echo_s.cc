#include "echo_s.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <regex>

bool VERBOSE;
using namespace std;

// **************************************************************************************
// * processConnection()
// * - Handles reading the line from the network and sending it back to the client.
// * - Returns true if the client sends "QUIT" command, false if the client sends "CLOSE".
// **************************************************************************************
bool processConnection(int sockFd) {
    bool quitProgram = false;
    bool keepGoing = true;

    while (keepGoing) {
        // Creating buffer array to store data from the client
        char buffer[1024];  
        // Setting elements in buffer to 0 to make sure it's cleared
        memset(buffer, 0, sizeof(buffer));

        // Reading data from the client into the buffer
        ssize_t bytesRead = read(sockFd, buffer, sizeof(buffer));

        // Handling situation if there was a read error
        if (bytesRead == -1) {
            cerr << "Error reading from socket" << endl;
            break;
        } 
        // If no bytes are read then the client has closed the connection
        else if (bytesRead == 0) {
            cout << "Client closed the connection" << endl;
            break;
        }  
        // Checking if the client sent CLOSE or QUIT
        else {
            string receivedData(buffer, bytesRead);

            // If client sent CLOSE writing that out to console
            if (receivedData.find("CLOSE") != string::npos) {
                cout << "Client sent CLOSE" << endl;
                break;
            } 
            // If client sent QUIT writing that out to console and raising quitProgram flag
            else if (receivedData.find("QUIT") != string::npos) {
                cout << "Client sent QUIT" << endl;
                quitProgram = true;
                break;
            } 
            // If the data recieved is neither then we echo back to the client
            else {
                ssize_t bytesWritten = write(sockFd, buffer, bytesRead);
                // Handling a writing error just in case
                if (bytesWritten == -1) {
                    cerr << "Error writing to socket" << endl;
                    break;
                }
            }
        }
    }

    // Close the connection and return the appropriate value
    close(sockFd);
    return quitProgram;
}

// **************************************************************************************
// * main()
// * - Sets up the sockets and accepts new connection until processConnection() returns 1
// **************************************************************************************
int main(int argc, char *argv[]) {
    // ********************************************************************
    // * Process the command line arguments
    // ********************************************************************
    int opt = 0;
    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
        case 'v':
            VERBOSE = true;
            break;
        case ':':
        case '?':
        default:
            cout << "usage: " << argv[0] << " -v" << endl;
            exit(-1);
        }
    }

    // *******************************************************************
    // * Creating the inital socket is the same as in a client.
    // ********************************************************************
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    // Error handling in case listening socket can't be created
    if (listenFd == -1) {
        perror("socket");
        exit(-1);
    }
    DEBUG << "Calling Socket() assigned file descriptor " << listenFd << ENDL;

    // ********************************************************************
    // * The bind() and calls take a structure that specifies the
    // * address to be used for the connection. On the cient it contains
    // * the address of the server to connect to. On the server it specifies
    // * which IP address and port to lisen for connections.
    // ********************************************************************
    struct sockaddr_in servaddr;
    srand(time(NULL));
    u_int16_t port = (rand() % 10000) + 1024;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // ********************************************************************
    // * Binding configures the socket with the parameters we have
    // * specified in the servaddr structure.  This step is implicit in
    // * the connect() call, but must be explicitly listed for servers.
    // ********************************************************************
    DEBUG << "Calling bind(" << listenFd << "," << &servaddr << "," << sizeof(servaddr) << ")" << ENDL;
    bool bindSuccessful = false;
    while (!bindSuccessful) {
      if (bind(listenFd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0) {
          bindSuccessful = true;
      } 
      // Handle bind failure, retry or choose a different port
      else {
          port = (rand() % 10000) + 1024;
          servaddr.sin_port = htons(port);
      }
    }
    cout << "Using port: " << port << endl;

    // ********************************************************************
    // * Setting the socket to the listening state is the second step
    // * needed to being accepting connections.  This creates a queue for
    // * connections and starts the kernel listening for connections.
    // ********************************************************************
    int listenQueueLength = 1;
    // Making sure that socket is set to listening state
    if (listen(listenFd, listenQueueLength) == -1) {
        perror("listen");
        exit(-1);
    }
    DEBUG << "Calling listen(" << listenFd << "," << listenQueueLength << ")" << ENDL;

    // ********************************************************************
    // * The accept call will sleep, waiting for a connection.  When 
    // * a connection request comes in the accept() call creates a NEW
    // * socket with a new fd that will be used for the communication.
    // ********************************************************************
    bool quitProgram = false;
    while (!quitProgram) {
      int connFd = accept(listenFd, NULL, NULL);
      if (connFd == -1) {
            perror("accept");
            continue;
      }

      DEBUG << "Calling accept(" << listenFd << "NULL,NULL)." << ENDL;

      if (connFd == -1) {
          perror("accept");
          continue;
      }

      DEBUG << "We have received a connection on " << connFd << ENDL;

      quitProgram = processConnection(connFd);
    }

    // Close the listening socket
    close(listenFd);

    return 0;
}
