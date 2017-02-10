/*
 * Copyright (C) 2017 iCub Facility - Istituto Italiano di Tecnologia
 * Author:  Marco Accame
 * email:   marco.accame@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/


// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "main.h"
#include "stm32l4xx_hal.h"
#include "usart.h"
#include "gpio.h"
#include "string.h"

// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "bsp.h"

// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------

#define BSP_FAKE_JUMP



// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------

extern int8_t bsp_led_global_init(void);

void Error_Handler(void);
void SystemClock_Config(void);


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------

//static uint32_t s_bsp_led_onmask = 0;


// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------


extern int8_t bsp_init(void)
{
    const uint8_t plus_systick = 0;
    HAL_Init(plus_systick);
    
    SystemClock_Config();
    
    bsp_led_global_init();
    
    return 0;
}

extern uint32_t bsp_sys_get_clock(void)
{
    return(SystemCoreClock);
}


extern void bsp_sys_reset(void)
{
    NVIC_SystemReset();
}

extern bool bsp_sys_jump2address(uint32_t address)
{
#if defined(BSP_FAKE_JUMP)
    return 0;
#else
    volatile uint32_t jumpaddr;
    void (*app_fn)(void) = NULL;

//    if(hl_res_NOK_generic == hl_sys_canjump(addr))
//    {
//        return(hl_res_NOK_generic);
//    }

    // prepare jump address 
    jumpaddr = *(volatile uint32_t*) (addr + 4);
    // prepare jumping function
    app_fn = (void (*)(void)) jumpaddr;
    // initialize user application's stack pointer 
    __set_MSP(*(volatile uint32_t*) addr);
    // jump.
    app_fn(); 
    
    // never in here.
    return(0); 
#endif     
}


extern int8_t bsp_led_init(uint8_t led, const void *dummyparam)
{
    if(led > bsp_led0)
    {
        return -1;
    }
    
    bsp_led_global_init();
    return 0;
}    

extern int8_t bsp_led_global_init(void) 
{
    static uint8_t initted = 0;
    
    if(1 == initted)
    {
        return 0;
    }
    
    initted = 1;
    
    
    MX_GPIO_Init();
    
//    s_bsp_led_onmask = 0;
        
    return 0;
}


extern int8_t bsp_led_on(uint8_t led)
{
    if(led > bsp_led0)
    {
        return -1;
    }
    
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    
    return 0;
}


extern int8_t bsp_led_off(uint8_t led) 
{    
    if(led > bsp_led0)
    {
        return -1;
    }
    
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    
    return 0;
}

extern int8_t bsp_led_toggle(uint8_t led)
{
    if(led > bsp_led0)
    {
        return -1;
    }
    
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    
//    if((1<<led) == (s_bsp_led_onmask & (1 << led) ))
//    {   // it is on
//        bsp_led_off(led);
//    }
//    else
//    {
//        bsp_led_on(led);
//    }
    
    return 0;
}

extern int8_t bsp_button_pressed(uint8_t btn)
{
    GPIO_PinState b1state = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
    
    return (GPIO_PIN_RESET == b1state) ? (1) : (0);
    
}


// flashstorage

static bool s_flashstorage_initted = false;
static uint64_t s_flashstorage_buffer[128] = {0};

static const uint32_t s_flashstorage_start = 0x08040000; // 256k, page #128
static const uint32_t s_flashstorage_size = 1024;

extern bool bsp_flashstorage_init(void)
{
    s_flashstorage_initted = true;
    
    HAL_FLASH_Unlock();
    //HAL_FLASH_OB_Unlock();
    
    return(s_flashstorage_initted);
}


extern bool bsp_flashstorage_isinitted(void)
{
    return(s_flashstorage_initted);     
}


extern bool bsp_flashstorage_isaddressvalid(uint32_t adr)
{
    if((adr < s_flashstorage_start) || (adr > s_flashstorage_start+s_flashstorage_size) )
    {
        return false;
    }
    return true;
}


extern uint32_t bsp_flashstorage_getbaseaddress(void)
{   
    return s_flashstorage_start;
}


extern uint32_t bsp_flashstorage_getsize(void)
{ 
    return s_flashstorage_size;
}


extern bool bsp_flashstorage_fullerase(void)
{
    // erase page
    FLASH_EraseInitTypeDef erase = {0};
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.Banks = FLASH_BANK_1;
    erase.Page = 128;
    erase.NbPages = 1;
    uint32_t pagenum = 0;
    HAL_FLASH_Unlock();
    int a = HAL_FLASHEx_Erase(&erase, &pagenum);
    a = a;
    HAL_FLASH_Lock();
    return true;
}


extern bool bsp_flashstorage_erase(uint32_t addr, uint32_t size)
{
    if(0 == bsp_flashstorage_isaddressvalid(addr))
    {
        return false;
    }
    if(0 == bsp_flashstorage_isaddressvalid(addr+size))
    {
        return false;
    }
    
    
    return false;
    
    // not supported yet.
    
//    // copy everything in ram,
//    bsp_flashstorage_read(s_flashstorage_start, s_flashstorage_size, s_flashstorage_buffer);
//    // erase fully
//    bsp_flashstorage_fullerase();
//    // copy pieces outsize of [addr, addr+size]
//    #warning TOBEDONE
//    
//    return 1;
}


extern bool bsp_flashstorage_read(uint32_t addr, uint32_t size, void *data)
{
    if(0 == bsp_flashstorage_isaddressvalid(addr))
    {
        return false;
    }
    if(0 == bsp_flashstorage_isaddressvalid(addr+size))
    {
        return false;
    }
    if(NULL == data)
    {
        return false;
    }
    
    // can read directly from flash
    void *st = (void*) addr;
    memcpy(data, st, size); 
    
    return true;
}


extern bool bsp_flashstorage_write(uint32_t addr, uint32_t size, const void *data)
{
    if(0 == bsp_flashstorage_isaddressvalid(addr))
    {
        return false;
    }
    if(0 == bsp_flashstorage_isaddressvalid(addr+size))
    {
        return false;
    }
    if(NULL == data)
    {
        return false;
    }
    
    // read all flash with buffer.  
    bsp_flashstorage_read(s_flashstorage_start, s_flashstorage_size, s_flashstorage_buffer);
    // change the buffer
    uint32_t reducedaddress = addr - s_flashstorage_start;
    memcpy(&s_flashstorage_buffer[reducedaddress/8], data, size);
    // erase page
    bsp_flashstorage_fullerase();
    
    HAL_FLASH_Unlock();
    
#if 0
    // write the whole buffer into the page
    uint32_t tmpadr = addr;
    uint64_t tmpdataadr = (uint64_t) &s_flashstorage_buffer[0];
    uint16_t iterations = 3; // 1024/256 - 1;
    for(uint8_t i=0; i<iterations; i++)
    {   
        // writing 256 bytes
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST, tmpadr, tmpdataadr);        
        tmpadr += 256;
        tmpdataadr += 256;
    }
    // writing the last 256 bytes
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST_AND_LAST, tmpadr, tmpdataadr);
#else
    uint32_t tmpadr = addr;
    for(uint16_t i=0; i<128; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, tmpadr, s_flashstorage_buffer[i]);
        tmpadr += 8;
    }
#endif
//FLASH_TYPEPROGRAM_DOUBLEWORD
    
    volatile uint32_t xx =  HAL_FLASH_GetError();
    
    xx= xx;
    
    HAL_FLASH_Lock();
    
    return true;
}

#if 0


HAL_FLASH_Unlock()

HAL_FLASH_Program()


#endif


// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------



/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the main internal regulator output voltage 
    */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
//#if defined(ACEMORISED)  
//#else  
//  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
//#endif
    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}


// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------



