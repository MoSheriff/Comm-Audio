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

NetworkingComponent::NetworkingComponent(AppType appType)
	: appType_(appType) {}

NetworkingComponent::~NetworkingComponent()
{
	leaveMulticastGroup();
	closesocket(udpSocket_);
	closesocket(udpMicSocket_);
	if (appType_ == CLIENT)
		closesocket(udpMicSendSocket_);	
	CloseHandle(hIoCp_);
	CloseHandle(tcpQueueSem_);
	CloseHandle(udpQueueSem_);
	CloseHandle(udpMicQueueSem_);
	DeleteCriticalSection(&tcpMutex_);
	DeleteCriticalSection(&udpMutex_);
	DeleteCriticalSection(&udpMicMutex_);
}

bool NetworkingComponent::processTcp(SocketInformation *sockInfo, WSABUF *data)
{
	static DWORD flags = 0;

	EnterCriticalSection(&tcpMutex_);
	tcpDataQueue_.push(*data);
	ReleaseSemaphore(tcpQueueSem_, 1, NULL);
	LeaveCriticalSection(&tcpMutex_);

	if (data->len != 0)
	{
		if (WSARecv(sockInfo->socket, &(sockInfo->dataBuf), 1, NULL, &flags, 
			&(sockInfo->overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				MessageBox(NULL, "WSARecv() failed.", "Error", MB_ICONERROR);
				return false;
			}
		}
	}

	return true;
}

bool NetworkingComponent::processUdp(SocketInformation *sockInfo, WSABUF *data)
{
	static DWORD flags = 0;

	EnterCriticalSection(&udpMutex_);
	udpDataQueue_.push(*data);
	ReleaseSemaphore(udpQueueSem_, 1, NULL);
	LeaveCriticalSection(&udpMutex_);

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

bool NetworkingComponent::processUdpMic(SocketInformation *sockInfo, WSABUF *data)
{
	static DWORD flags = 0;

	EnterCriticalSection(&udpMicMutex_);
	udpMicDataQueue_.push(*data);
	ReleaseSemaphore(udpMicQueueSem_, 1, NULL);
	LeaveCriticalSection(&udpMicMutex_);

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

			WSABUF data;
			data.buf = new char[bytesTransferred + 1];	
			if (bytesTransferred == 0)	
			{
				closesocket(sockInfo->socket);
				delete sockInfo;
			}
			else
			{
			    memcpy(data.buf, sockInfo->dataBuf.buf, bytesTransferred);
			}

			data.len = bytesTransferred;

			switch(completionKey)
			{
			case IOCP_TCP_READ:
				processTcp(sockInfo, &data);
				break;

			case IOCP_UDP_READ:
				processUdp(sockInfo, &data);
				break;

			case IOCP_UDPSOCK_READ:
				processUdpMic(sockInfo, &data);
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

bool NetworkingComponent::activateIoCp(SOCKET sock, DWORD completionKey)
{
	if (!AssociateSocketCompletionPort(hIoCp_, sock, completionKey))
		return false;

	SocketInformation *sockInfo = new SocketInformation;
	sockInfo->socket = sock;
	memset(&(sockInfo->overlapped), 0, sizeof(sockInfo->overlapped));
	sockInfo->dataBuf.len = UDPRECVBUF;
	sockInfo->dataBuf.buf = sockInfo->buffer;
	sockInfo->fromLen = sizeof(sockInfo->from);

	DWORD flags = 0;
	if (WSARecv(sockInfo->socket, &(sockInfo->dataBuf), 1, NULL, &flags, 
		&(sockInfo->overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return false;
	}

	return true;
}

bool NetworkingComponent::initializeUdp()
{
	if ((udpSocket_ = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		return false;

    if ((udpMicSendSocket_ = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
        return false;

	BOOL reuse = TRUE;
	if (setsockopt(udpSocket_, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(reuse)) == SOCKET_ERROR)
		return false;

	if ((udpMicSocket_ = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		return false;

	if (setsockopt(udpMicSocket_, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(reuse)) == SOCKET_ERROR)
		return false;

	struct sockaddr_in addrInfo;
	memset(&addrInfo, 0, sizeof(addrInfo));
	addrInfo.sin_family = AF_INET;
	addrInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	addrInfo.sin_port = htons(MULTICASTPORT);

	if (bind(udpSocket_, (sockaddr *) &addrInfo, sizeof(addrInfo)) == SOCKET_ERROR)
		return false;

	addrInfo.sin_port = htons(UDPPORT);
	if (bind(udpMicSocket_, (sockaddr *) &addrInfo, sizeof(addrInfo)) == SOCKET_ERROR)
		return false;

	// If this is client, associate socket with completion port so
	// we can listen from it. If it is server, no need to associate 
	// udp socket with completion port since we are not going to 
	// receive any udp data in server
	if (appType_ == CLIENT)
	{
		size_t len = UDPRECVBUF;
		if (setsockopt(udpSocket_, SOL_SOCKET, SO_RCVBUF, (char *) &len, sizeof(len)) != 0)
			return false;

		if (setsockopt(udpMicSocket_, SOL_SOCKET, SO_RCVBUF, (char *) &len, sizeof(len)) != 0)
			return false;

		if (!activateIoCp(udpSocket_, IOCP_UDP_READ))
			return false;

		if (!activateIoCp(udpMicSocket_, IOCP_UDPSOCK_READ))
			return false;
	}

	return true;
}

bool NetworkingComponent::initializeTcp()
{
	if ((tcpSocket_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		return false;

	if (appType_ == SERVER)
	{
		struct sockaddr_in server;
		memset(&server, 0, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = htonl(INADDR_ANY);
		server.sin_port = htons(LISTENPORT);

		if (bind(tcpSocket_, (struct sockaddr *) &server, sizeof(server)) == -1)
			return false;

		listen(tcpSocket_, 5);
	}

	return true;
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

	tcpQueueSem_ = CreateSemaphore(NULL, 0, 50, NULL);
	udpQueueSem_ = CreateSemaphore(NULL, 0, 50, NULL);
	udpMicQueueSem_ = CreateSemaphore(NULL, 0, 50, NULL);

	InitializeCriticalSection(&tcpMutex_);
	InitializeCriticalSection(&udpMutex_);
	InitializeCriticalSection(&udpMicMutex_);

	if (!initializeUdp())
		return false;

	return initializeTcp();
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

int NetworkingComponent::sendUDP(const char *buffer, size_t bufSize, const std::string& ipAddress)
{
	struct sockaddr_in dstAddr;
	dstAddr.sin_family = AF_INET;
	dstAddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());
	dstAddr.sin_port = htons(UDPPORT);

	return sendto(udpMicSendSocket_, buffer, bufSize, 0, (struct sockaddr *) &dstAddr, sizeof(dstAddr));
}

int NetworkingComponent::receiveUDP(WSABUF *buffer)
{
	WSABUF data;

	WaitForSingleObject(udpMicQueueSem_, INFINITE);
	EnterCriticalSection(&udpMicMutex_);

	if (!udpMicDataQueue_.empty())
	{
		data = udpMicDataQueue_.front();	
		buffer->buf = data.buf;
		buffer->len = data.len;
		udpMicDataQueue_.pop();
	}

	LeaveCriticalSection(&udpMicMutex_);
	return data.len;
}

int NetworkingComponent::connectToServer(const std::string& ipAddress, unsigned short port)
{
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ipAddress.c_str());
	server.sin_port = htons(port);

	if ((tcpSocket_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		return false;

	int ret = connect(tcpSocket_, (sockaddr *) &server, sizeof(server));

	if (!activateIoCp(tcpSocket_, IOCP_TCP_READ))
		return SOCKET_ERROR;

	return ret;
}

int NetworkingComponent::sendData(const void *data, size_t dataSize)
{
	return send(tcpSocket_, (char *) data, dataSize, 0);
}

int NetworkingComponent::sendData(SOCKET sock, const void *data, size_t dataSize)
{
	return send(sock, (char *) data, dataSize, 0);
}

int NetworkingComponent::receiveData(WSABUF *buffer)
{
	WSABUF data;

	WaitForSingleObject(tcpQueueSem_, INFINITE);
	EnterCriticalSection(&tcpMutex_);

	if (!tcpDataQueue_.empty())
	{
		data = tcpDataQueue_.front();	
		buffer->buf = data.buf;
		buffer->len = data.len;
		tcpDataQueue_.pop();
	}

	LeaveCriticalSection(&tcpMutex_);
	return data.len;
}

int NetworkingComponent::receiveMulticast(WSABUF *buffer)
{
	WSABUF data;

	WaitForSingleObject(udpQueueSem_, INFINITE);
	EnterCriticalSection(&udpMutex_);

	if (!udpDataQueue_.empty())
	{
		data = udpDataQueue_.front();	
		buffer->buf = data.buf;
		buffer->len = data.len;
		udpDataQueue_.pop();
	}

	LeaveCriticalSection(&udpMutex_);
	return data.len;
}

SOCKET NetworkingComponent::waitForClient()
{
	return waitForClient(std::string());
}

SOCKET NetworkingComponent::waitForClient(std::string& ipAddress)
{
	struct sockaddr_in clientInfo;
	int clientSize = sizeof(clientInfo);

	SOCKET client = accept(tcpSocket_, (sockaddr *) &clientInfo, &clientSize);
	ipAddress = std::string(inet_ntoa(clientInfo.sin_addr));

	//if (!activateIoCp(client, IOCP_TCP_READ))
	//	return INVALID_SOCKET;

	return client;
}

void NetworkingComponent::endTCPConnection(SOCKET sock)
{
	shutdown(sock, SD_BOTH);
	closesocket(sock);
}