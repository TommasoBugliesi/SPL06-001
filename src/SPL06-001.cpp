#include "SPL06-001.h"

#define CHECK_OK(OP) { \
    if (!(OP)) { \
        return false; \
    } \
}

SPL06_001::SPL06_001(uint8_t addr, TwoWire *wire) : _addr(addr), _wire(wire) {
    // -----
}

// Private Methods
bool SPL06_001::write_reg(uint8_t reg, uint8_t *value, size_t len) {
    this->_wire->beginTransmission(this->_addr);
    this->_wire->write(reg);
    this->_wire->write(value, len);
    return this->_wire->endTransmission() == 0;
}

bool SPL06_001::write_reg(uint8_t reg, uint8_t value) {
    return this->write_reg(reg, &value, 1);
}

bool SPL06_001::read_reg(uint8_t reg, uint8_t *value, size_t len) {
    this->_wire->beginTransmission(this->_addr);
    this->_wire->write(reg);
    if (this->_wire->endTransmission(false) != 0) {
        return false;
    }

    int n = this->_wire->requestFrom(this->_addr, len);
    if (n != len) {
        return false;
    }

    this->_wire->readBytes(value, len);

    return true;
}

uint8_t SPL06_001::read_reg(uint8_t reg) {
    uint8_t value = 0;
    this->read_reg(reg, &value, 1);
    return value;
}

// Public Methods
bool SPL06_001::begin() {
    CHECK_OK(this->read_reg(SPL06_001_REG_ID) == 0x10); // Check Product and Revision ID

    return true;
}