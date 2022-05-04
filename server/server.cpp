#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "9000"
#define DEFAULT_BUFLEN 512

int main()
{
	int iResult;
	WSADATA wsaData;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		fprintf(stderr, "getaddrinfo failed: %d\n", iResult);
		goto Cleanup;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		fprintf(stderr, "Error at socket(): %d\n", WSAGetLastError());
		goto Cleanup;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		fprintf(stderr, "bind failed with error: %d\n", WSAGetLastError());
		goto Cleanup;
	}
	freeaddrinfo(result);
	result = NULL;

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		fprintf(stderr, "listen failed with error: %d\n", WSAGetLastError());
		goto Cleanup;
	}

	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		fprintf(stderr, "accept failed with error: %d\n", WSAGetLastError());
		goto Cleanup;
	}

	closesocket(ListenSocket);
	ListenSocket = INVALID_SOCKET;

	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				fprintf(stderr, "send failed with error: %d\n", WSAGetLastError());
				goto Cleanup;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0) {
			printf("Connection closing...\n");
		}
		else {
			fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());
			goto Cleanup;
		}
	} while (iResult > 0);

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		fprintf(stderr, "shutdown failed with error: %d\n", WSAGetLastError());
		goto Cleanup;
	}

Cleanup:
	if (result != NULL) {
		freeaddrinfo(result);
	}
	if (ListenSocket != INVALID_SOCKET) {
		closesocket(ListenSocket);
	}
	WSACleanup();

	return 0;
}
