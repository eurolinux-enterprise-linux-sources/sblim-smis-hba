/*
 * $Id: cmpiSMIS_FCPortController.c,v 1.1.1.1 2009/05/12 21:46:34 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2006, 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Author: IBM
 *
 * Contributors: IBM
 *
 * Description:
 *      Fibre Channel Port Controller.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

#include "cmpiLinux_Common.h"
#include "Linux_Common.h"
#include "Linux_CommonHBA.h"

#include "cmpiSMIS_FC.h"

static char * _ClassName = "Linux_FCPortController";

/* ---------------------------------------------------------------------------*/
/* private declarations                                                       */


/* ---------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------------*/
/*                            Factory functions                               */
/* ---------------------------------------------------------------------------*/

/* ---------- method to create a CMPIObjectPath of this class ----------------*/

CMPIObjectPath * _makePath_FCPortController( const CMPIBroker * _broker,
                  const CMPIContext * ctx,
                  const CMPIObjectPath * ref,
                  const struct cim_hbaPort * sptr,
                  CMPIStatus * rc) 
   {
   CMPIObjectPath *op=NULL;
   char           deviceID[17];   /* to hold FCPortController id */
   char         * system_name = NULL;  /* save pointer returned by get_system_name */

   _OSBASE_TRACE(1,("--- _makePath_FCPortController() called"));

   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   system_name = get_system_name();
   if( ! system_name ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(2,("--- _makePath_FCPortController() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   op=CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                       _ClassName, rc );
   if( CMIsNullObject(op) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(2,("--- _makePath_FCPortController() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   CMAddKey(op, "SystemCreationClassName", CSCreationClassName, CMPI_chars);  
   CMAddKey(op, "SystemName", system_name, CMPI_chars);
   CMAddKey(op, "CreationClassName", _ClassName, CMPI_chars);  



   snprintf(deviceID,17,"%llx",*(unsigned long long*)sptr->port_attributes->PortWWN.wwn);
   deviceID[16]='\0';
   CMAddKey(op, "DeviceID", deviceID, CMPI_chars); 
     
   exit:
   /* free system_name */
   if ( system_name ) free(system_name);
   
   _OSBASE_TRACE(1,("--- _makePath_FCPortController() exited"));
   return op;
   } /* end _makePath_FCPortController */


   
/******************************************************************************/
/* Function: _makeInst_FCPortController                                       */
/* Purpose:  method to create a CMPIInstance of this class                    */
/******************************************************************************/
CMPIInstance * _makeInst_FCPortController( const CMPIBroker * _broker,
                const CMPIContext * ctx,
                const CMPIObjectPath * ref,
                const struct cim_hbaPort * sptr,
                CMPIStatus * rc) 
   {
   CMPIObjectPath    * op     = NULL;
   CMPIInstance      * ci     = NULL;
   
   /* Variables for settting FC port controller attributes */
   char                     deviceID[17];          /* to hold FCPortController id */
   char                     elementName[256];      /* to hold device name */
   static unsigned short    controllerType = 4;    /* controllerType = 4 means FC type  */
   static unsigned long     maxNumberController = 1; /* value ="1". On linux, 1 Port maps to 1 PortController */
   static char              *status_states[] = {"Unknown",   /* Blank to skip index 0 */  
                                                "Unknown",   /* from HBA PortState Unknown */
                                                "OK",        /* from HBA PortState Online */ 
                                                "Stopped",   /* from HBA PortState Offline */ 
                                                "Unknown",   /* from HBA PortState Bypassed */ 
                                                "Service",   /* from HBA PortState Diagnostics */ 
                                                "Error",     /* from HBA PortState Linkdown */ 
                                                "Error",     /* from HBA PortState Error */ 
                                                "Unknown"    /* from HBA PortState Loopback */                                             
                                               };
   static unsigned short    opStatus_states[9] = {0,   /* Blank to skip index 0 */ 
                                                  0,   /* from HBA PortState Unknown to Cim opState Unknown */
                                                  2,   /* from HBA PortState Online to Cim opState OK */
                                                  15,  /* from HBA PortState Offline to Cim opState Dormant */
                                                  1,   /* from HBA PortState Bypassed to Cim opState Other */
                                                  11,  /* from HBA PortState Diagnostics to Cim opState In Service */
                                                  1,   /* from HBA PortState Linkdown to Cim opState Other */
                                                  6,   /* from HBA PortState Error to Cim opState Error */
                                                  1,   /* from HBA PortState Loopback to Cim opState Other */
                                                };
   CMPIArray                   *opStatus=NULL;      /* OperationalStatus is an array of mulitple statuses, like OK and Completed */
   CMPIArray                   *opStatusDescs=NULL; /* StatusDescriptions is an array of mulitple status descriptions */
   static char                 *portStates[] = {"","Unknown","Online","Offline","Bypassed","Diagnostics","Linkdown","Error","Loopback"}; 
   unsigned short              temp_uint16;         /* temporary variable for setting cim properties */
   CMPIArray                   *tempCIMArray=NULL;  /* temporary variable for holding a cmpi array  */
   char                        *system_name = NULL; /* save pointer returned by get_system_name() */
   
   _OSBASE_TRACE(1,("--- _makeInst_FCPortController() called"));

   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   system_name = get_system_name();
   if( ! system_name )
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(2,("--- _makeInst_FCPortController() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                         _ClassName, rc );
   if( CMIsNullObject(op) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(2,("--- _makeInst_FCPortController() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   ci = CMNewInstance( _broker, op, rc);
   if( CMIsNullObject(ci) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
      _OSBASE_TRACE(2,("--- _makeInst_FCPortController() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   
   /* Set general class attributes */
   CMSetProperty( ci, "SystemCreationClassName",CSCreationClassName , CMPI_chars ); 
   CMSetProperty( ci, "SystemName", system_name, CMPI_chars );
   CMSetProperty( ci, "CreationClassName", _ClassName, CMPI_chars ); 
   
   
   /************************************************/
   /* Set Fibre Channel Port Controller attributes */
   /************************************************/
  
   /* Set attributes from PortWWN */
   snprintf(deviceID,17,"%llx",*(unsigned long long*)sptr->port_attributes->PortWWN.wwn);
   deviceID[16]='\0';   
   CMSetProperty( ci, "DeviceID", deviceID, CMPI_chars); 
   
   /* Set attributes from OSDeviceName */
   snprintf(elementName,256,"%s",sptr->port_attributes->OSDeviceName);
   elementName[255]='\0';      
   CMSetProperty( ci, "ElementName", elementName, CMPI_chars);


   /* Set adapter name attributes, Using PortWWN */
   CMSetProperty( ci, "Name", sptr->InstanceID, CMPI_chars);  
            
   /* Set misc hardcoded attributes */
   CMSetProperty( ci, "Caption", "Linux_FCPortController", CMPI_chars);
   CMSetProperty( ci, "Description", "This class represents instances of available Fibre Channel Port Controllers.", CMPI_chars);
   CMSetProperty( ci, "ControllerType", (CMPIValue*)&(controllerType), CMPI_uint16);
   CMSetProperty( ci, "MaxNumberControlled", (CMPIValue*)&(maxNumberController), CMPI_uint32);	
    	
   /* set status attributes */
   CMSetProperty( ci, "Status", status_states[sptr->port_attributes->PortState] , CMPI_chars);
   
   opStatus = CMNewArray(_broker,1,CMPI_uint16,rc);
   if( opStatus == NULL ) 
      {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,1,CMPI_uint16,rc)" );
      _OSBASE_TRACE(2,("--- _makeInst_FCPortController() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   else 
      {
      unsigned short opStatus_state;
      opStatus_state=opStatus_states[sptr->port_attributes->PortState];
      CMSetArrayElementAt(opStatus,0,(CMPIValue*)&(opStatus_state),CMPI_uint16);
      CMSetProperty( ci, "OperationalStatus", (CMPIValue*)&(opStatus), CMPI_uint16A);
      }

   opStatusDescs = CMNewArray(_broker,19,CMPI_string,rc);
   if( opStatusDescs == NULL ) 
      {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,20,CMPI_string,rc)" );
      _OSBASE_TRACE(2,("--- _makeInst_FCPortController() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   else 
      {
      CMPIString *val=NULL;
      char *opStatusDesc;
      int index=0;
      int match; /* the OperationalStatus single value */

      opStatusDesc=portStates[sptr->port_attributes->PortState];
      match=opStatus_states[sptr->port_attributes->PortState];
      for(index=0; index<19; index++)
         {
         if(index==match)
           val = CMNewString(_broker,opStatusDesc,rc);     
        else
           val = CMNewString(_broker,"",rc);     
         CMSetArrayElementAt(opStatusDescs,index,(CMPIValue*)&(val),CMPI_string); 
        }
      CMSetProperty( ci, "StatusDescriptions", (CMPIValue*)&(opStatusDescs), CMPI_stringA);        
      } 	 
      
   /* Set n/a attributes */
   temp_uint16=5; /* 5=n/a for EnabledState */
   CMSetProperty( ci, "EnabledState", (CMPIValue*)&(temp_uint16), CMPI_uint16);
   temp_uint16=6; /* 6=n/a for Availability and AdditionalAvailability */
   CMSetProperty( ci, "Availability", (CMPIValue*)&(temp_uint16), CMPI_uint16);
   CMSetProperty( ci, "AdditionalAvailability", (CMPIValue*)&(temp_uint16), CMPI_uint16);	
	
	 
   
   /****************************************************/
   /* End Set Fibre Channel Port Controller attributes */
   /****************************************************/

  exit:
   /* free system_name */
   if ( system_name ) free(system_name);

   _OSBASE_TRACE(1,("--- _makeInst_FCPortController() exited"));
   return ci;
} /* end _makeInst_FCPortController */

    		      
/* ---------------------------------------------------------------------------*/
/*                    end of cmpiSMIS_FCPortController.c                      */
/* ---------------------------------------------------------------------------*/

   	
