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
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <cstdlib>

#define MAX_RECV_SIZE   2048

constexpr char* BASE_URL = "tgftp.nws.noaa.gov";
constexpr char* FTP_PORT = "21";


typedef struct RecvResult_t
{
    int bytesRecieved;
    char* message;
} RecvResult;


int Send(SOCKET sock, const char* message, int messageLength, int flags = 0);
RecvResult Recv(SOCKET sock, int flags = 0);

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
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char* hostname = "tgftp.nws.noaa.gov\0";

    resultCode = getaddrinfo(BASE_URL, FTP_PORT, &hints, &result);  
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
    printf("Connected!\n");

    const char* fileName = "/SL.us008001/DF.of/DC.radar/DS.p19r0/SI.kmxx/sn.last";

    const char* user = "anonymous";
    const char* pass = "tjdickerson@gmail.com";
    
    // control socket vs data socket?
    { 
        char msg[2048];
        char* format = "init\r\nUSER %s\r\nPASS %s\r\nSYST\r\n\r\n\0";
        sprintf(msg, format, user, pass);
        int len = strlen(msg);

        Send(sock, msg, strlen(msg));

        // shutdown before recv for some reason.
        // if we dont it will like sit there and wait for userpass
        resultCode = shutdown(sock, SD_SEND);
        if (resultCode == SOCKET_ERROR)
        {
            printf("Failed to shutdown the socket...\n");
            closesocket(sock);
            WSACleanup();
            return resultCode;
        }   

        RecvResult res = Recv(sock);
        printf("Recieved: %d\n", res.bytesRecieved);
        printf("%s\n", res.message);

        if (res.message)
            free(res.message);
    }


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

    printf("Recieving data...\n");

    char buffer[MAX_RECV_SIZE];
    int bytesRecieved = 1;
    while (bytesRecieved > 0)
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









