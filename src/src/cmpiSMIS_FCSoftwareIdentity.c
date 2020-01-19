/*
 * $Id: cmpiSMIS_FCSoftwareIdentity.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 *      FC Port Controller's Software Asset, including Firmware and Driver.
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

static char * _ClassName = "Linux_FCSoftwareIdentity";


/* ---------------------------------------------------------------------------*/
/* private functions                                                          */
/* ---------------------------------------------------------------------------*/



/******************************************************************************/
/* Function: _makeKey_FCSoftwareIdentity                                                */
/* Purpose:  function to create an instanceID from the passed in adapter attr */
/* Inputs:   sptr which will hold the retrieved hba adapter data              */
/*           identityType which is either FIRMWARE or DRIVER                  */
/* Outputs:  Returns a created string holding the instanceID                  */
/* Notes:    Freeing the returned string is the responsibility of the caller. */
/******************************************************************************/
char * _makeKey_FCSoftwareIdentity( const struct cim_hbaAdapter * sptr, int identityType)
   {
   char           *instanceID=NULL; /* to hold FCSoftwareIdentity InstanceID */
   int            instanceID_len;   /* InstanceID length */

   _OSBASE_TRACE(3,("--- _makeKey_FCSoftwareIdentity() called"));

   /* Create instanceID string */
   instanceID_len= strlen(sptr->adapter_attributes->Manufacturer) +1 +
                   strlen(sptr->adapter_attributes->Model) +1 + 
                   8 +1 + /* 8 to hold "firmware" or "driver" */
                   strlen(sptr->adapter_attributes->DriverVersion) +
                   strlen(sptr->adapter_attributes->FirmwareVersion) + 1;
   instanceID= (char *) malloc(instanceID_len);
   switch (identityType)
      {
      case FIRMWARE : snprintf(instanceID,instanceID_len,"%s-%s-firmware-%s",
                               sptr->adapter_attributes->Manufacturer,
                               sptr->adapter_attributes->Model,
                               sptr->adapter_attributes->FirmwareVersion);
                      break;
      case DRIVER   : snprintf(instanceID,instanceID_len,"%s-%s-driver-%s",
                               sptr->adapter_attributes->Manufacturer,
                               sptr->adapter_attributes->Model,
                               sptr->adapter_attributes->DriverVersion);
                      break;
      default :       _OSBASE_TRACE(2,("--- _makeKey_FCSoftwareIdentity() failed : Unknown identityType specified."));
                      goto exit;       
      } /* end switch */
   instanceID[instanceID_len-1]='\0';

   exit:
   _OSBASE_TRACE(2,("--- _makeKey_FCSoftwareIdentity() exited"));
   return instanceID;

   } /* end _makeKey_FCSoftwareIdentity */
   



/* ---------------------------------------------------------------------------*/
/* end private functions                                                      */
/* ---------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------------*/
/*                            Factory functions                               */
/* ---------------------------------------------------------------------------*/

/******************************************************************************/
/* Function: _makePath_FCSoftwareIdentity                                     */
/* Purpose:  method to create a CMPIObjectPath of this class                  */
/* Notes:    There normally isn't an int argument passed into our provider    */
/*           functions.  I needed to pass in identityType because two         */
/*           instances are created (driver and firmware) for each exact copy  */
/*           of a cim_hbaAdapter, so needed another argument to determine if  */
/*           it is a driver or a firmware instance.                           */
/******************************************************************************/
CMPIObjectPath * _makePath_FCSoftwareIdentity( const CMPIBroker * _broker,
                  const CMPIContext * ctx,
                  const CMPIObjectPath * ref,
                  const struct cim_hbaAdapter * sptr,
                  int identityType,
                  CMPIStatus * rc) 
   {
   CMPIObjectPath *op=NULL;
   char           *instanceID;      /* to hold FCSoftwareIdentity InstanceID */
   char           *system_name = NULL; /* save pointer returned by get_system_name() */

   _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity() called"));

   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   system_name = get_system_name();
   if( ! system_name ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity() failed : %s",CMGetCharPtr(rc->msg)));
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
      _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   /* Create instanceID string */
   instanceID= _makeKey_FCSoftwareIdentity( sptr, identityType);
   if (instanceID==NULL)
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create InstanceID failed." );
                      _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity() failed : %s",CMGetCharPtr(rc->msg)));
                      goto exit;       
      } 
   CMAddKey(op, "InstanceID", instanceID, CMPI_chars); 
   free(instanceID);
     
   exit:
   _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity() exited"));
   return op;
   } /* end _makePath_FCSoftwareIdentity */





/******************************************************************************/
/* Function: _makeInst_FCSoftwareIdentity                                     */
/* Purpose:  method to create a CMPIInstance of this class                    */
/* Notes:    There normally isn't an int argument passed into our provider    */
/*           functions.  I needed to pass in identityType because two         */
/*           instances are created (driver and firmware) for each exact copy  */
/*           of a cim_hbaAdapter, so needed another argument to determine if  */
/*           it is a driver or a firmware instance.                           */
/******************************************************************************/
CMPIInstance * _makeInst_FCSoftwareIdentity( const CMPIBroker * _broker,
                const CMPIContext * ctx,
                const CMPIObjectPath * ref,
                const struct cim_hbaAdapter * sptr,
                int identityType,
                CMPIStatus * rc) 
   {
   CMPIObjectPath    * op     = NULL;
   CMPIInstance      * ci     = NULL;
   
   /* Variables for settting fibre attributes */
   char                        *instanceID;           /* to hold FCSoftwareIdentity InstanceID */
   unsigned short              temp_uint16;           /* temporary variable for setting cim properties */
   unsigned long               temp_uint32;           /* temporary variable for setting cim properties */
   unsigned long long          temp_uint64;           /* temporary variable for setting cim properties */
   CMPIArray                   *tempCIMArray=NULL;    /* temporary variable for holding a cmpi array  */
   char                        *temp_string;          /* temporary pointer to a string for setting cim properties */
   char                        *system_name;          /* save pointer returned by get_system_name() */
   int                         len;

   _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity() called"));

   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   system_name = get_system_name();
   if( ! system_name ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity() failed : %s",CMGetCharPtr(rc->msg)));
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
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   ci = CMNewInstance( _broker, op, rc);
   if( CMIsNullObject(ci) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   
   
   /************************/
   /* Set Fibre attributes */
   /************************/


   /* Create instanceID string */
   instanceID= _makeKey_FCSoftwareIdentity( sptr, identityType);
   if (instanceID==NULL)
      {
      CMSetStatusWithChars( _broker, rc,
                           CMPI_RC_ERR_FAILED, "Create InstanceID failed." );
                       _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity() failed : %s",CMGetCharPtr(rc->msg)));
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
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity() failed : %s",CMGetCharPtr(rc->msg)));
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
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity() failed : %s",CMGetCharPtr(rc->msg)));
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
   switch (identityType)
      {
      case FIRMWARE :
         len = strlen(sptr->adapter_attributes->FirmwareVersion) + strlen(sptr->adapter_attributes->Model) + 2; // . plus null
         temp_string = (char*)malloc(len);
         snprintf(temp_string, len, "%s.%s", sptr->adapter_attributes->Model, sptr->adapter_attributes->FirmwareVersion);
         CMSetProperty( ci, "VersionString", temp_string, CMPI_chars); 
         free(temp_string);
    break;
      case DRIVER   : 
         CMSetProperty( ci, "VersionString", sptr->adapter_attributes->DriverVersion, CMPI_chars); 
    break;
      } /* end switch */

   /* Set the other attributes */
   CMSetProperty( ci, "Manufacturer", sptr->adapter_attributes->Manufacturer, CMPI_chars); 
   CMSetProperty( ci, "Caption", "Linux_FCSoftwareIdentity", CMPI_chars); 
   CMSetProperty( ci, "Description", "FC Adapter", CMPI_chars); 
   CMSetProperty( ci, "TargetOperatingSystems", "Linux", CMPI_chars); 
      

   /****************************/
   /* End Set Fibre attributes */
   /****************************/


  exit:
   _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity() exited"));
   return ci;
} /* end _makeInst_FCSoftwareIdentity */


int _makePath_FCSoftwareIdentityList( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIResult * rslt,
                 const CMPIObjectPath * ref,
                 const struct hbaAdapterList * lptr,
                 CMPIStatus * rc)
{
   CMPIObjectPath     * op = NULL;
   int identityType;          /* FIRMWARE or DRIVER */
   char * instanceID = NULL;  /* temp instanceID for each adapter */
   void * keyList = NULL;     
   int x;
   int objNum = 0;

   _OSBASE_TRACE(1,("--- _makePath_FCSoftwareIdentityList() called"));
   // iterate port list
   if( lptr != NULL ) {

      for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next) {

        /* FIRMWARE instance and DRIVER instance */
        /* Normally for the providers there is only one instance creation for each adapter or port, */
        /* but with this provider there is a driver instance AND a firmware instance for each adapter. */
        for (x=1; x<=2; x++) {
           
       
           if(x == 1)
              identityType=FIRMWARE;
           else
              identityType=DRIVER;

            instanceID= _makeKey_FCSoftwareIdentity( lptr->sptr, identityType);
       
           if( isDuplicateKey(instanceID, &keyList, ADD_TO_LIST) ) {
             
            /* this SoftwareIdentity has been handled */
            free(instanceID);
              continue;
           }

           op = _makePath_FCSoftwareIdentity( _broker, ctx, ref,lptr->sptr, identityType, rc );
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
        } /* end for x */
      } /* end for lptr */
      
     /* free key list */
     isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS); 
      
   }

   _OSBASE_TRACE(1,("--- _makePath_FCSoftwareIdentityList() exited"));
   return objNum;
}

int _makeInst_FCSoftwareIdentityList( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIResult * rslt,
               const CMPIObjectPath * ref,
               const struct hbaAdapterList * lptr,
               CMPIStatus * rc)
{
   CMPIInstance       * ci    = NULL;
   int identityType;          /* FIRMWARE or DRIVER */
   char * instanceID = NULL;  /* temp instanceID for each adapter */
   void * keyList = NULL;     
   int x;
   int objNum = 0;

   _OSBASE_TRACE(1,("--- _makeInst_FCSoftwareIdentityList() called"));

   // iterate port list
   if( lptr != NULL ) {

      for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next) {

         /* FIRMWARE instance and DRIVER instance */
         /* Normally for the providers there is only one instance creation for each adapter or port, */
         /* but with this provider there is a driver instance AND a firmware instance for each adapter. */
         for (x=1; x<=2; x++) {

            if(x == 1)
               identityType=FIRMWARE;
            else
               identityType=DRIVER;

            instanceID= _makeKey_FCSoftwareIdentity( lptr->sptr, identityType);

            if( isDuplicateKey(instanceID, &keyList, ADD_TO_LIST) ) {

               /* this SoftwareIdentity has been handled */
               free(instanceID);
               continue;
            }

            ci = _makeInst_FCSoftwareIdentity( _broker, ctx, ref, lptr->sptr, identityType, rc );
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
         } /* end for x */
      } /* end for lptr */
      
      isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
   }

   _OSBASE_TRACE(1,("--- _makeInst_FCSoftwareIdentityList() exited"));
   return objNum;
}
/* ---------------------------------------------------------------------------*/
/*                    end of cmpiSMIS_FCSoftwareIdentity.c                    */
/* ---------------------------------------------------------------------------*/

