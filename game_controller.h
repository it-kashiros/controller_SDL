/*********************************************************************
 * \file   game_controller.h
 * \brief  ゲームコントローラー入力管理（SDL3完全版）
 *********************************************************************/
#pragma once
#include <SDL3/SDL.h>
#include <cmath>
#include <cstdint>

//==============================================================================
// ゲームパッド状態構造体
//==============================================================================
struct GamepadState {
    // 左スティック（-1.0 ~ 1.0）
    float leftStickX = 0.0f;
    float leftStickY = 0.0f;

    // 右スティック（-1.0 ~ 1.0）
    float rightStickX = 0.0f;
    float rightStickY = 0.0f;

    // トリガー（0.0 ~ 1.0）
    float leftTrigger = 0.0f;
    float rightTrigger = 0.0f;

    // 十字キー
    bool dpadUp = false;
    bool dpadDown = false;
    bool dpadLeft = false;
    bool dpadRight = false;

    // メインボタン（位置ベース）
    bool buttonDown = false;   // A / × / B(Switch)
    bool buttonRight = false;  // B / ○ / A(Switch)
    bool buttonLeft = false;   // X / □ / Y(Switch)
    bool buttonUp = false;     // Y / △ / X(Switch)

    // ショルダーボタン（LB/RB / L1/R1 / L/R）
    bool buttonL1 = false;
    bool buttonR1 = false;

    // トリガーボタン（LT/RT / L2/R2 / ZL/ZR）- デジタル判定
    bool buttonL2 = false;
    bool buttonR2 = false;

    // スティック押し込み（LS/RS / L3/R3）
    bool buttonL3 = false;
    bool buttonR3 = false;

    // システムボタン
    bool buttonStart = false;   // Start / Options / +
    bool buttonSelect = false;  // Back / Share / -

    // 追加ボタン
    bool buttonGuide = false;   // Xboxボタン / PSボタン / Homeボタン
    bool buttonMisc = false;    // Share / Mic / Capture

    // 接続状態
    bool connected = false;

    // いずれかのボタンが押されているか
    bool IsAnyButtonPressed() const {
        return buttonDown || buttonRight || buttonLeft || buttonUp ||
            buttonL1 || buttonR1 || buttonL2 || buttonR2 ||
            buttonL3 || buttonR3 || buttonStart || buttonSelect ||
            buttonGuide || buttonMisc ||
            dpadUp || dpadDown || dpadLeft || dpadRight;
    }

    // デッドゾーン適用
    static float ApplyDeadzone(float value, float deadzone = 0.15f) {
        if (std::fabs(value) < deadzone) return 0.0f;
        float sign = (value > 0) ? 1.0f : -1.0f;
        return sign * (std::fabs(value) - deadzone) / (1.0f - deadzone);
    }
};

//==============================================================================
// バイブレーション設定構造体
//==============================================================================
struct VibrationSettings {
    float leftMotor = 0.0f;
    float rightMotor = 0.0f;
    float duration = 0.0f;
};

//==============================================================================
// バッテリー情報構造体
//==============================================================================
struct BatteryInfo {
    bool isWired = false;
    bool hasBatteryInfo = false;
    int percent = -1;
    const char* levelText = "";
};

//==============================================================================
// センサーデータ構造体
//==============================================================================
struct SensorData {
    float gyroX = 0.0f;
    float gyroY = 0.0f;
    float gyroZ = 0.0f;

    float accelX = 0.0f;
    float accelY = 0.0f;
    float accelZ = 0.0f;

    bool hasGyro = false;
    bool hasAccel = false;
};

//==============================================================================
// タッチパッドデータ構造体
//==============================================================================
struct TouchpadData {
    bool hasTouchpad = false;
    int numTouchpads = 0;

    struct Finger {
        bool down = false;
        float x = 0.0f;
        float y = 0.0f;
    };
    Finger fingers[2];
};

//==============================================================================
// コントローラータイプ列挙型
//==============================================================================
enum class ControllerType {
    Unknown,
    Xbox360,
    XboxOne,
    PS4,
    PS5,
    NintendoSwitch,
    NintendoSwitchJoyconLeft,
    NintendoSwitchJoyconRight,
    NintendoSwitchJoyconPair,
    Other
};

//==============================================================================
// ゲームコントローラークラス（SDL3完全版）
//==============================================================================
class GameController {
public:
    // 初期化・終了・更新
    static bool Initialize();
    static void Finalize();
    static void Update();

    // 状態取得
    static const GamepadState& GetCurrentState() { return s_currentState; }
    static const GamepadState& GetPrevState() { return s_prevState; }
    static const char* GetControllerName();
    static ControllerType GetControllerType();
    static int GetPlayerIndex();

    // バイブレーション制御
    static void StartVibration(float intensity, float duration);
    static void StartVibrationEx(float leftMotor, float rightMotor, float duration);
    static void StartVibrationEx(const VibrationSettings& settings);
    static void StartTriggerVibration(float left, float right, float duration);
    static void StopVibration();
    static bool IsVibrating() { return s_isVibrating; }

    // LED制御
    static bool SetLED(uint8_t r, uint8_t g, uint8_t b);
    static bool HasLED();

    // センサー
    static bool EnableGyro(bool enable);
    static bool EnableAccelerometer(bool enable);
    static SensorData GetSensorData();
    static bool HasGyro();
    static bool HasAccelerometer();

    // タッチパッド
    static TouchpadData GetTouchpadData();
    static bool HasTouchpad();

    // バッテリー情報
    static BatteryInfo GetBatteryInfo();

    // Press判定
    static bool IsPressed_ButtonDown() { return s_currentState.buttonDown; }
    static bool IsPressed_ButtonRight() { return s_currentState.buttonRight; }
    static bool IsPressed_ButtonLeft() { return s_currentState.buttonLeft; }
    static bool IsPressed_ButtonUp() { return s_currentState.buttonUp; }
    static bool IsPressed_L1() { return s_currentState.buttonL1; }
    static bool IsPressed_R1() { return s_currentState.buttonR1; }
    static bool IsPressed_L2() { return s_currentState.buttonL2; }
    static bool IsPressed_R2() { return s_currentState.buttonR2; }
    static bool IsPressed_L3() { return s_currentState.buttonL3; }
    static bool IsPressed_R3() { return s_currentState.buttonR3; }
    static bool IsPressed_Start() { return s_currentState.buttonStart; }
    static bool IsPressed_Select() { return s_currentState.buttonSelect; }
    static bool IsPressed_Guide() { return s_currentState.buttonGuide; }
    static bool IsPressed_Misc() { return s_currentState.buttonMisc; }
    static bool IsPressed_DpadUp() { return s_currentState.dpadUp; }
    static bool IsPressed_DpadDown() { return s_currentState.dpadDown; }
    static bool IsPressed_DpadLeft() { return s_currentState.dpadLeft; }
    static bool IsPressed_DpadRight() { return s_currentState.dpadRight; }

    // Trigger判定
    static bool IsTrigger_ButtonDown() { return s_currentState.buttonDown && !s_prevState.buttonDown; }
    static bool IsTrigger_ButtonRight() { return s_currentState.buttonRight && !s_prevState.buttonRight; }
    static bool IsTrigger_ButtonLeft() { return s_currentState.buttonLeft && !s_prevState.buttonLeft; }
    static bool IsTrigger_ButtonUp() { return s_currentState.buttonUp && !s_prevState.buttonUp; }
    static bool IsTrigger_L1() { return s_currentState.buttonL1 && !s_prevState.buttonL1; }
    static bool IsTrigger_R1() { return s_currentState.buttonR1 && !s_prevState.buttonR1; }
    static bool IsTrigger_L2() { return s_currentState.buttonL2 && !s_prevState.buttonL2; }
    static bool IsTrigger_R2() { return s_currentState.buttonR2 && !s_prevState.buttonR2; }
    static bool IsTrigger_L3() { return s_currentState.buttonL3 && !s_prevState.buttonL3; }
    static bool IsTrigger_R3() { return s_currentState.buttonR3 && !s_prevState.buttonR3; }
    static bool IsTrigger_Start() { return s_currentState.buttonStart && !s_prevState.buttonStart; }
    static bool IsTrigger_Select() { return s_currentState.buttonSelect && !s_prevState.buttonSelect; }
    static bool IsTrigger_Guide() { return s_currentState.buttonGuide && !s_prevState.buttonGuide; }
    static bool IsTrigger_Misc() { return s_currentState.buttonMisc && !s_prevState.buttonMisc; }
    static bool IsTrigger_DpadUp() { return s_currentState.dpadUp && !s_prevState.dpadUp; }
    static bool IsTrigger_DpadDown() { return s_currentState.dpadDown && !s_prevState.dpadDown; }
    static bool IsTrigger_DpadLeft() { return s_currentState.dpadLeft && !s_prevState.dpadLeft; }
    static bool IsTrigger_DpadRight() { return s_currentState.dpadRight && !s_prevState.dpadRight; }

    // Release判定
    static bool IsRelease_ButtonDown() { return !s_currentState.buttonDown && s_prevState.buttonDown; }
    static bool IsRelease_ButtonRight() { return !s_currentState.buttonRight && s_prevState.buttonRight; }
    static bool IsRelease_ButtonLeft() { return !s_currentState.buttonLeft && s_prevState.buttonLeft; }
    static bool IsRelease_ButtonUp() { return !s_currentState.buttonUp && s_prevState.buttonUp; }
    static bool IsRelease_L1() { return !s_currentState.buttonL1 && s_prevState.buttonL1; }
    static bool IsRelease_R1() { return !s_currentState.buttonR1 && s_prevState.buttonR1; }
    static bool IsRelease_L2() { return !s_currentState.buttonL2 && s_prevState.buttonL2; }
    static bool IsRelease_R2() { return !s_currentState.buttonR2 && s_prevState.buttonR2; }
    static bool IsRelease_L3() { return !s_currentState.buttonL3 && s_prevState.buttonL3; }
    static bool IsRelease_R3() { return !s_currentState.buttonR3 && s_prevState.buttonR3; }
    static bool IsRelease_Start() { return !s_currentState.buttonStart && s_prevState.buttonStart; }
    static bool IsRelease_Select() { return !s_currentState.buttonSelect && s_prevState.buttonSelect; }
    static bool IsRelease_Guide() { return !s_currentState.buttonGuide && s_prevState.buttonGuide; }
    static bool IsRelease_Misc() { return !s_currentState.buttonMisc && s_prevState.buttonMisc; }
    static bool IsRelease_DpadUp() { return !s_currentState.dpadUp && s_prevState.dpadUp; }
    static bool IsRelease_DpadDown() { return !s_currentState.dpadDown && s_prevState.dpadDown; }
    static bool IsRelease_DpadLeft() { return !s_currentState.dpadLeft && s_prevState.dpadLeft; }
    static bool IsRelease_DpadRight() { return !s_currentState.dpadRight && s_prevState.dpadRight; }

    // スティック・トリガー値取得
    static float GetLeftStickX() { return s_currentState.leftStickX; }
    static float GetLeftStickY() { return s_currentState.leftStickY; }
    static float GetRightStickX() { return s_currentState.rightStickX; }
    static float GetRightStickY() { return s_currentState.rightStickY; }
    static float GetLeftTrigger() { return s_currentState.leftTrigger; }
    static float GetRightTrigger() { return s_currentState.rightTrigger; }

    // 接続状態
    static bool IsConnected() { return s_currentState.connected; }

private:
    static void UpdateState();
    static void OpenGamepad(SDL_JoystickID id);
    static void CloseGamepad();

    static SDL_Gamepad* s_pGamepad;
    static SDL_JoystickID s_gamepadId;

    static GamepadState s_currentState;
    static GamepadState s_prevState;

    static bool s_isVibrating;
    static Uint64 s_vibrationEndTime;
};
