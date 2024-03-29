#include <Arduino.h>

#include "LinkNode/CalibrationLinkNode.h"


// On doit envoyer les distances moyennes des anchors a l'host
class Calibrator
{
public:
    Calibrator();

    void loop();

    void startCalibration();
    void endCalibration();


    static void attachLinkNode(struct CalibrationLinkNode *calibration_data) {
        _calibration_data = calibration_data;
    };

    static void attachInitUWBAnchor(void (*handleInitUWBAnchor)()) {
        _handleInitUWBAnchor = handleInitUWBAnchor;
    };

    static void attachInitUWBTag(void (*handleInitUWBTag)()) {
        _handleInitUWBTag = handleInitUWBTag;
    };

private:
    static void (*_handleInitUWBAnchor)();
    static void (*_handleInitUWBTag)();

    static struct CalibrationLinkNode *_calibration_data;

    bool calibrationPhase = false;
    bool toogleMode = false;

    uint32_t nbAnchorsToCalibrate = 0;

    uint32_t timeOut = 15000; // temps max pour la calibration
    uint32_t switchModeDelay = 1000; // temps avant de switcher en mode tag
    uint32_t startTimeMS = 0; // date de début de la calibration
};

