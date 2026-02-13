/*********************************************************************
 * \file   main.cpp
 * \brief  コントローラー入力デバッグ用（SDL3完全版）
 *********************************************************************/
#include <cstdio>
#include <cstring>
#include <conio.h>
#include <windows.h>
#include "game_controller.h"

void ClearScreen() {
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void PrintLine(const char* pStr) {
    printf("%-79s\n", pStr);
}

void GetStickBar(char* pBuf, float value) {
    int pos = static_cast<int>((value + 1.0f) * 6.0f);
    if (pos < 0) pos = 0;
    if (pos > 12) pos = 12;
    pBuf[0] = '[';
    for (int i = 0; i < 13; i++) {
        if (i == 6) pBuf[i + 1] = '|';
        else if (i == pos) pBuf[i + 1] = '*';
        else pBuf[i + 1] = '-';
    }
    pBuf[14] = ']';
    pBuf[15] = '\0';
}

void GetTriggerBar(char* pBuf, float value) {
    int filled = static_cast<int>(value * 10);
    pBuf[0] = '[';
    for (int i = 0; i < 10; i++) {
        pBuf[i + 1] = (i < filled) ? '=' : ' ';
    }
    pBuf[11] = ']';
    pBuf[12] = '\0';
}

void AppendEvent(char* pEvent, size_t size, const char* pText) {
    if (strlen(pEvent) + strlen(pText) < size - 1) {
        strcat_s(pEvent, size, pText);
    }
}

const char* GetTypeString(ControllerType type) {
    switch (type) {
    case ControllerType::Xbox360:                 return "Xbox360";
    case ControllerType::XboxOne:                 return "XboxOne";
    case ControllerType::PS4:                     return "PS4";
    case ControllerType::PS5:                     return "PS5";
    case ControllerType::NintendoSwitch:          return "Switch";
    case ControllerType::NintendoSwitchJoyconLeft:  return "JoyL";
    case ControllerType::NintendoSwitchJoyconRight: return "JoyR";
    case ControllerType::NintendoSwitchJoyconPair:  return "JoyPair";
    default:                                      return "Other";
    }
}

int main() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    SMALL_RECT rect = { 0, 0, 79, 24 };
    SetConsoleWindowInfo(hConsole, TRUE, &rect);
    COORD bufSize = { 80, 25 };
    SetConsoleScreenBufferSize(hConsole, bufSize);

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    GameController::Initialize();
    GameController::EnableGyro(true);
    GameController::EnableAccelerometer(true);

    char line[128];
    char barLX[16], barLY[16], barRX[16], barRY[16];
    char barLT[16], barRT[16];
    bool isRunning = true;

    while (isRunning) {
        if (_kbhit()) {
            int key = _getch();
            switch (key) {
            case 27: isRunning = false; break;
            case 'v': case 'V': GameController::StartVibration(1.0f, 0.5f); break;
            case 'b': case 'B': GameController::StartVibration(0.3f, 0.3f); break;
            case 't': case 'T': GameController::StartTriggerVibration(0.5f, 0.5f, 0.3f); break;
            case 'r': case 'R': GameController::SetLED(255, 0, 0); break;
            case 'g': case 'G': GameController::SetLED(0, 255, 0); break;
            case 'l': case 'L': GameController::SetLED(0, 0, 255); break;
            case 'w': case 'W': GameController::SetLED(255, 255, 255); break;
            }
        }

        GameController::Update();
        ClearScreen();

        if (!GameController::IsConnected()) {
            PrintLine("===============================================================================");
            PrintLine("                    SDL3 CONTROLLER DEBUG MONITOR                              ");
            PrintLine("===============================================================================");
            PrintLine("");
            PrintLine(" Controller not connected...");
            PrintLine("");
            PrintLine(" Supported: Xbox, PlayStation, Switch Pro, Joy-Con, etc.");
            PrintLine("");
            for (int i = 0; i < 13; i++) PrintLine("");
            PrintLine("-------------------------------------------------------------------------------");
            PrintLine(" ESC: Exit");
            Sleep(100);
            continue;
        }

        const GamepadState& state = GameController::GetCurrentState();
        BatteryInfo battery = GameController::GetBatteryInfo();
        ControllerType ctrlType = GameController::GetControllerType();

        GetStickBar(barLX, state.leftStickX);
        GetStickBar(barLY, state.leftStickY);
        GetStickBar(barRX, state.rightStickX);
        GetStickBar(barRY, state.rightStickY);
        GetTriggerBar(barLT, state.leftTrigger);
        GetTriggerBar(barRT, state.rightTrigger);

        const char* pDU = state.dpadUp ? "[U]" : " U ";
        const char* pDD = state.dpadDown ? "[D]" : " D ";
        const char* pDL = state.dpadLeft ? "[L]" : " L ";
        const char* pDR = state.dpadRight ? "[R]" : " R ";

        const char* pBU = state.buttonUp ? "[^]" : " ^ ";
        const char* pBD = state.buttonDown ? "[v]" : " v ";
        const char* pBL = state.buttonLeft ? "[<]" : " < ";
        const char* pBR = state.buttonRight ? "[>]" : " > ";

        const char* pL1 = state.buttonL1 ? "[L1]" : " L1 ";
        const char* pR1 = state.buttonR1 ? "[R1]" : " R1 ";
        const char* pL2 = state.buttonL2 ? "[L2]" : " L2 ";
        const char* pR2 = state.buttonR2 ? "[R2]" : " R2 ";
        const char* pL3 = state.buttonL3 ? "[L3]" : " L3 ";
        const char* pR3 = state.buttonR3 ? "[R3]" : " R3 ";

        const char* pSel = state.buttonSelect ? "[SEL]" : " SEL ";
        const char* pSta = state.buttonStart ? "[STA]" : " STA ";
        const char* pGui = state.buttonGuide ? "[GUI]" : " GUI ";

        const char* pVibe = GameController::IsVibrating() ? "[VIBE]" : "      ";

        PrintLine("===============================================================================");
        PrintLine("                    SDL3 CONTROLLER DEBUG MONITOR                              ");
        PrintLine("===============================================================================");

        char batStr[16];
        if (battery.percent >= 0) sprintf_s(batStr, sizeof(batStr), "%d%%", battery.percent);
        else sprintf_s(batStr, sizeof(batStr), "%s", battery.levelText);

        sprintf_s(line, sizeof(line), " %-28s [%-7s] Bat:%-8s %s",
            GameController::GetControllerName(), GetTypeString(ctrlType), batStr, pVibe);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        sprintf_s(line, sizeof(line), " L Stick | X:%6.2f %s   Y:%6.2f %s",
            state.leftStickX, barLX, state.leftStickY, barLY);
        PrintLine(line);

        sprintf_s(line, sizeof(line), " R Stick | X:%6.2f %s   Y:%6.2f %s",
            state.rightStickX, barRX, state.rightStickY, barRY);
        PrintLine(line);

        sprintf_s(line, sizeof(line), " Trigger | L2:%5.2f %s    R2:%5.2f %s",
            state.leftTrigger, barLT, state.rightTrigger, barRT);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        sprintf_s(line, sizeof(line), "  D-PAD        %s                 MAIN             %s",
            pDU, pBU);
        PrintLine(line);

        sprintf_s(line, sizeof(line), "            %s   %s                            %s   %s",
            pDL, pDR, pBL, pBR);
        PrintLine(line);

        sprintf_s(line, sizeof(line), "               %s                                 %s",
            pDD, pBD);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        sprintf_s(line, sizeof(line), " Shoulder: %s %s                                     %s %s",
            pL1, pL2, pR2, pR1);
        PrintLine(line);

        sprintf_s(line, sizeof(line), " Stick   : %s                                             %s",
            pL3, pR3);
        PrintLine(line);

        sprintf_s(line, sizeof(line), " System  : %s  %s                                   %s",
            pSel, pGui, pSta);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        // 拡張機能表示
        char ext[80] = " Ext:";
        if (GameController::HasLED())   strcat_s(ext, sizeof(ext), " LED");
        if (GameController::HasGyro())  strcat_s(ext, sizeof(ext), " Gyro");
        if (GameController::HasAccelerometer()) strcat_s(ext, sizeof(ext), " Accel");
        if (GameController::HasTouchpad()) strcat_s(ext, sizeof(ext), " Touch");
        PrintLine(ext);

        // センサー表示
        if (GameController::HasGyro() || GameController::HasAccelerometer()) {
            SensorData s = GameController::GetSensorData();
            sprintf_s(line, sizeof(line), " Gyro:%5.1f %5.1f %5.1f  Accel:%5.1f %5.1f %5.1f",
                s.gyroX, s.gyroY, s.gyroZ, s.accelX, s.accelY, s.accelZ);
        } else {
            sprintf_s(line, sizeof(line), " Sensor: N/A");
        }
        PrintLine(line);

        // タッチパッド表示
        if (GameController::HasTouchpad()) {
            TouchpadData t = GameController::GetTouchpadData();
            sprintf_s(line, sizeof(line), " Touch: [%c]%.2f,%.2f  [%c]%.2f,%.2f",
                t.fingers[0].down ? '*' : ' ', t.fingers[0].x, t.fingers[0].y,
                t.fingers[1].down ? '*' : ' ', t.fingers[1].x, t.fingers[1].y);
        } else {
            sprintf_s(line, sizeof(line), " Touch: N/A");
        }
        PrintLine(line);

        PrintLine("===============================================================================");
        PrintLine(" ESC:Exit V/B:Vibe T:Trigger R/G/L/W:LED(Red/Green/bLue/White)");

        Sleep(16);
    }

    GameController::Finalize();

    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    return 0;
}
