/*-----------------------------------------------------------------------
--  SOURCE FILE:   Protocol.cpp   An application that is able to read from an RFID scanner
--
--  PROGRAM:       Wi-Fi transmitting and receiving protocol
--
--  FUNCTIONS:
--                  int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--							  		  LPSTR lspszCmdParam, int nCmdShow);
--                  LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
--					void Help(HWND hwnd);
--
--  DATE:          November 19, 2016
--
--  DESIGNER:      Robert Arendac,  Matt Goerwell, Aing Ragunathan
--
--  PROGRAMMER:    Robert Arendac,  Matt Goerwell, Aing Ragunathan
--
--  NOTES:
----------------------------------------------------------------------------*/

#define STRICT
#define _CRT_SECURE_NO_WARNINGS
#define _UNICODE
#define BUFFERSIZE MAX 
#define DATASIZE 1024

#include <windows.h>
#include <string>
#include <iostream>
#include <streambuf>
#include <vector>
#include <queue>
#include "MenuSettings.h"
#include "Transmit.h"

using namespace std;

//make the window pretty
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


//Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void connectToPort(HWND hwnd);
void help(HWND hwnd);
void ConfigurePort(HWND hwnd);						//for user to select new port
BOOL CALLBACK portProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
int genRand();
bool selectFileName();
void loadFileToBuffer(HANDLE hf);
void connect();
void generateIDLETimer();
//void generateRandTimer();
TIMERPROC generateRandTimer();

//Global variables
static TCHAR Name[] = TEXT("The Stall-in Protocol - Presented by the Bolsheviks");			//Name of main window
HANDLE readThread;									//Thread that reads data											
HWND hwnd;											//Handle to main window
HWND hstat;
int cmdShow = 0;									//flags for displaying main window
HINSTANCE mainInst;									//Instance of main window


static LPCSTR fileName = "COM1";					//Name of port 
static HANDLE port = INVALID_HANDLE_VALUE;			//Handle for port
static DCB dcbParams = { 0 };						//DCB structure

static char szItemName[2];							//used in dialog box for changing the port 




/*---------------------------------------------------------------------------------
--  FUNCTION:      WinMain
--
--  DATE:          November 16, 2016
--
--  DESIGNER:      Aing Ragunathan
--
--  PROGRAMMER:    Robert Arendac
--
--  INTERFACE:     int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--                                  	LPSTR lspszCmdParam, int nCmdShow)
--                        HINSTANCE hInst: Handle to current instance of application
--                        HINSTANCE hprevInstance:  Handle to previous instance
--                        LPSTR lspszCmdParam: Command line
--                        int nCmdShow: How window is shown
--
--  RETURNS:       WM_QUITS message value or 0 if no messages sent
--
--  NOTES:
--  Function used to create a window, register it, display it, and send messages.
--	Due to how win32 works, the "Program Start" state is WinMain and WinProc.  Also
--  registers the sub window.
-----------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;
	WNDCLASSEX Wcl;
	cmdShow = nCmdShow;
	mainInst = hInst;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = TEXT("MYMENU"); // The menu class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	//check if previous version of application is running
		//can cause problems with receiving data
		//implemented in the event driven example

	// Register the class
	if (!RegisterClassEx(&Wcl))
		return 0;

	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, -10, 0,
		750, 400, NULL, NULL, hInst, NULL);
	
	hstat = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, -10, 395,
		750, 200, NULL, NULL, hInst, NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, "Main window failed to open, program shutting down", "ERROR", MB_OK);
		return 0;
	}

	if (hstat== NULL) {
		MessageBox(NULL, "Main window failed to open, program shutting down", "ERROR", MB_OK);
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	ShowWindow(hstat, nCmdShow);
	UpdateWindow(hstat);



	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}


/*---------------------------------------------------------------------------------
--  FUNCTION:     WndProc
--
--  DATE:         November 16, 2016
--
--  DESIGNER:     Aing Ragunathan
--
--  PROGRAMMER:   Robert Arendac
--
--  INTERFACE:    LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
--	                                        WPARAM wParam, LPARAM lParam)
--                           HWND hwnd: A handle to the window
--                           UINT Message: Recieved message
--                           WPARAM wParam: Additional message information
--                           LPARAM lParam: Additional message information
--
--  RETURNS:      Result of message processing
--
--  NOTES:
--  This function is used to parse the menu selections and handle all system messages.
--  It also calls the function to connect to a RFID and creates a seperate
--  thread for reading from the RFID. Helper for start program state.
-----------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char str[100];

	switch (Message)
	{
	case WM_TIMER:
		switch (wParam) {
		case IDT_IDLE:
			OutputDebugString("timer finised\n");
			//KillTimer(hwnd, IDT_IDLE);
			generateRandTimer();
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))  //Parsing the menu selections
		{
		case IDM_HELPBOX:
			help(hwnd);
			break;
		case IDM_SELECTFILE:
			//open file browser
			if (selectFileName()) {
				OutputDebugString("file open successful\n");
			}
			else {
				OutputDebugString("failed to open file\n");
				return 0;
			}

			//destroy timers in connect
			KillTimer(hwnd, IDT_IDLE);
			OutputDebugString("timer killed\n");

			//start transferring selected file


			//start the timers in connect again
			sprintf(str, "random number: %d\n", rand());
			OutputDebugString(str);
			SetTimer(hwnd, IDT_IDLE, 10000, (TIMERPROC) NULL);


			break;
		case IDM_SELECTPORT:
			//allow user to select a port number using dialog box
			ConfigurePort(hwnd);

			//change the "filename" variable for the port to the one selected by the user
				//don't actually open the port, that's done in connect
			break;
		case IDM_DISCONNECT:
			if (readThread != NULL) {
				SuspendThread(readThread);
			}
			break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(hwnd, &ps);
			char *TextArray = "Hello";
			TextOut(hdc, 10, 10, TextArray, strlen(TextArray));
			EndPaint(hwnd, &ps);
		}
			break;
		case IDM_CONNECT:
			connect();
		}
		break;
	

	case WM_DESTROY:	// Terminate program
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	
	return 0;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:      connectToPort
--
--  DATE:          Oct 5, 2016
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  INTERFACE:     void connectToPort(HWND hwnd)
--                      HWND hwnd:  The handle of the main window
--
--  RETURNS:       void
--
--  NOTES:
--  Opens a serial port (nonoverlapped I/O).  User is notified if the port could
--  not be opened.  Otherwise it will start to configure the port settings.
-----------------------------------------------------------------------------------*/
void connectToPort(HWND hwnd) {
	port = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	
	if (port == INVALID_HANDLE_VALUE) {

		char s [100];
		sprintf(s, "port set to: %s", fileName);
		OutputDebugString(s);
		
		MessageBox(hwnd, TEXT("Failed to open port"), TEXT("ERROR"), MB_OK);

		//return;
	}
}

void ConfigurePort(HWND hwnd) {
	int ret = DialogBox(GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_PORTGET), hwnd, portProc);
	
	if (ret == IDOK) {
		MessageBox(hwnd, szItemName, "Notice",
			MB_OK | MB_ICONINFORMATION);
		
		fileName = (LPCSTR)szItemName;

		
	}
	else if (ret == -1) {
		MessageBox(hwnd, "Dialog failed!", "Error",
			MB_OK | MB_ICONINFORMATION);
	}
}

BOOL CALLBACK portProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch (Message)
	{
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			if (!GetDlgItemText(hwnd, IDT_INPUTBOX1, szItemName, 5))
				*szItemName = 0;
		case IDCANCEL:
			EndDialog(hwnd, wParam);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:     Help
--
--  DATE:         November 16, 2016
--
--  DESIGNER:     Robert Arendac
--
--  PROGRAMMER:   Matt Goerwell
--
--  INTERFACE:    void Help(HWND hwnd)
--                           HWND hwnd:  Window handle to generate message box
--
--  RETURNS:      Void
--
--  NOTES:
--  This function will display the help box.
-----------------------------------------------------------------------------------*/
void help(HWND hwnd) {
	MessageBox(hwnd, "<place help here>", TEXT("Help"), MB_OK);
}

int genRand() {
	int randNum = (double)rand() / (RAND_MAX + 1) * (101 - 1)+1;
	
	return randNum;
}

bool selectFileName() {
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name
	HANDLE hf;              // file handle

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 

	if (GetOpenFileName(&ofn) == TRUE)
		hf = CreateFile(ofn.lpstrFile,
			GENERIC_READ,
			0,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL);

	loadFileToBuffer(hf);

	return &ofn;
}


void loadFileToBuffer(HANDLE hf) {
	char   packet[DATASIZE];
	//char* packet;
	string temp;
	OVERLAPPED ol = { 0 };
	//DWORD g_BytesTransferred = 0;
	DWORD dwBytesRead = 0;

	vector<string> test;
	string testStr = "";
	//packet = (char*)malloc(DATASIZE);	//to avoid getting garbage values at the end
	

	if (hf == INVALID_HANDLE_VALUE) {
		OutputDebugString("CreateFile: error opening file");
		return;
	}

	//append a packet of DC1 characters

	transmitBuffer.clear();
	/*
	for (size_t i = 0; i < DATASIZE; i++) {
		packet[i] = char(17);
	}
	transmitBuffer.push_back(packet);
	*/
	do {
		for (size_t i = 0; i < DATASIZE; i++) {
			packet[i] = '\0';
		}


		//DATASIZE - 1 if there is a null character?
		//if (!ReadFileEx(hf, packet, DATASIZE, &ol, NULL)) {
		if (!ReadFile(hf, packet, DATASIZE, &dwBytesRead, &ol)) {
			OutputDebugString("READFILE: failed to read file");
		}
		ol.Offset += dwBytesRead;

		temp = packet;
		temp.resize(1024);

		transmitBuffer.push_back(temp);
	} while (strlen(packet) >= DATASIZE);


	//dwBytesRead = g_BytesTransferred;
	for (auto x : transmitBuffer) {
		cout << x;
		//cout << transmitBuffer.size();
	}

	//transmitBuffer.push(packet);

	
	//loop through file, append 1024 byte segments to the buffer as a new packet
	
	//update status of buffer
	hasSendData = true;
}

void generateIDLETimer() {
	SetTimer(hwnd, IDT_IDLE, 500, (TIMERPROC)NULL);
	//SetTimer(hwnd, IDT_IDLE, 500, generateRandTimer());
}

TIMERPROC generateRandTimer() {
	//if (!hasSendData) {
		SetTimer(hwnd, IDT_RAND, genRand(), requestingSend(port, hwnd));
		OutputDebugString("Starting rand timer\n");
	//}	

	return NULL;
}


void connect() {
	if (readThread == NULL) {
		DWORD id;

		//open the port that the user selected in SELECT PORT
		//if no port was selected, default to 0
		//check if port was opened properly
		//inform the user
		connectToPort(hwnd);


		//initialize rand timeout
			//check for data in transmitBuffer	
			//if we have data
				//generate a random timer
			//return handle to the timer;
		
		//generateRandTimer();


		//TEST CASE
		//create Idle Sequence Timer
		generateIDLETimer();
		
		//set the dcb length
		//dcbParams.DCBlength = sizeof(dcbParams);
		//SetCommState(port, &dcbParams);

		//start the reading threadsequenceRandTime
		//readThread = CreateThread(NULL, 0, findReader, NULL, 0, &id);
		//wait for com event
		
		
	}
	else {
		ResumeThread(readThread);
	}
}