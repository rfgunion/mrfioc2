#ifndef UDC_HPP_INC
#define UDC_HPP_INC

/*
 * Carl Lionberger calionberger@lbl.gov
 */

/*
 * UDC: Universal I/O fine Delay Channel.
 */

#include "mrf/object.h"

#include <epicsTypes.h>

class UDC : public mrf::ObjectInst<UDC>
{
public:
  
  UDC(const std::string& n) : mrf::ObjectInst<UDC>(n) {}
  virtual ~UDC() = 0;

  virtual bool enabled() const=0;
  virtual void enable(bool)=0;

  //! delay 0-1023 in increment of 10 pSecond.  
  virtual epicsUInt32 delay() const=0;
  virtual void setDelay(epicsUInt32)=0;

};

#endif	// UDC_HPP_INC
