/*
 * File added 2013 at LBNL
 * Carl Lionberger <calionberger@lbl.gov>
 *
 * mrfioc2 is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */


#include "drvemUDC.h"

#include <stdexcept>

#include <epicsTypes.h>

#include <mrfCommonIO.h>
#include "evrRegMap.h"
#include "drvem.h"

/*
 * define methods for the universal io fine delay module
 */

MRMUDM::MRMUDM(unsigned char s, EVRMRM& o, evrForm f):
   base(o.base),
   form(f),	/* don't need this yet */
   owner(o),
   slot(s),
   sh(s?4:0),
   used(0)
{
   /* we don't touch the hardware now because the constructor is called
    * unconditionally.
    */
   if (s&~1) 
     throw std::out_of_range("Universal IO module number");
   shadowDelay[0] = shadowDelay[1] = 0;
}

MRMUDM::~MRMUDM() {}

void MRMUDM::lock() const{owner.lock();};
void MRMUDM::unlock() const{owner.unlock();};


/*
 * defined in evrRegMap.h
#define U32_GPIODir     0x090
#define U32_GPIOIn      0x094
#define U32_GPIOOut     0x098
 */
/* 
 * put these in drvemUDC.h ?
 */
/* GPIO mapping for delay module */
#define EVR_UNIV_DLY_DIN    0x01
#define EVR_UNIV_DLY_SCLK   0x02
#define EVR_UNIV_DLY_LCLK   0x04
#define EVR_UNIV_DLY_DIS    0x08

bool
MRMUDM::enabled() {

#if 0
   if (!used) {
      /* set all 4 gpio pin directions to out */
      WRITE32(base, GPIODir, ((EVR_UNIV_DLY_DIN | EVR_UNIV_DLY_SCLK |
             EVR_UNIV_DLY_LCLK | EVR_UNIV_DLY_DIS) << sh));
      used = true;
   }
#endif
   if (READ32(base, GPIOOut) & (EVR_UNIV_DLY_DIS << sh))
      return false;
   else
      return true;
   
}


void 
MRMUDM::enable(bool s) {

   epicsUInt32 gpio;
   epicsUInt32 gpiodir;

   gpiodir = READ32(base, GPIODir);
   /* set all 4 gpio pin directions to out */
   WRITE32(base, GPIODir, gpiodir | ((EVR_UNIV_DLY_DIN | EVR_UNIV_DLY_SCLK |
          EVR_UNIV_DLY_LCLK | EVR_UNIV_DLY_DIS) << sh));
#if 0
   printf("MRMUDM::enable %p module %d called with flag %d and gpiodir %d\n",
           this, sh/4, s, gpiodir);
#endif
   gpio = READ32(base, GPIOOut) & ~(EVR_UNIV_DLY_DIS << sh);
   if (!s) gpio |= (EVR_UNIV_DLY_DIS << sh);
   WRITE32(base, GPIOOut, gpio);
}

epicsUInt32 
MRMUDM::delay(unsigned char c) const {

   if (c&~1) 
     throw std::out_of_range("Universal IO Channel number");

   return shadowDelay[c];
}

/*
 * see EvrUnivDlySetDelay() in erapi.c
 */
void
MRMUDM::setDelay(unsigned char c, epicsUInt32 d) {

   epicsUInt32 gpio;
   epicsInt32 sd, sr, i, sclk, din, lclk, dbit;

   if (c&~1) 
     throw std::out_of_range("Universal IO Channel number");
   shadowDelay[c] = d & 0x03ff;
#if 0
   printf("MRMUDM::setDelay %p module %d chan %d called with delay %d\n",
           this, sh/4, c, d);
#endif
   din = EVR_UNIV_DLY_DIN << sh;
   sclk = EVR_UNIV_DLY_SCLK << sh;
   lclk = EVR_UNIV_DLY_LCLK << sh;

   /* get the enable values of the two slots */
   gpio = READ32(base, GPIOOut) & ~((EVR_UNIV_DLY_DIN | EVR_UNIV_DLY_SCLK |
                                        EVR_UNIV_DLY_LCLK) |
                                       ((EVR_UNIV_DLY_DIN | EVR_UNIV_DLY_SCLK |
                                         EVR_UNIV_DLY_LCLK) << 4));

   /* This is reversed
   sd = ((shadowDelay[0] & 0x0ff) << 16) |
   ((shadowDelay[1] & 0x00f) << 12) | (shadowDelay[0] & 0x300) |
   (shadowDelay[1] >> 4);
   */
   sd = ((shadowDelay[1] & 0x0ff) << 16) |
   ((shadowDelay[0] & 0x00f) << 12) | (shadowDelay[1] & 0x300) |
   (shadowDelay[0] >> 4);

   /*
   printf("Generated shift word of 0x%x\n", sd);
   */

   sr = sd;
   for (i = 24; i; i--) {
      dbit = 0;
      if (sr & 0x00800000) dbit = din;
      WRITE32(base, GPIOOut, gpio | dbit);
      WRITE32(base, GPIOOut, gpio | dbit | sclk);
      WRITE32(base, GPIOOut, gpio | dbit);
      sr <<= 1;
   }
   WRITE32(base, GPIOOut, gpio | lclk);
   WRITE32(base, GPIOOut, gpio);
 
   /* latch enables active */
   sr = sd | 0x000880;
   for (i = 24; i; i--) {
      dbit = 0;
      if (sr & 0x00800000) dbit = din;
      WRITE32(base, GPIOOut, gpio | dbit);
      WRITE32(base, GPIOOut, gpio | dbit | sclk);
      WRITE32(base, GPIOOut, gpio | dbit);
      sr <<= 1;
   }
   WRITE32(base, GPIOOut, gpio | lclk);
   WRITE32(base, GPIOOut, gpio);
 
   
   sr = sd;
   for (i = 24; i; i--) {
      dbit = 0;
      if (sr & 0x00800000) dbit = din;
      WRITE32(base, GPIOOut, gpio | dbit);
      WRITE32(base, GPIOOut, gpio | dbit | sclk);
      WRITE32(base, GPIOOut, gpio | dbit);
      sr <<= 1;
   }
   WRITE32(base, GPIOOut, gpio | lclk);
   WRITE32(base, GPIOOut, gpio);
}
 
/*
 * define methods for the universal IO fine delay channel
 */

MRMUDC::MRMUDC(const std::string& n, unsigned char c, MRMUDM& m):
   UDC(n),
   chan(c % 2),
   module(m)
{
   /* as with udm, construction could occur in absence of hardware */
}

MRMUDC::~MRMUDC() {}

void MRMUDC::lock() const{module.lock();};
void MRMUDC::unlock() const{module.unlock();};

bool MRMUDC::enabled() const {
   return module.enabled();
}
void MRMUDC::enable(bool s) {
   module.enable(s);
}

epicsUInt32 MRMUDC::delay() const {
   return module.delay(chan);
}

void MRMUDC::setDelay(epicsUInt32 s) {
   module.setDelay(chan, s);
}




