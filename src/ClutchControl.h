#ifndef CLUTCH_CONTROL_H
#define CLUTCH_CONTROL_H


#include "BITDEFS.H"
#include <stdint.h>
#include <stdbool.h>


//#include "ShiftRegister.h"

// should probably do as a service really 


#ifdef __cplusplus
extern "C" {
#endif



bool ClutchUpdate(uint32_t newSpringStates); 
uint32_t GetSpringStates(void);
uint32_t GetClutchStatesLB(void);
uint32_t GetClutchStatesHB(void);
uint32_t gc(void);
uint32_t GetClutchDirections(void); 


//uint64_t GetClutchStates(void);

#ifdef __cplusplus
}
#endif


#endif // CLUTCH_CONTROL_H
