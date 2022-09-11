
#ifndef cf_adxl357_h
#define cf_adxl357_h

#include <SPI.h>
#include <Arduino.h>


#define DEV_MEMS_REG_ADDR         0x01
#define DEV_ID_REG_ADDR           0x02
#define DEV_VERSION_ID_REG_ADDR   0x03
#define STATUS_REG_ADDR           0x04
#define FIFO_ENTRY_REG_ADDR       0x05
#define REG_TEMP_HIGH             0x06
#define REG_TEMP_LOW              0x07

#define ACTION_ENABLE_REG_ADDR    0x24
#define SET_THRESHOLD_REG_ADDR    0x25
#define GET_ACTIVE_COUNT_REG_ADDR 0x26
#define SET_INT_PIN_MAP_REG_ADDR  0x2a

#define X_DATA_REG_ADDR           0x08
#define FIFO_DATA_REG_ADDR        0x11

#define POWER_CTR_REG_ADDR        0x2d

#define TEMPERATURE_REG_ADDR      0x06
#define FILTER_REG_ADDR           0x28

#define SET_RANGE_REG_ADDR        0x2c
#define REG_SELF_TEST             0x2E
#define RESET_REG_ADDR            0x2f

#define READ_BYTE                 0x01
#define WRITE_BYTE                0x00

#define BUFFER_SIZE_PER_AXIS      32

#define SPI_FREQUENCY_HZ          10000000


typedef enum {
    TEN_G    = 1,
    TWENTY_G = 2,
    FOURTY_G = 3,
} T_adxl_range;

class adxl357
{
  public:
    adxl357() {};
    ~adxl357() {};

    // for initialisation
    void    setPins(uint8_t aPIN_MISO, uint8_t aPIN_MOSI, uint8_t aPIN_SCK, uint8_t aPIN_SS);
    void    resetSensor();
    void    init();
    void    writeRange(T_adxl_range newrange);
    void    enableSensor();
    void    writeFilter(uint8_t hpf_corner, uint8_t odr_lpf);
    uint8_t readDeviceID();
    uint8_t readDeviceVersion();
    void    measureOffset();

    // for loop
    uint8_t readStatus();
    float   readTemperature_C();
    uint8_t readFifoEntryCount();
    void    readAllFromFifo(int32_t xarr[], int32_t yarr[], int32_t zarr[], uint8_t *len);
    void    writeSelfTest(uint8_t val);

  private:
    int32_t PIN_MISO = -1;
    int32_t PIN_MOSI = -1;
    int32_t PIN_SCK  = -1;
    int32_t PIN_SS   = -1;
    
    int32_t offset[3];

    int32_t fifo_to_axisdata(uint32_t fifodata);
    void writeReg(uint8_t address, uint8_t value);
    unsigned int readReg(uint8_t address);
};

#endif