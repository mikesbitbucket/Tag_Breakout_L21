/*******************************************************************************
  Main Source File

  Company:
 SupraTech

  File Name:
    main.c

  Summary:
 Tag Breakout Board Test Program
 * Rev 1.0

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "global_defs.h"
#include "misc.h"
#include "eic_functions.h"
#include <stdio.h>




#define EXPANDER_ADDR (0x0020)
#define EXPANDER_OUT_PORT0_ADDR (2)
#define EXPANDER_CONFIG_PORT0_ADDR (6)

uint8_t I2C_WR_Buffer[4], I2C_RD_Buffer[16];
uint8_t ExpanderPort1Shadow = 0, ExpanderPort2Shadow = 0;
uint8_t ExpanderPort1IOShadow = 0x10;  // Only P1.4 is input, others outputs
uint8_t ExpanderPort2IOShadow = 0xf0;  // Upper 4 bits are input

#define ACCEL_ADDR (0x0019)
#define ACCEL_WHO_AM_I_ADDR (0x0f)
#define ACCEL_CTRL1_ADDR (0x20)
#define ACCEL_CTRL2_ADDR (0x21)
#define ACCEL_CTRL3_ADDR (0x22)
#define ACCEL_CTRL4_ADDR (0x23)
#define ACCEL_CTRL5_ADDR (0x24)
#define ACCEL_CTRL6_ADDR (0x25)
#define ACCEL_CTRL7_ADDR (0x3f)

#define ACCEL_50HZ_HP   (0x44)


#define PROX_ADDR    (0x28)
#define PROX_WHO_AM_I_ADDR  (0xFA)

#define PULSE_OX_ADDR   (0x57)
#define PULSE_OX_ID_ADDR    (0xff)

uint16_t ADResult;

bool GPFlag;
uint8_t i,j;

uint8_t UART_RD_Buffer[16];


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    // Register the SYSTICK Callback
    SYSTICK_TimerCallbackSet (IncSysTick, (uintptr_t) NULL ); // Register the call back function
    
    EIC_CallbackRegister(EIC_PIN_6, Mag1_Interrupt, (uintptr_t) NULL);
    EIC_CallbackRegister(EIC_PIN_7, Mag2_Interrupt, (uintptr_t) NULL);
    
    SYSTICK_TimerStart(); // Start the SysTick Timer
    
    ADC_Enable();  // Enable ADC
    
    // Wake up the Detector and SW Power Buses
    SW_PWR_EN_Set();
    DETECT_EN_Set();
    
        
    // Tell the world we are alive
    printf("\r\n\r\n*** Test Start ***\r\n");

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
        
        SetHearbeatInterval(10);
        printf("Running on DPLL Internal Clock\r\n");
        printf("Green LED FAST Rate\r\n");
        SYSTICK_DelayMs(3000); // wait here while hearbeat goes
        // now switch to 32Khz crystal and FPLL
        GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1U) | GCLK_GENCTRL_SRC(8U) | GCLK_GENCTRL_GENEN_Msk;

        while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk)
        {
            /* wait for the Generator 0 synchronization */
        }
        
        printf("Now Running on FPLL Clock\r\n");
        
        SetHearbeatInterval(50);
        
        
        /**************************************************************************/
        // Check GREEN LED
        printf("Check Green Heartbeat LED...\r\n");
        SYSTICK_DelayMs(2000); // Small delay...
        
        
        /**************************************************************************/
        // Init I2C expander
        GPFlag = true;
        printf("Init Expander...\r\n");
        I2C_WR_Buffer[0] = EXPANDER_OUT_PORT0_ADDR;
        I2C_WR_Buffer[1] = ExpanderPort1Shadow;
        I2C_WR_Buffer[2] = ExpanderPort2Shadow;
        GPFlag = SERCOM2_I2C_Write(EXPANDER_ADDR, I2C_WR_Buffer, 3);  // Zeros the outputs

        while(SERCOM2_I2C_IsBusy()) continue;
        
        
        /**************************************************************************/
        // Set inputs and outputs
        I2C_WR_Buffer[0] = EXPANDER_CONFIG_PORT0_ADDR;
        I2C_WR_Buffer[1] = ExpanderPort1IOShadow;
        I2C_WR_Buffer[2] = ExpanderPort2IOShadow;
        GPFlag = SERCOM2_I2C_Write(EXPANDER_ADDR, I2C_WR_Buffer, 3);  // Sets IO direction
        
        while(SERCOM2_I2C_IsBusy()) continue;
        if(GPFlag)
        {
            printf("Expander Init Pass\r\n");
        }
        else
        {
            printf("Expander Init *** FAIL ***\r\n");
        }
        
        
        /**************************************************************************/
        // Try to turn on the R35
        // First reset it
        I2C_WR_Buffer[0] = EXPANDER_CONFIG_PORT0_ADDR;
        I2C_WR_Buffer[1] = (ExpanderPort1IOShadow & 0xef);  // Turn Reset R35 to output
        I2C_WR_Buffer[2] = ExpanderPort2IOShadow;
        GPFlag = SERCOM2_I2C_Write(EXPANDER_ADDR, I2C_WR_Buffer, 3);  // Sets IO direction
        while(SERCOM2_I2C_IsBusy()) continue;
        //R35 should be in reset now...
        SYSTICK_DelayMs(2); // Small delay...
        
        // turn on power
        I2C_WR_Buffer[0] = EXPANDER_OUT_PORT0_ADDR;
        ExpanderPort1Shadow |= 0x04; // Turn on R35 bit
        I2C_WR_Buffer[1] = ExpanderPort1Shadow;  // Turn on power to R35
        I2C_WR_Buffer[2] = ExpanderPort2Shadow;
        SERCOM2_I2C_Write(EXPANDER_ADDR, I2C_WR_Buffer, 3); // Write it out
        while(SERCOM2_I2C_IsBusy()) continue;
        SYSTICK_DelayMs(2); // Small delay...
        // Now release Reset
        I2C_WR_Buffer[0] = EXPANDER_CONFIG_PORT0_ADDR;
        I2C_WR_Buffer[1] = (ExpanderPort1IOShadow | 0x10);  // Turn Reset R35 to input
        I2C_WR_Buffer[2] = ExpanderPort2IOShadow;
        GPFlag = SERCOM2_I2C_Write(EXPANDER_ADDR, I2C_WR_Buffer, 3);  // Sets IO direction
        while(SERCOM2_I2C_IsBusy()) continue;
        SYSTICK_DelayMs(1500); // Give R35 time to settle clocks... and flash LED sequence - need at least 1 second
        // R35 should be on now
        
        
        /**************************************************************************/
        // Test Thermistor
        printf("Test Thermistor\r\n");
        ADC_ChannelSelect(ADC_POSINPUT_AIN10 , ADC_NEGINPUT_GND);  // Thermistor input
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        ADResult = ADC_ConversionResultGet();
        printf("Off Value: %d\r\n", ADResult);
        // Turn on the Thermistor Leg
        I2C_WR_Buffer[0] = EXPANDER_OUT_PORT0_ADDR;
        I2C_WR_Buffer[1] = (ExpanderPort1Shadow | 0x02);  // Turn on power
        I2C_WR_Buffer[2] = ExpanderPort2Shadow;
        SERCOM2_I2C_Write(EXPANDER_ADDR, I2C_WR_Buffer, 3); // Write it out
        while(SERCOM2_I2C_IsBusy()) continue;
        SYSTICK_DelayUs(20); // Small delay... for settling
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        ADResult = ADC_ConversionResultGet();
        printf("On Value: %d  ", ADResult);
        if(ADResult > 400)
        {
            printf("PASS\r\n");
        }
        else
        {
            printf("AIL\r\n");
        }
        
        
        /**************************************************************************/
        // Test Accelerometer
        I2C_WR_Buffer[0] = ACCEL_CTRL1_ADDR;
        I2C_WR_Buffer[1] = ACCEL_50HZ_HP;
        SERCOM2_I2C_Write(ACCEL_ADDR, I2C_WR_Buffer, 2); // Power Up the Accel
        while(SERCOM2_I2C_IsBusy()) continue;
        SYSTICK_DelayMs(1); // Let it wake up
        // Read Who Am I reg
        I2C_WR_Buffer[0] = ACCEL_WHO_AM_I_ADDR;
        I2C_RD_Buffer[0] = 0;
        SERCOM2_I2C_WriteRead(ACCEL_ADDR, I2C_WR_Buffer, 1, I2C_RD_Buffer, 1);
        while(SERCOM2_I2C_IsBusy()) continue;
        if(0x44 == I2C_RD_Buffer[0])
        {
            printf("Accelerometer PASS\r\n");
        }
        else
        {
            printf("Accelerometer FAIL - %d\r\n", I2C_RD_Buffer[0]);
        }
        
          
        
        /**************************************************************************/
        // Talk to Prox sensor
        // Read Who Am I reg
        // First turn it on at the Port Expander
        I2C_WR_Buffer[0] = EXPANDER_OUT_PORT0_ADDR;
        ExpanderPort1Shadow |= 0x04; // Turn on R35 bit
        I2C_WR_Buffer[1] = ExpanderPort1Shadow;  // Turn on power to R35
        I2C_WR_Buffer[2] = ExpanderPort2Shadow;
        SERCOM2_I2C_Write(EXPANDER_ADDR, I2C_WR_Buffer, 3); // Write it out
        while(SERCOM2_I2C_IsBusy()) continue;
        SYSTICK_DelayMs(2); // Small delay...
        
        I2C_WR_Buffer[0] = PROX_WHO_AM_I_ADDR;
        I2C_RD_Buffer[0] = 0;
        SERCOM2_I2C_WriteRead(PROX_ADDR, I2C_WR_Buffer, 1, I2C_RD_Buffer, 1);
        while(SERCOM2_I2C_IsBusy()) continue;
        if(0x23 == I2C_RD_Buffer[0])
        {
            printf("Prox Det PASS\r\n");
        }
        else
        {
            printf("Prox Det ***   FAIL   *** - %d\r\n", I2C_RD_Buffer[0]);
        }
        
        
        /**************************************************************************/
        // Talk to Pulse Ox sensor
        // Read ID Reg - Should be 0x15 readback
        I2C_WR_Buffer[0] = EXPANDER_OUT_PORT0_ADDR;
        ExpanderPort1Shadow |= 0x08; // Turn on 1.8v to Pulse Ox
        I2C_WR_Buffer[1] = ExpanderPort1Shadow;  // Turn on power to Pulse Ox
        I2C_WR_Buffer[2] = ExpanderPort2Shadow;
        SERCOM2_I2C_Write(EXPANDER_ADDR, I2C_WR_Buffer, 3); // Write it out
        while(SERCOM2_I2C_IsBusy()) continue;
        SYSTICK_DelayMs(50); // Small delay... let the Pulse Ox warm up

        I2C_WR_Buffer[0] = PULSE_OX_ID_ADDR;
        I2C_RD_Buffer[0] = 0;
        SERCOM2_I2C_WriteRead(PULSE_OX_ADDR, I2C_WR_Buffer, 1, I2C_RD_Buffer, 1);
        while(SERCOM2_I2C_IsBusy()) continue;
        if(0x15 == I2C_RD_Buffer[0])
        {
            printf("Pulse Ox PASS\r\n");
        }
        else
        {
            printf("Pulse Ox ***   FAIL   *** - %d\r\n", I2C_RD_Buffer[0]);
        }
        
        
        /**************************************************************************/
        // Test Sample Hold & DAC
        uint16_t High_Comp, Low_Comp;
        uint32_t Decay_Timer;
        
        // Test DAC and output
        printf("Test Reference Circuits\r\n");
        DAC_DataWrite(0, 500);
        LO_DAC_En_Set();  //  Sample it
        HI_DAC_En_Set();
        SYSTICK_DelayUs(5000); // Let it settle
        ADC_ChannelSelect(ADC_POSINPUT_AIN5 , ADC_NEGINPUT_GND);  // Low Comp
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        Low_Comp = ADC_ConversionResultGet(); // Store current value
        ADC_ChannelSelect(ADC_POSINPUT_AIN4 , ADC_NEGINPUT_GND);  // High Comp
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        High_Comp = ADC_ConversionResultGet(); // Store current value
        printf("DAC: 500   Hi:  %d Lo:  %d\r\n", High_Comp, Low_Comp);
        
        
        DAC_DataWrite(0, 1000);
        LO_DAC_En_Set();  //  Sample it
        HI_DAC_En_Set();
        SYSTICK_DelayUs(5000); // Let it settle
        ADC_ChannelSelect(ADC_POSINPUT_AIN5 , ADC_NEGINPUT_GND);  // Low Comp
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        Low_Comp = ADC_ConversionResultGet(); // Store current value
        ADC_ChannelSelect(ADC_POSINPUT_AIN4 , ADC_NEGINPUT_GND);  // High Comp
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        High_Comp = ADC_ConversionResultGet(); // Store current value
        printf("DAC: 1000  Hi: %d Lo: %d\r\n", High_Comp, Low_Comp);
        
        DAC_DataWrite(0, 2000);
        LO_DAC_En_Set();  //  Sample it
        HI_DAC_En_Set();
        SYSTICK_DelayUs(5000); // Let it settle
        ADC_ChannelSelect(ADC_POSINPUT_AIN5 , ADC_NEGINPUT_GND);  // Low Comp
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        Low_Comp = ADC_ConversionResultGet(); // Store current value
        ADC_ChannelSelect(ADC_POSINPUT_AIN4 , ADC_NEGINPUT_GND);  // High Comp
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        High_Comp = ADC_ConversionResultGet(); // Store current value
        printf("DAC: 2000  Hi: %d Lo: %d\r\n", High_Comp, Low_Comp);
        
        DAC_DataWrite(0, 3000);
        LO_DAC_En_Set();  //  Sample it
        HI_DAC_En_Set();
        SYSTICK_DelayUs(5000); // Let it settle
        ADC_ChannelSelect(ADC_POSINPUT_AIN5 , ADC_NEGINPUT_GND);  // Low Comp
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        Low_Comp = ADC_ConversionResultGet(); // Store current value
        ADC_ChannelSelect(ADC_POSINPUT_AIN4 , ADC_NEGINPUT_GND);  // High Comp
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        High_Comp = ADC_ConversionResultGet(); // Store current value
        printf("DAC: 3000  Hi: %d Lo: %d\r\n", High_Comp, Low_Comp);
        
        
        DAC_DataWrite(0, 2000);
        LO_DAC_En_Set();  //  Sample it
        HI_DAC_En_Set();
        SYSTICK_DelayUs(5000); // Let it settle
        // Get starting values
        ADC_ChannelSelect(ADC_POSINPUT_AIN5 , ADC_NEGINPUT_GND);  // Low Comp
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        Low_Comp = ADC_ConversionResultGet(); // Store current value
        ADC_ChannelSelect(ADC_POSINPUT_AIN4 , ADC_NEGINPUT_GND);  // High Comp
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        High_Comp = ADC_ConversionResultGet(); // Store current value
        printf("High Start: %d  Low Start: %d\r\n", High_Comp, Low_Comp);
        
        LO_DAC_En_Clear();  //  Open up sample and hold
        HI_DAC_En_Clear();
        // Now wait for decay
        Decay_Timer = GetSysTick();  //Get systick
        i=0; // Loop counter
        printf("Test Sample Hold\r\nWaiting");
        while(i < 20)  // Wait 10 seconds
        {
            if((uint32_t)(GetSysTick() - Decay_Timer) >= 50) // print ',' every 1/2 second 
            {
                Decay_Timer = GetSysTick();
                printf(".");
                i++;
            }
        } // end Decay wait
        printf("\r\n");
        GPFlag = true;
        ADC_ChannelSelect(ADC_POSINPUT_AIN5 , ADC_NEGINPUT_GND);  // Low Comp
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        ADResult = ADC_ConversionResultGet(); // Store current value
        if((abs(Low_Comp - ADResult)) > 60)
        {
            printf("Low Comp *** FAIL *** Delta: %d\r\n",(Low_Comp - ADResult));
            GPFlag = false;
        }
        else
        {
            printf("Low Comp Delta: %d\r\n", (Low_Comp - ADResult));
        }
        
        ADC_ChannelSelect(ADC_POSINPUT_AIN4 , ADC_NEGINPUT_GND);  // High Comp
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        ADResult = ADC_ConversionResultGet(); // Store current value
        if((abs(High_Comp - ADResult)) > 60)
        {
            printf("High Comp *** FAIL *** Delta: %d\r\n",(High_Comp - ADResult));
            GPFlag = false;
        }
        else
        {
            printf("High Comp Delta: %d\r\n", (High_Comp - ADResult));
        }
        if(GPFlag)
        {
            printf("Sample Hold PASS\r\n");
        }
        
        
        
        /**************************************************************************/
        // Test the Detector Steady State Values
        float RSSI_Val;
//        uint16_t BandGap_Result;
//        
//        ADC_ChannelSelect(0x19, ADC_NEGINPUT_GND);  // Get Bandgap voltage
//        ADC_REGS->ADC_SAMPCTRL = (uint8_t)ADC_SAMPCTRL_SAMPLEN(47UL); // need to write this when selecting Bandgap or temp
//        while(0U != ADC_REGS->ADC_SYNCBUSY)
//        {
//            /* Wait for Synchronization */
//        }
//        printf("ready to bandbap\r\n");
//        SYSTICK_DelayMs(50); // Let it settle
//        
//        SYSTICK_DelayUs(50); // Let it settle
//        ADC_ConversionStart();
//        //while(ADC_ConversionStatusGet() == false) continue;
//        SYSTICK_DelayUs(500); // Let it settle
//        BandGap_Result = ADC_ConversionResultGet(); // Store current value
//        printf("Bandgap: %d\r\n", BandGap_Result);
        
        // Lo RSSI
        ADC_ChannelSelect(ADC_POSINPUT_AIN7 , ADC_NEGINPUT_GND);  // Low Comp
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        ADResult = ADC_ConversionResultGet(); // Store current value
        RSSI_Val = ((float)ADResult / 4096.0) * 3.0;
        printf("Low RSSI: %d Voltage: %0.3f", ADResult, RSSI_Val);
        if(ADResult > 300)
        {
            printf(" Pass\r\n");
        }
        else
        {
            printf(" *** FAIL ***\r\n");
        }
        // Hi RSSI
        ADC_ChannelSelect(ADC_POSINPUT_AIN6 , ADC_NEGINPUT_GND);  // Low Comp
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        ADResult = ADC_ConversionResultGet(); // Store current value
        RSSI_Val = ((float)ADResult / 4096.0) * 3.0;
        printf("High RSSI: %d Voltage: %0.3f", ADResult, RSSI_Val);
        if(ADResult > 300)
        {
            printf(" Pass\r\n");
        }
        else
        {
            printf(" *** FAIL ***\r\n");
        }
        
        
        
        
        /**************************************************************************/
        // Now test R35 loop back - it should have had time to reset and run by now
        SYSTICK_DelayMs(20); // Give it a chance to echo back
        i = 'T';
        SERCOM1_USART_Write(&i, 1); // write out a test byte
        SYSTICK_DelayMs(2); // Give it a chance to echo back
        if(SERCOM1_USART_ReadCountGet())
        {
            // we have at least 1 byte
            i=0;
            SERCOM1_USART_Read(&i,1);
            if('T' == i)
            {
                printf("R35 Loopback: PASS\r\n");
            }
            else
            {
                printf("R35 Loopback: *** FAIL ***\r\n");
            }
        }
        else
            {
                printf("R35 Loopback: *** FAIL ***\r\n");
            }
        
        
        
        
        
        /**************************************************************************/
        // Now test the Strap Detector and Intrusion Detector
        GPFlag = true;
        printf("Cover Detector  Any Key to continue\r\n");
        FlushBuffer();  // clear keyboard read buffer
                
        while(0 == SERCOM3_USART_ReadCountGet()) continue;
        FlushBuffer();  // clear keyboard read buffer
        //Intrusion
        printf("LED Off\r\n");
        Strap_Discharge_Set();
        ADC_ChannelSelect(ADC_POSINPUT_AIN18 , ADC_NEGINPUT_GND);  // Intrusion
        SYSTICK_DelayUs(200); // Let it settle
        Strap_Discharge_Clear();
        SYSTICK_DelayUs(50); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        ADResult = ADC_ConversionResultGet();
        printf("Intrusion: %d\r\n", ADResult);
        if(ADResult > 500) GPFlag = false;
        // Get Strap
        ADC_ChannelSelect(ADC_POSINPUT_AIN1 , ADC_NEGINPUT_GND);  // Strap Detect
        SYSTICK_DelayUs(100); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        ADResult = ADC_ConversionResultGet();
        printf("Strap: %d\r\n", ADResult);
        if(ADResult > 500) GPFlag = false;
        
        // Now Turn on LED
        Strap_TX_LED_Set();
        SYSTICK_DelayUs(100); // Let it settle
        printf("LED On\r\n");
        ADC_ChannelSelect(ADC_POSINPUT_AIN18 , ADC_NEGINPUT_GND);  // Intrusion
        SYSTICK_DelayUs(100); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        ADResult = ADC_ConversionResultGet();
        printf("Intrusion: %d\r\n", ADResult);
        if(ADResult < 2000) GPFlag = false;
        // Get Strap
        ADC_ChannelSelect(ADC_POSINPUT_AIN1 , ADC_NEGINPUT_GND);  // Strap Detect
        SYSTICK_DelayUs(100); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        ADResult = ADC_ConversionResultGet();
        printf("Strap: %d\r\n", ADResult);
        if(ADResult < 2000) GPFlag = false;
        //Now try discharge
        i = ADResult; // Store last in a temp
        Strap_Discharge_Set();
        SYSTICK_DelayUs(500); // Let it settle
        ADC_ConversionStart();
        while(ADC_ConversionStatusGet() == false) continue;
        ADResult = ADC_ConversionResultGet();
        printf("Strap with Discharge: %d\r\n", ADResult);
        if((ADResult + 75) < i) GPFlag = false;
        Strap_TX_LED_Clear();
        Strap_Discharge_Clear();
                
        printf("Intrusion and Strap ");
        if(GPFlag)
        {
            printf("PASS\r\n");
        }
        else
        {
            printf("*** FAIL ***\r\n");
        }
        
        
        /**************************************************************************/
        bool Mag1Flag = false, Mag2Flag = false;
        
        // Test Magnets - put this test at the end
        FlushBuffer();  // clear keyboard read buffer
        printf("Test Magnets\r\nAny Key to continue\r\n");
        GPFlag = false;
        while((0 == SERCOM3_USART_ReadCountGet()) && (false == GPFlag)) // exit on key or mags
        {
            if(0 == Mag_Out_1_Get())
            {
                Mag1Flag = true;
            }
            if(0 == Mag_Out_2_Get())
            {
                Mag2Flag = true;
            }
            if(Mag2Flag && Mag1Flag) GPFlag = true;
        }
        
        
        printf("\r\n***   End of Test   ***\r\n\r\n");

        while(1) continue;
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

