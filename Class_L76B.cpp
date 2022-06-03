#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
//#include "Class_Graphics.cpp"


class cL76B{

    private:
        cGraphics *Gph          = nullptr;
        uart_inst *uartID       = uart1;
        uint32_t baudRate       = 9600;
        uint8_t dataBits        = 8;
        uint8_t stopBits        = 1;
        uart_parity_t parity    = UART_PARITY_NONE;
        uint8_t uartRXpin       = 5;
        uint8_t uartTXpin       = 4;
        
        uint32_t actualBaudRate = 0;
        uint32_t uartIRQ        = 0;

        irq_handler_t irqHandler = nullptr;
    public:
        bool isPositionREady    = false;        // have we recieved datagram with the position



    public: cL76B(irq_handler_t irqHandler){
        this->irqHandler = irqHandler;
        this->initialise();
    }

    public: cL76B(irq_handler_t irqHandler, uart_inst *uartID, uint8_t uartRXpin, uint8_t uartTXpin, uint32_t baudRate, uint8_t dataBits, uint8_t stopBits, uart_parity_t parity){
        this->irqHandler = irqHandler;
        this->uartID = uartID;
        this->uartRXpin = uartRXpin;
        this->uartTXpin = uartTXpin;
        this->baudRate = baudRate;
        this->dataBits = dataBits;
        this->stopBits = stopBits;
        this->parity = parity;
        this->initialise();
    }

    public: void setGph(cGraphics *Gph){
        this->Gph = Gph;
    }

    public: void initialise(){
        uart_init(this->uartID, this->baudRate);
        gpio_set_function(this->uartRXpin, GPIO_FUNC_UART);
        gpio_set_function(this->uartTXpin, GPIO_FUNC_UART);

        this->actualBaudRate = uart_set_baudrate(this->uartID, this->baudRate);

        uart_set_hw_flow(this->uartID, false, false);

        uart_set_format(this->uartID, this->dataBits, this->stopBits, this->parity);

        uart_set_fifo_enabled(this->uartID, false);

        this->uartIRQ = this->uartID == uart0 ? UART0_IRQ : UART1_IRQ;

        irq_set_exclusive_handler(this->uartIRQ, this->irqHandler);
        irq_set_enabled(this->uartIRQ, true);

        //uart_set_irq_enables(this->uartID, true, false);
    }

    public: void enable(){
        uart_set_irq_enables(this->uartID, true, false);
    }

    public: void onUartRx(){
        int i = 0;
        char *character;
        gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
        while (uart_is_readable(this->uartID)) {
            //char c   = uart_getc(this->uartID);
            //if(c == 10 || c == 13){c = '\n';} else { c='?';}
            //character[i] = uart_getc(this->uartID);
            i++;
            character = (char*)"piotr";
        }
        //character[i]='0';
        if(this->Gph!=nullptr){
            //sprintf(character, "char %d\n", sizeof(char));
            this->Gph->print(character);
            //sprintf(character, "uint8_t %d\n", sizeof(uint8_t));
            this->Gph->print(character);
        }
 
    }






};