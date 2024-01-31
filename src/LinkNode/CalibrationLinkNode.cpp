#include "CalibrationLinkNode.h"


CalibrationLinkNode *init_calibrationLinkNode()
{
#ifdef SERIAL_DEBUG
    Serial.println("init_calibrationLink");
#endif
    struct CalibrationLinkNode *p = (struct CalibrationLinkNode *)malloc(sizeof(struct CalibrationLinkNode));
    p->next = NULL;
    p->anchor_addr = 0;
    p->averageRange = 0.0;
    p->averageDbm = 0.0;
    p->numberOfValues = 0;
    p->isCalibrated = false;
    
    return p;
}

void add_link(CalibrationLinkNode *p, uint16_t addr)
{
#ifdef SERIAL_DEBUG
    Serial.println("add_calibrationLink");
#endif
    struct CalibrationLinkNode *temp = p;

    while (temp->next != NULL)
        temp = temp->next;

    //Create a anchor
    struct CalibrationLinkNode *a = (struct CalibrationLinkNode *)malloc(sizeof(struct CalibrationLinkNode));
    a->anchor_addr = addr;
    a->averageRange = 0.0;
    a->averageDbm = 0.0;
    a->numberOfValues = 0;
    a->isCalibrated = false;
    a->next = NULL;

    //Add anchor to end of struct MyLink
    temp->next = a;

    return;
}

CalibrationLinkNode *find_link(CalibrationLinkNode *p, uint16_t addr)
{
#ifdef SERIAL_DEBUG
    Serial.println("find_calibrationLink");
#endif
    if (addr == 0) {
    #ifdef SERIAL_DEBUG
        Serial.println("find_calibrationLink:Input addr is 0");
    #endif
        return NULL;
    }

    if (p->next == NULL) {
    #ifdef SERIAL_DEBUG
        Serial.println("find_calibrationLink:Link is empty");
    #endif
        return NULL;
    }

    struct CalibrationLinkNode *temp = p;

    while (temp->next != NULL) {
        temp = temp->next;

        if (temp->anchor_addr == addr)
            return temp;
    }

#ifdef SERIAL_DEBUG
    Serial.println("find_calibrationLink:Link is not exist");
#endif
    return nullptr;
}

void update_link(CalibrationLinkNode *p, uint16_t addr, float range, float dbm)
{
#ifdef SERIAL_DEBUG
    Serial.println("update_calibrationLink");
#endif
    struct CalibrationLinkNode *temp = find_link(p, addr);

    if (temp == NULL) {
    #ifdef SERIAL_DEBUG
        Serial.println("update_calibrationLink:Can't find addr");
    #endif
        return;
    }

    if (temp->isCalibrated) {
    #ifdef SERIAL_DEBUG
        Serial.println("update_calibrationLink:Anchor is calibrated");
    #endif
        return;
    }

    temp->averageRange = (temp->averageRange * temp->numberOfValues + range) / (temp->numberOfValues + 1);
    temp->averageDbm =   (temp->averageDbm   * temp->numberOfValues + dbm)   / (temp->numberOfValues + 1);
    temp->numberOfValues++;

    if (temp->numberOfValues >= maxValues)
        temp->isCalibrated = true;

    return;
}

void print_link(CalibrationLinkNode *p)
{
#ifdef SERIAL_DEBUG
    Serial.println("print_calibrationLink");
#endif
    if (p->next == NULL) {
    #ifdef SERIAL_DEBUG
        Serial.println("print_calibrationLink:Link is empty");
    #endif
        return;
    }

    struct CalibrationLinkNode *temp = p;

    while (temp->next != NULL) {
        temp = temp->next;
        Serial.print("Anchor addr:");
        Serial.print(temp->anchor_addr);
        Serial.print(" averageRange:");
        Serial.print(temp->averageRange);
        Serial.print(" averageDbm:");
        Serial.print(temp->averageDbm);
        Serial.print(" numberOfValues:");
        Serial.print(temp->numberOfValues);
        Serial.print(" isCalibrated:");
        Serial.println(temp->isCalibrated);
    }

    return;
}

void delete_link(CalibrationLinkNode *p, uint16_t addr)
{
#ifdef SERIAL_DEBUG
    Serial.println("delete_calibrationLink");
#endif
    if (addr == 0) {
    #ifdef SERIAL_DEBUG
        Serial.println("delete_calibrationLink:Input addr is 0");
    #endif
        return;
    }

    if (p->next == NULL) {
    #ifdef SERIAL_DEBUG
        Serial.println("delete_calibrationLink:Link is empty");
    #endif
        return;
    }

    struct CalibrationLinkNode *temp = p;

    while (temp->next != NULL) {
        if (temp->next->anchor_addr == addr) {
            struct CalibrationLinkNode *del = temp->next;
            temp->next = temp->next->next;
            free(del);
            return;
        }

        temp = temp->next;
    }
}

void reset_link(CalibrationLinkNode *p)
{
#ifdef SERIAL_DEBUG
    Serial.println("reset_calibrationLink");
#endif
    if (p->next == NULL) {
    #ifdef SERIAL_DEBUG
        Serial.println("reset_calibrationLink:Link is empty");
    #endif
        return;
    }

    struct CalibrationLinkNode *temp = p;

    while (temp->next != NULL) {
        struct CalibrationLinkNode *del = temp->next;
        temp->next = temp->next->next;
        free(del);
    }
}

// Rajouter le fait que la calibration est finie ou non
void make_link_json(CalibrationLinkNode *p, String *s)
{
#ifdef SERIAL_DEBUG
    Serial.println("make_calibrationLink_json");
#endif
    struct CalibrationLinkNode *temp = p;

    *s = "{\"CalibrationLink\":[";

    while (temp->next != NULL) {
        temp = temp->next;
        char c[100];
        sprintf(c, "{\"Ad\":%X,\"AvR\":%.2f,\"AvD\":%.1f,\"NVal\":%d}", temp->anchor_addr, temp->averageRange, temp->averageDbm, temp->numberOfValues);
        *s += c;

        if (temp->next != NULL)
            *s += ",";
    }

    *s += "]}";

    return;
}

bool calibration_is_finished(CalibrationLinkNode *p, uint16_t nbAnchors)
{
#ifdef SERIAL_DEBUG
    Serial.println("calibration_is_finished ?");
#endif
    if (p->next == NULL) {
    #ifdef SERIAL_DEBUG
        Serial.println("calibration_is_finished:Link is empty");
    #endif
        return false;
    }

    struct CalibrationLinkNode *temp = p;
    uint16_t nbAnchorsCal = 0;

    while (temp->next != NULL) {
        temp = temp->next;
        nbAnchorsCal++;

        if (!temp->isCalibrated) {
            return false;
        }
    }

    if (nbAnchors == 0 || nbAnchors == nbAnchorsCal) {
        return true;
    }

    return false;
}
