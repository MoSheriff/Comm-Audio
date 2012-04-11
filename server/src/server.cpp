#include "server.h"

int main(int argc, char **argv) {
	initializeSonglist();
	nc.initialize();
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)&IncomingConnectionProc,0,0,0);
	while(true) {
		if(playlist.size() != 0) {
			openFile();
		}
	}
}

void IncomingConnectionProc(void* ID) {
	char recieveBufData[50];
	char rcvBuf[50];
	DWORD bytesRead = 1;
	int recieveCheck;
	SOCKET clientSock;
	string clientIP;
	while(true) {
		clientSock = nc.waitForClient(clientIP);
		recieveCheck = recvfrom(clientSock, rcvBuf, 50, 0,0,0);
		if(clientList.empty()) {
			clientList.push_back(clientIP);
		}
		for(it = clientList.begin(); it != clientList.end(); it++) {
			if(*it == clientIP) {
				it = clientList.begin();
				break;
			}
		}
		if(it != clientList.begin()) { 
			clientList.push_back(clientIP);
		}
		if(recieveCheck != 0) { 
			rcvBuf[recieveCheck] = '\0';
			switch(rcvBuf[0]) {
				case '0':
					sscanf(rcvBuf, "0:%50c", recieveBufData);
					recieveBufData[recieveCheck-2] = '\0';
					playlist.push_back(songList[recieveBufData]);
					nc.endTCPConnection(clientSock);
					continue;
				case '1':
					sscanf(rcvBuf, "1:%50c", recieveBufData);
					recieveBufData[recieveCheck-2] = '\0';
					downloadFileName = stringToCharStar(songList[recieveBufData],0);
					CreateThread(0,0,(LPTHREAD_START_ROUTINE)SendFileProc,(LPVOID)clientSock,0,0);
					continue;
				case '2':
					memset(sendClientList, '\0', sizeof(sendClientList));
					for(it = clientList.begin(); it != clientList.end(); it++) {
						strcat(sendClientList, stringToCharStar(*it,1));
					}
					it = clientList.begin();
					sendClientList[strlen(sendClientList)-1] = '\0';
					nc.sendData(clientSock, sendClientList, strlen(sendClientList));
					nc.endTCPConnection(clientSock);
					continue;
				case '3':
					skip = true;
					nc.endTCPConnection(clientSock);
					continue;
				case '4':
					nc.sendData(clientSock, songTitles, strlen(songTitles));
					nc.endTCPConnection(clientSock);
					continue;
				case '5':
					clientList.remove(clientIP);
					nc.endTCPConnection(clientSock);
					continue;
				default:
					nc.endTCPConnection(clientSock);
					continue;
			}
		}
	}
}

void SendFileProc(SOCKET sock) {
	DWORD bytesRead = 1;
	HANDLE sendFile;
	sendFile = CreateFile(downloadFileName,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
	while (bytesRead != 0) {
		ReadFile(sendFile, sendFileBuf, READ_BUFFER_SIZE, &bytesRead,0);
		if(nc.sendData(sock, sendFileBuf, bytesRead) == -1) {
			printf("Send failed.");	
		};
	}
	CloseHandle(sendFile);
	bytesRead = 1;
	nc.endTCPConnection(sock);
}


void openFile() {
	fileName = stringToCharStar(playlist.front(),0);
	wavFile = CreateFile(fileName,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
	sendDataToClients();
}

void sendDataToClients() {
	DWORD bytesRead;
	double test;
	size_t sendBufferSize = READ_BUFFER_SIZE;
	test = ((double)clientList.size())/2;
	while(true) {
		if(skip == true) {
			skip = false;
			break;
		}
		ReadFile(wavFile, fileBuf, READ_BUFFER_SIZE, &bytesRead,0);
		if(bytesRead == 0) {
			break;
		}
		nc.sendMulticast(fileBuf, bytesRead);
		Sleep(5);
	}
	CloseHandle(wavFile);
	playlist.pop_front();
}

char* stringToCharStar(string temp, int flag) {
	char* result;
	result = new char[temp.size()+1];
	memset(result, '\0', strlen(result));
	if(flag == 1) {
	result[temp.size()] = ',';
	}
	memcpy(result, temp.c_str(), temp.size());
	return result;
}

void initializeSonglist() {
	songList.insert(pair<string,string>(songOne, songOneLoc));
	songList.insert(pair<string,string>(songTwo, songTwoLoc));
	songList.insert(pair<string,string>(songThree, songThreeLoc));
	songList.insert(pair<string,string>(songFour, songFourLoc));
	songList.insert(pair<string,string>(songFive, songFiveLoc));
	songList.insert(pair<string,string>(songSix, songSixLoc));
	songList.insert(pair<string,string>(songSeven, songSevenLoc));
	songList.insert(pair<string,string>(songEight, songEightLoc));
	songList.insert(pair<string,string>(songNine, songNineLoc));
	songList.insert(pair<string,string>(songTen, songTenLoc));
	songList.insert(pair<string,string>(songEleven, songElevenLoc));
	songList.insert(pair<string,string>(songTwelve, songTwelveLoc));
	songList.insert(pair<string,string>(songThirteen, songThirteenLoc));
	songList.insert(pair<string,string>(songFourteen, songFourteenLoc));
	songList.insert(pair<string,string>(songFifteen, songFifteenLoc));
	songList.insert(pair<string,string>(songSixteen, songSixteenLoc));
	songList.insert(pair<string,string>(songSeventeen, songSeventeenLoc));
	songList.insert(pair<string,string>(songEighteen, songEighteenLoc));
	songList.insert(pair<string,string>(songNineteen, songNineteenLoc));
	songList.insert(pair<string,string>(songTwenty, songTwentyLoc));
	strcpy(songTitles,stringToCharStar(songOne,1));
	strcat(songTitles, stringToCharStar(songTwo,1));
	strcat(songTitles, stringToCharStar(songThree,1));
	strcat(songTitles, stringToCharStar(songFour,1));
	strcat(songTitles, stringToCharStar(songFive,1));
	strcat(songTitles, stringToCharStar(songSix,1));
	strcat(songTitles, stringToCharStar(songSeven,1));
	strcat(songTitles, stringToCharStar(songEight,1));
	strcat(songTitles, stringToCharStar(songNine,1));
	strcat(songTitles, stringToCharStar(songTen,1));
	strcat(songTitles, stringToCharStar(songEleven,1));
	strcat(songTitles, stringToCharStar(songTwelve,1));
	strcat(songTitles, stringToCharStar(songThirteen,1));
	strcat(songTitles, stringToCharStar(songFourteen,1));
	strcat(songTitles, stringToCharStar(songFifteen,1));
	strcat(songTitles, stringToCharStar(songSixteen,1));
	strcat(songTitles, stringToCharStar(songSeventeen,1));
	strcat(songTitles, stringToCharStar(songEighteen,1));
	strcat(songTitles, stringToCharStar(songNineteen,1));
	strcat(songTitles, stringToCharStar(songTwenty,1));
	songTitles[strlen(songTitles)-1] = '\0'; 
}