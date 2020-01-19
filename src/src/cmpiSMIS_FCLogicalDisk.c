/*
 * $Id: cmpiSMIS_FCLogicalDisk.c,v 1.1.1.1 2009/05/12 21:46:34 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2006, 2007, 2009
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
 *      Logical representation of Fibre Channel disk.
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

#define TRUE 1
#define FALSE 0
#define STATUS 2

static char * _ClassName = "Linux_FCLogicalDisk";

/* ---------------------------------------------------------------------------*/
/*                            Factory functions                               */
/* ---------------------------------------------------------------------------*/

/******************************************************************************/
/* Function: _makePath_FCLogicalDisk                                          */
/* Purpose:  method to create a CMPIObjectPath of this class                  */
/******************************************************************************/
CMPIObjectPath * _makePath_FCLogicalDisk( const CMPIBroker * _broker,
                  const CMPIContext * ctx,
                  const CMPIObjectPath * ref,
                  CMPIStatus * rc,
                  struct cim_hbaLogicalDisk * sptr
                  ) 
   {
   CMPIObjectPath *op=NULL;
   char                          deviceID[DEVICEID_SIZE];         /* to hold FCLogicalDisk id */
   char                          * system_name = NULL; /* save pointer returned by get_system_name() */
   int                           status;
   int                           rc2 = 0;
   int                           numberofadapters = 0;
   int                           adapter_number = 0;
   char                          * adapter_name = NULL;
   struct hbaPortList            * lptr = NULL;
   int i = 0;
   _OSBASE_TRACE(1,("--- _makePath_FCLogicalDisk() called"));
  
   /* Setting System Name */
   system_name = get_system_name();
   if( ! system_name ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(4,("--- _makePath_FCLogicalDisk() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   /* Settiing op object */
   op=CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                       _ClassName, rc );
   if( CMIsNullObject(op) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(4,("--- _makePath_FCLogicalDisk() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   
   /***************/
   /* Adding Keys */
   /***************/
   CMAddKey(op, "SystemCreationClassName", CSCreationClassName, CMPI_chars);  
   CMAddKey(op, "SystemName", system_name, CMPI_chars);
   CMAddKey(op, "CreationClassName", _ClassName, CMPI_chars);  
   
   /************************/ 
   /* Getting OSDeviceName */
   /************************/
 
         strncpy(deviceID, sptr->OSDeviceName, DEVICEID_SIZE);
         deviceID[DEVICEID_SIZE - 1] = '\0';
         CMAddKey( op, "DeviceID", deviceID, CMPI_chars);

   exit:
   /* free system_name */
   if ( system_name ) free(system_name);
   system_name = NULL;
   _OSBASE_TRACE(1,("--- _makePath_FCLogicalDisk() exited"));
   return op;
   } /* end _makePath_FCLogicalDisk */





/******************************************************************************/
/* Function: _makeInst_FCLogicalDisk                                          */
/* Purpose:  method to create a CMPIInstance of this class                    */
/******************************************************************************/
CMPIInstance * _makeInst_FCLogicalDisk( const CMPIBroker * _broker,
                const CMPIContext * ctx,
                const CMPIObjectPath * ref,
                CMPIStatus * rc,
                struct cim_hbaLogicalDisk * sptr 
               ) 
   {
   CMPIObjectPath    * op     = NULL;
   CMPIInstance      * ci     = NULL;

   /*******************************************/
   /* Variables for settting fibre attributes */
   /*******************************************/
   char                     deviceID[DEVICEID_SIZE];      /* to hold FCLogicalDisk id */
   char                     IdentifyingDesc[] = "FcId of an FCP-2 SCSI Target device";
   CMPIArray                     *opStatus=NULL;            /* Array of mulitple statuses, like OK and Completed */
   CMPIArray                     *other_info=NULL;          /* StatusDescriptions is an array of mulitple status descriptions */
   CMPIArray                     *identifying_info=NULL;    /* An array of identifying descriptions */
   unsigned short                temp_uint16;               /* temporary variable for setting cim properties */
   unsigned long                 temp_uint32;               /* temporary variable for setting cim properties */
   unsigned long long            temp_uint64;               /* temporary variable for setting cim properties */
   CMPIArray                     *tempCIMArray=NULL;        /* temporary variable for holding a cmpi array  */
   char                          *temp_string;              /* temporary pointer to a string for setting cim properties */
   char                          *system_name = NULL;       /* save pointer returned by get_system_name() */
   int                           status;
   struct hbaPortList            *lptr = NULL;
   int i = 0;
 
   static char              *status_states[] = {"Unknown",
                                                "Other",
                                                "OK",
                                                "Degraded",
                                                "Stressed",
                                                "Predictive Failure",
                                                "Error",
                                                "Non-Recoverable Error",
                                                "Starting",
                                                "Stopping",
                                                "Stopped",
                                                "In Service",
                                                "No Contact",
                                                "Lost Communication",
                                                "Aborted",
                                                "Dormant",
                                                "Supporting Entity in Error",
                                                "Completed",
                                                "Power Mode",
                                                "DMTF Reserved", /* 19 - 0x8000 */
                                                "Vendor Reserved" /* 0x8000 ... */};

   _OSBASE_TRACE(1,("--- _makeInst_FCLogicalDisk() called"));

   /***********************/
   /* Setting system name */
   /***********************/
   system_name = get_system_name();
   if( ! system_name ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   /* Initializiing op object */
   op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                         _ClassName, rc );
   _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() made an op object"));
   if( CMIsNullObject(op) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDiskPort() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
 
   /* Initializiing ci object */
   ci = CMNewInstance( _broker, op, rc);
   _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() made a ci object"));
   if( CMIsNullObject(ci) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
      _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   
   /************************************/ 
   /* Setting general class attributes */
   /************************************/
   CMSetProperty( ci, "SystemCreationClassName",CSCreationClassName , CMPI_chars );
   _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set SystemCreationClassName")); 
   CMSetProperty( ci, "SystemName", system_name, CMPI_chars );
   _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set SystemName"));
   CMSetProperty( ci, "CreationClassName", _ClassName, CMPI_chars ); 
   _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set CreationClassName"));
   
   /************************/
   /* Set Fibre attributes */
   /************************/
   
   /************************/
   /* Setting OSDeviceName */
   /************************/

   strncpy(deviceID, sptr->OSDeviceName, DEVICEID_SIZE);
   deviceID[DEVICEID_SIZE - 1] = '\0';
   CMSetProperty( ci , "DeviceID", deviceID, CMPI_chars);
   _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set DeviceId"));


   /***********************/
   /* Setting Description */
   /***********************/
   CMSetProperty( ci, "Description", "This class represents instances of available Fibre Channel Logical Disk.", CMPI_chars);
   _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set Description"));

   /*******************/
   /* Setting Caption */
   /*******************/
   CMSetProperty( ci, "Caption", "Linux_FCLogicalDisk", CMPI_chars);
   _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set Caption"));

   /****************/
   /* Setting Name */
   /****************/
   CMSetProperty( ci, "Name", deviceID, CMPI_chars);  
    _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set Name: %s", deviceID));

   /***********************/
   /* Setting ElementName */
   /***********************/
   CMSetProperty( ci, "ElementName", "Other FC SCSI Disk Drive", CMPI_chars);
   _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set ElementName")); 

   /**********************/
   /* Setting NameFormat */
   /**********************/
   temp_uint16=12;
   CMSetProperty(ci, "NameFormat", (CMPIValue*)&(temp_uint16),CMPI_uint16);
   _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set NameFormat: %d", temp_uint16));

   /*************************/
   /* Setting NameNameSpace */
   /*************************/
   temp_uint16=8;
   CMSetProperty(ci, "NameNamespace", (CMPIValue*)&(temp_uint16),CMPI_uint16);
   _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set NameNameSpace: %d", temp_uint16));
   
   /*************************/
   /* Set Status Attributes */
   /*************************/
   opStatus = CMNewArray(_broker,1,CMPI_uint16,rc);
   if( opStatus == NULL ) 
      {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,1,CMPI_uint16,rc)" );
      _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   else 
      {
      unsigned short opStatus_state;
      opStatus_state=2;
      CMSetArrayElementAt(opStatus,0,(CMPIValue*)&(opStatus_state),CMPI_uint16);
      CMSetProperty( ci, "OperationalStatus", (CMPIValue*)&(opStatus), CMPI_uint16A);
      _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set OperationalStatus: %d",opStatus_state));
      }

   /**********************************/
   /* Setting IdentifyingDescription */
   /**********************************/
   identifying_info = CMNewArray(_broker,1,CMPI_string,rc);
   if( identifying_info == NULL )
      {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,1,CMPI_string,rc)" );
      _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   else 
      {
      CMPIString *value=NULL;
      value = CMNewString(_broker, IdentifyingDesc,rc);
      CMSetArrayElementAt(identifying_info,0,(CMPIValue*)&(value),CMPI_string);
      CMSetProperty( ci, "IdentifyingDescriptions", (CMPIValue*)&(identifying_info), CMPI_stringA);
      }
   /****************************************/
   /* Setting OtherIdentifying Information */
   /****************************************/
   
   other_info = CMNewArray(_broker,1,CMPI_string,rc);
   if( other_info == NULL )
   {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,1,CMPI_string,rc)" );
      _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
   } 
   else
   {
      CMPIString *val=NULL;
      char *other_information;
      other_information = (char*)malloc(BUFFER_SIZE);
      if (other_information == NULL)
      {
         _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() failed, not enough memory"));
         free (other_information);
         goto exit;
      }

      temp_uint64 = sptr->FcpLun;
      _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() FCPLun is %llx",temp_uint64));
      snprintf(other_information,BUFFER_SIZE,"%llx",temp_uint64);
      _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk(): set other_information to: %s",other_information));
      val = CMNewString(_broker,other_information,rc);
      CMSetArrayElementAt(other_info,0,(CMPIValue*)&(val),CMPI_string);
      CMSetProperty( ci, "OtherIdentifyingInfo", (CMPIValue*)&(other_info), CMPI_stringA);
      _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set OtherIdentifyingInfo"));
      free(other_information);
   }
   

   /**********************/
   /* Setting Primordial */
   /**********************/
   temp_uint16=TRUE;
   CMSetProperty(ci, "Primordial", (CMPIValue*)&(temp_uint16),CMPI_uint16);
   _OSBASE_TRACE(4,("--- _makeInst_FCLogicalDisk() set Primordial: %d", temp_uint16));

   CMSetProperty( ci, "Status", status_states[STATUS] , CMPI_chars);

 
   /****************************/
   /* End Set Fibre attributes */
   /****************************/


  exit:
   /* free system_name */
   if ( system_name ) free(system_name);

   _OSBASE_TRACE(1,("--- _makeInst_FCLogicalDisk() exited"));
   return ci;
} /* end _makeInst_FCLogicalDisk */


/* ---------------------------------------------------------------------------*/
/*                    end of cmpiSMIS_FCLogicalDisk.c                         */
/* ---------------------------------------------------------------------------*/

