#include "ILI9341.h"
#include "hardware/spi.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"



class cIli9341{

    public:
        uint8_t *buffer8 = NULL;                        // 8 bit pointer to screen buffer
        uint16_t *buffer16 = NULL;                      // 16 bit pointer to screen buffer 
        uint32_t *buffer32 = NULL;                      // 32 bit pointer to screen buffer
        uint16_t displayWidth = 240;                    // physical dimentions of the display
        uint16_t displayHeight = 320;
        uint16_t workWidth = 0;                         // size of the display buffer
        uint16_t workHeight = 0;
        uint16_t workX = 0;                             // display buffer location on the screen
        uint16_t workY = 0;
        uint8_t colorDepth = 2;                         // how many bytes per pixel
        uint16_t fps = 0;                               // if statistics enabled - shows fps values

    //private: 
        spi_inst* spiPort;
        uint8_t pinRST = 9;       
        uint8_t pinCS = 13;          
        uint8_t pinDC = 14;        
        uint8_t pinSCLK = 10;       
        uint8_t pinMOSI = 11;      
        uint8_t pinMISO = 12;      
        uint8_t pinLED = 15;      
        bool sendingScreenData = false;
        bool refreshScreenEnabled = true; 
        bool screenRefreshed = false;
        int  frameCount = 0;                            // used to calculate fps value
        int clockSys = 133 * MHZ;                       // system clock in Hz
        int clockSPI = 66.5 * MHZ;                      // SPI clock in Hz
        bool clockSysResult = false;                    // true if clock was setup successfuly
     
    public: cIli9341(spi_inst* spiPort, uint16_t displayWidth, uint16_t displayHeight){
        this->spiPort = spiPort;
        this->displayWidth = displayWidth;
        this->displayHeight = displayHeight;
    }

    public: cIli9341(spi_inst* spiPort, float sysClock, float clockSPI, int pinRST, int pinCS, int pinDC, int pinSCLK, int pinMOSI, int pinMISO, int pinLED){
        this->clockSys = clockSys;              // value in MHz
        this->clockSPI = clockSPI;              // value in MHz
        this->spiPort = spiPort;
        this->pinRST = pinRST;
        this->pinCS = pinCS;
        this->pinDC = pinDC;
        this->pinSCLK = pinSCLK;
        this->pinMOSI = pinMOSI;
        this->pinMISO = pinMISO;
        this->pinLED = pinLED;
        this->setupClocks();
    }

    public: uint16_t* createWorkArea(int workX, int workY, int workWidth, int workHeight){
        // data buffer will be created for the area of that size
        this->workX = workX;
        this->workY = workY;
        this->workWidth = workWidth;
        this->workHeight = workHeight; 
        // create display frame buffer now
        if(this->buffer16!=NULL){           // previous screen buffer must be deleted if existed
            free(this->buffer16);
            this->buffer16 = NULL;
        }
        this->buffer16 = (uint16_t*) calloc((this->workWidth*this->workHeight + 2), sizeof(uint16_t));      // last 2 * 16 bits for safety in case 32bit operations were performed on the array
        this->updateBufferPointers();
        return this->buffer16;
    }

    public: uint16_t* deleteWorkArea(){
        // deletes frame buffer
        this->workX = 0;
        this->workY = 0;
        this->workWidth = 0;
        this->workHeight = 0;
        if(this->buffer16!=NULL){
            free(buffer16);
        }
        buffer16 = NULL;
        this->updateBufferPointers();
        return NULL;
    }

    private: void updateBufferPointers(){
        this->buffer8 = (uint8_t*) this->buffer16;
        this->buffer32 = (uint32_t*) this->buffer16;
    }

    public: void setupClocks(float clockSys, float clockSPI){
        // clockSys and clockSPI - values should be given in MHZ
        this->clockSys = clockSys * MHZ;
        this->clockSPI = clockSPI * MHZ;
        this->setupClocks();
    }

    public: void setupClocks(){
        this->clockSysResult =  set_sys_clock_khz(this->clockSys / 1000, true);
        clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, this->clockSys, this->clockSys);
        this->clockSPI = spi_init(this->spiPort, this->clockSPI);
    }

    public: void initialise(){
        this->initialisePins();
        this->csDisable();
        this->csEnable();
        this->reset();
        this->writeCmd(4, 100, 0xEF, 0x03, 0x80, 0x02);
        this->writeCmd(4, 100, 0xCF, 0x00, 0xC1, 0x30);
        this->writeCmd(5, 100, 0xED, 0x64, 0x03, 0x12, 0x81);
        this->writeCmd(4, 100, 0xE8, 0x85, 0x00, 0x78);
        this->writeCmd(6, 100, 0xCB, 0x39, 0x2C, 0x00, 0x34, 0x02);
        this->writeCmd(2, 100, 0xF7, 0x20);
        this->writeCmd(3, 100, 0xEA, 0x00, 0x00);
        this->writeCmd(2, 100, PWCTR1  , 0x23);             // Power control VRH[5:0]
        this->writeCmd(2, 100, PWCTR2  , 0x10);             // Power control SAP[2:0];BT[3:0]
        this->writeCmd(3, 100, VMCTR1  , 0x3e, 0x28);       // VCM control
        this->writeCmd(2, 100, VMCTR2  , 0x86);             // VCM control2
        this->writeCmd(2, 100, MADCTL  , 0x48);             // Memory Access Control
        this->writeCmd(2, 100, VSCRSADD, 0x00);             // Vertical scroll zero
        this->writeCmd(2, 100, PIXFMT  , 0x55);
        this->writeCmd(3, 100, FRMCTR1 , 0x00, 0x18);
        this->writeCmd(4, 100, DFUNCTR , 0x08, 0x82, 0x27); // Display Function Control
        this->writeCmd(2, 100, 0xF2, 0x00);                 // 3Gamma Function Disable
        this->writeCmd(2, 100, GAMMASET , 0x01);            // Gamma curve selected
        this->writeCmd(16, 100, GMCTRP1 , 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00);// Set Gamma
        this->writeCmd(16, 100, GMCTRN1 , 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F);// Set Gamma
        this->writeCmd(2, 100, SLPOUT  , 0x80);             // Exit Sleep
        this->writeCmd(2, 100, DISPON  , 0x80);             // Display on
        this->writeCmd(1, 100, 0x00);                       // End of list
        this->bckOn();
    }

    private: void initialisePins(){
        gpio_set_function(pinSCLK, GPIO_FUNC_SPI);
        gpio_set_function(pinMOSI, GPIO_FUNC_SPI);
        gpio_set_function(pinMISO, GPIO_FUNC_SPI);
        gpio_init(pinDC);
        gpio_set_dir(pinDC, GPIO_OUT);
        gpio_init(pinLED);
        gpio_set_dir(pinLED, GPIO_OUT);
        gpio_init(pinRST);
        gpio_set_dir(pinRST, GPIO_OUT);
        gpio_init(pinCS);
        gpio_set_dir(pinCS, GPIO_OUT);
        gpio_init(pinLED);
        gpio_set_dir(pinLED, GPIO_OUT);
    }

    private: void setDataMode(){
        gpio_put(pinDC, 1);
        return;
    }

    private: void setCmdMode(){
        gpio_put(pinDC, 0);
        return;
    }

    private: void bckOn(){
        // backlight on
        gpio_put(pinLED, 1);
        return;
    }

    private: void bckOff(){
        // backlight off
        gpio_put(pinLED, 0);
        return;
    }

    private: void reset(){
        gpio_put(pinRST, 0);
        sleep_ms(10);
        gpio_put(pinRST, 1);
        sleep_ms(50);
        return;
    }

    private: void csEnable(){
        gpio_put(pinCS, 0);
        return;
    }

    private: void csDisable(){
        gpio_put(pinCS, 1);
    }

    public: void statTick(){
        // call this method every second for statistics to be active
        this->fps = this->frameCount;
        this->frameCount = 0;
    }

    private: void writeCmd(int num, ...){
        // writes to spi port and than waits requested time
        // parameters are: (number of variables, delay, n variables)
        // values should be 8bit
        num++;
        int delay = 0;
        uint8_t data[1];
        this->csEnable();
        this->setCmdMode();
        //initialise variable list
        va_list valist;
        //initialize valist for num number of arguments
        va_start(valist, num);
        //send all data from valist
        for(int i=0; i<num; i++){
            data[0] = va_arg(valist, int);
            if(i==0){delay = data[0]; continue;}  // read delay value
            spi_write_blocking(spiPort, data, 1);
            this->setDataMode();
        }
        //clean memory for valist
        va_end(valist);
    }

    private: void writeCmd16(int num, ...){
        // writes to spi port 
        // parameters are: (number of variables, delay, n variables)
        // values should be 16bit, command code is 8bit
        num++;
        int delay = 0;
        uint8_t data8[2];
        uint16_t *data16 = (uint16_t*) data8;
        this->csEnable();
        this->setCmdMode();
        //initialise variable list
        va_list valist;
        //initialize valist for num number of arguments
        va_start(valist, num);
        //send all data from valist
        for(int i=0; i<num; i++){
            data16[0] = va_arg(valist, int);
            if(i==0){delay = data16[0]; continue;}  // read delay value
            if(i==1){
                spi_write_blocking(this->spiPort, data8, 1);  
                this->setDataMode();
                continue;
            }  // this is the command value, and it is 8 bit
            // swap LSB with MSB
            uint8_t temp8 = data8[0];
            data8[0] = data8[1];
            data8[1] = temp8;
            spi_write_blocking(this->spiPort, data8, 2);
        }
        //clean memory for valist
        va_end(valist);
    }

    public: void refresh(){
        //writes the screen buffer to the display
        if (this->refreshScreenEnabled&&!this->sendingScreenData){ 
            this->sendingScreenData = true;
            this->writeCmd16(3, 0, CASET, this->workX, this->workWidth-1 + this->workX);   
            this->writeCmd16(3, 0, PASET, this->workY, this->workHeight-1 + this->workY);
            this->writeCmd(1, 0, RAMWR);       // Data Write Command to the display
            this->setDataMode();
            this->screenRefreshed = true;
            this->frameCount++;
            spi_write_blocking(this->spiPort, this->buffer8, this->workWidth*this->workHeight*this->colorDepth);
            this->sendingScreenData = false;
        }  // only refresh screen when screenBuffer ready
    }

    public: void clearDisplay(){
        // clears all physical area of the display
        uint8_t *tempBuffer8;
        tempBuffer8 = (uint8_t*) calloc(this->displayWidth, 2);
        for(uint16_t i=0; i<this->displayWidth; i++){tempBuffer8[i]=0;}      // clear the buffer
        for(uint16_t j=0; j<this->displayHeight; j++){                      // clear the display line by line
            this->writeCmd16(3, 0, CASET, 0, this->displayWidth-1);
            this->writeCmd16(3, 0, PASET, j, j);
            this->writeCmd(1, 0, RAMWR);       // Data Write Command to the display
            this->setDataMode();
            spi_write_blocking(this->spiPort, tempBuffer8, this->displayWidth*2);
        }
        free(tempBuffer8);
    }

    public: void disableRefreshingScreen(){
        while(this->sendingScreenData/*||!this->screenRefreshed*/){
            // wait till screen refreshing is finnished 
            // and screen was refreshed at least once 
        }
        this->refreshScreenEnabled = false;
    }

    public: void enableRefreshingScreen(){
        this->refreshScreenEnabled = true;
        this->screenRefreshed = false;
    }

    //tutaj zmiana w testowym branchu
};