#include "SPL06-001.h"

#define CHECK_OK(OP) { \
    if (!(OP)) { \
        Serial.println("Check Failed!"); \
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

bool SPL06_001::status() {
    uint8_t buffer[1];
    CHECK_OK(this->read_reg(SPL06_001_REG_INT_STS, buffer)); 
    this->COEF_RDY      = buffer[0] & 0b10000000;
    this->SENSOR_RDY    = buffer[0] & 0b01000000;
    this->TMP_RDY       = buffer[0] & 0b00100000;
    this->PRS_RDY       = buffer[0] & 0b00010000;

    return true;
}

// Public Methods
bool SPL06_001::begin() {
    CHECK_OK(this->read_reg(SPL06_001_REG_ID) == 0x10); // Check Product and Revision ID
    CHECK_OK(this->write_reg(SPL06_001_REG_RESET, 0b00001001)); // soft reset
    delay(50); // wait sensor ready and coefficients are available 

    CHECK_OK(this->status());

    if (this->SENSOR_RDY & this->COEF_RDY){
        /* Set measurement mode and type:
        Standby Mode: 
        000 : Idle / Stop background measurement
        Command Mode:
        001 : Pressure measurement
        010 : Temperature measurement
        011 : na.
        100 : na.
        Background Mode:
        101 : Continuous pressure measurement
        110 : Continuous temperature measurement
        111 : Continuous pressure and temperature
        */  
        CHECK_OK(this->write_reg(SPL06_001_REG_MEAS_CFG, 0b00000111));

        /*
        Set Pressure/Temperature measurement rate and oversampling
        */
        CHECK_OK(this->write_reg(SPL06_001_REG_PSR_CFG, (this->p_measurement_rate|this->p_oversampling_rate))); 
        CHECK_OK(this->write_reg(SPL06_001_REG_TMP_CFG, (this->t_measurement_rate|this->t_oversampling_rate))); 
        CHECK_OK(this->write_reg(SPL06_001_REG_CFG_REG, (this->t_oversampling_rate > EIGHT_TIMES ? BIT3 : 0) | (this->p_oversampling_rate > EIGHT_TIMES ? BIT2 : 0))); // measurement data shift

        // Read Calibration Coefficients
        uint8_t buff[18];
        CHECK_OK(this->read_reg(SPL06_001_REG_COEF_C0, buff, sizeof(buff)));
        calibration_coefficients.c0 = ((int16_t)(buff[0]) << 4) | (buff[1] >> 4);
        calibration_coefficients.c1 = ((int16_t)(buff[1] & 0x0F) << 8) | buff[2];
        calibration_coefficients.c00 = (((int32_t)(buff[3]) << 12) | ((int32_t)(buff[4]) << 4) | (buff[5] >> 4));
        calibration_coefficients.c10 = ((int32_t)(buff[5] & 0x0F) << 16) | ((int32_t)(buff[6]) << 8) | buff[7];
        calibration_coefficients.c01 = ((int16_t)(buff[8]) << 8) | buff[9];
        calibration_coefficients.c11 = ((int16_t)(buff[10]) << 8) | buff[11];
        calibration_coefficients.c20 = ((int16_t)(buff[12]) << 8) | buff[13];
        calibration_coefficients.c21 = ((int16_t)(buff[14]) << 8) | buff[15];
        calibration_coefficients.c30 = ((int16_t)(buff[16]) << 8) | buff[17];

        if (bitRead(calibration_coefficients.c0, 11)) {calibration_coefficients.c0 |= 0xF000;}
        if (bitRead(calibration_coefficients.c1, 11)) {calibration_coefficients.c1 |= 0xF000;}
        if (bitRead(calibration_coefficients.c00, 19)) {calibration_coefficients.c00 |= 0xFFF00000;}
        if (bitRead(calibration_coefficients.c10, 19)) {calibration_coefficients.c10 |= 0xFFF00000;}

        delay(250);
        this->init_completed = true;

        return true;
    }
    else {
        Serial.println("SPL06-001 init phase failed");
        return false;
    }
}
    
bool SPL06_001::measure() {
    // Perform measuring only if init phase completed
    if (this->init_completed){
        // Check for sensor status
        CHECK_OK(this->status());
        if (this->SENSOR_RDY & this->PRS_RDY & this->TMP_RDY){
            // Define buffer and reset class variables
            uint8_t buff[6];
            this->Pcomp = this->Tcomp = -1.0f;

            // Read raw pressure and temperature values
            CHECK_OK(this->read_reg(SPL06_001_REG_PSR_B2, buff, 6));

            int32_t Praw = (((int32_t) buff[0]) << 16) | (((int32_t) buff[1]) << 8) | buff[2];
            if (bitRead(Praw, 23)) {
                Praw |= 0xFF000000; // Set left bits to one for 2's complement conversion of negative number
            }
            int32_t Traw = (((int32_t) buff[3]) << 16) | (((int32_t) buff[4]) << 8) | buff[5];
            if (bitRead(Traw, 23)) {
                Traw |= 0xFF000000; // Set left bits to one for 2's complement conversion of negative number
            }
            Serial.println("----------");
            Serial.print("C0: "); Serial.println(calibration_coefficients.c0);
            Serial.print("C1: "); Serial.println(calibration_coefficients.c1);
            Serial.print("C00: "); Serial.println(calibration_coefficients.c00);
            Serial.print("C10: "); Serial.println(calibration_coefficients.c10);
            Serial.print("C01: "); Serial.println(calibration_coefficients.c01);
            Serial.print("C11: "); Serial.println(calibration_coefficients.c11);
            Serial.print("C20: "); Serial.println(calibration_coefficients.c20);
            Serial.print("C21: "); Serial.println(calibration_coefficients.c21);
            Serial.print("C30: "); Serial.println(calibration_coefficients.c30);
            Serial.print("Traw: "); Serial.println(Traw);
            Serial.print("Praw: "); Serial.println(Praw);
            Serial.println("----------");

            float Traw_sc = Traw / (float) scale_factor[this->t_oversampling_rate];
            float Praw_sc = Praw / (float) scale_factor[this->p_oversampling_rate];

            /* Pressure: 
            1. Read the calibration coefficients (c00, c10, c20, c30, c01, c11, and c21) from the Calibration Coefficient register.
            Note: The coefficients c00 and c10 are 20 bit 2´s complement numbers, c20, c30, c01, c11 and c21 are 16 bit 2´s complement numbers.
            2. Choose scaling factors kT (for temperature) and kP (for pressure) based on the chosen precision rate. The scaling factors are listed in Table 7.
            3. Read the pressure and temperature result from the registers or FIFO.
            Note: The measurements read from the result registers (or FIFO) are 24 bit 2´s complement numbers.
            Depending on the chosen measurement rates, the temperature may not have been measured since the last pressure measurement.
            4. Calculate scaled measurement results.
            Traw_sc = Traw/kT
            Praw_sc = Praw/kP
            5. Calculate compensated measurement results.
            P
            comp ( = c00+ P raw_sc *(c10 + P raw_sc *(c20+ P raw_sc *c30)) + T raw_sc *c01 + T raw_sc
            *P raw_sc *(c11+P raw_sc *
            */
            this->Pcomp = calibration_coefficients.c00 + Praw_sc * (calibration_coefficients.c10 + Praw_sc *(calibration_coefficients.c20 + Praw_sc * calibration_coefficients.c30)) + Traw_sc * calibration_coefficients.c01 + Traw_sc * Praw_sc * (calibration_coefficients.c11 + Praw_sc * calibration_coefficients.c21);
            
            /* Temperature: 
            1. Read the calibration coefficients ( c0 and c1 ) from the Calibration Coefficients (COEF) register.
                Note: The coefficients read from the coefficient register are 12 bit 2´s complement numbers.
                2. Choose scaling factor kT (for temperature) based on the chosen precision rate. The scaling factors are listed in Table 7.
                3. Read the temperature result from the temperature register or FIFO.
                Note: The temperature measurements read from the temperature result register (or FIFO) are 24 bit 2´s complement numbers.
                4. Calculate scaled measurement results.
                Traw_sc = Traw/kT
                5. Calculate compensated measurement results
                Tcomp (°C) = c0*0.5 + c1*Traw_sc
            */
            this->Tcomp = calibration_coefficients.c0 * 0.5 + calibration_coefficients.c1 * Traw_sc;

        }
        else{
            Serial.println("Pressure or Temperature data not ready");
            return false;
        }
    }
    return true;
}

float SPL06_001::pressure() {
    return this->Pcomp;
}

float SPL06_001::temperature() {
    return this->Tcomp;
}
   
