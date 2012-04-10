#include "server.h"

int main(int argc, char **argv) {
	initializeSonglist();

	nc.initialize();	
	playlist.push_back(songOneLoc);
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)&SendSongListProc,0,0,0);
	//CreateThread(0,0,(LPTHREAD_START_ROUTINE)&MusicProc,0,0,0);
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)&IncomingMessageProc,0,0,0);
	while(true) {
		if(playlist.size() != 0) {
			openFile();
			sendDataToClients();
		}
	}
}

void MusicProc(void *ID) {
}

void SendSongListProc(void *ID) {
	while(true) {
		SOCKET clientSock = nc.waitForClient();
		nc.sendData(clientSock, songTitles, strlen(songTitles));
	}
}

void IncomingMessageProc(void *ID) {
	char recieveBufData[50];
	while(true) {
		if(nc.receiveData(&recieveBuffer) != 0) { 
			recieveBuffer.buf[recieveBuffer.len] = '\0';
			switch(recieveBuffer.buf[0]) {
				case '0':
					sscanf(recieveBuffer.buf, "0:%50c", recieveBufData);
					recieveBufData[recieveBuffer.len-2] = '\0';
					playlist.push_back(songList[recieveBufData]);
					break;
				case '1':
					break;
			}
		}
	}
}

void initializeSonglist() {
	songList.insert(pair<string,string>(songOne, songOneLoc));
	songList.insert(pair<string,string>(songTwo, songTwoLoc));
	songList.insert(pair<string,string>(songThree, songThreeLoc));
	songTitles = stringToCharStar(songOne,1);
	strcat(songTitles, stringToCharStar(songTwo,1));
	strcat(songTitles, stringToCharStar(songThree,1));
	songTitles[strlen(songTitles)-1] = '\0'; 
}

void openFile() {
	fileName = stringToCharStar(playlist.front(),0);

	wavFile = CreateFile(fileName,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
	//getFileSize(fileName);
	//wavFile.open(fileName);
}

void sendDataToClients() {
	DWORD bytesRead;
	size_t sendBufferSize = READ_BUFFER_SIZE;
	//for(int i = 0; i < numFileChunks; i++) {
	while(true) {
		ReadFile(wavFile, fileBuf, READ_BUFFER_SIZE, &bytesRead,0);
		if(bytesRead == 0) 
			break;
		//wavFile.read(fileBuf, bytesRead);
		nc.sendMulticast(fileBuf, bytesRead);
		//printf("Data Sent.");
		Sleep(5);
	}
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

void getFileSize(char* fileName) {
	FILE* file;
	file = fopen(fileName, "r");
	fseek(file,0, SEEK_END);
	fileSize = ftell(file);
	numFileChunks = fileSize/READ_BUFFER_SIZE;
	fclose(file);
}
