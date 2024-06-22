/* 
 * File:   misc.h
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MISC_H
#define	MISC_H

#include "global_defs.h"

// TODO Insert appropriate #include <>

// TODO Insert C++ class definitions if appropriate



// Comment a function and leverage automatic documentation with slash star star
/**
    <p><b>Function prototype:</b></p>
  
    <p><b>Summary:</b></p>

    <p><b>Description:</b></p>

    <p><b>Precondition:</b></p>

    <p><b>Parameters:</b></p>

    <p><b>Returns:</b></p>

    <p><b>Example:</b></p>
    <code>
 
    </code>

    <p><b>Remarks:</b></p>
 */
// TODO Insert declarations or function prototypes (right here) to leverage 
// live documentation

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

//void IncSysTick(void);
void IncSysTick ( uintptr_t context );
void SetHearbeatInterval( uint32_t interval );
void ClearSysTick(void);
uint32_t GetSysTick(void);
void DoHeartBeat(void);
void FlushBuffer(void);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* MISC_H */

