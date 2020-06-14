// 
// @todo @portability
/*

@todo:
    - For linux I need to implement this with sys/socket.h and arpa/inet
    - Because of keeping up with the sockets and what not, this may be a good 
     candidate for a class.

*/

// This is needed to prevent windows.h from creating conflicts between winsock
// and winsock2
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <Urlmon.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <cstdlib>


#ifdef _WIN32
    #define sleep Sleep
#else
    //?
#endif

#define __DEBUG__ 1

#define MAX_RECV_SIZE   2048
#define RECV_WAIT       250

constexpr char* TARGET_HOSTNAME = "tgftp.nws.noaa.gov";
constexpr char* FTP_PORT = "21";
constexpr char* HTTP_PORT = "80";


typedef struct RecvResult_t
{
    int bytesRecieved;
    char* message;    
} RecvResult;

typedef struct PasvResult_t
{
    char ip[16];
    int  port;
} PasvResult;


int Send(SOCKET sock, const char* message, int messageLength, int flags = 0);
RecvResult Recv(SOCKET sock, int flags = 0);


void LoginCommand(SOCKET socket)
{
    char* login = "USER %s\r\nPASS %s\r\n\0";
    char  buffer[2048];
    sprintf(buffer, login, "anonymous", "tjdickerson@gmail.com");    

    printf("Sending: \n%s", buffer);

    Send(socket, buffer, strlen(buffer));
    sleep(RECV_WAIT);
    RecvResult res = Recv(socket);

    printf("Response: \n%s\n", res.message);

    if (res.message) free(res.message);
}

PasvResult PasvCommand(SOCKET socket)
{
    char* cmd = "PASV\r\n\0";   

    Send(socket, cmd, strlen(cmd));
    sleep(RECV_WAIT);
    RecvResult res = Recv(socket);

    printf("Response: \n%s\n", res.message);  
    char* openParen = strchr(res.message, '(');
    char* closeParen = strrchr(res.message, ')');
    char ipPort[24];
    char* delim = ",";

    strncpy(ipPort, (openParen + 1), (closeParen - 1) - openParen);
    size_t length = (closeParen - 1) - openParen;
    ipPort[length] = '\0';

    printf("%s\n", ipPort);

    PasvResult result = {};
    char* token = strtok(ipPort, delim);
    int i = 0;
    while(token)
    {
        if (i < 4)
        {
            strcat(result.ip, token);
            if (i < 3) strcat(result.ip, ".");
        }
        else
        {
            int part1 = atoi(token);
            token = strtok(NULL, delim);
            int part2 = atoi(token);

            result.port = (part1 * 256) + part2;
            break;
        }

        i += 1;
        token = strtok(NULL, delim);
    }

    printf("IP:   %s\n", result.ip);
    printf("Port: %d\n", result.port);

    if (res.message) free(res.message);
    return result;
}

void QuitCommand(SOCKET socket)
{
    char* quit = "QUIT\r\n\0";   

    Send(socket, quit, strlen(quit));
    sleep(RECV_WAIT);
    RecvResult res = Recv(socket);

    printf("Response: \n%s\n", res.message);

    if (res.message) free(res.message);
}


int SizeCommand(SOCKET socket, const char* filename)
{
    char* fmt = "SIZE %s\r\n\0";
    char cmd[1024];
    sprintf(cmd, fmt, filename);
    Send(socket, cmd, strlen(cmd));
    sleep(RECV_WAIT);
    RecvResult res = Recv(socket);

    char* space = strchr(res.message, ' ');
    char cFileSize[10];
    strncpy(cFileSize, space + 1, strlen(space) - 1);
    cFileSize[strlen(space)] = '\0';

    int retVal = atoi(cFileSize);

    if (res.message) free(res.message);

    return retVal;
}

bool RetrieveFile(SOCKET controlSock,
                  SOCKET dataSock, 
                  const char* remoteFilename, 
                  const char* localFilename)
{
    char* fmt = "RETR %s\r\n\0";
    char cmd[1024];
    sprintf(cmd, fmt, remoteFilename);

    Send(controlSock, cmd, strlen(cmd));
    sleep(RECV_WAIT);

    // debug
    RecvResult res = Recv(controlSock);
    printf("Response for RETR: %s\n", res.message);

    printf("Downloading...");

    FILE* fp = fopen(localFilename, "w+b");
    if (!fp) {
        printf("Failed to create file...");
        return false;
    }

    char buffer[MAX_RECV_SIZE];
    int  bytesRead = 1;

    while(bytesRead > 0)
    {
        bytesRead = recv(dataSock, buffer, MAX_RECV_SIZE, 0);
        fwrite(buffer, sizeof(unsigned char), bytesRead, fp);

        if (bytesRead < MAX_RECV_SIZE) break;
    }

    if (fp) fclose(fp);
    return true;
}

int DownloadFile()
{
    // This may be able to be done once at application startup @todo
    // init winsock
    WSADATA wsaData;    

    int resultCode;
    resultCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (resultCode != 0)
    {
        return resultCode;
    }
    // end init

    SOCKET sock = INVALID_SOCKET;
    SOCKET dataSock = INVALID_SOCKET;

    struct addrinfo *result = NULL, *ptr = NULL, *dataInfo = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char* hostname = "tgftp.nws.noaa.gov\0";

    resultCode = getaddrinfo(TARGET_HOSTNAME, FTP_PORT, &hints, &result);  
    if (resultCode != 0)
    {
        printf("Failed to get address info for %s\n", hostname);
        WSACleanup();
        return resultCode;
    }

    // Result may have multiple addresses in it, if the first one fails
    // we could try them all. @todo
    ptr = result;
    sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (sock == INVALID_SOCKET)
    {
        printf("Failed to create socket\n");
        freeaddrinfo(result);
        WSACleanup();
        int err = WSAGetLastError();
        return err;
    }

    printf("Connecting to: %s\n", ptr->ai_addr->sa_data);
    resultCode = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (resultCode == SOCKET_ERROR)
    {
        printf("Failed to connect.\n");
        closesocket(sock);
        sock = INVALID_SOCKET;
        freeaddrinfo(result);
        WSACleanup();
        int err = WSAGetLastError();
        return err;
    }

    // printf("Connecting data socket..\n");
    // resultCode = connect(dataSock, ptr->ai_addr, (int)ptr->ai_addrlen);
    // if (resultCode == SOCKET_ERROR)
    // {
    //     printf("Failed to connect.\n");
    //     closesocket(dataSock);
    //     dataSock = INVALID_SOCKET;
    //     freeaddrinfo(result);
    //     WSACleanup();
    //     int err = WSAGetLastError();
    //     return err;
    // }

    printf("Connected!\n");

    const char* filename = "/SL.us008001/DF.of/DC.radar/DS.p19r0/SI.kmxx/sn.last\0";

    const char* user = "anonymous";
    const char* pass = "tjdickerson@gmail.com";


    LoginCommand(sock);
    int size = SizeCommand(sock, filename);        
    printf("Size of %s is %d bytes.\n", filename, size);

    PasvResult pasvResult = PasvCommand(sock);

    sockaddr_in pasvConn;
    pasvConn.sin_family = AF_INET;
    pasvConn.sin_port = htons(pasvResult.port);
    pasvConn.sin_addr.s_addr = inet_addr(pasvResult.ip);

    printf("Connecting data sock to: %s:%d\n", pasvResult.ip, pasvResult.port);

    dataSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (dataSock == INVALID_SOCKET)
    {
        printf("Error creating data socket...\n");
    }

    resultCode = connect(dataSock, (LPSOCKADDR)&pasvConn, sizeof(struct sockaddr));
    if(resultCode == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        printf("Failed to connect datasocket.\n");
        closesocket(dataSock);
        closesocket(sock);
        sock = INVALID_SOCKET;
        dataSock = INVALID_SOCKET;
        freeaddrinfo(result);
        WSACleanup();
        
        return err;
    }

    printf("Connected data socket! Grats.\n");

    RetrieveFile(sock, dataSock, filename, "C:\\tmp\\testing_radar.nx3");        

    QuitCommand(sock);

    resultCode = shutdown(sock, SD_SEND);
    if (resultCode == SOCKET_ERROR)
    {
        printf("Failed to shutdown the socket...\n");
        closesocket(sock);
        WSACleanup();
        return resultCode;
    }   

    // free addrinfo result?
    closesocket(sock);
    WSACleanup();
    return 0;
}


int Send(SOCKET sock, const char* message, int messageLength, int flags) 
{
    int bytesLeft = messageLength;
    int totalBytesSent = 0;

    while(bytesLeft > 0)
    {
        int sentBytes = send(sock, message, messageLength, flags);  
        bytesLeft -= sentBytes;
        totalBytesSent += sentBytes;

        // @todo
        // I need to see if send can return 0 bytes legitimately and decide
        // how to determine when to break out of here if things don't go
        // correctly.
    } 

    return totalBytesSent;
}



RecvResult Recv(SOCKET sock, int flags) 
{
    RecvResult result = {};
    char temp[15000];
    int totalRecieved = 0;

    char buffer[MAX_RECV_SIZE];
    int bytesRecieved = 1;
    bool moreToRead = true;
    while (bytesRecieved > 0 && moreToRead)
    {
        bytesRecieved = recv(sock, buffer, MAX_RECV_SIZE, flags);       

        int err = WSAGetLastError();
        if (err)
        {
            printf("Error: %d", err);
            break;
        }

        // @todo
        // make sure it didn't send more than MAX_RECV_SIZE.


        // @speed
        // this is probably slower than I'd like
        buffer[bytesRecieved] = '\0';
        memcpy(&temp[totalRecieved], buffer, bytesRecieved);
        
        totalRecieved += bytesRecieved;             

        if (bytesRecieved < MAX_RECV_SIZE) break;
    }

    if (totalRecieved > 0)
    {
        result.bytesRecieved = totalRecieved;   
        result.message = (char*)malloc((totalRecieved + 1) * sizeof(char));     
        memcpy(result.message, &temp[0], totalRecieved);
        result.message[totalRecieved] = '\0';
    }

    return result;
}









