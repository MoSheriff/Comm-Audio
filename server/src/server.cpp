#include "server.h"

int main(int argc, char **argv) {
	initializeSonglist();
	nc.initialize();	
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)&SendSongListProc,0,0,0);
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)&IncomingMessageProc,0,0,0);
	while(true) {
		if(playlist.size() != 0) {
			openFile();
		}
	}
}

void MusicProc(void *ID) {
}

void SendSongListProc(void *ID) {
	string clientIP;
	while(true) {
		SOCKET clientSock = nc.waitForClient(clientIP);
		clientList.push_back(clientIP); // need to get the IP from the NetworkingComponent somehow
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
				case '2':
					break;
				case '3':
					skipVotes++;
					break;
				default:
					break;
			}
		}
	}
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
		if(skipVotes >= test) {
			skipVotes = 0;
			break;
		}
		ReadFile(wavFile, fileBuf, READ_BUFFER_SIZE, &bytesRead,0);
		if(bytesRead == 0) {
			break;
		}
		nc.sendMulticast(fileBuf, bytesRead);
		printf("Data Sent.");
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