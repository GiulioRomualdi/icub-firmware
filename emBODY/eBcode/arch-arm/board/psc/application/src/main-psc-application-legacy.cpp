/*
 * Copyright (C) 2019 iCub Facility - Istituto Italiano di Tecnologia
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


#undef TEST_ENABLED

#if     defined(TEST_ENABLED)

#undef TEST_HW_TIM
#define TEST_HW_TLV493D

#else

#undef TEST_HW_TIM
#undef TEST_HW_TLV493D

#endif // TEST_ENABLED



#if     defined(TEST_ENABLED)
void tests_launcher_init();
void tests_tick();

#if defined(TEST_HW_TLV493D)

#include "embot_hw_tlv493d.h"

//const embot::hw::TLV493D tlv = embot::hw::TLV493D::one;
//const embot::hw::I2C tlvi2c = embot::hw::I2C::one;

const embot::hw::TLV493D tlv = embot::hw::TLV493D::two;
const embot::hw::I2C tlvi2c = embot::hw::I2C::three;
embot::hw::tlv493d::Config tlvconfig { tlvi2c, 400000 };

#endif

#endif  // #if defined(TEST_ENABLED)

//#define DEBUG_TX_AT_START


#include "embot_app_theLEDmanager.h"
#include "embot_app_canprotocol.h"
#include "embot_app_theCANboardInfo.h"

#include "embot.h"

#include "embot_common.h"
#include "embot_binary.h"
#include "embot_dsp.h"

#include "stm32hal.h" 
#include "embot_hw_bsp.h"
#include "embot_hw.h"
#include "embot_hw_flash.h"
#include "embot_hw_sys.h"

#include "embot_hw_FlashStorage.h"
#include "embot_sys_theStorage.h"

#include "embot_app_theApplication.h"

#include "embot_app_application_theCANparserBasic.h"

#include "embot_app_application_theCANparserPOS.h"

#include "embot_app_application_thePOSreader.h"

#include "embot_app_application_theCANparserSkin.h"
#include "embot_app_application_theSkin.h"



static const embot::app::canprotocol::versionOfAPPLICATION vAP = {1, 2, 1};
static const embot::app::canprotocol::versionOfCANPROTOCOL vCP = {2, 0};

static void init(embot::sys::Task *t, void* param);
static void onidle(embot::sys::Task *t, void* param);
static void userdefonOSerror(void* param);


static const embot::sys::InitTask::Config initcfg = { 2048, init, nullptr };
static const embot::sys::IdleTask::Config idlecfg = { 512, nullptr, nullptr, onidle };
static const embot::common::Callback onOSerror = { userdefonOSerror, nullptr };

static const std::uint32_t address = embot::hw::flash::getpartition(embot::hw::FLASH::application).address;
constexpr bool initBSP = true;

int main(void)
{ 
    embot::app::theApplication::Config config { address, initBSP, embot::common::time1millisec, {initcfg, idlecfg, onOSerror} };
    embot::app::theApplication &appl = embot::app::theApplication::getInstance();    

    // it prepares the system to run at a given flash address, it inits the hw::bsp, 
    // and finally start scheduler which sets the callback executed by the scheduler on OS error, 
    // starts the idle task and the init task. this latter is executed at maximum priority, launches
    // its startup function and exits.
    appl.execute(config);  
        
    for(;;);    
}


static embot::sys::EventTask* start_evt_based(void);
static void start_usr_services(embot::sys::EventTask* evtsk);
static void start_sys_services();

static embot::sys::EventTask* eventbasedtask = nullptr;

static void init(embot::sys::Task *t, void* param)
{ 
    // start sys services: timer manager & callback manager
    start_sys_services();
    
    // make sure that the application and can protocol versions are synched
    embot::app::theCANboardInfo &canbrdinfo = embot::app::theCANboardInfo::getInstance();
    canbrdinfo.synch(vAP, vCP);
            
    // start the main task
    embot::sys::EventTask* maintask = start_evt_based();     
    
    // start the other services and link them to the main task
    start_usr_services(maintask);        
}

static void onidle(embot::sys::Task *t, void* param)
{
    static int a = 0;
    a++;    
}

static void userdefonOSerror(void *param)
{
    int code = 0;
    embot::sys::theScheduler::getInstance().getOSerror(code);
}


static void start_sys_services()
{
    // start them both w/ default config;
    embot::sys::theTimerManager::getInstance().start({});     
    embot::sys::theCallbackManager::getInstance().start({});    
}


static void eventbasedtask_onevent(embot::sys::Task *t, embot::common::EventMask evtmsk, void *p);
static void eventbasedtask_init(embot::sys::Task *t, void *p);


constexpr embot::common::Event evRXcanframe = 0x00000001 << 0;
constexpr embot::common::Event evPOS0Xacquire = 0x00000001 << 1;
constexpr embot::common::Event evPOS01dataready = 0x00000001 << 2;
constexpr embot::common::Event evPOS02dataready = 0x00000001 << 3;
static const embot::common::Event evSKINprocess = 0x00000001 << 4;

constexpr std::uint8_t maxOUTcanframes = 48;

constexpr std::array<embot::app::application::thePOSreader::Sensor, embot::app::application::thePOSreader::numberofpositions> POSsensors = 
{{
    {
        embot::hw::TLV493D::one,
        { embot::hw::I2C::one, 400000 }        
    },
    {
        embot::hw::TLV493D::two,
        { embot::hw::I2C::three, 400000 }   
    }   
}};

constexpr embot::app::application::thePOSreader::Events POSevents = { evPOS0Xacquire, {{ evPOS01dataready, evPOS02dataready }} };




static void alerteventbasedtask(void *arg);

static std::vector<embot::hw::can::Frame> outframes;


static embot::sys::EventTask* start_evt_based(void)
{           
    // create the main task 
    embot::sys::EventTask* tsk = new embot::sys::EventTask;  
    const embot::common::relTime waitEventTimeout = 50*embot::common::time1millisec;   
            
    embot::sys::EventTask::Config configEV;
    
    configEV.startup = eventbasedtask_init;
    configEV.onevent = eventbasedtask_onevent;
    configEV.param = nullptr;
    configEV.stacksize = 5*1024;
    configEV.priority = embot::sys::Priority::high200;
    configEV.timeout = waitEventTimeout;
    eventbasedtask->start(configEV);
    
    // and start it
    tsk->start(configEV);
    
    return tsk;
}

static void start_usr_services(embot::sys::EventTask* evtsk)
{    
#if defined(TEST_ENABLED)
    tests_launcher_init();
    return;
#endif // #if defined(TEST_ENABLED)  
    
    // led manager
    static const std::initializer_list<embot::hw::LED> allleds = {embot::hw::LED::one};  
    embot::app::theLEDmanager &theleds = embot::app::theLEDmanager::getInstance();     
    theleds.init(allleds);    
    theleds.get(embot::hw::LED::one).pulse(embot::common::time1second); 

    // start canparser basic
    embot::app::application::theCANparserBasic &canparserbasic = embot::app::application::theCANparserBasic::getInstance();
    embot::app::application::theCANparserBasic::Config configbasic;
    canparserbasic.initialise(configbasic);  
           
    // start agent of POSreader
    embot::app::application::thePOSreader &thepos = embot::app::application::thePOSreader::getInstance();
    embot::app::application::thePOSreader::Config configpos { eventbasedtask, POSsensors, POSevents };
    thepos.initialise(configpos); 
    
    // start parser of POS and link it to its agent: thePOSreader
    embot::app::application::theCANparserPOS &canparserpos = embot::app::application::theCANparserPOS::getInstance();
    embot::app::application::theCANparserPOS::Config configparserpos { &thepos };
    canparserpos.initialise(configparserpos);    

    // start agent of skin 
    embot::app::application::theSkin &theskin = embot::app::application::theSkin::getInstance();
    embot::app::application::theSkin::Config configskin;
    configskin.tickevent = evSKINprocess;
    configskin.totask = eventbasedtask;
    theskin.initialise(configskin);   
    
    // start canparser skin and link it to its agent
    embot::app::application::theCANparserSkin &canparserskin = embot::app::application::theCANparserSkin::getInstance();
    embot::app::application::theCANparserSkin::Config configparserskin { &theskin };
    canparserskin.initialise(configparserskin);      

    // finally start can. i keep it as last because i dont want that the isr-handler calls its onrxframe() 
    // before the eventbasedtask is created.
    embot::hw::can::Config canconfig;   // default is tx/rxcapacity=8
    canconfig.txcapacity = maxOUTcanframes;
    canconfig.onrxframe = embot::common::Callback(alerteventbasedtask, evtsk); 
    embot::hw::can::init(embot::hw::CAN::one, canconfig);
    embot::hw::can::setfilters(embot::hw::CAN::one, embot::app::theCANboardInfo::getInstance().getCANaddress());
    
}

static void alerteventbasedtask(void *arg)
{
    embot::sys::EventTask* evtsk = reinterpret_cast<embot::sys::EventTask*>(arg);
    if(nullptr != evtsk)
    {
        evtsk->setEvent(evRXcanframe);
    }
}




static void eventbasedtask_init(embot::sys::Task *t, void *p)
{
    embot::hw::result_t r = embot::hw::can::enable(embot::hw::CAN::one);  
    r = r;         
    outframes.reserve(maxOUTcanframes);
}
    


static void eventbasedtask_onevent(embot::sys::Task *t, embot::common::EventMask eventmask, void *p)
{   
    if(0 == eventmask)
    {   // timeout ...       
        return;
    }    
    // we clear the frames to be trasmitted
    outframes.clear();      
        
    if(true == embot::binary::mask::check(eventmask, evRXcanframe))
    {        
        embot::hw::can::Frame frame;
        std::uint8_t remainingINrx = 0;
        if(embot::hw::resOK == embot::hw::can::get(embot::hw::CAN::one, frame, remainingINrx))
        {            
            embot::app::application::theCANparserBasic &canparserbasic = embot::app::application::theCANparserBasic::getInstance();
            embot::app::application::theCANparserPOS &canparserpos = embot::app::application::theCANparserPOS::getInstance();
            embot::app::application::theCANparserSkin &canparserskin = embot::app::application::theCANparserSkin::getInstance();

            // process w/ the basic parser, if not recognised call the parse specific of the board
            if(true == canparserbasic.process(frame, outframes))
            {                   
            }
            else if(true == canparserpos.process(frame, outframes))
            {               
            }
            else if(true == canparserskin.process(frame, outframes))
            {               
            }
            
            if(remainingINrx > 0)
            {
                t->setEvent(evRXcanframe);                 
            }
        }        
    }
    
    
    if(true == embot::binary::mask::check(eventmask, evPOS0Xacquire))
    {        
        embot::app::application::thePOSreader &thepos = embot::app::application::thePOSreader::getInstance();
        thepos.process(evPOS0Xacquire, outframes);        
    }
    
    if(true == embot::binary::mask::check(eventmask, evPOS01dataready))
    {        
        embot::app::application::thePOSreader &thepos = embot::app::application::thePOSreader::getInstance();
        thepos.process(evPOS01dataready, outframes);        
    }
    
    if(true == embot::binary::mask::check(eventmask, evPOS02dataready))
    {        
        embot::app::application::thePOSreader &thepos = embot::app::application::thePOSreader::getInstance();
        thepos.process(evPOS02dataready, outframes);        
    }    
            
    if(true == embot::binary::mask::check(eventmask, evSKINprocess))
    {
        embot::app::application::theSkin &theskin = embot::app::application::theSkin::getInstance();
        theskin.tick(outframes);        
    }  
    
    // if we have any frame we transmit them
    std::uint8_t num = outframes.size();
    if(num > 0)
    {
        for(std::uint8_t i=0; i<num; i++)
        {
            embot::hw::can::put(embot::hw::CAN::one, outframes[i]);                                       
        }

        embot::hw::can::transmit(embot::hw::CAN::one);  
    } 
 
}

static void userdefonidle(void* param)
{
    static std::uint32_t cnt = 0;
    
    cnt++;
}


// - all the tests are in here

#if defined(TEST_ENABLED)


#if defined(TEST_HW_TIM)

embot::hw::timer::Timer timer2use = embot::hw::timer::Timer::six;
static const std::uint32_t maxpulses = 10;
static std::uint64_t tt0 = 0;
static std::uint64_t del =0;
static std::int64_t diff =0;
void toggleled(void*)
{
    static std::uint32_t count = 0;
    static std::uint8_t on = 0;
    static std::uint32_t num = 0;
    if(count == 5000)
    {
        
        std::uint64_t tt1 = embot::sys::timeNow();
        
        del = tt1 - tt0;
        del = del;
        
        diff = del - 500000;
        
        tt0 = tt1;
        
        if(0 == on)
        {
            embot::hw::led::off(embot::hw::led::LED::one); 
            on = 1;
        }
        else 
        {
            embot::hw::led::on(embot::hw::led::LED::one);
            on = 0;
            num++;
        }
        //embot::hw::led::toggle(embot::hw::led::LED::one);
        count = 0;
    }
    count ++;
    
    if(num >= maxpulses)
    {
        static std::uint8_t x = 0;
        num = 0;
        embot::hw::timer::stop(timer2use);
        embot::hw::timer::Config con;
        con.onexpiry.callback = toggleled;
        if(0 == x)
        {
            con.time = embot::common::time1microsec * 50;
            embot::hw::timer::configure(timer2use, con);
            x = 1;
        }
        else
        {
            con.time = embot::common::time1microsec * 100;
            embot::hw::timer::configure(timer2use, con);
            x = 0;
        }
        embot::hw::timer::start(timer2use);
    }
}

void test_tim_init(void)
{
    embot::hw::led::init(embot::hw::led::LED::one);
    
    embot::hw::led::on(embot::hw::led::LED::one); 
    embot::hw::led::off(embot::hw::led::LED::one); 
    
    embot::hw::timer::Config con;
    con.time = embot::common::time1microsec * 100;
    con.onexpiry.callback = toggleled;
    embot::hw::timer::init(timer2use, con);
    embot::hw::timer::start(timer2use);
}

#endif // defined(TEST_HW_TIM)





void tests_launcher_init()
{
    
#if defined(TEST_HW_TLV493D)       
//    embot::hw::tlv493d::Config tlvconfig { embot::hw::I2C::one, 400000 };
//    embot::hw::tlv493d::init(embot::hw::TLV493D::one, tlvconfig);    

    embot::hw::tlv493d::init(tlv, tlvconfig);       
#endif
    
     
#if defined(TEST_HW_TIM)    
    test_tim_init();
#endif

}

#if defined(TEST_HW_TLV493D) 
void counter(void *p)
{
    static int count = 0;    
    count++;
}
#endif


void tests_tick() 
{
    
    
#if defined(TEST_HW_TLV493D)
    embot::hw::tlv493d::Position vv = 0;    
    embot::hw::tlv493d::Position pp = 0;
    embot::common::Callback cbk(counter, nullptr);
    
    embot::hw::tlv493d::acquisition(tlv, cbk);  
    
    for(;;)
    {
        if(true == embot::hw::tlv493d::operationdone(tlv))
        {
            break;
        }
    }
    
    embot::hw::tlv493d::read(tlv, pp);   
    vv = pp;
    vv = vv + 1;
       
#endif
    
    
    
#if defined(TEST_HW_SI7051)
    
    embot::common::Time starttime = embot::sys::timeNow();
    embot::common::Time endtime = starttime;
      
    embot::hw::SI7051::Temperature temp;    

    const int ntimes = 1000;
    embot::common::Callback cbk(counter, nullptr);
    int num = 0;
    for(int i=0; i<ntimes; i++)
    {
   
        embot::hw::SI7051::acquisition(SI7051sensor, cbk);
        
        for(;;)
        {
            if(true == embot::hw::SI7051::operationdone(SI7051sensor))
            {
                break;
            }
        }
        
        embot::hw::SI7051::read(SI7051sensor, temp);    
     
    }


    endtime = embot::sys::timeNow();
    
    static embot::common::Time delta = 0;
    delta = endtime - starttime;
    delta = delta;
    num = num;
    // 7 millisec each
    
#endif
    
}

#endif // #if defined(TEST_ENABLED)

///

