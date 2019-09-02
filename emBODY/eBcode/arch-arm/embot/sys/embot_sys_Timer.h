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

#ifndef _EMBOT_SYS_TIMER_H_
#define _EMBOT_SYS_TIMER_H_

#include "embot_common.h"

#include "embot_sys.h"

#include "embot_sys_Action.h"

namespace embot { namespace sys {
    
    class Timer
    {
    public:
        
        enum class Status { idle = 0, counting = 1 }; 
        
        enum class Mode { oneshot = 0, someshots = 1, forever = 2 }; 
        
        struct Config
        {
            common::relTime countdown {0};          // time to count before execute the action
            sys::Action     onexpiry {};            // the action executed at expiry of the countdown
            Mode            mode {Mode::oneshot};   // the mode. we allow one shot, infinite shots or a limited number
            std::uint32_t   numofshots {0};         // in case of limited number this number tells how many. not zero....

            Config() = default;
            Config(common::relTime c, const sys::Action &a, Mode m = Mode::oneshot, std::uint32_t n = 0) : countdown(c), onexpiry(a), mode(m), numofshots(n) {} 
            void clear() { countdown = 0;  onexpiry.clear(); mode = Mode::oneshot; numofshots = 0;}
            bool isvalid() const 
            { 
                if((0 == countdown) || (false == onexpiry.isvalid()) || ((Mode::someshots == mode) && (0 == numofshots))) 
                { 
                    return false; 
                }
                else 
                { 
                    return true; 
                }  
            }
        };
      
        Timer();
        ~Timer();
        bool start(const Config &config); 
        bool stop();  
        bool execute(); // to be called by the TimerManager only. if a brave user calls it ... it does nothing because ... black magic woman      
        
        Status getStatus() const;              
        const Config& getConfig() const;
                

    private:        
        struct Impl;
        Impl *pImpl;    
    };

    
}} // namespace embot { namespace sys {


#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------
