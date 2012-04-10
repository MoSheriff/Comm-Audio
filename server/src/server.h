#include <iostream>
#include <fstream>
#include <stdio.h>
#include <winsock2.h>
#include <map>
#include <string>
#include <list>
#include <process.h>
#include "NetworkingComponent.h"
using namespace std;

#define songOneLocation string("Shine On You Crazy Diamond.wav")
#define songTwoLocation string("Welcome To The Machine.wav")
#define songThreeLocation string("Wish You Were Here.wav")
#define READ_BUFFER_SIZE 1024
#define NUM_OF_SONGS 3

//ifstream wavFile;
char* fileName;
char* songTitles;
char fileBuf[READ_BUFFER_SIZE];
int fileSize;
int numFileChunks;
WSABUF *recieveBuffer;
HANDLE wavFile;
HANDLE hPlaylistMutex = CreateMutex(NULL,FALSE,NULL);
HANDLE hSendMusic = CreateMutex(NULL,FALSE,NULL);
map<string,string> songList;
string songOne = "Shine On You Crazy Diamond";
string songTwo = "Welcome To The Machine";
string songThree = "Wish You Were Here";
string songOneLoc = songOneLocation;
string songTwoLoc = songTwoLocation;
string songThreeLoc = songThreeLocation;
list<string> playlist;
NetworkingComponent nc(NetworkingComponent::SERVER);

void getFileSize(char* fileName);
void initializeSonglist();
void openFile();
void sendDataToClients();
char* stringToCharStar(string temp, int flag);
void NetworkProc (void *ID);
void MusicProc (void *ID);
void UDPInputProc(void *ID);
