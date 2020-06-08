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

constexpr char* BASE_URL = "tgftp.nws.noaa.gov";
constexpr char* FTP_PORT = "21";

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
	// Now we can navigate/request data from the FTP...
	char msg[255];
	sprintf(msg, "SIZE %s\r\n", fileName);
	int sent = send(sock, msg, 255, 0);

	printf("Sent %d bytes?\n", sent);

	char inmsg[1024];
	int recvd = recv(sock, inmsg, 1024, 0);

	printf("Recieved %d bytes\n", recvd);

	char* recmsg = (char*)malloc(recvd + 1);
	memcpy(recmsg, &inmsg[0], recvd);

	recmsg[recvd] = '\0';
	printf("File size: %s", recmsg);

	free(recmsg);

	// Cleanup the connection to let the server know we are done.
	resultCode = shutdown(sock, SD_SEND);
	if (resultCode == SOCKET_ERROR)
	{
		printf("Failed to shutdown the socket...\n");
		closesocket(sock);
		WSACleanup();
		return resultCode;
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}