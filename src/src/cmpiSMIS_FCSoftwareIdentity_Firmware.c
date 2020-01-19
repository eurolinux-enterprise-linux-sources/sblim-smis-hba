/*
 * $Id: cmpiSMIS_FCSoftwareIdentity_Firmware.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2007, 2009
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
 *      Logical representation of FC firmware.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

#include "cmpiLinux_Common.h"
#include "Linux_Common.h"

#include "cmpiSMIS_FC.h"

static char * _ClassName = "Linux_FCSoftwareIdentity_Firmware";


/* ---------------------------------------------------------------------------*/
/* private functions                                                          */
/* ---------------------------------------------------------------------------*/



/******************************************************************************/
/* Function: _makeKey_FCSoftwareIdentity_Firmware                                               */
/* Purpose:  function to create an instanceID from the passed in adapter attr */
/* Inputs:   sptr which will hold the retrieved hba adapter data              */
/* Outputs:  Returns a created string holding the instanceID                  */
/* Notes:    Freeing the returned string is the responsibility of the caller. */
/******************************************************************************/
char * _makeKey_FCSoftwareIdentity_Firmware( const struct cim_hbaAdapter * sptr)
   {
   char           *instanceID=NULL; /* to hold FCSoftwareIdentity_Firmware InstanceID */
   int            instanceID_len;   /* InstanceID length */

   _OSBASE_TRACE(3,("--- _makeKey_FCSoftwareIdentity_Firmware() called"));

   /* Create instanceID string */
   instanceID_len= strlen(sptr->adapter_attributes->Manufacturer) +1 +
                   strlen(sptr->adapter_attributes->Model) +1 + 
                   8 +1 + /* 8 to hold "firmware"*/
                   
                   strlen(sptr->adapter_attributes->FirmwareVersion) + 1;
   instanceID= (char *) malloc(instanceID_len);
   
      
   snprintf(instanceID,instanceID_len,"%s-%s-firmware-%s",
                               sptr->adapter_attributes->Manufacturer,
                               sptr->adapter_attributes->Model,
                               sptr->adapter_attributes->FirmwareVersion);     
   instanceID[instanceID_len-1]='\0';

   _OSBASE_TRACE(2,("--- _makeKey_FCSoftwareIdentity_Firmware() exited"));
   return instanceID;

   } /* end _makeKey_FCSoftwareIdentity_Firmware */
   



/* ---------------------------------------------------------------------------*/
/* end private functions                                                      */
/* ---------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------------*/
/*                            Factory functions                               */
/* ---------------------------------------------------------------------------*/

/******************************************************************************/
/* Function: _makePath_FCSoftwareIdentity_Firmware                              */
/* Purpose:  method to create a CMPIObjectPath of this class                  */
/******************************************************************************/
CMPIObjectPath * _makePath_FCSoftwareIdentity_Firmware( const CMPIBroker * _broker,
                  const CMPIContext * ctx,
                  const CMPIObjectPath * ref,
                  const struct cim_hbaAdapter * sptr,
                  CMPIStatus * rc) 
   {
   CMPIObjectPath *op=NULL;
   char           *instanceID;      /* to hold FCSoftwareIdentity_Firmware InstanceID */
   char           *system_name = NULL; /* save pointer returned by get_system_name() */

   _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity_Firmware() called"));

   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   system_name = get_system_name();
   if( ! system_name ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity_Firmware() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   /* free system_name */
   free ( system_name );

   op=CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                       _ClassName, rc );
   if( CMIsNullObject(op) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity_Firmware() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   /* Create instanceID string */
   instanceID= _makeKey_FCSoftwareIdentity_Firmware( sptr );
   if (instanceID==NULL)
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create InstanceID failed." );
                      _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity_Firmware() failed : %s",CMGetCharPtr(rc->msg)));
                      goto exit;       
      } 
   CMAddKey(op, "InstanceID", instanceID, CMPI_chars); 
   free(instanceID);
     
   exit:
   _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity_Firmware() exited"));
   return op;
   } /* end _makePath_FCSoftwareIdentity_Firmware */





/******************************************************************************/
/* Function: _makeInst_FCSoftwareIdentity_Firmware                              */
/* Purpose:  method to create a CMPIInstance of this class                    */
/******************************************************************************/
CMPIInstance * _makeInst_FCSoftwareIdentity_Firmware( const CMPIBroker * _broker,
                const CMPIContext * ctx,
                const CMPIObjectPath * ref,
                const struct cim_hbaAdapter * sptr,
                CMPIStatus * rc) 
   {
   CMPIObjectPath    * op     = NULL;
   CMPIInstance      * ci     = NULL;
   int identityType=FIRMWARE;
   
   /* Variables for settting fibre attributes */
   char                        *instanceID;           /* to hold FCSoftwareIdentity_Firmware InstanceID */
   unsigned short              temp_uint16;           /* temporary variable for setting cim properties */
   unsigned long               temp_uint32;           /* temporary variable for setting cim properties */
   unsigned long long          temp_uint64;           /* temporary variable for setting cim properties */
   CMPIArray                   *tempCIMArray=NULL;    /* temporary variable for holding a cmpi array  */
   char                        *temp_string;          /* temporary pointer to a string for setting cim properties */
   char                        *system_name;          /* save pointer returned by get_system_name() */
   int                         len;

   _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Firmware() called"));

   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   system_name = get_system_name();
   if( ! system_name ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Firmware() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   /* free system_name */
   free( system_name );

   op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                         _ClassName, rc );
   if( CMIsNullObject(op) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Firmware() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   ci = CMNewInstance( _broker, op, rc);
   if( CMIsNullObject(ci) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Firmware() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   
   
   /************************/
   /* Set Fibre attributes */
   /************************/


   /* Create instanceID string */
   instanceID= _makeKey_FCSoftwareIdentity_Firmware( sptr );
   if (instanceID==NULL)
      {
      CMSetStatusWithChars( _broker, rc,
                           CMPI_RC_ERR_FAILED, "Create InstanceID failed." );
                       _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Firmware() failed : %s",CMGetCharPtr(rc->msg)));
                      goto exit;       
      } 
   CMSetProperty( ci, "InstanceID", instanceID, CMPI_chars); 
   CMSetProperty( ci, "Name", instanceID, CMPI_chars); 
   CMSetProperty( ci, "ElementName", instanceID, CMPI_chars); 
   free(instanceID);


   /* Set Classifications */
   tempCIMArray = CMNewArray(_broker,1,CMPI_uint16,rc);
   if( tempCIMArray == NULL ) 
      {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,1,CMPI_uint16,rc)" );
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Firmware() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   else 
      {
      temp_uint16=identityType;
      CMSetArrayElementAt(tempCIMArray,0,(CMPIValue*)&(temp_uint16),CMPI_uint16);
      CMSetProperty( ci, "Classifications", (CMPIValue*)&(tempCIMArray), CMPI_uint16A);
      }

   /* Set TargetOperatingSystems */
   tempCIMArray = CMNewArray(_broker,1,CMPI_string,rc);
   if( tempCIMArray == NULL ) 
      {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,1,CMPI_string,rc)" );
      _OSBASE_TRACE(2,("--- _FCSoftwareIdentity_Firmware() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   else 
      {
      CMPIString *val=NULL;
      val = CMNewString(_broker,"Linux",rc);
      CMSetArrayElementAt(tempCIMArray,0,(CMPIValue*)&(val),CMPI_string);
      CMSetProperty( ci, "TargetOperatingSystems", (CMPIValue*)&(tempCIMArray), CMPI_stringA);
      }


   /* Set the other attibutes dependant on identityType */
   
   len = strlen(sptr->adapter_attributes->FirmwareVersion) + strlen(sptr->adapter_attributes->Model) + 2; // . plus null
   temp_string = (char*)malloc(len);
   snprintf(temp_string, len, "%s.%s", sptr->adapter_attributes->Model, sptr->adapter_attributes->FirmwareVersion);
   CMSetProperty( ci, "VersionString", temp_string, CMPI_chars); 
   free(temp_string);
    

   /* Set the other attributes */
   CMSetProperty( ci, "Manufacturer", sptr->adapter_attributes->Manufacturer, CMPI_chars); 
   CMSetProperty( ci, "Caption", "Linux_FCSoftwareIdentity_Firmware", CMPI_chars); 
   CMSetProperty( ci, "Description", "FC Adapter", CMPI_chars); 
      

   /****************************/
   /* End Set Fibre attributes */
   /****************************/


  exit:
   _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Firmware() exited"));
   return ci;
} /* end _makeInst_FCSoftwareIdentity_Firmware */


int _makePath_FCSoftwareIdentity_FirmwareList( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIResult * rslt,
                 const CMPIObjectPath * ref,
                 const struct hbaAdapterList * lptr,
                 CMPIStatus * rc)
{
   CMPIObjectPath     * op = NULL;
   char * instanceID = NULL;  /* temp instanceID for each adapter */
   void * keyList = NULL;     
   int x;
   int objNum = 0;

   _OSBASE_TRACE(1,("--- _makePath_FCSoftwareIdentity_FirmwareList() called"));
   // iterate port list
   if( lptr != NULL ) {
   
		for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next) {
	
	        /* FirmwareR instance */
	        /* there is a Firmware for each adapter. */
	        	          	
	            instanceID= _makeKey_FCSoftwareIdentity_Firmware( lptr->sptr);
	       
	           if( isDuplicateKey(instanceID, &keyList, ADD_TO_LIST) ) {
	             
	            /* this SoftwareIdentity has been handled */
	            free(instanceID);
	              continue;
	           }
	
	           op = _makePath_FCSoftwareIdentity_Firmware( _broker, ctx, ref,lptr->sptr, rc );
	            if( op == NULL || rc->rc != CMPI_RC_OK ) {
	
	            if( rc->msg != NULL ) {
	                  _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
	               }
	
	               CMSetStatusWithChars( _broker, rc,
	                               CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM ObjectPath failed." );
	
	            /* free key list */
	            isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
	            
	            _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
	               return -1;
	            }
	            else {
	              CMReturnObjectPath( rslt, op );
	              objNum ++;
	           }
	        
	      } /* end for lptr */
	      
	     /* free key list */
	     isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
	      
   }

   _OSBASE_TRACE(1,("--- _makePath_FCSoftwareIdentity_FirmwareList() exited"));
   return objNum;
}

int _makeInst_FCSoftwareIdentity_FirmwareList( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIResult * rslt,
               const CMPIObjectPath * ref,
               const struct hbaAdapterList * lptr,
               CMPIStatus * rc)
{
   CMPIInstance       * ci    = NULL;
   char * instanceID = NULL;  /* temp instanceID for each adapter */
   void * keyList = NULL;     
   int x;
   int objNum = 0;

   _OSBASE_TRACE(1,("--- _makeInst_FCSoftwareIdentity_FirmwareList() called"));

   // iterate port list
   if( lptr != NULL ) {

	  	
		for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next) {
	
	         /* Firmware instance */
	         /*  there is a Firmware instance for each adapter. */
	         	
	            instanceID= _makeKey_FCSoftwareIdentity_Firmware( lptr->sptr);
	
	            if( isDuplicateKey(instanceID, &keyList, ADD_TO_LIST) ) {
	
	               /* this SoftwareIdentity has been handled */
	               free(instanceID);
	               continue;
	            }
	
	            ci = _makeInst_FCSoftwareIdentity_Firmware( _broker, ctx, ref, lptr->sptr, rc );
	            if( ci == NULL || rc->rc != CMPI_RC_OK ) {
	
	               if( rc->msg != NULL ) {
	                  _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
	               }
	               CMSetStatusWithChars( _broker, rc,
	                                 CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM Instance failed." );
	      
	               /* free key list */
	               isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
	
	               
	               _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
	               return -1;
	            }
	            else { 
	               
	               CMReturnInstance( rslt, ci ); 
	               objNum ++;
	            }
	        
	      } /* end for lptr */
	      
	      isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
	
   }

   _OSBASE_TRACE(1,("--- _makeInst_FCSoftwareIdentity_FirmwareList() exited"));
   return objNum;
}
/* ---------------------------------------------------------------------------*/
/*                    end of cmpiSMIS_FCSoftwareIdentity_Firmware.c             */
/* ---------------------------------------------------------------------------*/

