#include <SPI.h>
#include "DW1000Ranging.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "LinkNode/CalibrationLinkNode.h"
#include "LinkNode/TagLinkNode.h"
#include "Emitter.h"
#include "MessageDecoder.h"



#define SPI_SCK  18
#define SPI_MISO 19
#define SPI_MOSI 23

#define UWB_RST 27 // reset pin
#define UWB_IRQ 34 // irq pin
#define UWB_SS  21 // spi select pin

#define I2C_SDA 4
#define I2C_SCL 5

#define ANCHOR_ADD "02:00:00:00:00:00:00:01"



struct CalibrationLinkNode *calibration_data = nullptr;
struct TagLinkNode *tag_data = nullptr; // TODO: Remplir le Cpp de TagLinkNode

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Emitter emitter;
MessageDecoder messageDecoder;

uint16_t ADelay = 16611;
uint16_t TDelay = 16384;

bool calibrationPhase = false;



void init_display();
void init_uwb();
void init_emitter();
void init_messageDecoder();

void display_calibration(struct CalibrationLinkNode *p);
void display_tag(struct TagLinkNode *p);

/*
    TODO: Il faut faire une fonction de calibration pour trouver la distance 
    avec les autres balises.
*/


void setup()
{
    Serial.begin(115200);

    calibration_data = init_calibrationLinkNode();
    tag_data = init_tagLinkNode();

    init_display();
    init_uwb();
    init_emitter();
    init_messageDecoder();


    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0); // Start at top-left corner
    display.println(ANCHOR_ADD);
    display.display();
}

long int runtime = 0;

void loop()
{
    DW1000Ranging.loop();

    String message = emitter.read();

    if (message != "") {
        Command cmd = messageDecoder.decodeMessage(message.c_str());
    }

    if ((millis() - runtime) > 200) {
        display.clearDisplay();
        display_calibration(calibration_data);
        display_tag(tag_data);
        display.display();

        runtime = millis();
    }
}

void init_display()
{
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(1000);
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3C for 128x32
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }
}

// ATTENTION GROS BUGS
// Lors du changement de mode entre anchor et tag, cela fais crasher tous les autres tags et anchors
// il faudrait peut-etre faire SPI.end() et SPI.begin() lors du changement de mode
// il faudrait aussi reset la communication avec DW1000Ranging.initCommunication(UWB_RST, UWB_SS, UWB_IRQ); etc ...

void init_uwb()
{
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    DW1000Ranging.initCommunication(UWB_RST, UWB_SS, UWB_IRQ);

    DW1000.setAntennaDelay(ADelay);

    DW1000Ranging.attachNewRange([](){
        // on est en mode tag -> on update les moyennes des distances des anchors
        if (calibrationPhase) {
            update_link(calibration_data, 
                DW1000Ranging.getDistantDevice()->getShortAddress(), 
                DW1000Ranging.getDistantDevice()->getRange(), 
                DW1000Ranging.getDistantDevice()->getRXPower());
            
            return;
        }
        
        // on est en mode anchor -> on update les distances des tags
        fresh_link(tag_data, 
            DW1000Ranging.getDistantDevice()->getShortAddress(), 
            DW1000Ranging.getDistantDevice()->getRange(), 
            DW1000Ranging.getDistantDevice()->getRXPower());
    });

    // TODO: Refaire les deux fonctions suivantes
    // s'assurer qu'elles sont seulement appelées lorsqu'on est en mode anchor ou tag respectivement
    // Fonctionne lorsqu'on est en mode anchor
    DW1000Ranging.attachBlinkDevice([](DW1000Device *device){
        // on est en mode tag
        if (calibrationPhase) {
            add_link(calibration_data, device->getShortAddress());
            return;
        }

        // on est en mode anchor
        add_link(tag_data, device->getShortAddress());
    });

    // Fonctionne lorsqu'on est en mode tag
    DW1000Ranging.attachNewDevice([](DW1000Device *device){
        // on est en mode tag
        if (calibrationPhase) {
            add_link(calibration_data, device->getShortAddress());
            return;
        }

        // on est en mode anchor
        add_link(tag_data, device->getShortAddress());
    });

    DW1000Ranging.attachInactiveDevice([](DW1000Device *device){
        // on est en mode tag
        if (calibrationPhase) {
            delete_link(calibration_data, device->getShortAddress());
            return;
        }

        // on est en mode anchor
        delete_link(tag_data, device->getShortAddress());
    });

    DW1000Ranging.startAsAnchor((char*)ANCHOR_ADD, DW1000.MODE_SHORTDATA_FAST_LOWPOWER, false);
}

void init_emitter()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Connection to wifi");
    char c0[30];
    display.setCursor(0, 8);
    sprintf(c0, "SSID: %s", emitter.getSSID());
    display.println(c0);
    display.setCursor(0, 16);
    sprintf(c0, "Password: %s", emitter.getPassword());
    display.println(c0);
    display.display();

    emitter.tryToConnectToWifi();


    delay(2000);

    if (emitter.isConnectedToWifi()) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Connected to wifi");
        display.setCursor(0, 8);
        char c1[30];
        sprintf(c1, "IP: %s", emitter.getLocalIP().toString());
        display.println(c1);
        display.display();
    } else {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Not connected to wifi");
        display.display();
    }

    sleep(2);

    if (emitter.tryToConnectToHost()) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Connected to host");
        display.display();
    }
    else {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Not connected to host");
        display.display();
    }

    sleep(2);


    display.clearDisplay(); 
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Initialisation done");
    display.display();
}

// TODO: Il faut finir la fonction de calibration
// On doit mettre un timeout pour la calibration -> si on ne reçoit pas de message
// On doit aussi envoyer les distances a l'host -> pas dans cette fonction
void init_messageDecoder()
{
    messageDecoder.attachCmdStartCalibration([](){
        // On change l'anchor en tag
        DW1000.setAntennaDelay(TDelay);
        DW1000Ranging.startAsTag((char*)ANCHOR_ADD, DW1000.MODE_SHORTDATA_FAST_LOWPOWER, false);
        calibrationPhase = true;

        reset_link(calibration_data);
    });

    messageDecoder.attachCmdEndCalibration([](){
        // On change le tag en anchor
        DW1000.setAntennaDelay(ADelay);
        DW1000Ranging.startAsAnchor((char*)ANCHOR_ADD, DW1000.MODE_SHORTDATA_FAST_LOWPOWER, false);
        calibrationPhase = false;

        reset_link(tag_data);

        // On doit maintenant envoyer les distances moyennes des anchors a l'host
    });
}

void display_calibration(CalibrationLinkNode *p)
{
    if (p == nullptr) {
        return;
    }

    struct CalibrationLinkNode *tempCalibration = p;
    int row = 0;

    if (tempCalibration->next == NULL) {
        display.setTextSize(1);
        display.setCursor(0, row++ * 8);
        display.print("No Calibration");
    }

    while (tempCalibration->next != NULL) {
        tempCalibration = tempCalibration->next;

        char c[100];

        if (tempCalibration->isCalibrated) {
            sprintf(c, "%X  %.2f  %d dbm Ok", tempCalibration->anchor_addr, tempCalibration->averageRange, int(tempCalibration->averageDbm));
        }
        else 
            sprintf(c, "%X  %.2f  %d dbm", tempCalibration->anchor_addr, tempCalibration->averageRange, int(tempCalibration->averageDbm));


        display.setTextSize(1);
        display.setCursor(0, row++ * 8); // Start at top-left corner
        display.print(c);

        if (row >= 8) { 
            break;
        }
    }
}

void display_tag(TagLinkNode *p)
{
    if (p == nullptr) {
        return;
    }

    struct TagLinkNode *tempTag = p;
    int row = 4;

    if (tempTag->next == NULL) {
        display.setTextSize(1);
        display.setCursor(0, row++ * 8);
        display.print("No Tag");
    }

    while (tempTag->next != NULL) {
        tempTag = tempTag->next;

        char c[50];
        sprintf(c, "%X  %.2f  %d dbm", tempTag->tag_addr, tempTag->range, int(tempTag->dbm));

        display.setTextSize(1);
        display.setCursor(0, row++ * 8); // Start at top-left corner
        display.print(c);

        if (row >= 8) { 
            break;
        }
    }
}
