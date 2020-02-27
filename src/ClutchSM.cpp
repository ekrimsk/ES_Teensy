

/****************************************************************************
  Module
  TemplateFSM.c

  Revision
  1.0.1

  Description
  This is a template file for implementing flat state machines under the        
  Gen2 Events and Services Framework.

  Notes

  History
  When           Who     What/Why
  -------------- ---     --------
  01/15/12 11:12 jec      revisions for Gen2 framework
  11/07/11 11:26 jec      made the queue static
  10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
  10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
  ****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"

#include "ClutchSM.h"
#include "ClutchControl.h"  // header file for low level hardware control
#include "ES_Timers.h"

#include <Arduino.h>
// #include <Wire.h>
#include <math.h>
// #include <SparkFun_MMA8452Q.h>

#define MIN_HANDOFF 5
// #define MAX_HANDOFF 1000

/*----------------------------- Module Defines ----------------------------*/


#define DEBUG



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
relevant to the behavior of this state machine
*/
//bool CheckAccelerometer(void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static ClutchState_t CurrentState;
static uint32_t newState;   // clutch states after second part of handoff  
// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;


static uint16_t HandoffDelay = 50;   // ms, default -- add setter function 


// private functions 
static uint32_t Control2State(uint16_t newControl); 



/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function
InitTemplateFSM

Parameters
uint8_t : the priorty of this service

Returns
bool, false if error in initialization, true otherwise

Description
Saves away the priority, sets up the initial transition and does any
other required initialization for this state machine
Notes

Author
J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
bool InitClutchSM ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;



  // will want to put more though into initial conditions + calibration 
  

  
  CurrentState = WaitForControl;  // for now 

  Serial.println("Clutch SM Initialized");



  

  // post the initial transition event
  //ThisEvent.EventType = ES_INIT;
  //if (ES_PostToService( MyPriority, ThisEvent) == true)
  //{
  //  return true;
  //}else
  //{
  //  return false;
  //}

  // we post nothing here, just wait for control 

  return true;
}

/****************************************************************************
Function
PostTemplateFSM

Parameters
EF_Event ThisEvent , the event to post to the queue

Returns
boolean False if the Enqueue operation failed, True otherwise

Description
Posts an event to this state machine's queue
Notes

Author
J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostClutchSM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
Function
RunTemplateFSM

Parameters
ES_Event : the event to process

Returns
ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

Description
add your description here
Notes
uses nested switch/case to implement the machine.
Author
J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event RunClutchSM( ES_Event ThisEvent )
{
    ClutchState_t NextState = CurrentState;
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
                                   // end switch on Current State\
    // what if get new control in the middle of a handoff? -- should never happen 
    switch( CurrentState )
    {
        case WaitForControl:

            // do part one of the handoff 
            if (ThisEvent.EventType == EV_NEW_CONTROL)
            {
                NextState = WaitForHandoff;
                uint32_t oldState = GetSpringStates(); // current states 
                newState = Control2State(ThisEvent.EventParam); 

                ClutchUpdate(oldState | newState); 
                ES_Timer_InitTimer(HandoffTimer, HandoffDelay); 

                #ifdef DEBUG
                    Serial.println("Control Recieved"); 
                #endif  


            }
            //TODO -- add a debug else 
            break; 

        case WaitForHandoff:
            if ((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == HandoffTimer))
            {
                NextState = WaitForControl; 
                // exceute part two of the handoff -- determined only from control 
                ClutchUpdate(newState);    // computed from last control 
                #ifdef DEBUG
                    Serial.println("Handoff");
                #endif  
            }
            break; 


        //default:
            // something for debuggin state machine -- shouldnt happen 


    } // end state switch 

    CurrentState = NextState; 
    return ReturnEvent;
}



bool SetHandoffDelay(uint16_t newDelay)
{
    if (newDelay > MIN_HANDOFF)
    {
        HandoffDelay = newDelay;
        return true;
    }
    return false; 
}


/****************************************************************************
Function
QueryTemplateSM

Parameters
None

Returns
TemplateState_t The current state of the Template state machine

Description
returns the current state of the Template state machine
Notes

Author
J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
ClutchState_t QueryTemplateFSM ( void )
{
  return(CurrentState);
}




/***************************************************************************
  private functions
***************************************************************************/

// TODO -- rewrite all this stuff 
static uint32_t Control2State(uint16_t newControl)
{
    uint32_t newState = 0;

    for (int i = 0; i < 16; i++)
    {
        if (newControl & BIT0HI) // attach to output 
        {
            newState |= BIT31HI;
        }
        else
        {
            newState |= BIT30HI;
        }

        if (i < 15)
        {
            newState >>= 2; 
            newControl >>= 1; 
        }
    }
    return newState; 
}
