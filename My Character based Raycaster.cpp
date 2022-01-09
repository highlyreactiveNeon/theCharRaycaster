//One thing to note here is that the coordinate system used in this Program is slightly reversed that is the Y-axis is reversed (going down is positive and going up is negative)

#include <iostream>
#include <Windows.h>

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
float fPlayerXCoordi = 1.0f;				//Starting Player X Coordinate
float fPlayerYCoordi = 1.0f;				//Starting Player Y Coordinate
float fPlayerFocusViewRayAngle = 0.0f;		//I am measuring the angle from the positive x-axis
float fPlayerFOV = PI / 4;					//(Field of View) Measured in Radians
float fSpeed = 5.0f;						//Player Speed for translation and rotation
float fSpeedMultiplier = 0.00060f;			//IDK why I used this extra thing
float fStepSizeOfTheRayCaster = 0.1f;		//Reduces this improves the textures a lil' bit


int main() {
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[iScreenWidth * iScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//The Map - '#' is a wall, '.' is a empty space
	wstring map;
	map += L"#################";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#.......#.......#";
	map += L"#.......#.......#";
	map += L"#.......#.......#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#################";

	//The Game loop
	while (1) {

		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerFocusViewRayAngle -= (fSpeed * 0.5f) * fSpeedMultiplier;

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerFocusViewRayAngle += (fSpeed * 0.5f) * fSpeedMultiplier;

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerXCoordi += ((fSpeed)*fSpeedMultiplier) * cosf(fPlayerFocusViewRayAngle);
			fPlayerYCoordi += ((fSpeed)*fSpeedMultiplier) * sinf(fPlayerFocusViewRayAngle);
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerXCoordi -= ((fSpeed)*fSpeedMultiplier) * cosf(fPlayerFocusViewRayAngle);
			fPlayerYCoordi -= ((fSpeed)*fSpeedMultiplier) * sinf(fPlayerFocusViewRayAngle);
		}


		//loop going through all the coloumns
		for (int iX = 0; iX < iScreenWidth; iX++) {

			bool bFirstHorizontalLoop = true;
			bool bCheckRateOfChangeOfDist = true;

			static float fPreviousDistToWall = NULL;

			static int iRateOfChangeOfDist = NULL;

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

				int iTestX = (int)(fPlayerXCoordi + fUnitVecX * fDistanceOfWallFromPlayer);
				int iTestY = (int)(fPlayerYCoordi + fUnitVecY * fDistanceOfWallFromPlayer);

				//checking if the ray is out of the bounds
				if (iTestX < 0 || iTestX >= iScreenWidth || iTestY < 0 || iTestY >= iScreenHeight) {
					bIsBoundary = true;
					fDistanceOfWallFromPlayer = fMapDepth;
				}
				else {
					if (map[iTestX + iTestY * iMapHeight] == '#') {
						bDidPlayerHitWall = true;

					}
				}

			}

			if (!bFirstHorizontalLoop) {
				if(bCheckRateOfChangeOfDist){
					if (fPreviousDistToWall < fDistanceOfWallFromPlayer)
						iRateOfChangeOfDist = 1;
					else if (fPreviousDistToWall == fDistanceOfWallFromPlayer)
						iRateOfChangeOfDist = 0;
					else
						iRateOfChangeOfDist = -1;

					bCheckRateOfChangeOfDist = false;
				}
			}

			//calculating the size of the ceiling and the floor for the current position
			int iCeilingHeight = (float)(iScreenHeight / 2.0) - iScreenHeight / (fDistanceOfWallFromPlayer);
			int iFloorHeight = iScreenHeight - iCeilingHeight;

			//shading walls based on the distanece from the player
			short sShade = ' ';
			if (fDistanceOfWallFromPlayer <= fMapDepth / 4.0f)			sShade = 0x2588;
			else if (fDistanceOfWallFromPlayer < fMapDepth / 2.5f)		sShade = 0x2593;
			else if (fDistanceOfWallFromPlayer < fMapDepth / 1.5f)		sShade = 0x2592;
			else if (fDistanceOfWallFromPlayer < fMapDepth)				sShade = 0x2591;
			else														sShade = ' ';

			//printing the game
			for (int iY = 0; iY < iScreenHeight; iY++) {
				if (iY < iCeilingHeight) {
					screen[iX + iY * iScreenWidth] = ' ';
				}
				else if (iY >= iCeilingHeight && iY < iFloorHeight) {
					screen[iX + iY * iScreenWidth] = sShade;
				}
				else if (iY >= iFloorHeight) {
					// Shade floor based on distance
					float b = 1.0f - (((float)iY - iScreenHeight / 2.0f) / ((float)iScreenHeight / 2.0f));
					if (b < 0.25)		sShade = '#';
					else if (b < 0.5)	sShade = 'x';
					else if (b < 0.75)	sShade = '.';
					else if (b < 0.9)	sShade = '-';
					else				sShade = ' ';
					screen[iY * iScreenWidth + iX] = sShade;
				}
			}

			fPreviousDistToWall = fDistanceOfWallFromPlayer;
			bFirstHorizontalLoop = false;
		}

		swprintf_s(screen, 30, L"Angle=%3.2f X=%3.2f Y=%3.2f", fPlayerFocusViewRayAngle, fPlayerXCoordi, fPlayerYCoordi);

		// Display Frame
		screen[iScreenWidth * iScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, iScreenWidth * iScreenHeight, { 0,0 }, &dwBytesWritten);
	}
}
