#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
using namespace std;

#include <stdio.h>
#include <Windows.h>


int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0; // 1/4th of a Semicircle
float fDepth = 16.0f;

int main()
{
    // Create Screen Buffer
    wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    wstring map;
    map += L"################";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#.....##########";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#.....##########";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";

    chrono::time_point<chrono::system_clock> tp1 = chrono::system_clock::now();
    chrono::time_point<chrono::system_clock> tp2;

    // Game loop
    while (1)
    {   
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();



        // Controls 
        // Handle CCW Rotation
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (1.5f) * fElapsedTime;
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (1.5f) * fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }



        }
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }


        }


        // Iterate through each column of pixels on the screen
        for (int x = 0; x < nScreenWidth; x++)
        {   
            // Iterate per pixel
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV; 
            
            /*****************************************
            * Increment each ray until slams into wall
            ******************************************/

            // Increment each ray until slams into wall
            float fDistanceToWall = 0; 
            bool bHitWall = false;
            bool bBoundary = false;
            // Calculate test point: first need a unit vector
            float fEyeX = sinf(fRayAngle);
            float fEyeY = cosf(fRayAngle);

            // // Calculate which cell the test block lands on 
            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += 0.1f;
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);
                
                // Test if ray is out of bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                }
                else
                {
                    // Ray is in bounds so test to see if the ray cell is a wall block
                    if (map[nTestY * nMapWidth + nTestX] == '#')
                    {
                        bHitWall = true;
                        vector<pair<float, float>> p; // distance, dot product
                        for (int tx = 0; tx < 2; tx++)
                            for (int ty = 0; ty < 2; ty++)
                            {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }
                        // Sort pairs from closest to farthest
                        sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first;});

                        float fBound = 0.003;
                        if (acos(p.at(0).second) < fBound) bBoundary = true;
                        if (acos(p.at(1).second) < fBound) bBoundary = true;
                        //if (acos(p.at(2).second) < fBound) bBoundary = true;
                        


                    }
                }
            }

            // Calculate distance to ceiling and floor
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling; // Mirrors the ceiling


            short nShade;
            if (fDistanceToWall <= fDepth / 4.0f)          nShade = 0x2588; // Very closes, fully white
            else if (fDistanceToWall < fDepth / 3.0f)      nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)      nShade = 0x2592;
            else if (fDistanceToWall < fDepth)             nShade = 0x2591;
            else                                           nShade = ' '; // Very far, black
            if (bBoundary)  nShade = ' '; //Black it out 

            for (int y = 0; y < nScreenHeight; y++)
            {
                if (y < nCeiling)
                    screen[y * nScreenWidth + x] = ' ';
                else if (y > nCeiling && y <= nFloor )
                    screen[y * nScreenWidth + x] = nShade;
                else
                {
                    // Shade floor based on distance
                    short nShadeFloor;
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25)                                nShadeFloor = '#';
                    else if (b < 0.5)                            nShadeFloor = 'x';
                    else if (b < 0.75)                           nShadeFloor = '.';
                    else if (b < 0.9)                            nShadeFloor = '-';
                    else                                         nShadeFloor = ' ';
                    screen[y * nScreenWidth + x] = nShadeFloor;
                }  
            }
        }

        // Display Stats

        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);
        
        // Display Map

        for (int nx = 0; nx < nMapWidth; nx++)
            for (int ny = 0; ny < nMapWidth; ny++)
            {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
            }

        screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'o';

        // Write to the screen
        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(
            hConsole, // the handle
            screen, // the buffer
            nScreenWidth * nScreenHeight, // tells how many bytes
            { 0,0 }, // Coordinates where text has to be written
            &dwBytesWritten); // useless tracker 
    }


    return 0;
}

