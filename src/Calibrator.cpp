#include "Calibrator.h"


void (*Calibrator::_handleInitUWBAnchor)() = nullptr;
void (*Calibrator::_handleInitUWBTag)() = nullptr;

Calibrator::Calibrator()
{
}

void Calibrator::loop()
{
    if (!toogleMode) {
        // Si on atteins le timeout, on doit changer le mode du DW1000 en mode anchor
        if (calibrationPhase && millis() - startTimeMS > timeOut) {
            toogleMode = true;
            calibrationPhase = false;
        }

        return;
    }

    // On doit encore attendre avant de changer le mode du DW1000
    if (millis() - startTimeMS < switchModeDelay) {
        return;
    }

    toogleMode = false;

    // On doit changer le mode du DW1000
    if (calibrationPhase) {
        if (_handleInitUWBTag != nullptr) {
            _handleInitUWBTag();
        }

        return;
    }
    
    if (_handleInitUWBAnchor != nullptr) {
        _handleInitUWBAnchor();
    }

    calibrationPhase = false;
    return;
}

void Calibrator::startCalibration()
{
    if (calibrationPhase) {
        return;
    }

    calibrationPhase = true;
    toogleMode = true;
    startTimeMS = millis();
}

void Calibrator::endCalibration()
{
    if (!calibrationPhase) {
        return;
    }

    calibrationPhase = false;
    toogleMode = true;
}
