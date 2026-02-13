/*********************************************************************
 * \file   game_controller.cpp
 * \brief  ゲームコントローラー入力管理（SDL3完全版）
 *********************************************************************/
#include "game_controller.h"

//==============================================================================
// 静的メンバ変数の定義
//==============================================================================
SDL_Gamepad* GameController::s_pGamepad = nullptr;
SDL_JoystickID GameController::s_gamepadId = 0;
GamepadState GameController::s_currentState = {};
GamepadState GameController::s_prevState = {};
bool GameController::s_isVibrating = false;
Uint64 GameController::s_vibrationEndTime = 0;

//==============================================================================
// 定数定義
//==============================================================================
namespace {
    constexpr float STICK_DEADZONE = 0.15f;
    constexpr float TRIGGER_DIGITAL_THRESHOLD = 0.5f;

    template<typename T>
    T Clamp(T value, T minVal, T maxVal) {
        if (value < minVal) return minVal;
        if (value > maxVal) return maxVal;
        return value;
    }
}

//==============================================================================
// 初期化
//==============================================================================
bool GameController::Initialize() {
    if (!SDL_Init(SDL_INIT_GAMEPAD)) {
        return false;
    }

    s_pGamepad = nullptr;
    s_gamepadId = 0;
    s_currentState = {};
    s_prevState = {};
    s_isVibrating = false;
    s_vibrationEndTime = 0;

    int count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
    if (gamepads && count > 0) {
        OpenGamepad(gamepads[0]);
    }
    SDL_free(gamepads);

    return true;
}

//==============================================================================
// 終了処理
//==============================================================================
void GameController::Finalize() {
    StopVibration();
    CloseGamepad();
    SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
}

//==============================================================================
// ゲームパッドを開く
//==============================================================================
void GameController::OpenGamepad(SDL_JoystickID id) {
    if (s_pGamepad) return;

    s_pGamepad = SDL_OpenGamepad(id);
    if (s_pGamepad) {
        s_gamepadId = id;
    }
}

//==============================================================================
// ゲームパッドを閉じる
//==============================================================================
void GameController::CloseGamepad() {
    if (s_pGamepad) {
        SDL_CloseGamepad(s_pGamepad);
        s_pGamepad = nullptr;
        s_gamepadId = 0;
        s_currentState = {};
    }
}

//==============================================================================
// 更新
//==============================================================================
void GameController::Update() {
    // イベント処理
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_GAMEPAD_ADDED:
            if (!s_pGamepad) {
                OpenGamepad(event.gdevice.which);
            }
            break;

        case SDL_EVENT_GAMEPAD_REMOVED:
            if (s_pGamepad && event.gdevice.which == s_gamepadId) {
                CloseGamepad();

                int count = 0;
                SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
                if (gamepads && count > 0) {
                    OpenGamepad(gamepads[0]);
                }
                SDL_free(gamepads);
            }
            break;
        }
    }

    UpdateState();

    if (s_isVibrating && SDL_GetTicks() >= s_vibrationEndTime) {
        StopVibration();
    }
}

//==============================================================================
// 状態更新
//==============================================================================
void GameController::UpdateState() {
    s_prevState = s_currentState;

    if (!s_pGamepad) {
        s_currentState.connected = false;
        return;
    }

    s_currentState.connected = true;

    // ボタン
    s_currentState.buttonDown = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_SOUTH);
    s_currentState.buttonRight = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_EAST);
    s_currentState.buttonLeft = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_WEST);
    s_currentState.buttonUp = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_NORTH);

    s_currentState.buttonL1 = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    s_currentState.buttonR1 = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    s_currentState.buttonL3 = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK);
    s_currentState.buttonR3 = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_RIGHT_STICK);

    s_currentState.buttonStart = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_START);
    s_currentState.buttonSelect = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_BACK);
    s_currentState.buttonGuide = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_GUIDE);
    s_currentState.buttonMisc = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_MISC1);

    // 十字キー
    s_currentState.dpadUp = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);
    s_currentState.dpadDown = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    s_currentState.dpadLeft = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    s_currentState.dpadRight = SDL_GetGamepadButton(s_pGamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);

    // スティック
    auto normalizeAxis = [](Sint16 value) -> float {
        return Clamp(static_cast<float>(value) / 32767.0f, -1.0f, 1.0f);
        };

    float rawLX = normalizeAxis(SDL_GetGamepadAxis(s_pGamepad, SDL_GAMEPAD_AXIS_LEFTX));
    float rawLY = normalizeAxis(SDL_GetGamepadAxis(s_pGamepad, SDL_GAMEPAD_AXIS_LEFTY));
    float rawRX = normalizeAxis(SDL_GetGamepadAxis(s_pGamepad, SDL_GAMEPAD_AXIS_RIGHTX));
    float rawRY = normalizeAxis(SDL_GetGamepadAxis(s_pGamepad, SDL_GAMEPAD_AXIS_RIGHTY));

    s_currentState.leftStickX = GamepadState::ApplyDeadzone(rawLX, STICK_DEADZONE);
    s_currentState.leftStickY = GamepadState::ApplyDeadzone(rawLY, STICK_DEADZONE);
    s_currentState.rightStickX = GamepadState::ApplyDeadzone(rawRX, STICK_DEADZONE);
    s_currentState.rightStickY = GamepadState::ApplyDeadzone(rawRY, STICK_DEADZONE);

    // トリガー
    auto normalizeTrigger = [](Sint16 value) -> float {
        return Clamp(static_cast<float>(value) / 32767.0f, 0.0f, 1.0f);
        };

    s_currentState.leftTrigger = normalizeTrigger(SDL_GetGamepadAxis(s_pGamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER));
    s_currentState.rightTrigger = normalizeTrigger(SDL_GetGamepadAxis(s_pGamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));

    s_currentState.buttonL2 = (s_currentState.leftTrigger > TRIGGER_DIGITAL_THRESHOLD);
    s_currentState.buttonR2 = (s_currentState.rightTrigger > TRIGGER_DIGITAL_THRESHOLD);
}

//==============================================================================
// コントローラー名取得
//==============================================================================
const char* GameController::GetControllerName() {
    if (!s_pGamepad) return "Not Connected";
    const char* name = SDL_GetGamepadName(s_pGamepad);
    return name ? name : "Unknown";
}

//==============================================================================
// コントローラータイプ取得
//==============================================================================
ControllerType GameController::GetControllerType() {
    if (!s_pGamepad) return ControllerType::Unknown;

    SDL_GamepadType type = SDL_GetGamepadType(s_pGamepad);
    switch (type) {
    case SDL_GAMEPAD_TYPE_XBOX360:
        return ControllerType::Xbox360;
    case SDL_GAMEPAD_TYPE_XBOXONE:
        return ControllerType::XboxOne;
    case SDL_GAMEPAD_TYPE_PS4:
        return ControllerType::PS4;
    case SDL_GAMEPAD_TYPE_PS5:
        return ControllerType::PS5;
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
        return ControllerType::NintendoSwitch;
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
        return ControllerType::NintendoSwitchJoyconLeft;
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
        return ControllerType::NintendoSwitchJoyconRight;
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
        return ControllerType::NintendoSwitchJoyconPair;
    default:
        return ControllerType::Other;
    }
}

//==============================================================================
// プレイヤーインデックス取得
//==============================================================================
int GameController::GetPlayerIndex() {
    if (!s_pGamepad) return -1;
    return SDL_GetGamepadPlayerIndex(s_pGamepad);
}

//==============================================================================
// バイブレーション開始
//==============================================================================
void GameController::StartVibration(float intensity, float duration) {
    StartVibrationEx(intensity, intensity, duration);
}

void GameController::StartVibrationEx(float leftMotor, float rightMotor, float duration) {
    if (!s_pGamepad) return;

    Uint16 left = static_cast<Uint16>(Clamp(leftMotor, 0.0f, 1.0f) * 65535.0f);
    Uint16 right = static_cast<Uint16>(Clamp(rightMotor, 0.0f, 1.0f) * 65535.0f);

    SDL_RumbleGamepad(s_pGamepad, left, right, static_cast<Uint32>(duration * 1000.0f));

    s_isVibrating = true;
    s_vibrationEndTime = SDL_GetTicks() + static_cast<Uint64>(duration * 1000.0f);
}

void GameController::StartVibrationEx(const VibrationSettings& settings) {
    StartVibrationEx(settings.leftMotor, settings.rightMotor, settings.duration);
}

//==============================================================================
// トリガー振動
//==============================================================================
void GameController::StartTriggerVibration(float left, float right, float duration) {
    if (!s_pGamepad) return;

    Uint16 leftVal = static_cast<Uint16>(Clamp(left, 0.0f, 1.0f) * 65535.0f);
    Uint16 rightVal = static_cast<Uint16>(Clamp(right, 0.0f, 1.0f) * 65535.0f);

    SDL_RumbleGamepadTriggers(s_pGamepad, leftVal, rightVal, static_cast<Uint32>(duration * 1000.0f));
}

//==============================================================================
// バイブレーション停止
//==============================================================================
void GameController::StopVibration() {
    if (s_pGamepad) {
        SDL_RumbleGamepad(s_pGamepad, 0, 0, 0);
    }
    s_isVibrating = false;
}

//==============================================================================
// LED設定
//==============================================================================
bool GameController::SetLED(uint8_t r, uint8_t g, uint8_t b) {
    if (!s_pGamepad) return false;
    return SDL_SetGamepadLED(s_pGamepad, r, g, b);
}

bool GameController::HasLED() {
    if (!s_pGamepad) return false;
    return SDL_GetBooleanProperty(
        SDL_GetGamepadProperties(s_pGamepad),
        SDL_PROP_GAMEPAD_CAP_RGB_LED_BOOLEAN, false);
}

//==============================================================================
// センサー
//==============================================================================
bool GameController::EnableGyro(bool enable) {
    if (!s_pGamepad) return false;
    return SDL_SetGamepadSensorEnabled(s_pGamepad, SDL_SENSOR_GYRO, enable);
}

bool GameController::EnableAccelerometer(bool enable) {
    if (!s_pGamepad) return false;
    return SDL_SetGamepadSensorEnabled(s_pGamepad, SDL_SENSOR_ACCEL, enable);
}

bool GameController::HasGyro() {
    if (!s_pGamepad) return false;
    return SDL_GamepadHasSensor(s_pGamepad, SDL_SENSOR_GYRO);
}

bool GameController::HasAccelerometer() {
    if (!s_pGamepad) return false;
    return SDL_GamepadHasSensor(s_pGamepad, SDL_SENSOR_ACCEL);
}

SensorData GameController::GetSensorData() {
    SensorData data = {};
    if (!s_pGamepad) return data;

    data.hasGyro = HasGyro();
    data.hasAccel = HasAccelerometer();

    float gyro[3] = {};
    float accel[3] = {};

    if (data.hasGyro) {
        SDL_GetGamepadSensorData(s_pGamepad, SDL_SENSOR_GYRO, gyro, 3);
        data.gyroX = gyro[0];
        data.gyroY = gyro[1];
        data.gyroZ = gyro[2];
    }

    if (data.hasAccel) {
        SDL_GetGamepadSensorData(s_pGamepad, SDL_SENSOR_ACCEL, accel, 3);
        data.accelX = accel[0];
        data.accelY = accel[1];
        data.accelZ = accel[2];
    }

    return data;
}

//==============================================================================
// タッチパッド
//==============================================================================
bool GameController::HasTouchpad() {
    if (!s_pGamepad) return false;
    return SDL_GetNumGamepadTouchpads(s_pGamepad) > 0;
}

TouchpadData GameController::GetTouchpadData() {
    TouchpadData data = {};
    if (!s_pGamepad) return data;

    data.numTouchpads = SDL_GetNumGamepadTouchpads(s_pGamepad);
    data.hasTouchpad = (data.numTouchpads > 0);

    if (data.hasTouchpad) {
        for (int i = 0; i < 2; i++) {
            bool down = false;
            float x = 0, y = 0, pressure = 0;
            SDL_GetGamepadTouchpadFinger(s_pGamepad, 0, i, &down, &x, &y, &pressure);
            data.fingers[i].down = down;
            data.fingers[i].x = x;
            data.fingers[i].y = y;
        }
    }

    return data;
}

//==============================================================================
// バッテリー情報
//==============================================================================
BatteryInfo GameController::GetBatteryInfo() {
    BatteryInfo info = {};
    if (!s_pGamepad) return info;

    int percent = 0;
    SDL_PowerState state = SDL_GetGamepadPowerInfo(s_pGamepad, &percent);

    info.hasBatteryInfo = (state != SDL_POWERSTATE_UNKNOWN);
    info.percent = percent;

    switch (state) {
    case SDL_POWERSTATE_ON_BATTERY:
        info.isWired = false;
        if (percent > 70) info.levelText = "Full";
        else if (percent > 40) info.levelText = "Medium";
        else if (percent > 10) info.levelText = "Low";
        else info.levelText = "Empty";
        break;
    case SDL_POWERSTATE_CHARGING:
        info.isWired = true;
        info.levelText = "Charging";
        break;
    case SDL_POWERSTATE_CHARGED:
        info.isWired = true;
        info.percent = 100;
        info.levelText = "Charged";
        break;
    case SDL_POWERSTATE_NO_BATTERY:
        info.isWired = true;
        info.percent = 100;
        info.levelText = "Wired";
        break;
    default:
        info.levelText = "Unknown";
        break;
    }

    return info;
}
