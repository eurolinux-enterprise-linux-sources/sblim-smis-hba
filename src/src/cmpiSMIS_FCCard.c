/*
 * $Id: cmpiSMIS_FCCard.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 *	cmpiSMIS_FCCard implements class representing Fibre Channel cards.
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

static char * _ClassName = "Linux_FCCard";
/* the tag is 'Manufacture(64)-Model(256)-SerialNo(64)' and 1 byte for '\0'*/
#define TAG_LENGTH 387

/* ---------------------------------------------------------------------------*/
/* private functions                                                          */
/* ---------------------------------------------------------------------------*/
char* 
_makeKey_FCCard(const struct cim_hbaAdapter* adapterPtr)
{
   char* tag = (char*) malloc(TAG_LENGTH);
   HBA_ADAPTERATTRIBUTES* adapterAttr = adapterPtr->adapter_attributes;
   snprintf(tag,TAG_LENGTH,"%s-%s-%s",adapterAttr->Manufacturer,
      adapterAttr->Model, adapterAttr->SerialNumber);
   tag[TAG_LENGTH - 1] = '\0';
   return tag;
}

static unsigned int 
setInstanceProperty( CMPIInstance* ci, const struct cim_hbaAdapter* adapterPtr, char* host_name)
{
   char* tag = NULL;
   HBA_ADAPTERATTRIBUTES* adapterAttr = adapterPtr->adapter_attributes;
   unsigned short temp_uint16;

   /* Set general class attributes */
   CMSetProperty( ci, "CreationClassName", _ClassName, CMPI_chars ); 
   /************************/
   /* Set Fibre attributes */
   /************************/
   tag = _makeKey_FCCard(adapterPtr);
   CMSetProperty( ci, "Tag", tag, CMPI_chars);

   /* mandatory properties */
   CMSetProperty( ci, "Manufacturer", adapterAttr->Manufacturer,CMPI_chars);
   CMSetProperty( ci, "Model", adapterAttr->Model, CMPI_chars);
   
   /* optional properties */
   CMSetProperty( ci, "ElementName", adapterAttr->ModelDescription, CMPI_chars);

   /* the name property is same as Tag */
   CMSetProperty( ci, "Name", tag , CMPI_chars);
   
   CMSetProperty( ci, "Version", adapterAttr->HardwareVersion, CMPI_chars);
   CMSetProperty( ci, "SerialNumber", adapterAttr->SerialNumber, CMPI_chars);
      
   /* Set misc hardcoded attributes */
   CMSetProperty( ci, "Caption", "Linux_FCCard", CMPI_chars);
   CMSetProperty( ci, "Description", 
      "This class represents instances that contain the related FC devices.", CMPI_chars);
   temp_uint16 = 2;
   CMSetProperty( ci, "RemovalConditions", (CMPIValue*)&temp_uint16, CMPI_uint16);  
   free(tag);
   return 0;
}

/* ---------------------------------------------------------------------------*/
/* end private functions                                                      */
/* ---------------------------------------------------------------------------*/


/* ---------------------------------------------------------------------------*/
/*                            Factory functions                               */
/* ---------------------------------------------------------------------------*/

/******************************************************************************/
/* Function: _makePath_FCCard                                                 */
/* Purpose:  method to create a CMPIObjectPath of this class                  */
/******************************************************************************/
CMPIObjectPath * 
_makePath_FCCard( const CMPIBroker * _broker,
                     const CMPIContext * ctx,
                     const CMPIObjectPath * ref,
                     const struct cim_hbaAdapter * adapterPtr,
                     CMPIStatus * rc) 
{
   CMPIObjectPath *op=NULL;
   char           *tag;   /* to hold FCCard tag */
   char           *host_name;   
   _OSBASE_TRACE(1,("--- _makePath_FCCard() called"));
   
   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   if( !(host_name = get_system_name()) )
   {
      CMSetStatusWithChars( _broker, rc,
         CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(2,("--- _makePath_FCCard() failed : %s",CMGetCharPtr(rc->msg)));
   }
   else
   {
      op=CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
         _ClassName, rc );
      if( CMIsNullObject(op) ) 
      {
         CMSetStatusWithChars( _broker, rc,
            CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
         _OSBASE_TRACE(2,("--- _makePath_FCCard() failed : %s",CMGetCharPtr(rc->msg)));
      }
      else
      {
         CMAddKey(op, "CreationClassName", _ClassName, CMPI_chars);  
         
         tag = _makeKey_FCCard(adapterPtr);
         CMAddKey(op, "Tag", tag, CMPI_chars); 
         free(tag);
      }
   }
   free(host_name);
   _OSBASE_TRACE(1,("--- _makePath_FCCard() exited"));
   return op;
} /* end _makePath_FCCard */





/******************************************************************************/
/* Function: _makeInst_FCCard                                                 */
/* Purpose:  method to create a CMPIInstance of this class                    */
/******************************************************************************/
CMPIInstance * 
_makeInst_FCCard( const CMPIBroker * _broker,
                        const CMPIContext * ctx,
                        const CMPIObjectPath * ref,
                        const struct cim_hbaAdapter * sptr,
                        CMPIStatus * rc) 
{
   CMPIObjectPath    * op     = NULL;
   CMPIInstance      * ci     = NULL;
   
   /* Variables for setting fibre attributes */
   char *host_name = NULL;
   
   
   _OSBASE_TRACE(1,("--- _makeInst_FCCard() called"));
   
   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system data.                                                       */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   if( !(host_name = get_system_name()) ) 
   {
      CMSetStatusWithChars( _broker, rc,
         CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(2,("--- _makeInst_FCCard() failed : %s",CMGetCharPtr(rc->msg)));
   }
   else
   {
      op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
         _ClassName, rc );
      if( CMIsNullObject(op) ) 
      {
         CMSetStatusWithChars( _broker, rc,
            CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
         _OSBASE_TRACE(2,("--- _makeInst_FCCard() failed : %s",CMGetCharPtr(rc->msg)));
      }
      else
      {
         ci = CMNewInstance( _broker, op, rc);
         if( CMIsNullObject(ci) ) 
         {
            CMSetStatusWithChars( _broker, rc,
               CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
            _OSBASE_TRACE(2,("--- _makeInst_FCCard() failed : %s",CMGetCharPtr(rc->msg)));
         }
         else
         {
            setInstanceProperty(ci,sptr,host_name);// error handler?
         }
      }
   }
   free(host_name);
   _OSBASE_TRACE(1,("--- _makeInst_FCCard() exited"));
   return ci;
} /* end _makeInst_FCCard */


/* ---------------------------------------------------------------------------*/
/*                    end of cmpiSMIS_FCCard.c                                */
/* ---------------------------------------------------------------------------*/


