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
#define songFourLocation string("Golden Slumbers.wav")
#define songFiveLocation string("Step Right Up.wav")
#define songSixLocation string("Weasel Stomping Day.wav")
#define READ_BUFFER_SIZE 1024
#define NUM_OF_SONGS 3

//ifstream wavFile;
char* fileName;
char* songTitles;
char fileBuf[READ_BUFFER_SIZE];
int fileSize;
int numFileChunks;
int skipVotes;
WSABUF recieveBuffer;
HANDLE wavFile;
HANDLE hPlaylistMutex = CreateMutex(NULL,FALSE,NULL);
HANDLE hSendMusic = CreateMutex(NULL,FALSE,NULL);
map<string,string> songList;
list<string> playlist;
list<string> clientList;
list<string>::iterator it;
string songOne = "Shine On You Crazy Diamond";
string songTwo = "Welcome To The Machine";
string songThree = "Wish You Were Here";
string songFour = "Golden Slumbers";
string songFive = "Step Right Up";
string songSix = "Weasel Stomping Day";
string songOneLoc = songOneLocation;
string songTwoLoc = songTwoLocation;
string songThreeLoc = songThreeLocation;
string songFourLoc = songFourLocation;
string songFiveLoc = songFiveLocation;
string songSixLoc = songSixLocation;
NetworkingComponent nc(NetworkingComponent::SERVER);

void getFileSize(char* fileName);
void initializeSonglist();
void openFile();
void sendDataToClients();
char* stringToCharStar(string temp, int flag);
void SendSongListProc (void *ID);
void MusicProc (void *ID);
void IncomingMessageProc(void *ID);
