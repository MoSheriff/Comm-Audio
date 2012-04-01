#ifndef NETWORKING_COMPONENT
#define NETWORKING_COMPONENT

#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")

class NetworkingComponent
{
public:
	static const size_t POOLSIZE = 4;
	static const size_t UDPRECVBUF = 1024 * 1024;
	static const unsigned short MULTICASTPORT = 45000;
	static const char *MULTICASTIP;

	~NetworkingComponent();
	bool initialize();
	int sendMulticast(const char *buffer, size_t bufSize);
	int joinMulticastGroup();
	int leaveMulticastGroup();

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
	HANDLE hIoCp_;

	DWORD WINAPI WorkerThread();
	bool processTcp(SocketInformation *sockInfo, DWORD bytesTransferred);
	bool processUdp(SocketInformation *sockInfo, DWORD bytesTransferred);
	static DWORD WINAPI WorkerStarter(void *thisInstance)
	{
		NetworkingComponent *component = (NetworkingComponent *) thisInstance;
		return component->WorkerThread();
	}
};

#endif