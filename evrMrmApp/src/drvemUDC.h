/*
 * File added 2013 at LBNL
 * Carl Lionberger <calionberger@lbl.gov>
 *
 * mrfioc2 is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */


#ifndef EVRMRMUDC_HPP_INC
#define EVRMRMUDC_HPP_INC

#include "evr/udc.h"

#include "evrRegMap.h"

class EVRMRM;

/* 
 * This is the hardware unit; when talking to a channel you usually have to 
 * talk to both that are on a universal I/O.  
 */
class MRMUDM
{
public:
   MRMUDM(unsigned char slot, EVRMRM&, evrForm);
   virtual ~MRMUDM();

   virtual void lock() const;
   virtual void unlock() const;

   virtual bool enabled();
   virtual void enable(bool);
   
   virtual epicsUInt32 delay(unsigned char) const;
   virtual void setDelay(unsigned char, epicsUInt32);

   private:

   volatile unsigned char *base;
   evrForm form;
   EVRMRM& owner;
   unsigned char slot;
   unsigned char sh;
   bool used;
   epicsUInt32 shadowDelay[2];
};

/*
 * A single channel of universal fine delay
 */
class MRMUDC : public UDC
{
public:

   MRMUDC(const std::string&, unsigned char, MRMUDM&);
   virtual ~MRMUDC();

   virtual void lock() const;
   virtual void unlock() const;

   virtual bool enabled() const;
   virtual void enable(bool);
   
   virtual epicsUInt32 delay() const;
   virtual void setDelay(epicsUInt32);

private:

   unsigned char chan;
   MRMUDM& module;
};


#endif 
