
#ifndef SPL06_001_H
#define SPL06_001_H

#include <Arduino.h>
#include <Wire.h>

#define SPL06_001_ADDR0             (0x77)
#define SPL06_001_ADDR1             (0x76)

// Register list
#define SPL06_001_REG_PSR_B2        (0x00)
#define SPL06_001_REG_PSR_B1        (0x01)
#define SPL06_001_REG_PSR_B0        (0x02)
#define SPL06_001_REG_TMP_B2        (0x03)
#define SPL06_001_REG_TMP_B1        (0x04)
#define SPL06_001_REG_TMP_B0        (0x05)
#define SPL06_001_REG_PSR_CFG       (0x06)
#define SPL06_001_REG_TMP_CFG       (0x07)
#define SPL06_001_REG_MEAS_CFG      (0x08)
#define SPL06_001_REG_CFG_REG       (0x09)
#define SPL06_001_REG_INT_STS       (0x0A)
#define SPL06_001_REG_FIFO_STS      (0x0B)
#define SPL06_001_REG_RESET         (0x0C)
#define SPL06_001_REG_ID            (0x0D)

// Calibration Coefficients (COEF)
#define SPL06_001_REG_COEF_C0       (0x10)
#define SPL06_001_REG_COEF_C0C1     (0x11)
#define SPL06_001_REG_COEF_C1       (0x12)
#define SPL06_001_REG_COEF_C00a     (0x13)
#define SPL06_001_REG_COEF_C00b     (0x14)
#define SPL06_001_REG_COEF_C00C10   (0x15)
#define SPL06_001_REG_COEF_C10a     (0x16)
#define SPL06_001_REG_COEF_C10b     (0x17)
#define SPL06_001_REG_COEF_C01a     (0x18)
#define SPL06_001_REG_COEF_C01b     (0x19)
#define SPL06_001_REG_COEF_C11a     (0x1A)
#define SPL06_001_REG_COEF_C11b     (0x1B)
#define SPL06_001_REG_COEF_C20a     (0x1C)
#define SPL06_001_REG_COEF_C20b     (0x1D)
#define SPL06_001_REG_COEF_C21a     (0x1E)
#define SPL06_001_REG_COEF_C21b     (0x1F)
#define SPL06_001_REG_COEF_C30a     (0x20)
#define SPL06_001_REG_COEF_C30b     (0x21)

class SPL06_001 {
    private:
        uint8_t _addr;
        TwoWire *_wire = NULL;
        bool write_reg(uint8_t reg, uint8_t *value, size_t len) ;
        bool write_reg(uint8_t reg, uint8_t value) ;
        bool read_reg(uint8_t reg, uint8_t *value, size_t len = 1) ;
        uint8_t read_reg(uint8_t reg) ;

    public:
        SPL06_001(uint8_t addr = SPL06_001_ADDR0, TwoWire *wire = &Wire) ;
        bool begin() ;
};

#endif
