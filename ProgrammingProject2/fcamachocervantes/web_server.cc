#include "web_server.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <regex>
#include <fstream>

bool VERBOSE;
using namespace std;

int readRequest(int socketFD, string &fileName)
{
    //Constant values 
    regex endOfRequest("\\r\\n\\r\\n");
    regex validFile(".*((file[0-9]\\.html)|(image[0-9]\\.jpg)).*");
    regex validHead("GET");
    string getRequest = "";
    int returnCode = 400;
    char buffer[10];

    //Reading all buffer data
    while (true)
    {
        //Reading in data from the buffer and appending into one string
        int bytesRead = recv(socketFD, buffer, sizeof(buffer), 0);
        getRequest.append(buffer, bytesRead);

        //Breaking loop once \r\n\r\n is found
        if (regex_search(getRequest, endOfRequest))
        {
            break;
        }
    }

    //Verbos debug showing the HTTP header
    DEBUG << "HTTP Header" << ENDL;
    DEBUG << getRequest << ENDL;
    
    //Getting the name of file using regex
    if (regex_search(getRequest, validHead))
    {
        smatch match;
        regex_search(getRequest, match, validFile);
        fileName = match[1];
        if (regex_search(fileName, validFile))
        {
            returnCode = 200;
            return returnCode;
        }
        else
        {
            return returnCode;
        }
    }
    else
    {
        returnCode = 400;
        return returnCode;
    }

    return returnCode;
}

void sendLine(int socketFD, string stringToSend)
{   
    //Converting the string to a char array that is 2 bytes longer than the string
    char buffer[stringToSend.size() + 2];

    //Copy the string content to the buffer
    strcpy(buffer, stringToSend.c_str());

    //Replace the last two bytes of the array with <CR> and <LF>
    buffer[stringToSend.size()] = '\r';
    buffer[stringToSend.size() + 1] = '\n';

    //Use write to send the modified array
    ssize_t bytesSent = write(socketFD, buffer, sizeof(buffer));

    if (bytesSent == -1) {
        perror("write");
    } else {
        DEBUG << ENDL;
        DEBUG << "Sent: " << stringToSend << ENDL;
    }
}

void send404(int socketFD)
{
    //HTTP response with error code 404 and content-type header
    string response = "HTTP/1.0 404 Not Found\r\n";
    response += "Content-Type: text/html\r\n";
    //Blank line to terminate the header
    response += "\r\n"; 

    //HTML body with a friendly error message
    response += "<html><body><h1>404 Not Found</h1>";
    response += "<p>The requested file was not found on this server.</p>";
    response += "</body></html>";

    //Send the response using the sendLine function
    sendLine(socketFD, response);
}

void send400(int socketFD)
{
    //HTTP response with error code 400
    string response = "HTTP/1.0 400 Bad Request\r\n";
    //Blank line to indicate the end of the response
    response += "\r\n"; 

    //Send the response using the sendLine function
    sendLine(socketFD, response);
}

void send200(int socketFD, string filename)
{
    //Use stat() function to get file size and check if the file exists
    struct stat fileStat;
    if (stat(filename.c_str(), &fileStat) == -1)
    {
        cout << "stat failed" << endl;
        //If stat fails, send a 404 response and exit the function
        send404(socketFD);
        return;
    }

    //Determine content type based on file extension
    string contentType = "text/html";
    size_t dotPosition = filename.find_last_of(".");
    if (dotPosition != string::npos)
    {
        string extension = filename.substr(dotPosition + 1);
        if (extension == "jpg" || extension == "jpeg")
        {
            contentType = "image/jpeg";
        }
    }

    //Construct the HTTP response header with status line, content-type, and content-length
    string response = "HTTP/1.0 200 OK\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + to_string(fileStat.st_size) + "\r\n";

    //Send the response header using the sendLine function
    sendLine(socketFD, response);

    //Send the file content
    ifstream fileStream(filename, ios::binary);

    //Allocate 10 bytes of memory
    char *buffer = new char[10]; 

    //Feading while the filestream is still good
    while (fileStream.good())
    {
        //Feading 10 bytes of data
        fileStream.read(buffer, 10);

        //Clear out the memory with bzero or memset
        memset(buffer + fileStream.gcount(), 0, 10 - fileStream.gcount());

        //Write the bytes read from the file to the socket
        ssize_t bytesSent = write(socketFD, buffer, 10);
        if (bytesSent == -1)
        {
            perror("write");
            delete[] buffer;
            return;
        }
    }

    //Clean up and close the file
    fileStream.close();
    delete[] buffer;
}
//**************************************************************************************
//* processConnection()
//* - Handles reading the line from the network and sending it back to the client.
//* - Returns true if the client sends "QUIT" command, false if the client sends "CLOSE".
//**************************************************************************************
int processConnection(int sockFd)
{
    string fileName = "";
    int returnCode = readRequest(sockFd, fileName);

    switch (returnCode)
    {
    case 400:
        send400(sockFd);
        break;
    case 404:
        send404(sockFd);
        break;
    case 200:
        send200(sockFd, fileName);
        break;
    default:
        break;
    }
    return 0;
}

//**************************************************************************************
//* main()
//* - Sets up the sockets and accepts new connection until processConnection() returns 1
//**************************************************************************************
int main(int argc, char *argv[])
{
    //********************************************************************
    //* Process the command line arguments
    //********************************************************************
    int opt = 0;
    while ((opt = getopt(argc, argv, "v")) != -1)
    {
        switch (opt)
        {
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

    //*******************************************************************
    //* Creating the inital socket is the same as in a client.
    //********************************************************************
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    //Error handling in case listening socket can't be created
    if (listenFd == -1)
    {
        perror("socket");
        exit(-1);
    }
    DEBUG << "Calling Socket() assigned file descriptor " << listenFd << ENDL;

    //********************************************************************
    //* The bind() and calls take a structure that specifies the
    //* address to be used for the connection. On the cient it contains
    //* the address of the server to connect to. On the server it specifies
    //* which IP address and port to lisen for connections.
    //********************************************************************
    struct sockaddr_in servaddr;
    srand(time(NULL));
    u_int16_t port = (rand() % 5001) + 5000;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    //********************************************************************
    //* Binding configures the socket with the parameters we have
    //* specified in the servaddr structure.  This step is implicit in
    //* the connect() call, but must be explicitly listed for servers.
    //********************************************************************
    DEBUG << "Calling bind(" << listenFd << "," << &servaddr << "," << sizeof(servaddr) << ")" << ENDL;
    bool bindSuccessful = false;
    while (!bindSuccessful)
    {
        if (bind(listenFd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0)
        {
            bindSuccessful = true;
        }
        //Handle bind failure, retry or choose a different port
        else
        {
            port = (rand() % 5001) + 5000;
            servaddr.sin_port = htons(port);
        }
    }
    cout << "Using port: " << port << endl;

    //********************************************************************
    //* Setting the socket to the listening state is the second step
    //* needed to being accepting connections.  This creates a queue for
    //* connections and starts the kernel listening for connections.
    //********************************************************************
    int listenQueueLength = 1;
    //Making sure that socket is set to listening state
    if (listen(listenFd, listenQueueLength) == -1)
    {
        perror("listen");
        exit(-1);
    }
    DEBUG << "Calling listen(" << listenFd << "," << listenQueueLength << ")" << ENDL;

    //********************************************************************
    //* The accept call will sleep, waiting for a connection.  When
    //* a connection request comes in the accept() call creates a NEW
    //* socket with a new fd that will be used for the communication.
    //********************************************************************
    bool quitProgram = false;
    while (!quitProgram)
    {
        int connFd = accept(listenFd, NULL, NULL);
        if (connFd == -1)
        {
            perror("accept");
            continue;
        }

        DEBUG << "Calling accept(" << listenFd << "NULL,NULL)." << ENDL;

        if (connFd == -1)
        {
            perror("accept");
            continue;
        }

        DEBUG << "We have received a connection on " << connFd << ENDL;

        int tempVal = processConnection(connFd);
        if (tempVal == 0)
        {
            quitProgram = false;
        }
        else
        {
            quitProgram = true;
        } 
    }

    //Close the listening socket
    close(listenFd);

    return 0;
}
