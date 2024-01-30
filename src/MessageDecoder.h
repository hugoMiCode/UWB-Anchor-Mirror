#include <ArduinoJson.h>

/*
    Commande:
    reset la course.
    démarrer une course.
    calibrer les Anchors.
*/

enum class Command {
    NONE = 0,
    RESET_RACE = 1,
    START_RACE = 2,
    START_CALIBRATION = 3,
    END_CALIBRATION = 4
};

// TODO: améliorer la robustesse du décodeur

class MessageDecoder {
public:
// TODO: faire cela en une fonction
    Command decodeMessage(const char* message);
    void executeCommand(Command cmd);


    static void attachCmdResetRace(void (*handleCmdResetRace)()) {
        _handleCmdResetRace = handleCmdResetRace;
    };

    static void attachCmdStartRace(void (*handleCmdStartRace)()) {
        _handleCmdStartRace = handleCmdStartRace;
    };

    static void attachCmdStartCalibration(void (*handleCmdStartCalibration)()) {
        _handleCmdStartCalibration = handleCmdStartCalibration;
    };

    static void attachCmdEndCalibration(void (*handleCmdEndCalibration)()) {
        _handleCmdEndCalibration = handleCmdEndCalibration;
    };


private:
    static void (*_handleCmdResetRace)();
    static void (*_handleCmdStartRace)();
    static void (*_handleCmdStartCalibration)();
    static void (*_handleCmdEndCalibration)();
};
