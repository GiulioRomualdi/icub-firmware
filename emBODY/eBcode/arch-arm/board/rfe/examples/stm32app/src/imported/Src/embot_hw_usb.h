
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

// - include guard ----------------------------------------------------------------------------------------------------

#ifndef _EMBOT_HW_USB_H_
#define _EMBOT_HW_USB_H_

#include "embot_common.h"
//#include "embot_hw.h" per evitare di ricompilare tutto!!!
#include "C:\VALE_WORKSPACE\CODE\iCub_world\firmware\icub-firmware-MY_RFE\emBODY\eBcode\arch-arm\embot\hw\embot_hw.h"



namespace embot { namespace hw { namespace usb {
    
    enum class MessageSize {min=0, max=10} ;
    struct Message            
    {
        std::uint16_t   size;
        std::uint8_t    data[static_cast<std::uint32_t>(MessageSize::max)];
        Message():size(0){data[0] = '\0';};
    };
    
    enum class Port { one = 0, none = 32, maxnumberof = 1};
    /*NOTE: currently this driver is only a wrap of stm32 usb stack plus a fifo for received message. 
            The trasmission is performed in busy wait loop, so no tx-fifo is necessary.
            The received messages are put in a rx-fifo and @onrxframe callback is invoked    */
    struct Config
    {
        std::uint8_t                rxcapacity;
        embot::common::Callback     onrxmessage;
        Config() : rxcapacity(8),  onrxmessage(nullptr, nullptr) {}
    };
    
    bool supported(Port p);
    
    bool initialised(Port p);
    
    result_t init(Port p, const Config &config);
    
    result_t enable(Port p);
    
    result_t disable(Port p);
    
//    result_t put(Port p, const Message &msg); //not implemented
//    
//    std::uint8_t outputqueuesize(Port p); //not implemented
//    
//    result_t transmit(Port p); //not implemnted
    
    result_t transmitimmediately(Port P, Message &msg); 
    
    std::uint8_t inputqueuesize(Port p);
    
    result_t get(Port p, Message &msg, std::uint8_t &remaining);
    
    void callbackOnRXcompletion(std::uint8_t* Buf, std::uint32_t *Len);//momentaneamente la metto qui
    
    
    
    
}}} // namespace embot { namespace hw { namespace usb {



#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------


