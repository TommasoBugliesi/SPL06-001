
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

enum MeasurementRate {
    RATE_1_PER_SEC                  = 0b00000000, // 1 measurement per second 
    RATE_2_PER_SEC                  = 0b00000001, // 2 measurements per second
    RATE_4_PER_SEC                  = 0b00000010, // 4 measurements per second
    RATE_8_PER_SEC                  = 0b00000011, // 8 measurements per second
    RATE_16_PER_SEC                 = 0b00000100, // 16 measurements per second
    RATE_32_PER_SEC                 = 0b00000101, // 32 measurements per second
    RATE_64_PER_SEC                 = 0b00000110, // 64 measurements per second
    RATE_128_PER_SEC                = 0b00000111  // 128 measurements per second
};

enum OversamplingRate {
    SINGLE                          = 0b00000000, // Single oversampling
    TWO_TIMES                       = 0b00000001, // 2 times oversampling (Low Power)
    FOUR_TIMES                      = 0b00000010, // 4 times oversampling
    EIGHT_TIMES                     = 0b00000011, // 8 times oversampling
    SIXTEEN_TIMES                   = 0b00000100, // 16 times oversampling (Standard)
    THIRTY_TWO_TIMES                = 0b00000101, // 32 times oversampling
    SIXTY_FOUR_TIMES                = 0b00000110, // 64 times oversampling (High Precision)
    ONE_HUNDRED_TWENTY_EIGHT_TIMES  = 0b00000111  // 128 times oversampling
};

class SPL06_001 {
    private:
        // I2C setup
        uint8_t _addr;
        TwoWire *_wire = NULL;

        // Registers and configuration options
        uint8_t PRS_RDY         = 0;
        uint8_t TMP_RDY         = 0;
        uint8_t SENSOR_RDY      = 0;
        uint8_t COEF_RDY        = 0;
        bool    init_completed  = false;

        uint8_t p_measurement_rate = (RATE_8_PER_SEC << 4) & 0xFF;
        uint8_t p_oversampling_rate = SIXTY_FOUR_TIMES & 0xFF;
        uint8_t t_measurement_rate = (RATE_8_PER_SEC << 4) & 0xFF;
        uint8_t t_oversampling_rate = SIXTY_FOUR_TIMES & 0xFF;

        // Calibration 
        struct {
            int16_t c0, c1, c01, c11, c20, c21, c30;
            int32_t c00, c10;
        } calibration_coefficients;

        // Measure
        const uint32_t scale_factor[8] = {
            524288, 1572864, 3670016, 7864320, 253952, 516096, 1040384, 2088960 // Ref. Table 7 : Compensation Scale Factors
        };
        float Pcomp = -1, Tcomp = -1;

        // Private Methods
        bool write_reg(uint8_t reg, uint8_t *value, size_t len) ;
        bool write_reg(uint8_t reg, uint8_t value) ;
        bool read_reg(uint8_t reg, uint8_t *value, size_t len = 1) ;
        uint8_t read_reg(uint8_t reg) ;

        bool status();

    public:
        // Public Methods
        SPL06_001(uint8_t addr = SPL06_001_ADDR0, TwoWire *wire = &Wire) ;
        bool begin() ;        
        bool measure();
        
        float pressure() ;
        float temperature() ;
};

#endif
