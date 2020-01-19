/*
 * $Id: cmpiSMIS_FCSoftwareIdentity_Driver.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 *      Logical representation of FC Software Driver.
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

static char * _ClassName = "Linux_FCSoftwareIdentity_Driver";


/* ---------------------------------------------------------------------------*/
/* private functions                                                          */
/* ---------------------------------------------------------------------------*/



/******************************************************************************/
/* Function: _makeKey_FCSoftwareIdentity_Driver                                               */
/* Purpose:  function to create an instanceID from the passed in adapter attr */
/* Inputs:   sptr which will hold the retrieved hba adapter data              */
/* Outputs:  Returns a created string holding the instanceID                  */
/* Notes:    Freeing the returned string is the responsibility of the caller. */
/******************************************************************************/
char * _makeKey_FCSoftwareIdentity_Driver( const struct cim_hbaAdapter * sptr)
   {
   char           *instanceID=NULL; /* to hold FCSoftwareIdentity_Driver InstanceID */
   int            instanceID_len;   /* InstanceID length */

   _OSBASE_TRACE(3,("--- _makeKey_FCSoftwareIdentity_Driver() called"));

   /* Create instanceID string */
   instanceID_len= strlen(sptr->adapter_attributes->Manufacturer) +1 +
                   strlen(sptr->adapter_attributes->Model) +1 + 
                   6 +1 + /* 8 to hold "driver" */
                   strlen(sptr->adapter_attributes->DriverVersion) + 1;
   instanceID= (char *) malloc(instanceID_len);
   
      
   snprintf(instanceID,instanceID_len,"%s-%s-driver-%s",
                               sptr->adapter_attributes->Manufacturer,
                               sptr->adapter_attributes->Model,
                               sptr->adapter_attributes->DriverVersion);     
   instanceID[instanceID_len-1]='\0';

   _OSBASE_TRACE(2,("--- _makeKey_FCSoftwareIdentity_Driver() exited"));
   return instanceID;

   } /* end _makeKey_FCSoftwareIdentity_Driver */
   



/* ---------------------------------------------------------------------------*/
/* end private functions                                                      */
/* ---------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------------*/
/*                            Factory functions                               */
/* ---------------------------------------------------------------------------*/

/******************************************************************************/
/* Function: _makePath_FCSoftwareIdentity_Driver                              */
/* Purpose:  method to create a CMPIObjectPath of this class                  */
/******************************************************************************/
CMPIObjectPath * _makePath_FCSoftwareIdentity_Driver( const CMPIBroker * _broker,
                  const CMPIContext * ctx,
                  const CMPIObjectPath * ref,
                  const struct cim_hbaAdapter * sptr,
                  CMPIStatus * rc) 
   {
   CMPIObjectPath *op=NULL;
   char           *instanceID;      /* to hold FCSoftwareIdentity_Driver InstanceID */
   char           *system_name = NULL; /* save pointer returned by get_system_name() */

   _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity_Driver() called"));

   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   system_name = get_system_name();
   if( ! system_name ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity_Driver() failed : %s",CMGetCharPtr(rc->msg)));
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
      _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity_Driver() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   /* Create instanceID string */
   instanceID= _makeKey_FCSoftwareIdentity_Driver( sptr );
   if (instanceID==NULL)
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create InstanceID failed." );
                      _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity_Driver() failed : %s",CMGetCharPtr(rc->msg)));
                      goto exit;       
      } 
   CMAddKey(op, "InstanceID", instanceID, CMPI_chars); 
   free(instanceID);
     
   exit:
   _OSBASE_TRACE(2,("--- _makePath_FCSoftwareIdentity_Driver() exited"));
   return op;
   } /* end _makePath_FCSoftwareIdentity_Driver */





/******************************************************************************/
/* Function: _makeInst_FCSoftwareIdentity_Driver                              */
/* Purpose:  method to create a CMPIInstance of this class                    */
/******************************************************************************/
CMPIInstance * _makeInst_FCSoftwareIdentity_Driver( const CMPIBroker * _broker,
                const CMPIContext * ctx,
                const CMPIObjectPath * ref,
                const struct cim_hbaAdapter * sptr,
                CMPIStatus * rc) 
   {
   CMPIObjectPath    * op     = NULL;
   CMPIInstance      * ci     = NULL;
   int identityType=DRIVER;
   /* Variables for settting fibre attributes */
   char                        *instanceID;           /* to hold FCSoftwareIdentity_Driver InstanceID */
   unsigned short              temp_uint16;           /* temporary variable for setting cim properties */
   unsigned long               temp_uint32;           /* temporary variable for setting cim properties */
   unsigned long long          temp_uint64;           /* temporary variable for setting cim properties */
   CMPIArray                   *tempCIMArray=NULL;    /* temporary variable for holding a cmpi array  */
   char                        *temp_string;          /* temporary pointer to a string for setting cim properties */
   char                        *system_name;          /* save pointer returned by get_system_name() */
   int                         len;

   _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Driver() called"));

   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   system_name = get_system_name();
   if( ! system_name ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Driver() failed : %s",CMGetCharPtr(rc->msg)));
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
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Driver() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   ci = CMNewInstance( _broker, op, rc);
   if( CMIsNullObject(ci) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Driver() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   
   
   /************************/
   /* Set Fibre attributes */
   /************************/


   /* Create instanceID string */
   instanceID= _makeKey_FCSoftwareIdentity_Driver( sptr );
   if (instanceID==NULL)
      {
      CMSetStatusWithChars( _broker, rc,
                           CMPI_RC_ERR_FAILED, "Create InstanceID failed." );
                       _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Driver() failed : %s",CMGetCharPtr(rc->msg)));
                      goto exit;       
      } 
   _OSBASE_TRACE(1,("--- ----> Before CMSetProperty"));
   CMSetProperty( ci, "InstanceID", instanceID, CMPI_chars); 
   _OSBASE_TRACE(1,("--- ----> InstanceID set"));
   CMSetProperty( ci, "Name", instanceID, CMPI_chars); 
   _OSBASE_TRACE(1,("--- ----> Name set"));
   CMSetProperty( ci, "ElementName", instanceID, CMPI_chars); 
   _OSBASE_TRACE(1,("--- ----> ElementName set"));
   free(instanceID);


   /* Set Classifications */
   tempCIMArray = CMNewArray(_broker,1,CMPI_uint16,rc);
   if( tempCIMArray == NULL ) 
      {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,1,CMPI_uint16,rc)" );
      _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Driver() failed : %s",CMGetCharPtr(rc->msg)));
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
      _OSBASE_TRACE(2,("--- _FCSoftwareIdentity_Driver() failed : %s",CMGetCharPtr(rc->msg)));
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
   CMSetProperty( ci, "VersionString", sptr->adapter_attributes->DriverVersion, CMPI_chars); 
    

   /* Set the other attributes */
   CMSetProperty( ci, "Manufacturer", sptr->adapter_attributes->Manufacturer, CMPI_chars); 
   CMSetProperty( ci, "Caption", "Linux_FCSoftwareIdentity_Driver", CMPI_chars); 
   CMSetProperty( ci, "Description", "FC Adapter", CMPI_chars); 
      

   /****************************/
   /* End Set Fibre attributes */
   /****************************/


  exit:
   _OSBASE_TRACE(2,("--- _makeInst_FCSoftwareIdentity_Driver() exited"));
   return ci;
} /* end _makeInst_FCSoftwareIdentity_Driver */


int _makePath_FCSoftwareIdentity_DriverList( const CMPIBroker * _broker,
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

   _OSBASE_TRACE(1,("--- _makePath_FCSoftwareIdentity_DriverList() called"));
   // iterate port list
   if( lptr != NULL ) {
   
		for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next) {
	
	        /* DRIVER instance */
	        /* there is a driver for each adapter. */
	        	          	
	            instanceID= _makeKey_FCSoftwareIdentity_Driver( lptr->sptr);
	       
	           if( isDuplicateKey(instanceID, &keyList, ADD_TO_LIST) ) {
	             
	            /* this SoftwareIdentity has been handled */
	            free(instanceID);
	              continue;
	           }
	
	           op = _makePath_FCSoftwareIdentity_Driver( _broker, ctx, ref,lptr->sptr, rc );
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

   _OSBASE_TRACE(1,("--- _makePath_FCSoftwareIdentity_DriverList() exited"));
   return objNum;
}

int _makeInst_FCSoftwareIdentity_DriverList( const CMPIBroker * _broker,
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

   _OSBASE_TRACE(1,("--- _makeInst_FCSoftwareIdentity_DriverList() called"));

   // iterate port list
   if( lptr != NULL ) {

	  	
		for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next) {
   _OSBASE_TRACE(1,("--- ----> 1"));
	
	         /* DRIVER instance */
	         /*  there is a driver instance for each adapter. */
	         	
	            instanceID= _makeKey_FCSoftwareIdentity_Driver( lptr->sptr);
   _OSBASE_TRACE(1,("--- ----> 2"));
	
	            if( isDuplicateKey(instanceID, &keyList, ADD_TO_LIST) ) {
	
	               /* this SoftwareIdentity has been handled */
	               free(instanceID);
	               continue;
	            }
   _OSBASE_TRACE(1,("--- ----> 3"));

		if (_broker == NULL)
   			_OSBASE_TRACE(1,("--- ----> 31"));
		if (ctx == NULL)
   			_OSBASE_TRACE(1,("--- ----> 32"));
		if (ref == NULL)
   			_OSBASE_TRACE(1,("--- ----> 33"));
		if (lptr->sptr == NULL)
   			_OSBASE_TRACE(1,("--- ----> 34"));
		if (rc == NULL)
   			_OSBASE_TRACE(1,("--- ----> 35"));
	
	            ci = _makeInst_FCSoftwareIdentity_Driver( _broker, ctx, ref, lptr->sptr, rc );
   _OSBASE_TRACE(1,("--- ----> 4"));
	            if( ci == NULL || rc->rc != CMPI_RC_OK ) {
	
	               if( rc->msg != NULL ) {
	                  _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
	               }
   _OSBASE_TRACE(1,("--- ----> 5"));
	               CMSetStatusWithChars( _broker, rc,
	                                 CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM Instance failed." );
   _OSBASE_TRACE(1,("--- ----> 6"));
	      
	               /* free key list */
	               isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
   _OSBASE_TRACE(1,("--- ----> 7"));
	
	               
	               _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
	               return -1;
	            }
	            else { 
	               
	               CMReturnInstance( rslt, ci ); 
   _OSBASE_TRACE(1,("--- ----> 8"));
	               objNum ++;
	            }
	        
	      } /* end for lptr */
	      
	      isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
   _OSBASE_TRACE(1,("--- ----> 9"));
	
   }

   _OSBASE_TRACE(1,("--- _makeInst_FCSoftwareIdentity_DriverList() exited"));
   return objNum;
}
/* ---------------------------------------------------------------------------*/
/*                    end of cmpiSMIS_FCSoftwareIdentity_Driver.c             */
/* ---------------------------------------------------------------------------*/

