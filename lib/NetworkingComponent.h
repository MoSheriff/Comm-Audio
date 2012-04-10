#ifndef NETWORKING_COMPONENT
#define NETWORKING_COMPONENT

#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <queue>
#pragma comment(lib, "ws2_32.lib")

class NetworkingComponent
{
public:
	static const size_t POOLSIZE = 8;
	static const size_t UDPRECVBUF = 1024 * 1024;
	static const unsigned short MULTICASTPORT = 45000;
	static const unsigned short LISTENPORT = 63400;
	static const unsigned short UDPPORT = 64500;
	static const char *MULTICASTIP;

	enum AppType {
		CLIENT, SERVER
	};

	NetworkingComponent(AppType appType);
	~NetworkingComponent();
	bool initialize();
	int sendMulticast(const char *buffer, size_t bufSize);
	int receiveMulticast(WSABUF *buffer);
	int joinMulticastGroup();
	int leaveMulticastGroup();
	int connectToServer(const std::string& ipAddress, unsigned short port);
	int sendData(SOCKET sock, const void *data, size_t dataSize);
	int sendData(const void *data, size_t dataSize);
	int receiveData(WSABUF *buffer);
	SOCKET waitForClient(std::string& ipAddress);
	SOCKET waitForClient();
	int sendUDP(SOCKET sock, const char *buffer, size_t bufSize, const std::string& ipAddress, unsigned short port);
	int receiveUDP(WSABUF *buffer);

private:
	static const size_t IOCP_TCP_READ = 0;
    static const size_t IOCP_UDP_READ = 1;
	static const size_t IOCP_UDPSOCK_READ = 2;

	typedef struct {
		OVERLAPPED overlapped;
		SOCKET socket;
		char buffer[NetworkingComponent::UDPRECVBUF];
		WSABUF dataBuf;
		DWORD bytesSend;
		DWORD bytesRecv;
		struct sockaddr_in from;
		int fromLen;
	} SocketInformation;

	SOCKET udpSocket_;
	SOCKET udpMicSocket_;
	SOCKET tcpSocket_;
	HANDLE hIoCp_;
	AppType appType_;
	HANDLE tcpQueueSem_;
	HANDLE udpQueueSem_;
	HANDLE udpMicQueueSem_;
	CRITICAL_SECTION tcpMutex_;
	CRITICAL_SECTION udpMutex_;
	CRITICAL_SECTION udpMicMutex_;
	std::queue<WSABUF> tcpDataQueue_;
	std::queue<WSABUF> udpDataQueue_;
	std::queue<WSABUF> udpMicDataQueue_;

	DWORD WINAPI WorkerThread();
	bool processTcp(SocketInformation *sockInfo, WSABUF *data);
	bool processUdp(SocketInformation *sockInfo, WSABUF *data);
	bool processUdpMic(SocketInformation *sockInfo, WSABUF *data);
	bool initializeUdp();
	bool initializeTcp();
	bool activateIoCp(SOCKET sock, DWORD completionKey);
	
	static DWORD WINAPI WorkerStarter(void *thisInstance)
	{
		NetworkingComponent *component = (NetworkingComponent *) thisInstance;
		return component->WorkerThread();
	}
};

#endif