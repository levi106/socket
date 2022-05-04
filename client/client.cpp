#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "9000"
#define DEFAULT_BUFLEN 512

int main(int argc, char** argv)
{
	int iResult;
	WSADATA wsaData;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	SOCKET ConnectSocket = INVALID_SOCKET;
	const char* sendbuf = "hello world";
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	if (argc != 2) {
		fprintf(stderr, "usage: %s server-name\n", argv[0]);
		return 1;
	}

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		fprintf(stderr, "getaddrinfo failed with error: %d\n", iResult);
		goto Cleanup;
	}
	
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			fprintf(stderr, "socket failed with error: %d\n", WSAGetLastError());
			goto Cleanup;
		}
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);
	result = NULL;

	if (ConnectSocket == INVALID_SOCKET) {
		fprintf(stderr, "Unable to connect to server\n");
		goto Cleanup;
	}

	printf("Press enter to send bytes\n");
	getchar();

	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		fprintf(stderr, "send failed with error: %d\n", WSAGetLastError());
		goto Cleanup;
	}

	printf("Bytes Sent: %d\n", iResult);

	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		fprintf(stderr, "shutdown failed with error: %d\n", WSAGetLastError());
		goto Cleanup;
	}

	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
		}
		else if (iResult == 0) {
			printf("Connection closed\n");
		}
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
		}
	} while (iResult > 0);

Cleanup:
	if (ConnectSocket != INVALID_SOCKET) {
		closesocket(ConnectSocket);
	}
	WSACleanup();

	return 0;
}