#include "NetworkingComponent.h"
#include <WS2tcpip.h>

const char *NetworkingComponent::MULTICASTIP = "234.5.6.7";

// some wrappers for completion port creation
static HANDLE CreateNewCompletionPort()
{
	return CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

static bool AssociateSocketCompletionPort(HANDLE hIoPort, SOCKET socket, DWORD completionKey)
{
	HANDLE h = CreateIoCompletionPort((HANDLE) socket, hIoPort, completionKey, 0);
	return h == hIoPort;
}

NetworkingComponent::~NetworkingComponent()
{
	leaveMulticastGroup();
	closesocket(udpSocket_);
	CloseHandle(hIoCp_);
}

bool NetworkingComponent::processTcp(SocketInformation *sockInfo, DWORD bytesTransferred)
{
	static DWORD flags = 0;

	if (bytesTransferred == 0)
	{
		shutdown(sockInfo->socket, SD_BOTH);
		closesocket(sockInfo->socket);
		free(sockInfo);
		return true;
	}

	if (WSARecv(sockInfo->socket, &(sockInfo->dataBuf), 1, NULL, &flags, 
		&(sockInfo->overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			MessageBox(NULL, "WSARecv() failed.", "Error", MB_ICONERROR);
			return false;
		}
	}

	return true;
}

bool NetworkingComponent::processUdp(SocketInformation *sockInfo, DWORD bytesTransferred)
{
	static DWORD flags = 0;

	if (WSARecvFrom(sockInfo->socket, &(sockInfo->dataBuf), 1, NULL, &flags, 
		(struct sockaddr *) &(sockInfo->from), &(sockInfo->fromLen), &(sockInfo->overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			MessageBox(NULL, "WSARecv() failed.", "Error", MB_ICONERROR);
			return false;
		}
	}

	return true;
}

DWORD WINAPI NetworkingComponent::WorkerThread()
{
	BOOL completionStatus;
	DWORD bytesTransferred;
	DWORD err;
	ULONG_PTR completionKey;
	OVERLAPPED *overlap;
	SocketInformation *sockInfo;
	DWORD flags = 0;

	for (;;)
	{
		completionStatus = GetQueuedCompletionStatus(hIoCp_, &bytesTransferred, &completionKey, &overlap, INFINITE);
		err = GetLastError();

		if (completionStatus) 
		{
			sockInfo = (SocketInformation *) overlap;

			switch(completionKey)
			{
			case IOCP_TCP_READ:
				processTcp(sockInfo, bytesTransferred);
				break;

			case IOCP_UDP_READ:
				processUdp(sockInfo, bytesTransferred);
				break;
			}
		} 
		else 
		{
			if (overlap != NULL) 
			{
				//PrintGeneralError("Failed completed IO request in worker thread.");
			}
			else 
			{
				if (err != WAIT_TIMEOUT)
				{
					//PrintGeneralError("Bad call to GetQueuedCompletionStatus.");
				}
			}
		}
	}
}

bool NetworkingComponent::initialize()
{
	if ((hIoCp_ = CreateNewCompletionPort()) == NULL)
		return false;

	HANDLE workerThreads[POOLSIZE];

	for (int i = 0; i < POOLSIZE; i++)
	{
		workerThreads[i] = CreateThread(NULL, 0, WorkerStarter, (void *) this, 0, NULL);
		if (workerThreads[i] == NULL)
			return false;
	}

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	if ((udpSocket_ = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		return false;

	BOOL reuse = TRUE;
	if (setsockopt(udpSocket_, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(reuse)) == SOCKET_ERROR)
		return false;

	struct sockaddr_in addrInfo;
	memset(&addrInfo, 0, sizeof(addrInfo));
	addrInfo.sin_family = AF_INET;
	addrInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	addrInfo.sin_port = htons(MULTICASTPORT);

	if (bind(udpSocket_, (sockaddr *) &addrInfo, sizeof(addrInfo)) == SOCKET_ERROR)
		return false;

	SocketInformation *sockInfo = new SocketInformation;
	memset(&(sockInfo->overlapped), 0, sizeof(sockInfo->overlapped));
	sockInfo->socket = udpSocket_;
	sockInfo->dataBuf.buf = sockInfo->buffer;
	sockInfo->dataBuf.len = UDPRECVBUF;
	sockInfo->fromLen = sizeof(sockInfo->from);

	if (setsockopt(sockInfo->socket, SOL_SOCKET, SO_RCVBUF, (char *) &sockInfo->dataBuf.len, sizeof(sockInfo->dataBuf.len)) != 0)
		return false;

	if (!AssociateSocketCompletionPort(hIoCp_, udpSocket_, IOCP_UDP_READ))
		return false;

	DWORD flags = 0;
	if (WSARecvFrom(sockInfo->socket, &(sockInfo->dataBuf), 1, NULL, &flags, 
		(struct sockaddr *) &(sockInfo->from), &(sockInfo->fromLen), &(sockInfo->overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return false;
	}

	return true;
}

int NetworkingComponent::joinMulticastGroup()
{
	IP_MREQ minterface;

	minterface.imr_multiaddr.s_addr = inet_addr(MULTICASTIP);
	minterface.imr_interface.s_addr = INADDR_ANY;

	return setsockopt(udpSocket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&minterface, sizeof(minterface));
}

int NetworkingComponent::leaveMulticastGroup()
{
	IP_MREQ minterface;

	minterface.imr_multiaddr.s_addr = inet_addr(MULTICASTIP);
	minterface.imr_interface.s_addr = INADDR_ANY;

	return setsockopt(udpSocket_, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&minterface, sizeof(minterface));
}

int NetworkingComponent::sendMulticast(const char *buffer, size_t bufSize)
{
	static struct sockaddr_in dstAddr;
	dstAddr.sin_family = AF_INET;
	dstAddr.sin_addr.s_addr = inet_addr(MULTICASTIP);
	dstAddr.sin_port = htons(MULTICASTPORT);

	return sendto(udpSocket_, buffer, bufSize, 0, (struct sockaddr *) &dstAddr, sizeof(dstAddr));
}

int main()
{
	NetworkingComponent nc;
	nc.initialize();

	system("pause");
}