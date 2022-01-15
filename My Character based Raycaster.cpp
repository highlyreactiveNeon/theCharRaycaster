//One thing to note here is that the coordinate system used in this Program is slightly reversed that is the Y-axis is reversed (going down is positive and going up is negative)

//to increase the fps of the game increase the fStepSizeOfTheRayCaster in Player Info Variables
//but decreseing the above decreases flickering by a much

#include <iostream>
#include <Windows.h>
#include <chrono>

using namespace std;

//defining constants
const float PI = 3.14159f;

//defining required variables
	//Screen Variables
int iScreenWidth = 120;						//Application Width
int iScreenHeight = 40;						//Application Height

//Map Variables
int iMapHeight = 17;						//Minimap Height in the Screen
int iMapWidth = 17;							//Minimap Width in the Screen
float fMapDepth = 15.0f;					//Limit for the Ray Caster

//Player Info Variable
float fPlayerXCoordi = 2.0f;				//Starting Player X Coordinate
float fPlayerYCoordi = 8.0f;				//Starting Player Y Coordinate
float fPlayerFocusViewRayAngle = 0.0f;		//I am measuring the angle from the positive x-axis
float fPlayerFOV = PI / 4;					//(Field of View) Measured in Radians
float fSpeed = 5.0f;						//Player Speed for translation and rotation
float fStepSizeOfTheRayCaster = 0.02f;		//Reduces this improves the textures a lil' bit
bool bIsSpaceAvailForMiniMapRay;
short cPlayerCharacter;

//Should not have been global variables
int iTestX;
int iTestY;

//function that takes any angle in radians as input and returns a integer angle that is between -179deg to 180deg
int RadianManyToOneDegree(float beginAngle) {
	int iBeginAngleInDegree;
	int iAfterAngleInDegree;

	if (beginAngle != 0) {
		iBeginAngleInDegree = (beginAngle / PI) * 180;
		iAfterAngleInDegree = iBeginAngleInDegree % 180;

		return iAfterAngleInDegree;
	}
	else
		return 0;
}

int main() {

	// Create Screen Buffer
	wchar_t* screen = new wchar_t[iScreenWidth * iScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//The Map - '#' is a wall, '.' is a empty space
	wstring map;
	map += L"#################";
	map += L"#..#............#";
	map += L"#..#............#";
	map += L"#..#...###......#";
	map += L"#..#............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#......##########";
	map += L"#......#........#";
	map += L"#......#........#";
	map += L"#......#........#";
	map += L"#...............#";
	map += L"#......##########";
	map += L"#...............#";
	map += L"#################";

	auto tpThen = chrono::system_clock::now();
	auto tpNow = chrono::system_clock::now();

	//The Game loop
	while (1) {

		//for the block detection system
		bool bIsFirstLoop = true;

		bIsSpaceAvailForMiniMapRay = true;

		tpNow = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tpNow - tpThen;
		tpThen = tpNow;
		float fElapsedTime = elapsedTime.count();

		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
			fPlayerFocusViewRayAngle -= (fSpeed * 0.5f) * fElapsedTime;

			if (fPlayerFocusViewRayAngle < -PI) {
				fPlayerFocusViewRayAngle = PI;
			}
		}

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			fPlayerFocusViewRayAngle += (fSpeed * 0.5f) * fElapsedTime;

			if (fPlayerFocusViewRayAngle > PI) {
				fPlayerFocusViewRayAngle = -PI;
			}
		}

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerXCoordi += ((fSpeed)*fElapsedTime) * cosf(fPlayerFocusViewRayAngle);
			fPlayerYCoordi += ((fSpeed)*fElapsedTime) * sinf(fPlayerFocusViewRayAngle);

			if (map[(int)fPlayerYCoordi * iMapWidth + (int)fPlayerXCoordi] == '#') {
				fPlayerXCoordi -= ((fSpeed)*fElapsedTime) * cosf(fPlayerFocusViewRayAngle);
				fPlayerYCoordi -= ((fSpeed)*fElapsedTime) * sinf(fPlayerFocusViewRayAngle);
			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerXCoordi -= ((fSpeed)*fElapsedTime) * cosf(fPlayerFocusViewRayAngle);
			fPlayerYCoordi -= ((fSpeed)*fElapsedTime) * sinf(fPlayerFocusViewRayAngle);

			if (map[(int)fPlayerYCoordi * iMapWidth + (int)fPlayerXCoordi] == '#') {
				fPlayerXCoordi += ((fSpeed)*fElapsedTime) * cosf(fPlayerFocusViewRayAngle);
				fPlayerYCoordi += ((fSpeed)*fElapsedTime) * sinf(fPlayerFocusViewRayAngle);
			}
		}

		//loop going through all the coloumns
		for (int iX = 0; iX < iScreenWidth; iX++) {
			float fCurrentRayAngle = (fPlayerFocusViewRayAngle - fPlayerFOV / 2) + (((float)iX / (float)iScreenWidth) * fPlayerFOV);			//This is a temporary ray for the calculation of distances from the player

			//Getting the unit vectors of the currentRayAngle
			float fUnitVecX = cosf(fCurrentRayAngle);
			float fUnitVecY = sinf(fCurrentRayAngle);

			//setting up some booleans
			bool bIsBoundary = false;
			bool bDidPlayerHitWall = false;

			//setting up some variables
			float fDistanceOfWallFromPlayer = 0.0f;

			while (!bIsBoundary && !bDidPlayerHitWall) {
				fDistanceOfWallFromPlayer += fStepSizeOfTheRayCaster;

				iTestX = (int)(fPlayerXCoordi + fUnitVecX * fDistanceOfWallFromPlayer);
				iTestY = (int)(fPlayerYCoordi + fUnitVecY * fDistanceOfWallFromPlayer);

				//checking if the ray is out of the bounds
				if (iTestX < 0 || iTestX >= iMapWidth || iTestY < 0 || iTestY >= iMapHeight) {
					bIsBoundary = true;
					fDistanceOfWallFromPlayer = fMapDepth;
				}
				else {
					if (map[iTestX + iTestY * iMapHeight] == '#') {
						bDidPlayerHitWall = true;
					}
				}
			}

			//block detecting system
			static int iPrevTestX;
			static int iPrevTextY;
			bool bIsGap = false;
			if (bIsFirstLoop) {
				bIsFirstLoop = false;
				iPrevTestX = iTestX;
				iPrevTextY = iTestY;
			}
			else {
				if (iPrevTestX != iTestX || iPrevTextY != iTestY) {
					bIsGap = true;
				}
			}
			iPrevTestX = iTestX;
			iPrevTextY = iTestY;


			//calculating the size of the ceiling and the floor for the current position
			int iCeilingHeight = (float)(iScreenHeight / 2.0) - iScreenHeight / (fDistanceOfWallFromPlayer);
			int iFloorHeight = iScreenHeight - iCeilingHeight;

			//shading walls based on the distanece from the player
			short sShade = ' ';
			if (bIsGap)													sShade = ' ';
			else if (fDistanceOfWallFromPlayer <= fMapDepth / 4.0f)		sShade = 0x2588;
			else if (fDistanceOfWallFromPlayer < fMapDepth / 2.5f)		sShade = 0x2593;
			else if (fDistanceOfWallFromPlayer < fMapDepth / 1.5f)		sShade = 0x2592;
			else if (fDistanceOfWallFromPlayer < fMapDepth)				sShade = 0x2591;
			else														sShade = ' ';

			//printing the 3D world on the screen
			for (int iY = 0; iY < iScreenHeight; iY++) {
				if (iY < iCeilingHeight) {
					screen[iX + iY * iScreenWidth] = ' ';
				}
				else if (iY >= iCeilingHeight && iY < iFloorHeight) {
					screen[iX + iY * iScreenWidth] = sShade;
				}
				else if (iY >= iFloorHeight) {
					//shading floor based on distance
					float b = 1.0f - (((float)iY - iScreenHeight / 2.0f) / ((float)iScreenHeight / 2.0f));
					if (b < 0.25)		sShade = 'x';
					else if (b < 0.5)	sShade = 'x';
					else if (b < 0.75)	sShade = '.';
					else if (b < 0.9)	sShade = '-';
					else				sShade = ' ';
					screen[iY * iScreenWidth + iX] = sShade;
				}
			}
		}

		//converting radians to degree scale
		int iPlayerCurrentAngle = RadianManyToOneDegree(fPlayerFocusViewRayAngle);

		//for printing stats on the top left side of the screen
		swprintf_s(screen, 45, L"Angle=%4ddeg X=%5.2f Y=%5.2f", -iPlayerCurrentAngle, fPlayerXCoordi, fPlayerYCoordi);

		//drawing minimap to the screen
		for (int iY = 0; iY < iMapHeight; iY++) {
			int iCounterX = 0;

			for (int iX = iScreenWidth - iMapWidth; iX < iScreenWidth; iX++) {
				screen[iY * iScreenWidth + iX] = map[iY * iMapWidth + iCounterX];

				iCounterX++;
			}
		}

		//drawing the ray of the player on the map based on the space available on the minimap
		if (fPlayerFocusViewRayAngle <= (PI / 8) && fPlayerFocusViewRayAngle > -(PI / 8)) {												//right
			if (screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi + 1)] != '#') {
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi + 1)] = '>';
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 'O';
			}
			else
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = '>';
		}
		else if (fPlayerFocusViewRayAngle <= 3 * (PI / 8) && fPlayerFocusViewRayAngle > (PI / 8)) {										//botton-right
			if (screen[(int)(fPlayerYCoordi + 1) * iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi + 1)] != '#') {
				screen[(int)(fPlayerYCoordi + 1) * iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi + 1)] = 0x2519;
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 'O';
			}
			else
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 0x2519;
		}
		else if (fPlayerFocusViewRayAngle <= 5 * (PI / 8) && fPlayerFocusViewRayAngle > 3 * (PI / 8)) {									//bottom
			if (screen[(int)(fPlayerYCoordi + 1) * iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] != '#') {
				screen[(int)(fPlayerYCoordi + 1) * iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 'v';
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 'O';
			}
			else
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 'v';
		}
		else if (fPlayerFocusViewRayAngle <= 7 * (PI / 8) && fPlayerFocusViewRayAngle > 5 * (PI / 8)) {									//bottom-left
			if (screen[(int)(fPlayerYCoordi + 1) * iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi - 1)] != '#') {
				screen[(int)(fPlayerYCoordi + 1) * iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi - 1)] = 0x2515;
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 'O';
			}
			else
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 0x2515;
		}
		else if (fPlayerFocusViewRayAngle <= PI && fPlayerFocusViewRayAngle > 7 * (PI / 8)) {											//left
			if (screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi - 1)] != '#') {
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi - 1)] = '<';
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 'O';
			}
			else
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 0x2515;
		}
		else if (fPlayerFocusViewRayAngle >= -PI && fPlayerFocusViewRayAngle <= -7 * (PI / 8)) {										//left
			if (screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi - 1)] != '#') {
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi - 1)] = '<';
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 'O';
			}
			else
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = '<';
		}
		else if (fPlayerFocusViewRayAngle > -7 * (PI / 8) && fPlayerFocusViewRayAngle <= -5 * (PI / 8)) {								//top-left
			if (screen[(int)(fPlayerYCoordi - 1) * iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi - 1)] != '#') {
				screen[(int)(fPlayerYCoordi - 1) * iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi - 1)] = 0x250D;
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 'O';
			}
			else
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 0x250D;
		}
		else if (fPlayerFocusViewRayAngle > -5 * (PI / 8) && fPlayerFocusViewRayAngle <= -3 * (PI / 8)) {								//top
			if (screen[(int)(fPlayerYCoordi - 1) * iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] != '#') {
				screen[(int)(fPlayerYCoordi - 1) * iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 0x02C4;
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 'O';
			}
			else
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 0x02C4;
		}
		else if (fPlayerFocusViewRayAngle > -3 * (PI / 8) && fPlayerFocusViewRayAngle <= -(PI / 8)) {									//top-right
			if (screen[(int)(fPlayerYCoordi - 1) * iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi + 1)] != '#') {
				screen[(int)(fPlayerYCoordi - 1) * iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi + 1)] = 0x2511;
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 'O';
			}
			else
				screen[(int)(fPlayerYCoordi)*iScreenWidth + ((iScreenWidth - iMapWidth) + (int)fPlayerXCoordi)] = 0x2511;
		}


		//To draw the whole frame on the screen
		screen[iScreenWidth * iScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, iScreenWidth * iScreenHeight, { 0,0 }, &dwBytesWritten);
	}
}
