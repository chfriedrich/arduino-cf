
#include "cf_adxl357.h"
#include <SPI.h>

void adxl357::init()
{
    if(PIN_MISO!=-1 && PIN_MOSI!=-1 && PIN_SCK!=-1 && PIN_SS!=-1)
    {
        // use custom spi pins
        SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_SS);
    }
    else
    {
        // use default spi pins (from arduino board definition file)
        SPI.begin();
    }

    // Slave select pin initialisieren
    pinMode(PIN_SS, OUTPUT);
    digitalWrite(PIN_SS, HIGH);

    offset[0] = offset[1] = offset[2] = 0;

    resetSensor();
}

void adxl357::setPins(uint8_t aPIN_MISO, uint8_t aPIN_MOSI, uint8_t aPIN_SCK, uint8_t aPIN_SS)
{
    PIN_MISO = aPIN_MISO;
    PIN_MOSI = aPIN_MOSI;
    PIN_SCK  = aPIN_SCK;
    PIN_SS   = aPIN_SS;
}

void adxl357::writeRange(T_adxl_range newrange)
{
    uint8_t temp = readReg(SET_RANGE_REG_ADDR);
    // bit 0 and 1 are range bits
    temp &= 0xFC;
    temp |= newrange;
    writeReg(SET_RANGE_REG_ADDR, temp);
}

void adxl357::resetSensor()
{   
    writeReg(RESET_REG_ADDR, 0x52);
    
    /*
    for(uint8_t i=0; i<32; i++)
    {
        xdata_raw[i] = 0;
        ydata_raw[i] = 0;
        zdata_raw[i] = 0;
    }
    */
}

// enable temperature and acceleration measurement
void adxl357::enableSensor()
{
    uint8_t temp = readReg(POWER_CTR_REG_ADDR);
    // bit 0 ... 1 for standby mode, 0 for measurement mode
    // bit 1 ... 1 for temperature measurement disable, 0 for enable
    temp &= 0xFC;
    writeReg(POWER_CTR_REG_ADDR, temp);
}

// returns acceleration data in degrees celsius
float adxl357::readTemperature_C()
{
    uint8_t hbyte = readReg(REG_TEMP_HIGH);
    uint8_t lbyte = readReg(REG_TEMP_LOW);
    uint16_t temp = ((uint16_t)hbyte << 8) + (uint16_t)lbyte;
    return 25.0 + (float)(temp - 1852) / (-9.05);
}

// for setting low and high pass filters (see datasheet page 38)
// after reset, all filters are disabled
void adxl357::writeFilter(uint8_t hpf_corner, uint8_t odr_lpf)
{
    uint8_t temp = (hpf_corner << 4) | odr_lpf;
    writeReg(FILTER_REG_ADDR, temp);
}

uint8_t adxl357::readFifoEntryCount()
{
    return readReg(FIFO_ENTRY_REG_ADDR);
}

void adxl357::writeSelfTest(uint8_t val)
{
    writeReg( REG_SELF_TEST, val & 0x03 );
}

// converts one fifo entry to acceleration data for one axis
int32_t adxl357::fifo_to_axisdata(uint32_t fifodata)
{
    // the right 4 bits are empty or indicators, remove them
    int32_t val = (fifodata >> 4) & 0xFFFFF;

    // convert unsigned to signed
    if (val & 0x80000)
        val = (val & 0x7ffff) - 0x80000;

    return val;
}

uint8_t adxl357::readDeviceID()
{
    return readReg(DEV_ID_REG_ADDR);
}

uint8_t adxl357::readDeviceVersion()
{
    return readReg(DEV_VERSION_ID_REG_ADDR);
}

uint8_t adxl357::readStatus()
{
    return readReg(STATUS_REG_ADDR);
}

// ###################################################

// write single 8 bit register
void adxl357::writeReg(uint8_t address, uint8_t value)
{
  uint8_t dataToSend = (address << 1) | WRITE_BYTE;
  SPI.beginTransaction(SPISettings(SPI_FREQUENCY_HZ, MSBFIRST , SPI_MODE0));
  digitalWrite(PIN_SS, LOW);
  SPI.transfer(dataToSend);
  SPI.transfer(value);
  digitalWrite(PIN_SS, HIGH);
  SPI.endTransaction();
}

// read single 8 bit register
unsigned int adxl357::readReg(uint8_t address)
{
  unsigned int result = 0;
  uint8_t dataToSend = (address << 1) | READ_BYTE;
  SPI.beginTransaction(SPISettings(SPI_FREQUENCY_HZ, MSBFIRST , SPI_MODE0));
  digitalWrite(PIN_SS, LOW);
  SPI.transfer(dataToSend);
  result = SPI.transfer(0x00);
  digitalWrite(PIN_SS, HIGH);
  SPI.endTransaction();
  return result;
}

// ###################################################

void adxl357::measureOffset()
{
    offset[0] = offset[1] = offset[2] = 0;
    int32_t count = 0;
    int64_t xsum = 0;
    int64_t ysum = 0;
    int64_t zsum = 0;

    int32_t st = micros();
    while(count < 5000)  // 5000 samples for averaging
    {
        int32_t t = micros();
        if(t > st+2000)
        {
            st = t;
            int32_t xarr[BUFFER_SIZE_PER_AXIS];
            int32_t yarr[BUFFER_SIZE_PER_AXIS];
            int32_t zarr[BUFFER_SIZE_PER_AXIS];
            uint8_t len;
            readAllFromFifo(xarr, yarr, zarr, len);
            for(int i=0; i<len; i++)
            {
                xsum += xarr[i];
                ysum += yarr[i];
                zsum += zarr[i];
            }
            count += len;
        }
    }
    offset[0] = xsum / (int64_t)count;
    offset[1] = ysum / (int64_t)count;
    offset[2] = zsum / (int64_t)count;

    Serial.println("x offset: " + String(offset[0]));
    Serial.println("y offset: " + String(offset[1]));
    Serial.println("z offset: " + String(offset[2]));
}

void adxl357::readAllFromFifo(int32_t xarr[], int32_t yarr[], int32_t zarr[], uint8_t &len)
{
    // Read all data from fifo
    uint32_t fifodata[96];
    SPI.beginTransaction(SPISettings(SPI_FREQUENCY_HZ, MSBFIRST , SPI_MODE0));
    digitalWrite(PIN_SS, LOW);
    SPI.transfer( (FIFO_DATA_REG_ADDR << 1) | READ_BYTE );

    uint8_t fiforeg_cnt = 0;
    while(fiforeg_cnt < 96)
    {
        uint32_t fiforeg = 0;
        for(int i=0; i<3; i++)
        {
            uint8_t readout = SPI.transfer(0x00);
            fiforeg |= ( (uint32_t)readout << ((2-i)*8) );
        }

        // check fifo empty indicator
        if( fiforeg & 0x02 )
        {
            break;
        }
        else
        {
            fifodata[fiforeg_cnt] = fiforeg;
            fiforeg_cnt++;
        }
    }
    digitalWrite(PIN_SS, HIGH);
    SPI.endTransaction();

    // Transaction complete
    // sort data
    // fifodata, [0] enthält die ältesten daten,
    // [fiforeg_cnt-1] die neuesten
    // [0] hat immer am niederwertigsten bit eine 1, ist daher immer die x achse
    // [1] ist immer y
    // [2] ist immer z, usw.

    int8_t i = 0;
    int8_t count = 0;
    while( i < fiforeg_cnt )
    {
        xarr[count] = fifo_to_axisdata(fifodata[i])   - offset[0];
        yarr[count] = fifo_to_axisdata(fifodata[i+1]) - offset[1];
        zarr[count] = fifo_to_axisdata(fifodata[i+2]) - offset[2];
        i += 3;
        count++;
    }
    len = count;
}
