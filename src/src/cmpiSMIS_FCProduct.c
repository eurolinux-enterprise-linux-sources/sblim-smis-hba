/*
 * $Id: cmpiSMIS_FCProduct.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 *      Represents instances of available Fibre Channel Products.
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

/* ---------------------------------------------------------------------------*/
/*                          Hardcoded strings                                 */
/* ---------------------------------------------------------------------------*/

static const char * _FCProduct_Caption = 
   "Linux_FCProduct";
static const char * _FCProduct_Description = 
   "This class represents instances of available Fibre Channel Products.";
static char * _ClassName = "Linux_FCProduct";

/* ---------------------------------------------------------------------------*/
/*                         Product Properties                                 */
/* ---------------------------------------------------------------------------*/
#define PROD_PROP_NAME                "Name"
#define PROD_PROP_IDENTIFYINGNUMBER   "IdentifyingNumber"
#define PROD_PROP_VENDOR              "Vendor"
#define PROD_PROP_VERSION             "Version"
#define PROD_PROP_CAPTION             "Caption"
#define PROD_PROP_DESCRIPTION         "Description"
#define PROD_PROP_ELEMENTNAME         "ElementName"
#define PROD_PROP_SKUNUMBER           "SKUNumber"
#define PROD_PROP_WARRANTYSTARTDATE   "WarrantyStartDate"
#define PROD_PROP_WARRANTYDURATION    "WarrantyDuration"

/* ---------------------------------------------------------------------------*/
/*                            Factory functions                               */
/* ---------------------------------------------------------------------------*/

/******************************************************************************/
/* Function: _makePath_FCProduct                                              */
/* Purpose:  method to create a CMPIObjectPath of this class                  */
/******************************************************************************/
CMPIObjectPath * 
_makePath_FCProduct( const CMPIBroker * _broker,
                  const CMPIContext * ctx,
                  const CMPIObjectPath * ref,
                  const struct cim_hbaAdapter * sptr,
                  CMPIStatus * rc) 
{
   CMPIObjectPath *op = NULL;
   char           *system_name = NULL; 	/* save pointer returned by get_system_name */

   _OSBASE_TRACE(1,("--- _makePath_FCProduct() called"));
   
   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   system_name = get_system_name();
   if( ! system_name ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "FCProduct no host name found" );
      _OSBASE_TRACE(2,("--- _makePath_FCProduct() failed : %s",CMGetCharPtr(rc->msg)));
      goto final_exit;
   }
   /* free system_name */
   free( system_name );

   /* create object path */
   op=CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                       _ClassName, rc );

   if( CMIsNullObject(op) ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "FCProduct Create CMPIObjectPath failed." );
      _OSBASE_TRACE(2,("--- _makePath_FCProduct() failed : %s",CMGetCharPtr(rc->msg)));
      goto final_exit;
   }

   /* fill in keys */
   CMAddKey(op, PROD_PROP_NAME, 
            sptr->adapter_attributes->ModelDescription, CMPI_chars);  
   CMAddKey(op, PROD_PROP_IDENTIFYINGNUMBER, 
            sptr->adapter_attributes->SerialNumber, CMPI_chars);
   CMAddKey(op, PROD_PROP_VENDOR, 
            sptr->adapter_attributes->Manufacturer, CMPI_chars);  
   CMAddKey(op, PROD_PROP_VERSION, 
            sptr->adapter_attributes->Model, CMPI_chars);  
   
final_exit:
   _OSBASE_TRACE(1,("--- _makePath_FCProduct() exited"));
   return op;
} /* end _makePath_FCProduct */





/******************************************************************************/
/* Function: _makeInst_FCProduct                                              */
/* Purpose:  method to create a CMPIInstance of this class                    */
/******************************************************************************/
CMPIInstance * 
_makeInst_FCProduct( const CMPIBroker * _broker,
                const CMPIContext * ctx,
                const CMPIObjectPath * ref,
                const struct cim_hbaAdapter * sptr,
                CMPIStatus * rc) 
{
   CMPIObjectPath *op = NULL;
   CMPIInstance *ci = NULL;
   char         *system_name = NULL; /* save pointer returned by get_system_name */
   char          elementName[322];   /* Manufacture(64)-(1)Model(256) */   

   _OSBASE_TRACE(1,("--- _makeInst_FCProduct() called"));

   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   system_name = get_system_name();
   if( ! system_name ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "FCProduct no host name found" );
      _OSBASE_TRACE(2,("--- _makePath_FCProduct() failed : %s",CMGetCharPtr(rc->msg)));
      goto final_exit;
   }
   /* free system_name */
   free( system_name );

   /* create object path */
   op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                         _ClassName, rc );
                         
   if( CMIsNullObject(op) ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "FCProduct Create CMPIObjectPath failed." );
      _OSBASE_TRACE(2,("--- _makeInst_FCProduct() failed : %s",CMGetCharPtr(rc->msg)));
      goto final_exit;
   }

   /* create instance */
   ci = CMNewInstance( _broker, op, rc);
   
   if( CMIsNullObject(ci) ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "FCProduct Create CMPIInstance failed." );
      _OSBASE_TRACE(2,("--- _makeInst_FCProduct() failed : %s",CMGetCharPtr(rc->msg)));
      goto final_exit;
   }
   
   /* Set instance key attributes */
   CMSetProperty(ci, PROD_PROP_NAME, 
                 sptr->adapter_attributes->ModelDescription, CMPI_chars);  
   CMSetProperty(ci, PROD_PROP_IDENTIFYINGNUMBER, 
                 sptr->adapter_attributes->SerialNumber, CMPI_chars);
   CMSetProperty(ci, PROD_PROP_VENDOR, 
                 sptr->adapter_attributes->Manufacturer, CMPI_chars);  
   CMSetProperty(ci, PROD_PROP_VERSION, 
                 sptr->adapter_attributes->Model, CMPI_chars);  
   
   /* Set FC Product attributes  */
   CMSetProperty(ci, PROD_PROP_CAPTION, 
                 _FCProduct_Caption, CMPI_chars);  
   CMSetProperty(ci, PROD_PROP_DESCRIPTION, 
                 _FCProduct_Description, CMPI_chars);  
           
   snprintf(elementName, 322, "%s_%s", sptr->adapter_attributes->Manufacturer,
   	sptr->adapter_attributes->Model);      
   elementName[321] = '\0';
   CMSetProperty(ci, PROD_PROP_ELEMENTNAME, 
                 elementName, CMPI_chars);  

   /* leave other attributes unset */
   /* SKUNumber, WarrantyStartDate, WarrantyDuration */

final_exit:
   _OSBASE_TRACE(1,("--- _makeInst_FCProduct() exited"));
   return ci;
} /* end _makeInst_FCProduct */


/******************************************************************************/
/* Function: _makeKey_FCProduct                                               */
/* Purpose:  method to create a Key of this hbaAdapter attribute              */
/******************************************************************************/
char* _makeKey_FCProduct(const struct cim_hbaAdapter *sptr)
{
   int key_len = strlen(sptr->adapter_attributes->SerialNumber) + 1 +
                 strlen(sptr->adapter_attributes->Manufacturer) + 1 +
                 strlen(sptr->adapter_attributes->Model) + 1 +
                 strlen(sptr->adapter_attributes->ModelDescription) + 1;
   char * key = (char *)malloc(key_len);
   snprintf(key, key_len, "%s-%s-%s-%s", 
              sptr->adapter_attributes->SerialNumber,
              sptr->adapter_attributes->Manufacturer,
              sptr->adapter_attributes->Model,
              sptr->adapter_attributes->ModelDescription );
   key[key_len-1] = 0;
   return key;                          
}

/******************************************************************************/
/* Function: _makePath_FCProductList                                          */
/* Purpose:  method to iterate the lptr and                                   */
/*           create eacj object path of this hbaAdapter attribute             */
/* Return:   Number of object path                                            */
/******************************************************************************/
int _makePath_FCProductList( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIResult * rslt,
                 const CMPIObjectPath * ref,
                 const struct hbaAdapterList * lptr,
                 CMPIStatus * rc)
{
	CMPIObjectPath     * op    = NULL;
   char *key = NULL;
   void * keyList = NULL;
	int pathNum = 0;
	
	_OSBASE_TRACE(1,("--- %s _makePath_FCProductList() called", _ClassName));
	
   /* iterate adapters list */
   if( lptr != NULL ) {
   	
      for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next ) {
      	
      	 key = _makeKey_FCProduct(lptr->sptr);
      	 if ( isDuplicateKey(key, &keyList, ADD_TO_LIST) != FALSE ) {
            /* This is a duplicated key, release the buffer of key and search the next adapter */
            /* If not, free the key in isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);  */
      	    free(key);
      	    continue;
      	 }
      	       	 
         /* make object path from cim_hbaAdapter */
         op = _makePath_FCProduct( _broker, ctx, ref,lptr->sptr, rc );
         if( op == NULL || rc->rc != CMPI_RC_OK ) {

            if( rc->msg != NULL ) {
              _OSBASE_TRACE(2,("--- %s _makePath_FCProductList() failed : %s",_ClassName, CMGetCharPtr(rc->msg)));
            }
            CMSetStatusWithChars( _broker, rc,
                               CMPI_RC_ERR_FAILED, "transformation from internal structure to CIM ObjectPath failed." );

            _OSBASE_TRACE(2,("--- %s _makePath_FCProductList() failed : %s",_ClassName, CMGetCharPtr(rc->msg)));
            break;
         }
         else { 
            CMReturnObjectPath( rslt, op ); 
            pathNum ++;
         }
      }
      /* release the key list and all keys */
      isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
   }

   _OSBASE_TRACE(1,("--- %s _makePath_FCProductList() exited", _ClassName));
   return pathNum;
}

/******************************************************************************/
/* Function: _makeInst_FCProductList                                          */
/* Purpose:  method to iterate the lptr and                                   */
/*           create eacj instance of this hbaAdapter attribute                */
/******************************************************************************/
int _makeInst_FCProductList( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIResult * rslt,
               const CMPIObjectPath * ref,
               const struct hbaAdapterList * lptr,
               CMPIStatus * rc)
{
	CMPIInstance       * ci    = NULL;
   char *key = NULL;
   void * keyList = NULL;
	int pathNum = 0;
	
	_OSBASE_TRACE(1,("--- %s _makeInst_FCProductList() called", _ClassName));
	
   /* iterate adapters list */
   if( lptr != NULL ) {
   	
      for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next ) {
      	
      	 key = _makeKey_FCProduct(lptr->sptr);
      	 if ( isDuplicateKey(key, &keyList, ADD_TO_LIST) != FALSE ) {
            /* This is a duplicated key, release the buffer of key and search the next adapter */
            /* If not, free the key in isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);  */
      	    free(key);
      	    continue;
      	 }
      	       	 
         /* make instance from cim_hbaAdapter */
         ci = _makeInst_FCProduct( _broker, ctx, ref, lptr->sptr, rc );
         if( ci == NULL || rc->rc != CMPI_RC_OK ) {

            if( rc->msg != NULL ) {
              _OSBASE_TRACE(2,("--- %s _makeInst_FCProductList() failed : %s",_ClassName, CMGetCharPtr(rc->msg)));
            }
            CMSetStatusWithChars( _broker, rc,
                               CMPI_RC_ERR_FAILED, "transformation from internal structure to CIM Instance failed." );

            _OSBASE_TRACE(2,("--- %s _makeInst_FCProductList() failed : %s",_ClassName, CMGetCharPtr(rc->msg)));
            break;
         }
         else { 
            CMReturnInstance( rslt, ci ); 
            pathNum ++;
         }
      }
      /* release the key list and all keys */
      isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
   }

   _OSBASE_TRACE(1,("--- %s _makeInst_FCProductList() exited", _ClassName));
   return pathNum;
}

/* ---------------------------------------------------------------------------*/
/*                    end of cmpiSMIS_FCProduct.c                             */
/* ---------------------------------------------------------------------------*/

