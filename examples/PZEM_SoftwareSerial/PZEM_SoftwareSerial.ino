/*
 * Exemple : PZEM004T_Enhanced sur port série logiciel (SoftwareSerial)
 *
 * Cartes visées : Arduino Uno / Nano / Mega, ESP8266
 * (le chemin SoftwareSerial est compilé quand PZEM004_SOFTSERIAL est actif :
 *  AVR ou ESP8266, et sauf si PZEM004_NO_SWSERIAL est défini)
 */

#include "PZEM004T_Enhanced.h"

#if defined(PZEM004_SOFTSERIAL)
#include <SoftwareSerial.h>

// RX = 10, TX = 11 : à adapter à votre câblage
SoftwareSerial pzemSerial(10, 11);

// Par référence sur l'instance SoftwareSerial (appelle begin(9600))
PZEM004T_Enhanced pzem(pzemSerial);

// Alternative par broches : crée son propre SoftwareSerial en interne
// PZEM004T_Enhanced pzem(10, 11);
#endif

void setup() {
    Serial.begin(115200);
    delay(2000);

#if defined(PZEM004_SOFTSERIAL)
    Serial.println("PZEM004T_Enhanced - SoftwareSerial");
#else
    Serial.println("SoftwareSerial non disponible sur cette cible");
#endif

    // Optionnel : réinitialiser le compteur d'énergie interne
    // if (pzem.resetEnergy()) Serial.println("Compteur d'énergie réinitialisé");

    // Optionnel : changer l'adresse Modbus du module (0x01..0xF7)
    // pzem.setAddress(0x01);

    // Affiche l'adresse Modbus réelle du module
    Serial.print("Adresse Modbus : ");
    Serial.println(pzem.readAddress(), HEX);
}

void loop() {
#if defined(PZEM004_SOFTSERIAL)
    float V = pzem.voltage();          // Tension en V
    float I = pzem.current();          // Courant en A
    float P = pzem.power();            // Puissance active en W
    float S = pzem.apparentPower();    // Puissance apparente en VA
    float N = pzem.nonActivePower();   // Puissance non-active en VAR
    float PF = pzem.pf();              // Facteur de puissance
    float F = pzem.frequency();        // Fréquence en Hz
    float E = pzem.energy();           // Énergie en kWh

    if (isnan(V)) {
        Serial.println("Erreur de lecture (Vérifiez les fils RX/TX et l'alimentation)");
    } else {
        Serial.println("--- Mesures ---");
        Serial.print("Tension   : "); Serial.print(V); Serial.println(" V");
        Serial.print("Courant   : "); Serial.print(I); Serial.println(" A");
        Serial.print("P active  : "); Serial.print(P); Serial.println(" W");
        Serial.print("S app.    : "); Serial.print(S); Serial.println(" VA");
        Serial.print("N non-act.: "); Serial.print(N); Serial.println(" VAR");
        Serial.print("PF        : "); Serial.println(PF);
        Serial.print("Fréquence : "); Serial.print(F); Serial.println(" Hz");
        Serial.print("Énergie   : "); Serial.print(E); Serial.println(" kWh");
    }

    Serial.println();
#endif
    delay(2000);
}
