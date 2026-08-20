/*
  PZEM004T_Enhanced.h
  Version 1.0.0
  Bibliothèque unifiée pour le compteur d'énergie PZEM-004T v3.0
  Basée sur la bibliothèque de Jakub Mandula (2019), synchronisée avec son master actuel
  Ajouts : apparentPower(), reactivePower(), readAddress(), constructeurs par référence,
           support ESP32 (pins RX/TX), correction du nom de champ frequency

  Licence MIT (voir LICENSE) :
  Copyright (c) 2026 Olivier FOURNET
  Copyright (c) 2019 Jakub Mandula (bibliothèque originale PZEM-004T v3.0)
*/

#ifndef PZEM004T_ENHANCED_H
#define PZEM004T_ENHANCED_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// #define PZEM004_NO_SWSERIAL
#if (not defined(PZEM004_NO_SWSERIAL)) && (defined(__AVR__) || defined(ESP8266) && (not defined(ESP32)))
#define PZEM004_SOFTSERIAL
#endif

#if defined(PZEM004_SOFTSERIAL)
#include <SoftwareSerial.h>
#endif

#define PZEM_DEFAULT_ADDR   0xF8
#define PZEM_BAUD_RATE      9600

class PZEM004T_Enhanced {
public:
    // Constructeur vide pour tableaux / affectation ultérieure (pzem = PZEM004T_Enhanced(port))
    PZEM004T_Enhanced();

#if defined(PZEM004_SOFTSERIAL)
    // Par broches : crée son propre SoftwareSerial (ancienne forme, toujours fonctionnelle)
    PZEM004T_Enhanced(uint8_t receivePin, uint8_t transmitPin, uint8_t addr = PZEM_DEFAULT_ADDR);
    // Par référence sur une instance SoftwareSerial (appelle begin)
    PZEM004T_Enhanced(SoftwareSerial& port, uint8_t addr = PZEM_DEFAULT_ADDR);
    // Par référence sur un Stream (ne fait PAS appel à begin)
    PZEM004T_Enhanced(Stream& port, uint8_t addr = PZEM_DEFAULT_ADDR);
#endif

#if defined(ESP32)
    // ESP32 : nécessite les broches RX/TX (begin avec SERIAL_8N1)
    PZEM004T_Enhanced(HardwareSerial& port, uint8_t receivePin, uint8_t transmitPin, uint8_t addr = PZEM_DEFAULT_ADDR);
#else
    PZEM004T_Enhanced(HardwareSerial& port, uint8_t addr = PZEM_DEFAULT_ADDR);
#endif
    // Forme par pointeur, conservée pour rétrocompatibilité
    PZEM004T_Enhanced(HardwareSerial* port, uint8_t addr = PZEM_DEFAULT_ADDR);
    ~PZEM004T_Enhanced();

    // --- Mesures directes (comme l'original) ---
    float voltage();      // Tension en V
    float current();      // Courant en A
    float power();        // Puissance active P en W
    float energy();       // Énergie en kWh
    float frequency();    // Fréquence en Hz
    float pf();           // Facteur de puissance (cos φ)

    // --- NOUVEAUTÉS : Puissances calculées ---
    float apparentPower();   // Puissance apparente S = U * I (en VA)
    float reactivePower();   // Puissance réactive Q = S * sin(φ) (en VAR)
    float powerFactor();     // Alias de pf() pour clarté

    // --- Fonctions de configuration ---
    bool setAddress(uint8_t addr);
    uint8_t getAddress();
    uint8_t readAddress(bool update = false); // Lit l'adresse dans le registre du module
    bool setPowerAlarm(uint16_t watts);
    bool getPowerAlarm();
    bool resetEnergy();
    void search();

private:
    // Registres et commandes Modbus
    enum {
        REG_VOLTAGE      = 0x0000,
        REG_CURRENT_L    = 0x0001,
        REG_CURRENT_H    = 0x0002,
        REG_POWER_L      = 0x0003,
        REG_POWER_H      = 0x0004,
        REG_ENERGY_L     = 0x0005,
        REG_ENERGY_H     = 0x0006,
        REG_FREQUENCY    = 0x0007,
        REG_PF           = 0x0008,
        REG_ALARM        = 0x0009,
        WREG_ALARM_THR   = 0x0001,
        WREG_ADDR        = 0x0002,
        CMD_RHR          = 0x03, // Read Holding Registers
        CMD_RIR          = 0x04, // Read Input Registers
        CMD_WSR          = 0x06, // Write Single Register
        CMD_CAL          = 0x41,
        CMD_REST         = 0x42,
        UPDATE_TIME      = 200,  // Cache des valeurs (ms)
        READ_TIMEOUT     = 100,  // Timeout de réception (ms)
        INVALID_ADDRESS  = 0x00
    };

    Stream* _serial;
    bool _isSoft;
    uint8_t _addr;
    bool _isConnected;          // Flag mis à jour lors d'une communication réussie

#if defined(PZEM004_SOFTSERIAL)
    SoftwareSerial* _localSWserial = nullptr; // Pointeur vers le SoftwareSerial local (s'il a été créé ici)
#endif

    struct {
        float voltage;
        float current;
        float power;
        float energy;
        float frequency;
        float pf;
        uint16_t alarms;
    } _currentValues;

    uint64_t _lastRead;

    void init(Stream* port, bool isSoft, uint8_t addr);
    bool updateValues();
    uint16_t receive(uint8_t *resp, uint16_t len);
    bool sendCmd8(uint8_t cmd, uint16_t rAddr, uint16_t val, bool check = false, uint16_t slave_addr = 0xFFFF);
    void setCRC(uint8_t *buf, uint16_t len);
    bool checkCRC(const uint8_t *buf, uint16_t len);
    uint16_t CRC16(const uint8_t *data, uint16_t len);
};

// ============================================================
//  IMPLÉMENTATION (tout en ligne pour un fichier .h unique)
// ============================================================

PZEM004T_Enhanced::PZEM004T_Enhanced() :
    _serial(nullptr), _isSoft(false), _addr(PZEM_DEFAULT_ADDR), _isConnected(false),
    _lastRead((uint64_t)0 - UPDATE_TIME) {}

#if defined(PZEM004_SOFTSERIAL)
PZEM004T_Enhanced::PZEM004T_Enhanced(uint8_t receivePin, uint8_t transmitPin, uint8_t addr) {
    _localSWserial = new SoftwareSerial(receivePin, transmitPin);
    _localSWserial->begin(PZEM_BAUD_RATE);
    init((Stream *)_localSWserial, true, addr);
}

PZEM004T_Enhanced::PZEM004T_Enhanced(SoftwareSerial& port, uint8_t addr) {
    port.begin(PZEM_BAUD_RATE);
    init((Stream *)&port, true, addr);
}

PZEM004T_Enhanced::PZEM004T_Enhanced(Stream& port, uint8_t addr) {
    init((Stream *)&port, true, addr);
}
#endif

#if defined(ESP32)
PZEM004T_Enhanced::PZEM004T_Enhanced(HardwareSerial& port, uint8_t receivePin, uint8_t transmitPin, uint8_t addr) {
    port.begin(PZEM_BAUD_RATE, SERIAL_8N1, receivePin, transmitPin);
    init((Stream *)&port, false, addr);
}
#else
PZEM004T_Enhanced::PZEM004T_Enhanced(HardwareSerial& port, uint8_t addr) {
    port.begin(PZEM_BAUD_RATE);
    init((Stream *)&port, false, addr);
}
#endif

PZEM004T_Enhanced::PZEM004T_Enhanced(HardwareSerial* port, uint8_t addr) {
    port->begin(PZEM_BAUD_RATE);
    init((Stream *)port, false, addr);
}

PZEM004T_Enhanced::~PZEM004T_Enhanced() {
#if defined(PZEM004_SOFTSERIAL)
    if (_localSWserial) delete _localSWserial;
#endif
}

void PZEM004T_Enhanced::init(Stream* port, bool isSoft, uint8_t addr) {
    if (addr < 0x01 || addr > 0xF8) addr = PZEM_DEFAULT_ADDR;
    _addr = addr;
    _serial = port;
    _isSoft = isSoft;
    _isConnected = false;
    _lastRead = 0;
    _lastRead -= UPDATE_TIME; // Force une lecture immédiate
}

bool PZEM004T_Enhanced::updateValues() {
    static uint8_t response[25];
    if (_lastRead + UPDATE_TIME > millis()) return true;

    _lastRead = millis(); // Throttle la lecture même en cas d'échec

    sendCmd8(CMD_RIR, 0x00, 0x0A, false);

    if (receive(response, 25) != 25) return false;

    _currentValues.voltage   = ((uint32_t)response[3] << 8 | (uint32_t)response[4]) / 10.0;
    _currentValues.current   = ((uint32_t)response[5] << 8 | (uint32_t)response[6] |
                                (uint32_t)response[7] << 24 | (uint32_t)response[8] << 16) / 1000.0;

    _currentValues.power     = 1000.0 * ((uint32_t)response[9] << 8 | (uint32_t)response[10] |
                                (uint32_t)response[11] << 24 | (uint32_t)response[12] << 16) / 10.0;

    _currentValues.energy    = ((uint32_t)response[13] << 8 | (uint32_t)response[14] |
                                (uint32_t)response[15] << 24 | (uint32_t)response[16] << 16) / 1000.0;
    _currentValues.frequency = ((uint32_t)response[17] << 8 | (uint32_t)response[18]) / 10.0;
    _currentValues.pf        = ((uint32_t)response[19] << 8 | (uint32_t)response[20]) / 100.0;
    _currentValues.alarms    = ((uint32_t)response[21] << 8 | (uint32_t)response[22]);

    return true;
}

float PZEM004T_Enhanced::voltage()   { return updateValues() ? _currentValues.voltage : NAN; }
float PZEM004T_Enhanced::current()   { return updateValues() ? _currentValues.current : NAN; }
float PZEM004T_Enhanced::power()     { return updateValues() ? _currentValues.power : NAN; }
float PZEM004T_Enhanced::energy()    { return updateValues() ? _currentValues.energy : NAN; }
float PZEM004T_Enhanced::frequency() { return updateValues() ? _currentValues.frequency : NAN; }
float PZEM004T_Enhanced::pf()        { return updateValues() ? _currentValues.pf : NAN; }
float PZEM004T_Enhanced::powerFactor() { return pf(); }

// --- NOUVEAUTÉS : calculs P, S, Q ---
float PZEM004T_Enhanced::apparentPower() {
    float V = voltage();
    float I = current();
    if (isnan(V) || isnan(I)) return NAN;
    return V * I;  // S = U * I en VA
}

float PZEM004T_Enhanced::reactivePower() {
    float S = apparentPower();
    float PF = pf();
    if (isnan(S) || isnan(PF)) return NAN;
    // sin(φ) = sqrt(1 - cos²(φ))
    float sinPhi = sqrt(1.0 - PF * PF);
    return S * sinPhi;  // Q = S * sin(φ) en VAR
}

// --- Fonctions de configuration ---
bool PZEM004T_Enhanced::sendCmd8(uint8_t cmd, uint16_t rAddr, uint16_t val, bool check, uint16_t slave_addr) {
    uint8_t sendBuffer[8], respBuffer[8];
    if ((slave_addr == 0xFFFF) || (slave_addr < 0x01) || (slave_addr > 0xF7)) slave_addr = _addr;

    sendBuffer[0] = slave_addr;
    sendBuffer[1] = cmd;
    sendBuffer[2] = (rAddr >> 8) & 0xFF;
    sendBuffer[3] = rAddr & 0xFF;
    sendBuffer[4] = (val >> 8) & 0xFF;
    sendBuffer[5] = val & 0xFF;
    setCRC(sendBuffer, 8);

    _serial->write(sendBuffer, 8);

    if (check) {
        if (receive(respBuffer, 8) != 8) return false;
        for (uint8_t i = 0; i < 8; i++) if (sendBuffer[i] != respBuffer[i]) return false;
    }
    return true;
}

bool PZEM004T_Enhanced::setAddress(uint8_t addr) {
    if (addr < 0x01 || addr > 0xF7) return false;
    if (!sendCmd8(CMD_WSR, WREG_ADDR, addr, true)) return false;
    _addr = addr;
    return true;
}

uint8_t PZEM004T_Enhanced::getAddress() { return _addr; }

uint8_t PZEM004T_Enhanced::readAddress(bool update) {
    static uint8_t response[7];
    if (!sendCmd8(CMD_RHR, WREG_ADDR, 0x01, false)) return INVALID_ADDRESS;
    if (receive(response, 7) != 7) return INVALID_ADDRESS;

    uint8_t addr = (uint8_t)(((uint32_t)response[3] << 8) | (uint32_t)response[4]);

    if (update) _addr = addr;
    return addr;
}

bool PZEM004T_Enhanced::setPowerAlarm(uint16_t watts) {
    if (watts > 25000) watts = 25000;
    return sendCmd8(CMD_WSR, WREG_ALARM_THR, watts, true);
}

bool PZEM004T_Enhanced::getPowerAlarm() {
    return updateValues() ? (_currentValues.alarms != 0x0000) : NAN;
}

bool PZEM004T_Enhanced::resetEnergy() {
    uint8_t buffer[] = {0x00, CMD_REST, 0x00, 0x00};
    uint8_t reply[5];
    buffer[0] = _addr;
    setCRC(buffer, 4);
    _serial->write(buffer, 4);
    uint16_t length = receive(reply, 5);
    if (length == 0 || length == 5) return false;
    return true;
}

uint16_t PZEM004T_Enhanced::receive(uint8_t *resp, uint16_t len) {
#if defined(PZEM004_SOFTSERIAL)
    if (_isSoft) ((SoftwareSerial *)_serial)->listen();
#endif
    unsigned long startTime = millis();
    uint8_t index = 0;
    while ((index < len) && (millis() - startTime < READ_TIMEOUT)) {
        if (_serial->available() > 0) {
            resp[index++] = (uint8_t)_serial->read();
        }
        yield();
    }
    if (!checkCRC(resp, index)) {
        _isConnected = false;
        return 0;
    }
    _isConnected = true;
    return index;
}

bool PZEM004T_Enhanced::checkCRC(const uint8_t *buf, uint16_t len) {
    if (len <= 2) return false;
    uint16_t crc = CRC16(buf, len - 2);
    return ((uint16_t)buf[len - 2] | (uint16_t)buf[len - 1] << 8) == crc;
}

void PZEM004T_Enhanced::setCRC(uint8_t *buf, uint16_t len) {
    if (len <= 2) return;
    uint16_t crc = CRC16(buf, len - 2);
    buf[len - 2] = crc & 0xFF;
    buf[len - 1] = (crc >> 8) & 0xFF;
}

static const uint16_t crcTable[] PROGMEM = {
    0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
    0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
    0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
    0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
    0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
    0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
    0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
    0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
    0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
    0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
    0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
    0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
    0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
    0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
    0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
    0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
    0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
    0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
    0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
    0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
    0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
    0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
    0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
    0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
    0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
    0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
    0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
    0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
    0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
    0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
    0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
    0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040
};

uint16_t PZEM004T_Enhanced::CRC16(const uint8_t *data, uint16_t len) {
    uint8_t nTemp;
    uint16_t crc = 0xFFFF;
    while (len--) {
        nTemp = *data++ ^ crc;
        crc >>= 8;
        crc ^= (uint16_t)pgm_read_word(&crcTable[nTemp]);
    }
    return crc;
}

void PZEM004T_Enhanced::search() {
#if (not defined(PZEM004T_DISABLE_SEARCH))
    static uint8_t response[7];
    for (uint16_t addr = 0x01; addr <= 0xF8; addr++) {
        sendCmd8(CMD_RIR, 0x00, 0x01, false, addr);
        if (receive(response, 7) != 7) continue;
        Serial.print("Device on addr: ");
        Serial.println(addr);
    }
#endif
}

#endif // PZEM004T_ENHANCED_H
