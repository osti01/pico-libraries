#include <string>
#include "Graphics.h"
#include "colors.h"
#include "ATASCII.h"

class cGraphics{

    public:


    private:
        uint16_t    *buffer16;              // screen Buffer from received Display Driver Class
        char        *textBuffer;            // text buffer for the text display mode
        uint32_t    *textColorBuffer;       // it contains 16b character color / 16b character background color
        uint8_t     *textTranspBuffer;      // it contains 1b character opacity / 1b character background opacity

        uint8_t     screenOrientation;
        uint16_t    displayWidth, displayHeight; 
        uint16_t    iSize, jSize;           // text mode screen dimentions
        uint16_t    corX, corY;             // in the text mode allows to calculate character position correcty depending on screen orientation
        int         iToX, iToY, jToX, jToY; // in the text mode convert screen coordinates depending on the orientation 
        uint8_t     charCorX, charCorY;     // in the text mode alows to correctly display 8x8 fonts in certain screen orientations
        

        uint16_t    cursPos             = 0;        //cursor position for the text mode
        uint16_t    fontColor           = YELLOW;
        uint16_t    backColor           = BLACK;
        bool        fontTransp          = false;
        bool        backTransp          = false;
        bool        cursorVisible       = false;

        uint16_t    windowLeftBrd, windowRightBrd, windowTopBrd, windowBottomBrd;       // working area for safe drawing functions


    public: cGraphics(uint16_t *buffer16, uint8_t screenOrientation,  uint16_t displayWidth, uint16_t displayHeight){
        this->buffer16 = buffer16;
        this->screenOrientation = screenOrientation;
        this->displayWidth = displayWidth;
        this->displayHeight = displayHeight;

        this->setWindowSize(0, this->displayWidth-1, 0, this->displayHeight-1);
        this->clearAll();
        this->textBuffer = (char*)malloc(this->getNumberOfCharecters()*sizeof(char)+2);
        this->textColorBuffer = (uint32_t*)malloc(this->getNumberOfCharecters()*sizeof(uint32_t)+2);
        this->textTranspBuffer = (uint8_t*)malloc(this->getNumberOfCharecters()*sizeof(uint8_t)/4+2);        // every character transparency is encoded on 2bits
        this->clearTextBuffer();
        this->setOrientation(this->screenOrientation);

    }

    public: void setWindowSize(uint16_t leftBrd, uint16_t rightBrd, uint16_t topBrd, uint16_t bottomBrd){
        this->windowLeftBrd     = leftBrd;
        this->windowRightBrd    = rightBrd;
        this->windowTopBrd      = topBrd;
        this->windowBottomBrd   = bottomBrd;
    }

    public: void setOrientation(uint8_t screenOrientation){
        this->screenOrientation = screenOrientation;
        switch(screenOrientation){
            case GPH_ORI_UP:
                this->iSize = this->displayWidth/8;
                this->jSize = this->displayHeight/8;
                this->iToX = 1; this->iToY = 0; this->jToY = 1; this->jToX = 0;
                this->corX = 0; this->corY = 0;
                this->charCorX = 0; this->charCorY = 0;
            break;
            case GPH_ORI_DN:
                this->iSize = this->displayWidth/8;
                this->jSize = this->displayHeight/8;
                this->iToX = -1; this->iToY = 0; this->jToY = -1;this-> jToX = 0;
                this->corX = this->iSize - 1; this->corY = this->jSize - 1;
                this->charCorX = 7; this->charCorY = 7;
            break;
            case GPH_ORI_LT:
                this->iSize = this->displayHeight/8;
                this->jSize = this->displayWidth/8;
                this->iToX = 0; this->iToY = -1; this-> jToY = 0; this-> jToX = 1;
                this->corX = 0; this->corY = this->iSize - 1;
                this->charCorX = 0; this->charCorY = 7;
            break;
            case GPH_ORI_RT:
                this->iSize = this->displayHeight/8;
                this->jSize = this->displayWidth/8;
                this->iToX = 0; this->iToY = 1; this->jToY = 0; this->jToX = -1;
                this->corX = this->jSize - 1; this->corY = 0;
                this->charCorX = 7; this->charCorY = 0;
            break;
        }
    }

    public: void clearAll(){
        uint32_t *buffer = (uint32_t*) this->buffer16;
        long size = this->displayWidth*this->displayHeight/2;
        for(long i=0; i<size; i++){
            buffer[i] = 0x00000000;
        }
    }

    public: void fadeScreen(){
        uint16_t *buffer = (uint16_t*) this->buffer16;
        long size = this->displayWidth*this->displayHeight;
        for(long i=0; i<size; i++){
            buffer[i] = this->colorFix((this->colorFix(buffer[i]) & 0b11110111110111101111011111011110) >> 1);
        }
    }

    public: void plot(int x, int y, uint16_t color){
        this->buffer16[x + this->displayWidth * y] = this->colorFix(color);
    }

    public: void sPlot(int x, int y, uint16_t color){
        if(x>=this->windowLeftBrd && x<=this->windowRightBrd && y>=this->windowTopBrd && y<=this->windowBottomBrd){
            this->plot(x, y, color);
        }
    }

    public: void line(int x1, int y1, int x2, int y2, uint16_t color){
        // Extremely Fast Line Algorithm Var E (Addition Fixed Point PreCalc ModeX)
        // Copyright 2001-2, By Po-Han Lin
        // Freely useable in non-commercial applications as long as credits 
        // to Po-Han Lin and link to http://www.edepot.com is provided in 
        // source code and can been seen in compiled executable.  
        // Commercial applications please inquire about licensing the algorithms.
        //
        // Lastest version at http://www.edepot.com/phl.html
        // Note: This version is for small displays like on cell phones.
        // with 256x256 max resolution.  For displays up to 65536x65536
        // please visit http://www.edepot.com/linee.html

        color = this->colorFix(color);
       	bool yLonger=false;
    	int shortLen=y2-y1;
    	int longLen=x2-x1;
    	if (abs(shortLen)>abs(longLen)) {
    		int swap=shortLen;
    		shortLen=longLen;
    		longLen=swap;				
    		yLonger=true;
    	}
    	int decInc;
    	if (longLen==0) decInc=0;
    	else decInc = (shortLen << 8) / longLen;

    	if (yLonger) {
    		if (longLen>0) {
    			longLen+=y1;
    			for (int j=0x80+(x1<<8);y1<=longLen;++y1) {
                    this->buffer16[(j>>8) + this->displayWidth * y1] = color;
    				j+=decInc;
    			}
    			return;
    		}
    		longLen+=y1;
    		for (int j=0x80+(x1<<8);y1>=longLen;--y1) {
                this->buffer16[(j>>8) + this->displayWidth * y1] = color;
    			j-=decInc;
    		}
    		return;	
    	}

    	if (longLen>0) {
    		longLen+=x1;
    		for (int j=0x80+(y1<<8);x1<=longLen;++x1) {
                this->buffer16[x1 + this->displayWidth * (j >> 8)] = color;
    			j+=decInc;
    		}
    		return;
    	}
    	longLen+=x1;
    	for (int j=0x80+(y1<<8);x1>=longLen;--x1) {
            this->buffer16[x1 + this->displayWidth * (j >> 8)] = color;
    		j-=decInc;
    	}
    }

    public: void sLine(int x1, int y1, int x2, int y2, uint16_t color){
        int xLen, yLen;
        bool lineDrawable = false;
        //bool horizontal = false;
        if( !((x1<this->windowLeftBrd && x2<this->windowLeftBrd)            //checking if both points are outside drawing area
            || (x1>this->windowRightBrd && x2>this->windowRightBrd)         // on the same side of it
            || (y1<this->windowTopBrd&&y2<this->windowTopBrd)               // if thats the case 
            || (y1>this->windowBottomBrd&&y2>this->windowBottomBrd))){      // nothing to draw
            xLen = x2-x1;
            yLen = y2-y1;
            if(xLen==0 && yLen==0){
                // Line is just a point
                this->sPlot(x1, y1, color);
            }else{
                // Line has length > 0
                if(abs(xLen)>=abs(yLen)){
                    // line is "horizontal"
                    //horizontal = true;
                    lineDrawable = false;
                    lineDrawable = this->sFindFirstPnt(&x1, &y1, &x2, &y2, this->windowLeftBrd, this->windowRightBrd, this->windowTopBrd, this->windowBottomBrd);
                    if(lineDrawable){
                        this->sFindScndPnt(&x1, &y1, &x2, &y2, this->windowLeftBrd, this->windowRightBrd, this->windowTopBrd, this->windowBottomBrd);
                    }
                }else{
                    // line is "vertical"
                    lineDrawable = false;
                    lineDrawable = this->sFindFirstPnt(&y1, &x1, &y2, &x2, this->windowTopBrd, this->windowBottomBrd, this->windowLeftBrd, this->windowRightBrd);
                    if(lineDrawable){
                        this->sFindScndPnt(&y1, &x1, &y2, &x2, this->windowTopBrd, this->windowBottomBrd, this->windowLeftBrd, this->windowRightBrd);
                    }
                }
            }
        }
        if(lineDrawable){this->line(x1, y1, x2, y2, color);} 
    }

    private: void sortOnI(int *i1, int *j1, int *i2, int *j2){
        // orders coordinates so that: i1 < i2
        if(*i1>*i2){
            // reordering coordinates
            int ti = *i1;
            int tj = *j1;
            *i1 = *i2;
            *j1 = *j2;
            *i2 = ti;
            *j2 = tj;
        }
    }

    private: inline bool isOnScreen(int i, int j, int iMin, int iMax, int jMin, int jMax){
        bool result = false;
        if(i>=iMin && i<=iMax && j>=jMin && j<=jMax){
            result = true;
        }
        return result;
    }

    private: bool sFindFirstPnt(int *i1, int *j1, int *i2, int *j2, uint16_t iMin, uint16_t iMax, uint16_t jMin, uint16_t jMax){
        // this function finds the first safe to draw point in the window if it exists 
        // to be able to work both for "horisontal" and "vertical" lines it uses i & j coordinates, rather that x & y
        // returns true if the point was found
        bool result = false;
        float a, b;
        this->sortOnI(i1, j1, i2, j2);
        a = (float)((*j2)-(*j1))/(float)((*i2)-(*i1));          
        b = ((*j1)-a*(*i1));                              
        if(this->isOnScreen(*i1, *j1, iMin, iMax, jMin, jMax)){
            result = true;
        }else{
            // must keep looking for the first point
            if((*i1)<=iMax){                            // if point before right border of the window
                *i1 = iMin;
                *j1 = a*(*i1) + b;
                if(this->isOnScreen(*i1, *j1, iMin, iMax, jMin, jMax)){result = true;}else{
                    // point was not found on the left border, time to check top and bottom borders
                    if(a!=0){                           // if a=0 the line does not cross top nor bottom
                        if(a>0){
                            *j1 = jMin;                 // check the top border
                            *i1 = ((*j1)-b)/a;
                            if(this->isOnScreen(*i1, *j1, iMin, iMax, jMin, jMax)){result = true;}
                        }else{
                            *j1 = jMax;                 // check the bottom border
                            *i1 = ((*j1)-b)/a;      
                            if(this->isOnScreen(*i1, *j1, iMin, iMax, jMin, jMax)){result=true;}
                        }
                    }
                }
            }
        }  
        return result;
    }

    private: void sFindScndPnt(int *i1, int *j1, int *i2, int *j2, uint16_t iMin, uint16_t iMax, uint16_t jMin, uint16_t jMax){
        // this function finds the second safe to draw point in the window if it exists (and if the first was found, this must surely exist)
        // to be able to work both for "horisontal" and "vertical" lines it uses i & j coordinates, rather that x & y
        // returns true if the point was found
        float a, b;
        if(!this->isOnScreen(*i2, *j2, iMin, iMax, jMin, jMax)){
            a = (float)((*j2)-(*j1))/(float)((*i2)-(*i1));                  
            b = ((*j1)-a*(*i1));                                            
            if(*i2>=iMin){                                                           // if point after left border of the window
                *i2 = iMax;
                *j2 = a*(*i2)+b;
                if(!this->isOnScreen(*i2, *j2, iMin, iMax, jMin, jMax)){              // if end point is not on the right border
                    if(a!=0){                                                       // if a=0 the line does not cross top nor bottom
                        if(a>0){
                            *j2 = jMax;                                            // check the jMax border
                            *i2 = ((*j2)-b)/a;
                        }else{
                            *j2 = jMin;                                            // check the jMin border
                            *i2 = ((*j2)-b)/a;      
                        }
                    }
                }
            }
        }
    }

    public: void rectangle(int x1, int y1, int width, int height, uint16_t color){
        int x, y;
        int x2 = x1 + width;
        int y2 = y1 + height;
        color = colorFix(color);
        for (x=x1; x<x2; x++){
            this->buffer16[y1 * this->displayWidth + x] = color;
            this->buffer16[y2 * this->displayWidth + x] = color;
        }
        for (y=y1; y<y2; y++){
            this->buffer16[y * this->displayWidth + x1] = color;
            this->buffer16[y * this->displayWidth + x2] = color;
        }
    }

    public: void sRectangle(int x1, int y1, int width, int height, uint16_t color){
        bool drawX1 = true;
        bool drawX2 = true;
        bool drawY1 = true;
        bool drawY2 = true;
        int x, y;
        int x2 = x1 + width;
        int y2 = y1 + height;
        color = colorFix(color);
        if(!((x1<this->windowLeftBrd && x2<this->windowLeftBrd)                             // checking if both points are outside drawing area
            || (x1>this->windowRightBrd && x2>this->windowRightBrd)                         // on the same side of it
            || (y1<this->windowTopBrd&&y2<this->windowTopBrd)                               // if thats the case 
            || (y1>this->windowBottomBrd&&y2>this->windowBottomBrd))){                      // nothing to draw                  
            if(x1<this->windowLeftBrd){   x1=this->windowLeftBrd;    drawX1 = false;}       // check is x1 border vissible
            if(x2>this->windowRightBrd){  x2=this->windowRightBrd;   drawX2 = false;}       // check is x2 border vissible
            if(y1<this->windowTopBrd){    y1=this->windowTopBrd;     drawY1 = false;}       // check is y1 border vissible
            if(y2>this->windowBottomBrd){ y2=this->windowBottomBrd;  drawY2 = false;}       // check is y2 border vissible

            if(drawX1){
                for(y=y1; y<=y2; y++){
                    this->buffer16[y * this->displayWidth + x1] = color;
                }
            }

            if(drawX2){
                for(y=y1; y<=y2; y++){
                    this->buffer16[y * this->displayWidth + x2] = color;
                }
            }

            if(drawY1){
                for(x=x1; x<=x2; x++){
                    this->buffer16[y1 * this->displayWidth + x] = color;
                }
            }

            if(drawY2){
                for(x=x1; x<=x2; x++){
                    this->buffer16[y2 * this->displayWidth + x] = color;
                }
            }
        }
    }    

    private: uint16_t colorFix(uint16_t color){
        return (color >> 8 | color << 8);
    } 

    // ********************************************************
    // ********   Text Mode Methods from this point   ********
    // ********************************************************


    public: void print(std::string text, uint16_t color, uint16_t bColor, bool transp, bool bTransp){
        // puts null terminated string into display's text buffer
        // int size = this->getNumberOfCharecters();
        int i = 0;
        uint8_t transparency; 
        while(text[i]!=0){
            if(text[i]==10){
                this->cursorNextLine();
            }else{
                this->textBuffer[this->cursPos] = text[i];
                this->textColorBuffer[this->cursPos] = (uint32_t) color | (uint32_t) bColor << 16;   // make 32b value of 2 x 16bit color value
                transparency = this->textTranspBuffer[this->cursPos >> 2];          // read transparency Byte for 4 letters
                transparency = transparency & ~(0b11 << ((this->cursPos%4)*2));     // clear 2 bits for transparency for the current letter to 0
                transparency = transparency | ((transp | (bTransp << 1))<<((this->cursPos%4)*2));     // set bits acordingly to transp and bTransp variables
                this->textTranspBuffer[this->cursPos >> 2] = transparency;

                this->cursPos++;
            }
            this->cursorFix();       // check if cursor is outside textBuffer - if yes add new line and put the cursor on the begining of it
            i++;
        }
        //gph_dispTextBuffer();
    }

    public: void print(std::string sText){
        this->print(sText, this->fontColor, this->backColor, this->fontTransp, this->backTransp);
    }

    public: void cursorShow(){
        this->cursorVisible = true;
    }

    public: void cursorHide(){
        this->cursorVisible = false;
    }
    
    public: void cursorSet(int i, int j){
        if(i>=0 && i<this->iSize && j>=0 && j<this->jSize){
            this->cursPos = i + j*this->iSize;
        }
    }

    private: void cursorPrint(){
        //this->cursorFix();
        this->displayCharacter('_', (this->cursPos%this->iSize)*8, (this->cursPos/this->iSize)*8, this->fontColor, this->backColor, false, true);
    }

    public: void cursorNextLine(){
        // moves cursor position to the begining of next line, but does not check if it is outside the buffer
        this->cursPos = ((this->cursPos/this->iSize)+1)*this->iSize;        // cursor position on the first column of the next line
        this->cursorFix();
    }
    
    private: void cursorFix(){                              // if cursor outside the screen moves text up and sets the cursor on begining of last line 
        int size = this->getNumberOfCharecters();
        if(this->cursPos>=size){
            this->textMoveUP();
            this->cursPos = (this->iSize*(this->jSize-1));                // cursor position on the first column of the last line
        }
    }

    public: void refresh(){
        int size = this->getNumberOfCharecters();
        int i, j;
        int x, y;

        for(int c=0; c<size; c++){
            // calculate characters position on the virtual display (iSize x jSize)
            i = c % this->iSize;
            j = c / this->iSize;

            // now depending on the conversion parameters - lets convert positions to real display coordinates 
            x = this->corX + ((this->iToX)*i)+((this->jToX)*j);
            y = this->corY + ((this->jToY)*j)+((this->iToY)*i);

            bool transp = (this->textTranspBuffer[c >> 2] >> (c%4*2)) & 0x1;
            bool bTransp = (this->textTranspBuffer[c >> 2] >> ((c%4*2)+1)) & 0x1;

            if(this->textBuffer[c]!=0){this->displayCharacter(this->textBuffer[c],
                8*x , 8*y, 
                this->textColorBuffer[c] & 0xffff, 
                (this->textColorBuffer[c] >> 16) & 0xffff, 
                transp, 
                bTransp);
            }
        }    

        if(this->cursorVisible){
            this->cursorPrint();
        }

    }

    public: void displayString(std::string text, int x, int y){
        int i = 0;
        while(text[i]!=0){
            this->displayCharacter(text[i], x + i * 8, y);
            i++;
        }
    }

    public: void displayCharacter(int charCode, int x, int y){
        this->displayCharacter(charCode, x, y, this->fontColor, this->backColor, this->fontTransp, this->backTransp);
    }

    public: void displayCharacter(int charCode, int x, int y, uint16_t color, uint16_t bColor, bool transp, bool bTransp){
        int baseAddr = charCode * 8;    // base address for the character to be displayed
        int i;                          // read line bytes sequentialy to display
        bool pixelActive;               // 1 if pixel active, 0 if not active
        uint8_t j;                      // character consists of 8 lines
        uint8_t bytes;                  // bytes to draw on the screen
        color = colorFix(color);
        bColor = colorFix(bColor);

        for(j=0; j<8; j++){
            bytes = ATASCII[baseAddr + j];
            for(i=0; i<8; i++){
                pixelActive = (bool)((bytes >> (7-i)) & 0b1);
                if(pixelActive){
                    if(!transp){this->buffer16[((j*this->jToY)+(i*this->iToY) + y + this->charCorY) * this->displayWidth + (i*this->iToX)+(j*this->jToX) + x + this->charCorX] = color;}
                }
                else{
                    if(!bTransp){this->buffer16[((j*this->jToY)+(i*this->iToY) + y + this->charCorY) * this->displayWidth + (i*this->iToX)+(j*this->jToX) + x + this->charCorX] = bColor;} 
                }
            }
        }
    }

    public: void textMoveUP(){
        // moves all letters in the textBuffer one line up to make space for next line
        //int size = this->getNumberOfCharecters();
        int i, j;
        // rewrite all the lines one line up
        for( j=0; j<this->jSize-1; j++){
            for(i=0; i<this->iSize; i++){
                this->textBuffer[j*this->iSize+i] = this->textBuffer[(j+1)*this->iSize+i];
                this->textColorBuffer[j*this->iSize+i] = this->textColorBuffer[(j+1)*this->iSize+i];
            }
        }

        // moving the transparency array here
        int noBytes = ((float)((this->jSize - 1) * this->iSize/4))+1;           // number of all bytes to be moved 
        int byteShift = this->iSize / 4;                                     // how far are the source bytes
        int bitShift = (this->iSize % 4*2);                               // how many bits to be shifted

        for(i=0; i<noBytes; i++){
            uint16_t transp = (this->textTranspBuffer[i+byteShift + 1] << 8) | this->textTranspBuffer[i+byteShift]; // read two neighboring bytes
            transp = transp >> bitShift;                                      // shift the bits 
            this->textTranspBuffer[i] = (uint8_t) transp;         // save the shifted transparency value

        } 
        // fill the last line with 0
        for(i=0; i<this->iSize; i++){
            this->textBuffer[(this->jSize-1)*this->iSize + i] = 0;
            this->textColorBuffer[(this->jSize-1)*this->iSize + i] = 0x00000000;
        }


    }    

    private: void clearTextBuffer(){
        this->fillTextBuffer(0);
    }

    private: void fillTextBuffer(char character){
        int size = this->getNumberOfCharecters() * sizeof(char);
        for(int i=0; i<size; i++){
            this->textBuffer[i] = character;                  // fill the buffer with given character
            this->textColorBuffer[i] = 0x00000000;
            this->textTranspBuffer[i/4] = 0x00;
        }
    }

    private: int getNumberOfCharecters(){
        return this->displayWidth/8*this->displayHeight/8;           // returns total number of characters in the text mode
    }

    public: void setFontColor(uint16_t fontColor){
        this->fontColor = fontColor;
    }

    public: void setBackColor(uint16_t backColor){
        this->backColor = backColor;
    }

    public: void setFontTransp(bool fontTransp){
        this->fontTransp = fontTransp;
    }

    public: void setBackTransp(bool backTransp){
        this->backTransp = backTransp;
    }

    public: void setDefaults(uint16_t fontColor, uint16_t backColor, bool fontTransp, bool backTransp){
        this->fontColor = fontColor;
        this->backColor = backColor;
        this->fontTransp = fontTransp;
        this->backTransp = backTransp;
    }
};