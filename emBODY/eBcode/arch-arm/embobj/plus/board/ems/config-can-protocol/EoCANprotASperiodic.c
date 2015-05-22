/*
 * Copyright (C) 2015 iCub Facility - Istituto Italiano di Tecnologia
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


#include "stdlib.h"
#include "EoCommon.h"

// in here is whatever is required to offer parsing of can frames and copying into protocol data structures.

#include "EoProtocol.h"
#include "EoProtocolAS.h"

// but also to retrieve information of other things ...

#include "EOtheCANmapping.h"

// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "EOtheCANprotocolCfg.h" 


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------

typedef enum 
{
    processForce    = 0,    // keep it 0 so that we index teh array in position 0*3 = 0
    processTorque   = 1     // keep it 1 so taht we index teh array in position 1*3 = 3
} strainProcessMode_t;


typedef enum 
{
    processHES0TO6      = 0,    
    processHES7TO14     = 1     
} maisProcessMode_t;

// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------

static void* s_eocanprotASperiodic_get_entity(eOprotEndpoint_t endpoint, eOprot_entity_t entity, eOcanframe_t *frame, eOcanport_t port, uint8_t *index);

static eOresult_t s_eocanprotASperiodic_parser_process_forcetorque(eOcanframe_t *frame, eOcanport_t port, strainProcessMode_t mode);

static eOresult_t s_eocanprotASperiodic_parser_process_maisvalue(eOcanframe_t *frame, eOcanport_t port, maisProcessMode_t mode);


static void s_former_PER_AS_prepare_frame(eOcanprot_descriptor_t *descriptor, eOcanframe_t *frame, uint8_t len, uint8_t type);

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------


extern eOresult_t eocanprotASperiodic_parser_PER_AS_MSG__UNCALIBFORCE_VECTOR_DEBUGMODE(eOcanframe_t *frame, eOcanport_t port)
{
    // this can frame is from strain only ... i dont do the check that the board must be a strain
    // i retrieve the strain entity related to the frame    
    eOas_strain_t *strain = NULL;
    eOprotIndex_t index = EOK_uint08dummy;
    
    if(NULL == (strain = s_eocanprotASperiodic_get_entity(eoprot_endpoint_analogsensors, eoprot_entity_as_strain, frame, port, &index)))
    {
        #warning -> TODO: add diagnostics about not found board as in s_eo_icubCanProto_mb_send_runtime_error_diagnostics()
        return(eores_OK);  
    }    
    
    // copy into status... it should be done using the EOarray ....
    // memcpy(&(strain->status.uncalibratedvalues.data[0]), &frame->data[0], 6);   
    eo_array_Assign((EOarray*)(&strain->status.uncalibratedvalues), 0, &(frame->data[0]), 3);
    return(eores_OK);    
}

extern eOresult_t eocanprotASperiodic_parser_PER_AS_MSG__UNCALIBTORQUE_VECTOR_DEBUGMODE(eOcanframe_t *frame, eOcanport_t port)
{
    // this can frame is from strain only ... i dont do the check that the board must be a strain
    // i retrieve the strain entity related to the frame    
    eOas_strain_t *strain = NULL;
    eOprotIndex_t index = EOK_uint08dummy;
    
    if(NULL == (strain = s_eocanprotASperiodic_get_entity(eoprot_endpoint_analogsensors, eoprot_entity_as_strain, frame, port, &index)))
    {
        #warning -> TODO: add diagnostics about not found board as in s_eo_icubCanProto_mb_send_runtime_error_diagnostics()
        return(eores_OK);  
    }    
    
    // copy into status... it should be done using the EOarray ....
    // memcpy(&(status->uncalibratedvalues.data[6]), &frame->data[0], 6); 
    eo_array_Assign((EOarray*)(&strain->status.uncalibratedvalues), 3, &(frame->data[0]), 3);
    return(eores_OK);    
}


extern eOresult_t eocanprotASperiodic_former_PER_AS_MSG__FORCE_VECTOR(eOcanprot_descriptor_t *descriptor, eOcanframe_t *frame)
{
    s_former_PER_AS_prepare_frame(descriptor, frame, 6, ICUBCANPROTO_PER_AS_MSG__FORCE_VECTOR); 
    memcpy(&frame->data[0], descriptor->cmd.value, 6);      
    return(eores_OK);                
}

extern eOresult_t eocanprotASperiodic_parser_PER_AS_MSG__FORCE_VECTOR(eOcanframe_t *frame, eOcanport_t port)
{
    // this can frame is from strain only ... i dont do the check that the board must be a strain
    // process force
    s_eocanprotASperiodic_parser_process_forcetorque(frame, port, processForce);
    return(eores_OK);
}


extern eOresult_t eocanprotASperiodic_former_PER_AS_MSG__TORQUE_VECTOR(eOcanprot_descriptor_t *descriptor, eOcanframe_t *frame)
{
    s_former_PER_AS_prepare_frame(descriptor, frame, 6, ICUBCANPROTO_PER_AS_MSG__TORQUE_VECTOR); 
    memcpy(&frame->data[0], descriptor->cmd.value, 6);      
    return(eores_OK);                
}


extern eOresult_t eocanprotASperiodic_parser_PER_AS_MSG__TORQUE_VECTOR(eOcanframe_t *frame, eOcanport_t port)
{
    // this can frame is from strain only ... i dont do the check that the board must be a strain
    // process torque
    s_eocanprotASperiodic_parser_process_forcetorque(frame, port, processTorque);
    return(eores_OK);
}


extern eOresult_t eocanprotASperiodic_parser_PER_AS_MSG__HES0TO6(eOcanframe_t *frame, eOcanport_t port)
{
    // this can frame is from mais only ... i dont do the check that the board must be a mais
    // process values
    s_eocanprotASperiodic_parser_process_maisvalue(frame, port, processHES0TO6);
    return(eores_OK);    
}


extern eOresult_t eocanprotASperiodic_parser_PER_AS_MSG__HES7TO14(eOcanframe_t *frame, eOcanport_t port)
{
    // this can frame is from mais only ... i dont do the check that the board must be a mais
    // process values
    s_eocanprotASperiodic_parser_process_maisvalue(frame, port, processHES7TO14);
    return(eores_OK);    
}


extern eOresult_t eocanprotASperiodic_parser_00(eOcanframe_t *frame, eOcanport_t port)
{
    return(eores_OK);
}

extern eOresult_t eocanprotASperiodic_former_00(eOcanprot_descriptor_t *descriptor, eOcanframe_t *frame)
{
    return(eores_OK);
}

extern eOresult_t eocanprotASperiodic_parser_01(eOcanframe_t *frame, eOcanport_t port)
{
    return(eores_OK);
}

extern eOresult_t eocanprotASperiodic_former_01(eOcanprot_descriptor_t *descriptor, eOcanframe_t *frame)
{
    return(eores_OK);
}


// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------
// empty-section



// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------

static void* s_eocanprotASperiodic_get_entity(eOprotEndpoint_t endpoint, eOprot_entity_t entity, eOcanframe_t *frame, eOcanport_t port, uint8_t *index)
{
    void * ret = NULL;
    uint8_t ii = 0;
    eOcanmap_location_t loc = {0};
    
    loc.port = port;
    loc.addr = EOCANPROT_FRAME_GET_SOURCE(frame);    
    loc.insideindex = eocanmap_insideindex_none;
    
    ii = eo_canmap_GetEntityIndexExtraCheck(eo_canmap_GetHandle(), loc, endpoint, entity);
    
    if(EOK_uint08dummy == ii)
    {     
        #warning -> TODO: add diagnostics about not found board as in s_eo_icubCanProto_mb_send_runtime_error_diagnostics()
        return(NULL);
    }
    
    ret = eoprot_entity_ramof_get(eoprot_board_localboard, endpoint, entity, ii);
    
    if(NULL != index)
    {
        *index = ii;        
    }  

    return(ret);   
}


static eOresult_t s_eocanprotASperiodic_parser_process_forcetorque(eOcanframe_t *frame, eOcanport_t port, strainProcessMode_t mode)
{
    // this can frame is from strain only ... i dont do the check that the board must be a strain
    // i retrieve the strain entity related to the frame    
    eOas_strain_t *strain = NULL;
    eOprotIndex_t index = EOK_uint08dummy;
    
    if(NULL == (strain = s_eocanprotASperiodic_get_entity(eoprot_endpoint_analogsensors, eoprot_entity_as_strain, frame, port, &index)))
    {
        #warning -> TODO: add diagnostics about not found board as in s_eo_icubCanProto_mb_send_runtime_error_diagnostics()
        return(eores_OK);  
    }    
        
    // set incoming force values
    switch(strain->config.mode)
    {
        case eoas_strainmode_txcalibrateddatacontinuously:
        case eoas_strainmode_txalldatacontinuously:
        {
            eo_array_Assign((EOarray*)(&strain->status.calibratedvalues), 3*mode, &(frame->data[0]), 3);
        } break;

        case eoas_strainmode_txuncalibrateddatacontinuously:
        {
            eo_array_Assign((EOarray*)(&strain->status.uncalibratedvalues), 3*mode, &(frame->data[0]), 3);
        } break;
        
        case eoas_strainmode_acquirebutdonttx:
        {
            // i dont do anything in here. but i dont return nok. because it may be that we must empty a rx buffer of canframes rx just before
            // that we have silenced the strain.
        } break;
        
        default:
        {
            //i must never be here
            #warning -> TODO: add diagnostics about unknown mode as in s_eo_icubCanProto_mb_send_runtime_error_diagnostics()
        }
    }
    
    return(eores_OK);
}

static eOresult_t s_eocanprotASperiodic_parser_process_maisvalue(eOcanframe_t *frame, eOcanport_t port, maisProcessMode_t mode)
{
    // this can frame is from strain only ... i dont do the check that the board must be a strain
    // i retrieve the strain entity related to the frame    
    eOas_mais_t *mais = NULL;
    eOprotIndex_t index = EOK_uint08dummy;
    
    if(NULL == (mais = s_eocanprotASperiodic_get_entity(eoprot_endpoint_analogsensors, eoprot_entity_as_mais, frame, port, &index)))
    {
        #warning -> TODO: add diagnostics about not found board as in s_eo_icubCanProto_mb_send_runtime_error_diagnostics()
        return(eores_OK);  
    }  

    EOarray* array = (EOarray*)&mais->status.the15values;
    if(processHES0TO6 == mode)
    {
        eo_array_Assign(array, 0, &(frame->data[0]), 7); // 7 bytes of frame->data starting from array position 0 (0, 1, .. , 6)
    }
    else //if(processHES7TO14 == mode)
    {
        eo_array_Assign(array, 7, &(frame->data[0]), 8); // 8 bytes of frame->data starting from array position 7 (7, 8, .. , 14)
    }
        
    return(eores_OK);       
}

static void s_former_PER_AS_prepare_frame(eOcanprot_descriptor_t *descriptor, eOcanframe_t *frame, uint8_t len, uint8_t type)
{   // for periodic the descriptor->address contains ... the origin
    uint8_t origin = descriptor->loc.addr;
    frame->id           = EOCANPROT_CREATE_CANID_PERIODIC(eocanprot_msgclass_periodicAnalogSensor, origin, type);
    frame->id_type      = eocanframeID_std11bits;
    frame->frame_type   = eocanframetype_data; 
    frame->size         = len;
}

// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------





