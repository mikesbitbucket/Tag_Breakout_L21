/* 
 * File:   global_defs.h
 * Author: Mike
 *
 * Created on Nov 3, 2023
 */

#ifndef GLOBAL_DEFS_H
#define	GLOBAL_DEFS_H

#ifdef	__cplusplus
extern "C" {
#endif

//Stuff for 8 bit processors
//#pragma warning disable 520  // Turns off the Function Not Called Warning
//#pragma warning disable 2053  // Turns off the Function Not Called Warning - Inline functions
    
//#include "mcc_generated_files/system/system.h"
//#include <xc.h>
    
#pragma GCC diagnostic ignored "-Wunused-variable"   // ignore unused variable...


    
#include <stdint.h>
#include <stdbool.h>
    
#define TRUE 1
#define FALSE 0

    
#define HEARTBEAT_INTERVAL  (0.01)  // seconds for interrupt from systick timer
#define LED_HEARTBEAT_INTERVAL 50  // Interval for heartbeat tick - 50 = 500ms @ 10ms tick




#ifdef	__cplusplus
}
#endif

#endif	/* GLOBAL_DEFS_H */

