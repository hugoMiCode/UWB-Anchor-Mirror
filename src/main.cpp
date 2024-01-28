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

/*
    TODO: Il faut faire une fonction de calibration pour trouver la distance 
    avec les autres balises.
*/


void setup()
{
    Serial.begin(115200);

    calibration_data = init_calibrationLinkNode();

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

void loop()
{
    DW1000Ranging.loop();

    String message = emitter.read();

    if (message != "") {
        Command cmd = messageDecoder.decodeMessage(message.c_str());
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
            
            // test -> cela ne marche pas
            print_link(calibration_data);

            // Serial.println("newRange: calibrationPhase");
            // Serial.println(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
            // Serial.println(DW1000Ranging.getDistantDevice()->getRange());
            // Serial.println(DW1000Ranging.getDistantDevice()->getRXPower());

            return;
        }
        
        // on est en mode anchor -> on update les distances des tags
        fresh_link(tag_data, 
            DW1000Ranging.getDistantDevice()->getShortAddress(), 
            DW1000Ranging.getDistantDevice()->getRange(), 
            DW1000Ranging.getDistantDevice()->getRXPower());
    });

    // Peut être qu'il faut remplacer cette fonction par attachNewDevice 
    // DW1000Ranging.attachBlinkDevice([](DW1000Device *device){
    //     // on est en mode tag
    //     if (calibrationPhase) {
    //         add_link(calibration_data, device->getShortAddress());

    //         Serial.println("blinkDevice: calibrationPhase");
    //         return;
    //     }

    //     // on est en mode anchor
    //     add_link(tag_data, device->getShortAddress());
    // });

    DW1000Ranging.attachNewDevice([](DW1000Device *device){
        // on est en mode tag
        if (calibrationPhase) {
            add_link(calibration_data, device->getShortAddress());

            Serial.println("newDevice: calibrationPhase");
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
// On doit mettre un timeout pour la calibration et récupérer assez de distances par anchor pour faire une moyenne
// On doit aussi envoyer les distances a l'host -> pas dans cette fonction
void init_messageDecoder()
{
    // La calibration consiste a a changer l'anchor en tag et a trouver les distances avec les autres anchors
    // on remet ensuite l'anchor en anchor et on envoie les distances a l'host
    messageDecoder.attachCmdCalibrateAnchor([](){
        // On change l'anchor en tag
        DW1000.setAntennaDelay(TDelay);
        DW1000Ranging.startAsTag((char*)ANCHOR_ADD, DW1000.MODE_SHORTDATA_FAST_LOWPOWER, false);
        calibrationPhase = true;
    
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0); // Start at top-left corner
        display.println("Calibration");
        display.display();
    });
}
