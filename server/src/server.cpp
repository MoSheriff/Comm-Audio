#include "server.h"

int main(int argc, char **argv) {
	HANDLE one, two, three;
	nc.initialize();	
	initializeSonglist();
	playlist.push_back(songOneLoc);
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)&NetworkProc,0,0,0);
	//CreateThread(0,0,(LPTHREAD_START_ROUTINE)&MusicProc,0,0,0);
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)&UDPInputProc,0,0,0);
	//while(true) {
		if(playlist.size() != 0) {
			readFile();
			sendDataToClients();
		}
	//}
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

void readFile() {
	fileName = stringToCharStar(playlist.front());
	getFileSize(fileName);
	wavFile.open(fileName);
}

void sendDataToClients() {
	for(int i = 0; i < numFileChunks; i++) {
		wavFile.read(fileBuf, READ_BUFFER_SIZE);
		nc.sendMulticast(fileBuf, READ_BUFFER_SIZE);
		wait(1);
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
