
/* @file       hal_cfg.c
	@brief      This file keeps internal implementation of the osal.
	@author     marco.accame@iit.it
    @date       11/27/2009
**/


// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "hal.h"
#include "stdlib.h"
#include "osal.h"

#include "stdio.h"

// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "hal_cfg.h"


static void s_hal_cfg_on_fatalerror(hal_fatalerror_t errorcode, const char * errormsg);


extern const hal_cfg_t hal_cfg = 
{   
    .cpu_family             = (hal_cpufamily_t) HAL_CPUFAM,
    .cpu_type               = (hal_cputype_t) HAL_CPUTYPE,
    .cpu_freq               = HAL_CPUFREQ,
    .sys_stacksize          = HAL_SYS_STACKSIZE,
    .sys_heapsize           = HAL_SYS_HEAPSIZE,
    .display_enable         = (hal_boolval_t) HAL_SPIDISPLAY_ENABLE,
    .eth_enable             = (hal_boolval_t) HAL_ETH_ENABLE,
    .eth_dmatxbuffer_num    = HAL_ETH_DMA_TX_BUF,
    .eth_dmarxbuffer_num    = HAL_ETH_DMA_RX_BUF,
    .can1_enable            = (hal_boolval_t) HAL_CAN1_ENABLE,
    .can1_rxqnorm_num       = HAL_CAN1_INPBUFFCAPACITY,
    .can1_txqnorm_num       = HAL_CAN1_OUTBUFFCAPACITY,
    .can1_txqhigh_num       = 0,
    .can2_enable            = (hal_boolval_t) HAL_CAN2_ENABLE,
    .can2_rxqnorm_num       = HAL_CAN2_INPBUFFCAPACITY,
    .can2_txqnorm_num       = HAL_CAN2_OUTBUFFCAPACITY,
    .can2_txqhigh_num       = 0,
	.spi1_enable            = (hal_boolval_t) HAL_SPI1_ENABLE,
	.spi2_enable            = (hal_boolval_t) HAL_SPI2_ENABLE,
	.spi3_enable            = (hal_boolval_t) HAL_SPI3_ENABLE,
    .timers_num             = HAL_TIMERS_NUMBER,
    .arch                   = 
    {
        .nothingsofar                       = 0
    },
    .extfn                  =
    {
        .usr_on_fatal_error                 = s_hal_cfg_on_fatalerror,
        .osal_system_scheduling_suspend     = osal_system_scheduling_suspend,
        .osal_system_scheduling_restart     = osal_system_scheduling_restart
    }
    
};


extern const hal_cfg_t *hal_cfgMINE = &hal_cfg;


//void SysTick_Handler(void)
//{
//    hal_void_fp_void_t systickhandler = hal_sys_systick_gethandler();
//
//    if(NULL != systickhandler)
//    {
//        systickhandler();
//    }
//}


static void s_hal_cfg_on_fatalerror(hal_fatalerror_t errorcode, const char * errormsg)
{
    volatile uint32_t xxx = (uint32_t)errorcode;
    errorcode = errorcode;
    char str[128];
    if(NULL != errormsg)
    {
//        hal_display_putstring(4, (uint8_t*)errormsg);
        snprintf(str, sizeof(str)-1, "hal error #%d: %s", errorcode, errormsg);
    }
    
    hal_trace_puts(str);

    xxx = xxx;

    for(;;);

}







// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------



