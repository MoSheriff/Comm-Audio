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
	static const size_t POOLSIZE = 4;
	static const size_t UDPRECVBUF = 1024 * 1024;
	static const unsigned short MULTICASTPORT = 45000;
	static const unsigned short LISTENPORT = 63400;
	static const char *MULTICASTIP;

	enum AppType {
		CLIENT, SERVER
	};

	NetworkingComponent(AppType appType);
	~NetworkingComponent();
	bool initialize();
	int sendMulticast(const char *buffer, size_t bufSize);
	int joinMulticastGroup();
	int leaveMulticastGroup();
	int connectToServer(const std::string& ipAddress, unsigned short port);
	int sendData(SOCKET sock, const void *data, size_t dataSize);
	int sendData(const void *data, size_t dataSize);
	int receiveData(WSABUF *buffer);
	SOCKET waitForClient(std::string& ipAddress);
	SOCKET waitForClient();

private:
	static const size_t IOCP_TCP_READ = 0;
    static const size_t IOCP_UDP_READ = 1;

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
	SOCKET tcpSocket_;
	HANDLE hIoCp_;
	AppType appType_;
	HANDLE queueSem_;
	CRITICAL_SECTION mutex_;
	std::queue<WSABUF> tcpDataQueue_;

	DWORD WINAPI WorkerThread();
	bool processTcp(SocketInformation *sockInfo, DWORD bytesTransferred);
	bool processUdp(SocketInformation *sockInfo, DWORD bytesTransferred);
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