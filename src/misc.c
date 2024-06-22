/* ************************************************************************** */
/** Misc Functions

  @Company
    SupraTech

  @File Name
    misc.c

  @Summary
    Misc functions - timers, etc.

  @Description
    Various button handling functions
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */


#include "definitions.h" 
#include "global_defs.h"
#include "misc.h"




/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */
/** Descriptive Data Item Name

  @Summary
    Brief one-line summary of the data item.
    
  @Description
    Full description, explaining the purpose and usage of data item.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
  @Remarks
    Any additional remarks
 */
static uint32_t MySysTick = 0;
static uint32_t Heartbeat_tmr = 0;

static uint32_t Hearbeat_Interval = LED_HEARTBEAT_INTERVAL;
    
//static uint8_t LED_Heartbeat_tmr;




/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */

/** 
  @Function
    int ExampleLocalFunctionName ( int param1, int param2 ) 

  @Summary
    Brief one-line description of the function.

  @Description
    Full description, explaining the purpose and usage of the function.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

  @Precondition
    List and describe any required preconditions. If there are no preconditions,
    enter "None."

  @Parameters
    @param param1 Describe the first parameter to the function.
    
    @param param2 Describe the second parameter to the function.

  @Returns
    List (if feasible) and describe the return values of the function.
    <ul>
      <li>1   Indicates an error occurred
      <li>0   Indicates an error did not occur
    </ul>

  @Remarks
    Describe any special behavior not described above.
    <p>
    Any additional remarks.

  @Example
    @code
    if(ExampleFunctionName(1, 2) == 0)
    {
        return 3;
    }
 */
//static int ExampleLocalFunction(int param1, int param2) {
//    return 0;
//}








/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */


// *****************************************************************************

/** 
  @Function
    IncSysTick 

  @Summary
 Increments the System Tic

  @Remarks
 In SysTic
 */
//void IncSysTick(void)
void IncSysTick( uintptr_t context )
{
    // Note: Context is not used but needed for function definition - if left void, will throw error
    MySysTick++;
    
    // We also do the heartbeat here so we don't have to call another routine
    if((uint32_t)(GetSysTick() - Heartbeat_tmr) >= Hearbeat_Interval)
    {
        Heartbeat_tmr = GetSysTick(); // get new time val
        //LED_Toggle();  // Toggle the on board LED
        GREEN_LED_Toggle();
        
    } // End LED Beat
}

/** 
  @Function
    SetHearbeatInterval 

  @Summary
 Sets the LED Heartbeat interval

  @Remarks
 Set new heartbeat interval
 */
//void IncSysTick(void)
void SetHearbeatInterval( uint32_t interval )
{
    // Note: Context is not used but needed for function definition - if left void, will throw error
    Hearbeat_Interval = interval;
        
} // End SetHeartbeatInterval

/** 
  @Function
    ClearSysTick 

  @Summary
 Clears the System Tic

  @Remarks
 Clear SysTic
 */
void ClearSysTick(void)
{
    MySysTick = 0;
}

// *****************************************************************************

/** 
  @Function
    GetSysTick 

  @Summary
    Gets the current SysTick

  @Remarks
    Get and return current SysTick
 */
uint32_t GetSysTick(void)
{
    return MySysTick;  // Is this atomic on this processor or do we have to disable interrupts
}



// *****************************************************************************

/** 
  @Function
    DoHeartBeat 

  @Summary
 * Does any heartbeat related tasks - blink LED, etc

  @Remarks
 *  Heartbeat Stuff
 */

void DoHeartBeat()
{
    //static size_t counter = 0;
    uint8_t i;
    
    // Heartbeat check - Also the Blink Status of the LED
    if((uint32_t)(GetSysTick() - Heartbeat_tmr) >= LED_HEARTBEAT_INTERVAL)
    {
        Heartbeat_tmr = GetSysTick(); // get new time val
        
        //LED_Toggle();  // Toggle the on board LED
        GREEN_LED_Toggle();
        
    } // End LED Beat
    
        
}  // end heartbeat
    
void FlushBuffer(void)
{
    uint8_t i,count, junk;
    
    if(SERCOM3_USART_ReadCountGet())
    {
        // got some data, flush it
        count = SERCOM3_USART_ReadCountGet();
        for(i=0; i < count; i++)
        {
            SERCOM3_USART_Read(&junk, 1 );  // Read a byte
        }
    }
}

/* *****************************************************************************
 End of File
 */
