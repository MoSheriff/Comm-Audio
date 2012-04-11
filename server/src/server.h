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
#define songSevenLocation string("All Apologies.wav");
#define songEightLocation string("All For Swinging You Around.wav");
#define songNineLocation string("I'm Tweeked _ Attack Of The 20lb.wav");
#define songTenLocation string("In Your Eyes.wav");
#define songElevenLocation string("Mass Romantic.wav");
#define songTwelveLocation string("Mercy Street.wav");
#define songThirteenLocation string("One Love One People Get Ready.wav");
#define songFourteenLocation string("Resolution.wav");
#define songFifteenLocation string("Soul Vaccination.wav");
#define songSixteenLocation string("The Price.wav");
#define songSeventeenLocation string("Things Ain't What They Used To Be.wav");
#define songEighteenLocation string("Tom Traubert's Blues (Four Sheets.wav");
#define songNineteenLocation string("What Is Hip.wav");
#define songTwentyLocation string("Wildflowers.wav");
#define READ_BUFFER_SIZE 1024
#define NUM_OF_SONGS 20

//ifstream wavFile;
char* fileName;
char* downloadFileName;
char songTitles[500];
char fileBuf[READ_BUFFER_SIZE];
int fileSize;
int numFileChunks;
double skipVotes = 0;
WSABUF recieveBuffer;
HANDLE wavFile;
HANDLE sendFile;
HANDLE hPlaylistMutex = CreateMutex(NULL,FALSE,NULL);
HANDLE hSendMusic = CreateMutex(NULL,FALSE,NULL);
map<string,string> songList;
list<string> playlist;
list<string> clientList;
list<SOCKET> clientSockList;
list<string>::iterator it;

string songOne       = "Shine On You Crazy Diamond";
string songTwo       = "Welcome To The Machine";
string songThree     = "Wish You Were Here";
string songFour      = "Golden Slumbers";
string songFive      = "Step Right Up";
string songSix       = "Weasel Stomping Day";
string songSeven     = "All Apologies";
string songEight     = "All For Swinging You Around";
string songNine      = "I'm Tweeked _ Attack Of The 20lb";
string songTen       = "In Your Eyes";
string songEleven    = "Mass Romantic";
string songTwelve    = "Mercy Street";
string songThirteen  = "One Love People Get Ready";
string songFourteen  = "Resolution";
string songFifteen   = "Soul Vaccination";
string songSixteen  = "The Price";
string songSeventeen = "Things Ain't What They Used to Be";
string songEighteen  = "Tom Traubert's Blues (Four Sheets";
string songNineteen  = "What Is Hip";
string songTwenty    = "Wildflowers";

string songOneLoc       = songOneLocation;
string songTwoLoc       = songTwoLocation;
string songThreeLoc     = songThreeLocation;
string songFourLoc      = songFourLocation;
string songFiveLoc      = songFiveLocation;
string songSixLoc       = songSixLocation;
string songSevenLoc     = songSevenLocation;
string songEightLoc     = songEightLocation;
string songNineLoc      = songNineLocation;
string songTenLoc       = songTenLocation;
string songElevenLoc    = songElevenLocation;
string songTwelveLoc    = songTwelveLocation;
string songThirteenLoc  = songThirteenLocation;
string songFourteenLoc  = songFourteenLocation;
string songFifteenLoc   = songFifteenLocation;
string songSixteenLoc  = songSixteenLocation;
string songSeventeenLoc = songSeventeenLocation;
string songEighteenLoc  = songEighteenLocation;
string songNineteenLoc  = songNineteenLocation;
string songTwentyLoc    = songTwentyLocation;

NetworkingComponent nc(NetworkingComponent::SERVER);

void getFileSize(char* fileName);
void initializeSonglist();
void openFile();
void sendDataToClients();
char* stringToCharStar(string temp, int flag);
void IncomingConnectionProc(void *ID);