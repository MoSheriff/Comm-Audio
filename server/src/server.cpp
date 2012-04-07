#include "server.h"

int main(int argc, char **argv) {
	initializeSonglist();

	nc.initialize();	
	playlist.push_back(songTwoLoc);
	//CreateThread(0,0,(LPTHREAD_START_ROUTINE)&NetworkProc,0,0,0);
	//CreateThread(0,0,(LPTHREAD_START_ROUTINE)&MusicProc,0,0,0);
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)&UDPInputProc,0,0,0);
	while(true) {
		if(playlist.size() != 0) {
			openFile();
			sendDataToClients();
		}
	}
}

void MusicProc(void *ID) {
}

void NetworkProc(void *ID) {
	while(true) {
		SOCKET clientSock = nc.waitForClient();
		nc.sendData(clientSock, songTitles, strlen(*songTitles));
	}
}

void UDPInputProc(void *ID) {
	while(true) {
		nc.receiveData(recieveBuffer);
		playlist.push_back(recieveBuffer->buf);
	}	
}

void initializeSonglist() {
	songList.insert(pair<string,string>(songOne, songOneLoc));
	songList.insert(pair<string,string>(songTwo, songTwoLoc));
	songList.insert(pair<string,string>(songThree, songThreeLoc));
	songTitles[0] = stringToCharStar(songList[songOne]);
	songTitles[1] = stringToCharStar(songList[songTwo]);
	songTitles[2] = stringToCharStar(songList[songThree]);
}

void openFile() {
	fileName = stringToCharStar(playlist.front());
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
		printf("Data Sent.");
		Sleep(5);
		//wait(1);
	}
	playlist.pop_front();
}

char* stringToCharStar(string temp) {
	char* result;
	result = new char[temp.size()+1];
	result[temp.size()] = '\0';
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
