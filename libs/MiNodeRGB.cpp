#include "MiNodeRGB.h"

MiNodeRGB::MiNodeRGB() :
CLOCK(NULL) , DATA(NULL)
{
  this->baseId = MINODE_ID_MODULE_RGB;
}

MiNodeRGB::~MiNodeRGB()
{
  if(CLOCK) {
    delete CLOCK;
  }

  if(DATA) {
    delete DATA;
  }
}

void MiNodeRGB::attach(ConnName connName)
{
  if(this->cn != MN_NC) {
    return;
  }

  MiNodeComponent::initConnector(connName);

  PinName pinName1 = MiNodeConn::calcP0Name(this->cn);
  PinName pinName2 = MiNodeConn::calcP1Name(this->cn);
  if(CLOCK) {
    delete CLOCK;
  }
  CLOCK = new DigitalOut(pinName1);
  DATA = new DigitalOut(pinName2);
}

void MiNodeRGB::rgbClockWrite(int value)
{
  CLOCK->write(value);
}

void MiNodeRGB::rgbDataWrite(int value)
{
  DATA->write(value);
}

void MiNodeRGB::rgbDelay()
{

}

void MiNodeRGB::chooseColor(MiNodeColor c)
{
  switch(c)
  {
    case MiNode_RGB_COLOR_RED:
      rgbWrite(255,0,0);
      break;
    case MiNode_RGB_COLOR_GREEN:
      rgbWrite(0,255,0);
      break;
    case MiNode_RGB_COLOR_BLUE:
      rgbWrite(0,0,255);
      break;
    case MiNode_RGB_COLOR_YELLOW:
      rgbWrite(255,255,0);
      break;
    case MiNode_RGB_COLOR_PINK:
      rgbWrite(255,0,255);
      break;
    case MiNode_RGB_COLOR_CYAN:
      rgbWrite(0,255,255);
      break;
    case MiNode_RGB_COLOR_WHITE:
      rgbWrite(255,255,255);
      break;
    default :
      rgbWrite(255,0,0);
      break;
  }
}

void MiNodeRGB::setRGBInHex(unsigned int color)
{
  int red=0;
  int green=0;
  int blue=0;

  red = color/0x10000;
  green = color%0x10000/0x100;
  blue = color%0x100;

  rgbWrite(red,green,blue);
}

void MiNodeRGB::setRGB(uint8_t r,uint8_t g,uint8_t b)
{
  rgbWrite(r,g,b);
}

void MiNodeRGB::setHSL(uint16_t h,uint8_t s,uint8_t l)
{
        div_t dt; 
        h = round(h);
        
        s = round(s);
        if(s>99) s=99;
        if(s<0) s=0;

        l = round(l);
        if(l>99) l=99;
        if(l<0) l=0;
        
        h = h % 360;
        dt = div((((100 - abs(2 * l - 100)) * s) << 8), 10000); //chroma, [0,255]
        int c = dt.quot;
        dt =  div(h, 60);//[0,6]
        int  h1 = dt.quot;
        dt = div((h - h1 * 60) * 256, 60);//[0,255]
        int  h2 = dt.quot;
        int  temp = abs((((h1 % 2) << 8) + h2) - 256);
        int  x = (c * (256 - (temp))) >> 8;//[0,255], second largest component of this color
        int  r$;
        int  g$;
        int  b$;
        if (h1 == 0) {
            r$ = c; g$ = x; b$ = 0;
        } else if (h1 == 1) {
            r$ = x; g$ = c; b$ = 0;
        } else if (h1 == 2) {
            r$ = 0; g$ = c; b$ = x;
        } else if (h1 == 3) {
            r$ = 0; g$ = x; b$ = c;
        } else if (h1 == 4) {
            r$ = x; g$ = 0; b$ = c;
        } else if (h1 == 5) {
            r$ = c; g$ = 0; b$ = x;
        }
        dt = div((l * 2 << 8), 100);     
        dt = div((dt.quot - c), 2);
        int  m = dt.quot;
        uint8_t  r = r$ + m;
        uint8_t  g = g$ + m;
        uint8_t  b = b$ + m;
        rgbWrite(r,g,b);  
}


void MiNodeRGB::rgbWrite(uint8_t r,uint8_t g,uint8_t b)
{
  uint32_t rgb = 0;

  // set bit 7,15,23
  rgb = (1<<23) | (1<<15) | (1<<7);

  // be divided by 2 because the chip only need 7 bits
  r>>=1;
  g>>=1;
  b>>=1;

  rgb |= (g << 15) | (r << 7) | (b << 0);

  rgbClockWrite(0); //P15
  rgbDataWrite(0);  //P16

  rgbDelay();

  // out put at least 32 zeros
  for(int i = 0; i < 32; i ++) {
      rgbDataWrite(0);
      rgbDelay();
      rgbClockWrite(1);
      rgbDelay();
      rgbClockWrite(0);
  }

  rgbDelay();

  // out put 24 bits RGB data to channel 1,2,3.
  for(int i = 0; i < 24; i ++) {
      if(rgb & (1 << 23))
          rgbDataWrite(1);
      else
          rgbDataWrite(0);
      rgb <<= 1;

      rgbDelay();
      rgbClockWrite(1);
      rgbDelay();
      rgbClockWrite(0);
  }

  rgbDelay();

  // out put 24 bit rgb data for channel 4,5,6. data value is ignored. here set to 0
  rgb = 0;
  rgb = (1<<23) | (1<<15) | (1<<7);
  for(int i = 0; i < 24; i ++) {
      if(rgb & (1 << 23))
          rgbDataWrite(1);
      else
          rgbDataWrite(0);
      rgb <<= 1;

      rgbDelay();
      rgbClockWrite(1);
      rgbDelay();
      rgbClockWrite(0);
  }
}

