/*
 * $Id: cmpiSMIS_FCRealizesProvider.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 *      Association of FC Card and FC Port Controller Logical Device.
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"
#include "Linux_Common.h"
#include "cmpiLinux_Common.h"
#include "cmpiSMIS_FC.h"


static const CMPIBroker * _broker;


/* ---------------------------------------------------------------------------*/
/* private declarations                                                       */

static char * _ClassName     = "Linux_FCRealizes";
static char * _RefLeft       = "Antecedent";
static char * _RefRight      = "Dependent";
static char * _RefLeftClass  = "Linux_FCCard";
static char * _RefRightClass = "Linux_FCPortController";

/*
 * Usually, one FCCard can contains more than one FCPortController.
 * The relationship between FCCard and FCPortController may be 
 *    1:1 ( single port on one card ) or
 *    1:N ( multiple ports on one card )
 */

/* ---------------------------------------------------------------------------*/
/*                        _makePath_FCRealizes()                              */
/* ---------------------------------------------------------------------------*/
/* method to create object path for Linux_FCRealizes                         */
/*
 * ref         - source object path
 * _ClassName  - "Linux_FCRealizes"
 * lAdapterPtr - pointer to hbaAdapterList, only using current sptr
 * lPortPtr    - pointer to hbaPortList, only using current sptr
 * return      - object path (SUCCESS) or NULL(FAILED)
 */
/* ---------------------------------------------------------------------------*/
CMPIObjectPath * _makePath_FCRealizes(  const CMPIBroker * _broker,
                 const CMPIContext * ctx,
                 const CMPIObjectPath * ref,
                 char * _ClassName,
                 const struct cim_hbaAdapter * sAdapterPtr, /* hbaAdapter */
                 const struct cim_hbaPort * sPortPtr,       /* hbaPort    */
                 CMPIStatus * rc)
{
   CMPIObjectPath * cop = NULL;                /* class object path     */
   CMPIObjectPath * lop = NULL;                /* left end object path  */
   CMPIObjectPath * rop = NULL;                /* right end object path */
   
   _OSBASE_TRACE(1,("--- _makePath_FCRealizes() called"));
   
   if ( sAdapterPtr == NULL || sPortPtr == NULL )
      return NULL;
   
   cop = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                          _ClassName, rc );
   if( rc->rc != CMPI_RC_OK ) cop=NULL;
            
   lop = _makePath_FCCard( _broker, ctx, ref, sAdapterPtr, rc );
   if( rc->rc != CMPI_RC_OK ) lop=NULL;       

   rop = _makePath_FCPortController( _broker, ctx, ref, sPortPtr, rc );
   if( rc->rc != CMPI_RC_OK ) rop=NULL;       
         
   if( cop == NULL || lop == NULL || rop == NULL) {
               
      _OSBASE_TRACE(1,("--- %s CMPI _makePath_FCRealizes() failed creating object paths.",_ClassName));
      CMSetStatusWithChars( _broker, rc,
                               CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(1,("--- %s CMPI _makePath_FCRealizes() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
      return NULL;

   }
         
   CMAddKey( cop, _RefLeft, (CMPIValue*)&(lop), CMPI_ref ); 
   CMAddKey( cop, _RefRight, (CMPIValue*)&(rop), CMPI_ref ); 
   
   _OSBASE_TRACE(1,("--- _makePath_FCRealizes() exited"));
   return cop;
}

/* ---------------------------------------------------------------------------*/
/*                        _makeInst_FCRealizes()                              */
/* ---------------------------------------------------------------------------*/
/* method to create object path for Linux_FCRealizes                         */
/*
 * ref         - source object path
 * _ClassName  - "Linux_FCRealizes"
 * lAdapterPtr - pointer to hbaAdapterList, only using current sptr
 * lPortPtr    - pointer to hbaPortList, only using current sptr
 * return      - object path (SUCCESS) or NULL(FAILED)
 */
/* ---------------------------------------------------------------------------*/
CMPIInstance * _makeInst_FCRealizes(  const CMPIBroker * _broker,
                 const CMPIContext * ctx,
                 const CMPIObjectPath * ref,
                 char * _ClassName,
                 const struct cim_hbaAdapter * sAdapterPtr, /* hbaAdapter */
                 const struct cim_hbaPort * sPortPtr,       /* hbaPort    */
                 CMPIStatus * rc)
{
   CMPIObjectPath * cop = NULL;                /* class object path     */
   CMPIObjectPath * lop = NULL;                /* left end object path  */
   CMPIObjectPath * rop = NULL;                /* right end object path */
   CMPIInstance   * cin = NULL;                /* class instance        */
   
   _OSBASE_TRACE(1,("--- _makeInst_FCRealizes() called"));
   
   if ( sAdapterPtr == NULL || sPortPtr == NULL )
      return NULL;
   
   cop = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                             _ClassName, rc );
   if( rc->rc != CMPI_RC_OK ) cop=NULL;
            
   lop = _makePath_FCCard( _broker, ctx, ref, sAdapterPtr, rc );
   if( rc->rc != CMPI_RC_OK ) rop=NULL;       

   rop = _makePath_FCPortController( _broker, ctx, ref, sPortPtr, rc );
   if( rc->rc != CMPI_RC_OK ) rop=NULL;       

   cin = CMNewInstance( _broker, cop, rc);
   if( rc->rc != CMPI_RC_OK ) cin=NULL;     
   
   if( cop == NULL || rop == NULL || cin == NULL ) {
               
      _OSBASE_TRACE(1,("--- %s CMPI _makeInst_FCRealizes() failed creating object paths.",_ClassName));
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(1,("--- %s CMPI _makeInst_FCRealizes() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
      return NULL;

   }
         
   CMSetProperty( cin, _RefLeft, (CMPIValue*)&(lop), CMPI_ref ); 
   CMSetProperty( cin, _RefRight, (CMPIValue*)&(rop), CMPI_ref ); 
   
   _OSBASE_TRACE(1,("--- _makeInst_FCRealizes() exited"));
   return cin;
}



/* ---------------------------------------------------------------------------*/
/*                 _makePathInst_FCPortControllerList()                       */
/* ---------------------------------------------------------------------------*/
/* method to create path/inst list of one CARD for Linux_FCPortController    */
/*
 * ref        - source object path
 * inst       - 1: return instance   0: return object path
 * lptr       - pointer to hbaAdapterPortList
 *              iterator all list items to find the ports on the same card
 *              search from the lptr
 * return     - -1 ( FAILED ) or N (>=0) number of path/inst
 * NOTICE:
 *    Card using Manufacture(64)-Model(256)-SerialNo(64) as its key
 *    We need to search all adapter to find the adapters 
 *       having the same Manufacturer-Model-SerialNo
 */
/* ---------------------------------------------------------------------------*/
int _makePathInst_FCPortControllerList( const CMPIBroker * _broker,
                 const CMPIContext * ctx,
                 const CMPIResult * rslt,
                 const CMPIObjectPath * ref,
                 int inst,
                 const struct hbaAdapterPortList * lptr,
                 CMPIStatus * rc)
{
   CMPIObjectPath * cop;                   /* valid when inst == 0      */
   CMPIInstance   * cin;                   /* valid when inst == 1      */
   const struct hbaAdapterPortList * saved_lptr;
   struct hbaPortList * port_lptr = NULL;
   char * card_manuf = NULL;
   char * card_model = NULL;
   char * card_serno = NULL;
   
   int portNum = 0;
   
   _OSBASE_TRACE(1,("--- _makePathInst_FCPortControllerList() called"));

   if ( lptr == NULL || lptr->adapter_sptr == NULL || lptr->port_lptr == NULL )
      return -1;
   
   saved_lptr = lptr;
   card_manuf = lptr->adapter_sptr->adapter_attributes->Manufacturer;
   card_model = lptr->adapter_sptr->adapter_attributes->Model;
   card_serno = lptr->adapter_sptr->adapter_attributes->SerialNumber;

   for( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next ) {
   
      if ( saved_lptr != lptr ) {
      	if ( strcmp( card_serno, lptr->adapter_sptr->adapter_attributes->SerialNumber ) != 0 ||
      	     strcmp( card_model, lptr->adapter_sptr->adapter_attributes->Model ) != 0 ||
      	     strcmp( card_manuf, lptr->adapter_sptr->adapter_attributes->Manufacturer ) != 0 ) {
      	  
      	  /* this is another card */
      	  /* here we may use break for better performance if it's in order */
      	  continue;
      	}
      }   
   
      /* we need to create object path between each pair of FCCard and FCPortController */
      for ( port_lptr = lptr->port_lptr; port_lptr && rc->rc == CMPI_RC_OK ; port_lptr = port_lptr->next ) {
            
         if ( inst == 1 ) {
            
            /* return instances */
            cin = _makeInst_FCPortController( _broker, ctx, ref, port_lptr->sptr, rc);
            
            if ( rc->rc != CMPI_RC_OK || cin == NULL ) {
            
               _OSBASE_TRACE(2,("--- %s CMPI _makePathInst_FCPortControllerList() failed creating object paths.",_ClassName));
               CMSetStatusWithChars( _broker, rc,
                                     CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
               _OSBASE_TRACE(1,("--- %s CMPI _makePathInst_FCPortControllerList() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
               return -1;
            }
            CMReturnInstance( rslt, cin );
         }
         else {
            
            /* return object paths */
            cop = _makePath_FCPortController( _broker, ctx, ref, port_lptr->sptr, rc);
   
            if ( rc->rc != CMPI_RC_OK || cop == NULL ) {
            
               _OSBASE_TRACE(2,("--- %s CMPI _makePathInst_FCPortControllerList() failed creating object paths.",_ClassName));
               CMSetStatusWithChars( _broker, rc,
                                     CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
               _OSBASE_TRACE(1,("--- %s CMPI _makePathInst_FCPortControllerList() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
               return -1;
            }
            CMReturnObjectPath( rslt, cop );
         }
   
         portNum ++;
      } /* end of for ( port_lptr = lptr->port_list; port_lptr ...) */
   }
   
   _OSBASE_TRACE(1,("--- _makePathInst_FCPortControllerList() exited"));
   return portNum;
}

/* ---------------------------------------------------------------------------*/
/*                    _makeAssoc_FCRealizesPortList()                         */
/* ---------------------------------------------------------------------------*/
/* method to create path/inst list of one CARD for Linux_FCRealizes          */
/*
 * ref        - source object path
 * _ClassName - "Linux_FCRealizes"
 * inst       - 1: return instance   0: return object path
 * lptr       - pointer to hbaAdapterPortList
 *              iterator all list items to find the ports on the same card
 *              search from the lptr
 * return     - -1 ( FAILED ) or N (>=0) number of path/inst
 * NOTICE:
 *    Card using Manufacture(64)-Model(256)-SerialNo(64) as its key
 *    We need to search all adapter to find the adapters 
 *       having the same Manufacturer-Model-SerialNo
 */
/* ---------------------------------------------------------------------------*/
int _makeAssoc_FCRealizesPortList( const CMPIBroker * _broker,
                 const CMPIContext * ctx,
                 const CMPIResult * rslt,
                 const CMPIObjectPath * ref,
                 char * _ClassName,
                 int inst,
                 const struct hbaAdapterPortList * lptr,
                 CMPIStatus * rc)
{
   CMPIObjectPath * cop;                   /* valid when inst == 0      */
   CMPIInstance   * cin;                   /* valid when inst == 1      */
   const struct hbaAdapterPortList * saved_lptr;
   struct hbaPortList * port_lptr = NULL;
   char * card_manuf = NULL;
   char * card_model = NULL;
   char * card_serno = NULL;
   
   int portNum = 0;
   
   _OSBASE_TRACE(1,("--- _makeAssoc_FCRealizesPortList() called"));

   if ( lptr == NULL || lptr->adapter_sptr == NULL || lptr->port_lptr == NULL )
      return -1;
   
   saved_lptr = lptr;
   card_manuf = lptr->adapter_sptr->adapter_attributes->Manufacturer;
   card_model = lptr->adapter_sptr->adapter_attributes->Model;
   card_serno = lptr->adapter_sptr->adapter_attributes->SerialNumber;

   for( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next ) {
   
      if ( saved_lptr != lptr ) {
      	if ( strcmp( card_serno, lptr->adapter_sptr->adapter_attributes->SerialNumber ) != 0 ||
      	     strcmp( card_model, lptr->adapter_sptr->adapter_attributes->Model ) != 0 ||
      	     strcmp( card_manuf, lptr->adapter_sptr->adapter_attributes->Manufacturer ) != 0 ) {
      	  
      	  /* this is another card */
      	  /* here we may use break for better performance if it's in order */
      	  continue;
      	}
      }   
   
      /* we need to create object path between each pair of FCCard and FCPortController */
      for ( port_lptr = lptr->port_lptr; port_lptr && rc->rc == CMPI_RC_OK ; port_lptr = port_lptr->next ) {
            
         if ( inst == 1 ) {
            
            /* return instances */
            cin = _makeInst_FCRealizes(_broker, ctx, ref, _ClassName, 
                                       lptr->adapter_sptr, port_lptr->sptr, rc);
            
            if ( rc->rc != CMPI_RC_OK || cin == NULL ) {
            
               _OSBASE_TRACE(2,("--- %s CMPI _makeAssoc_FCRealizesPortList() failed creating object paths.",_ClassName));
               CMSetStatusWithChars( _broker, rc,
                                     CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
               _OSBASE_TRACE(1,("--- %s CMPI _makeAssoc_FCRealizesPortList() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
               return -1;
            }
            CMReturnInstance( rslt, cin );
         }
         else {
            
            /* return object paths */
            cop = _makePath_FCRealizes(_broker, ctx, ref, _ClassName, 
                                       lptr->adapter_sptr, port_lptr->sptr, rc);
   
            if ( rc->rc != CMPI_RC_OK || cop == NULL ) {
            
               _OSBASE_TRACE(2,("--- %s CMPI _makeAssoc_FCRealizesPortList() failed creating object paths.",_ClassName));
               CMSetStatusWithChars( _broker, rc,
                                     CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
               _OSBASE_TRACE(1,("--- %s CMPI _makeAssoc_FCRealizesPortList() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
               return -1;
            }
            CMReturnObjectPath( rslt, cop );
         }
   
         portNum ++;
      } /* end of for ( port_lptr = lptr->port_list; port_lptr ...) */
   }
   
   _OSBASE_TRACE(1,("--- _makeAssoc_FCRealizesPortList() exited"));
   return portNum;
}


/* ---------------------------------------------------------------------------*/
/*                     _makeAssoc_FCRealizesCardList()                        */
/* ---------------------------------------------------------------------------*/
/* method to create path/inst list for all CARD for Linux_FCRealizes         */
/*
 * ref        - source object path
 * _ClassName - "Linux_FCRealizes"
 * inst       - 1: return instance   0: return object path
 * lptr       - pointer to hbaAdapterPortList, 
 *              will processing all list and remove duplicated key
 * return     - -1 ( FAILED ) or N (>=0) number of path/inst
 */
/* ---------------------------------------------------------------------------*/
int _makeAssoc_FCRealizesCardList( const CMPIBroker * _broker,
                 const CMPIContext * ctx,
                 const CMPIResult * rslt,
                 const CMPIObjectPath * ref,
                 char * _ClassName,
                 int inst,
                 const struct hbaAdapterPortList * lptr,
                 CMPIStatus * rc)
{
   CMPIObjectPath * cop;                   /* valid when inst == 0      */
   CMPIInstance   * cin;                   /* valid when inst == 1      */

   char *key = NULL;                       /* temporary pointer to save key of current hbaAdapter */
   void * keyList = NULL;
   int objNum = 0;
   int retval;
   
   _OSBASE_TRACE(1,("--- _makeAssoc_FCRealizesCardList() called"));

   if ( lptr == NULL )
      return -1;

   /* we need to create object path between each pair of FCCard and FCPortController */
   for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next ) {

      key = _makeKey_FCCard(lptr->adapter_sptr);
      if ( isDuplicateKey(key, &keyList, ADD_TO_LIST) != FALSE ) {
         /* This is a duplicated key, release the buffer of key and search the next adapter */
         /* If not, free the key in isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);  */
         free(key);
         continue;
      }

      /* return instances */
      retval = _makeAssoc_FCRealizesPortList( _broker, ctx, rslt, ref, _ClassName, inst, lptr, rc);
         
      if ( rc->rc != CMPI_RC_OK || retval == -1 ) {
         
         _OSBASE_TRACE(2,("--- %s CMPI _makeAssoc_FCRealizesCardList() failed creating object paths.",_ClassName));
         CMSetStatusWithChars( _broker, rc,
                               CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      
         /* release key list */
         isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);

         _OSBASE_TRACE(1,("--- %s CMPI _makeAssoc_FCRealizesCardList() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
         return -1;
      }

      objNum += retval;
   } /* end of for (; lptr ...) */

   /* release key list */
   isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
   
   _OSBASE_TRACE(1,("--- _makeAssoc_FCRealizesCardList() exited"));
   return objNum;
   
}

/* ---------------------------------------------------------------------------*/
/*                      _assoc_create_refs_FCRealizes()                   */
/* ---------------------------------------------------------------------------*/
/* method to return CMPIInstance(s) / CMPIObjectPath(s) of related objects    */
/* and the association itself                                                 */
/*                                                                            */
/* combination of int <inst> and int <associators> :
 * 0 0 -> referenceNames()
 * 1 0 -> references()
 * 0 1 -> associatorNames()
 * 1 1 -> associators()
 */
/* return value : SUCCESS = 0 ; FAILED = -1                                   */
/* !!! this method returns each found CMPIInstance / CMPIObjectPath object to */
/* the Object Manager (OM)                                                    */
/* ---------------------------------------------------------------------------*/

int _assoc_create_refs_FCRealizes( const CMPIBroker * _broker,
              const CMPIContext * ctx,
              const CMPIResult * rslt,
              const CMPIObjectPath * ref,
              char * _ClassName,
              char * _RefLeftClass,
              char * _RefRightClass,
              char * _RefLeft,
              char * _RefRight,
              int inst,
              int associators,
              CMPIStatus * rc) 
{

   CMPIInstance    * cin  = NULL;          /* The association class instance */
   CMPIObjectPath  * cop = NULL;           /* The association class object path */
   char            * sourceName = NULL;    /* class name of the source side of association */
   char            * targetName = NULL;    /* class name of other side of association */
   CMPIObjectPath  * sop    = NULL;        /* The source side object path */
   CMPIObjectPath  * top    = NULL;        /* The target side object path, not the association class op */
   CMPIInstance    * tin    = NULL;        /* The target side instance, not the association class inst */
   CMPIObjectPath  * lop    = NULL;        /* The left side object path */
   CMPIObjectPath  * rop    = NULL;        /* The right side object path */

   struct hbaAdapterPortList * lptr  = NULL ;
   struct hbaPortList        * port_lptr = NULL;
   struct hbaAdapterPortList * rm    = NULL ;
   char *key = NULL;                       /* temporary pointer to save key of current hbaAdapter */
   void * keyList = NULL;
   
   _OSBASE_TRACE(1,("--- _assoc_create_refs_FCRealizes() called"));

   targetName = _assoc_targetClass_Name(_broker,ref,_RefLeftClass,_RefRightClass,rc);
   if (targetName == _RefRightClass)
      sourceName=_RefLeftClass;
   else
      sourceName=_RefRightClass;

   if( enum_all_hbaAdapterPorts( &lptr ) != 0 ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Could not list hba adapters." );
      _OSBASE_TRACE(2,("--- _assoc_create_refs_FCRealizes() exited"));
      return -1;
   }

   /* check if source instance does exist */
   /* For this association it is a one to N mapping between FCCard and FCPortController */
   /* Also this will find the hbaAdapter associated with the passed in object path if the sop is at the right end */ 
   // iterate adapter list

   rm = lptr;
   if( lptr == NULL )
      goto exit;

   if (sourceName == _RefLeftClass) {
      
      /* sop is at the left end, find an adapter */
      for( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next ) {

         sop = _makePath_FCCard( _broker, ctx, ref, lptr->adapter_sptr, rc);
         if(matchObjectPathKeys(ref,sop) == TRUE) {
            CMRelease(sop);
            goto ref_found;
         }
         CMRelease(sop);
         
      }
      
      /* can not find the hba adapter */
      goto exit;
   }
   else {
      
      /* sop is at the right end, find a port */
      /* port_lptr saves the current port  */
      /* lptr->adapter_sptr saves the current adapter */
      for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next) {

         for ( port_lptr = lptr->port_lptr; port_lptr && rc->rc == CMPI_RC_OK; port_lptr = port_lptr->next ) {

            sop = _makePath_FCPortController( _broker, ctx, ref, port_lptr->sptr, rc );
            if(matchObjectPathKeys(ref,sop) == TRUE) {
               CMRelease(sop);
               goto ref_found;
            }
            CMRelease(sop);
         }

      } /* end for lptr */

      /* can not find the hba port */
      goto exit;
   }

ref_found:
   
   /*** associatorNames() && associators() ***/
   if( associators == 1 ) {
      
      if (targetName==_RefLeftClass) {
         
         if ( inst == 1 ) {
            /* only return the Inst of FCCard */
            tin = _makeInst_FCCard ( _broker, ctx, ref, lptr->adapter_sptr, rc );
            if ( rc->rc == CMPI_RC_OK && tin != NULL )
               CMReturnInstance( rslt, tin );
         }
         else {
            /* only return the path of FCCard */
            top = _makePath_FCCard ( _broker, ctx, ref, lptr->adapter_sptr, rc );
            if ( rc->rc == CMPI_RC_OK && top != NULL )
               CMReturnObjectPath( rslt, top ); 
         }
      }
      else {
         
         /* needs to return all FCPortController path related with the Card */
         _makePathInst_FCPortControllerList( _broker, ctx, rslt, ref, inst, lptr, rc);
         
      }
   } /* end if associatorNames */   


   /*** referenceNames() && references() ***/
   if( associators == 0 ) {
      
      if (targetName==_RefLeftClass) {
         
         /* only return one reference between FCCard and sop (ref) */
         if ( inst == 1 ) {
         
            /* references ( instances ) */
            cin = _makeInst_FCRealizes( _broker, ctx, ref, _ClassName, 
                                        lptr->adapter_sptr, port_lptr->sptr, rc);
            if ( rc->rc == CMPI_RC_OK && cin != NULL )
               CMReturnInstance( rslt, cin );

         }
         else {
            
            /* referenceNames ( object paths ) */
            cop = _makePath_FCRealizes( _broker, ctx, ref, _ClassName, 
                                        lptr->adapter_sptr, port_lptr->sptr, rc);
            if ( rc->rc == CMPI_RC_OK && cop != NULL )
               CMReturnObjectPath( rslt, cop );

         }
      } /* end of targetName==_RefLeftClass */
      else {

         /* we need to create object path between each pair of FCCard and FCPortController */
         _makeAssoc_FCRealizesPortList( _broker, ctx, rslt, ref, _ClassName, inst, lptr, rc);
         
      } /* end of targetName==_RefRightClass */
   } /* end if referenceNames */   

exit:
  if(rm) free_hbaAdapterPortList(rm);
  _OSBASE_TRACE(1,("--- _assoc_create_refs_FCRealizes() exited"));
  return 0;
}


/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


/******************************************************************************/
/* Function: SMIS_FCRealizesProviderCleanup                     */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderCleanup( CMPIInstanceMI * mi,
            const CMPIContext * ctx, CMPIBoolean trm) 
{
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
}



/******************************************************************************/
/* Function: SMIS_FCRealizesProviderEnumInstanceNames           */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderEnumInstanceNames( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref) 
{
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaAdapterPortList * lptr  = NULL ;
   struct hbaAdapterPortList * rm    = NULL ;

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() called",_ClassName));

   if( enum_all_hbaAdapterPorts( &lptr ) != 0 ) {
      CMSetStatusWithChars( _broker, &rc,
                            CMPI_RC_ERR_FAILED, "Could not list hba adapters." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }

   rm = lptr;
   // iterate adapter list
   if( lptr != NULL ) {
      
      _makeAssoc_FCRealizesCardList( _broker, ctx, rslt, ref, 
                                     _ClassName, 0, lptr, &rc);

      /* release adapters list */   
      free_hbaAdapterPortList(rm);
   }

   if ( rc.rc == CMPI_RC_OK )
      CMReturnDone( rslt );
      
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() exited",_ClassName));
   return rc;
} /* end SMIS_FCRealizesProviderEnumInstanceNames */
   

/******************************************************************************/
/* Function: SMIS_FCRealizesProviderEnumInstances               */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderEnumInstances( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref,
            const char ** properties) 
{
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaAdapterPortList * lptr  = NULL ;
   struct hbaAdapterPortList * rm    = NULL ;
   
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() called",_ClassName));

   if( enum_all_hbaAdapterPorts( &lptr ) != 0 ) {
      CMSetStatusWithChars( _broker, &rc,
                            CMPI_RC_ERR_FAILED, "Could not list hba adapters." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }

   rm = lptr;
   // iterate adapter list
   if( lptr != NULL ) {
      
      _makeAssoc_FCRealizesCardList( _broker, ctx, rslt, ref, 
                                     _ClassName, 1, lptr, &rc);      
      /* release adapters list */
      free_hbaAdapterPortList(rm);
   }

   if ( rc.rc == CMPI_RC_OK )
      CMReturnDone( rslt );

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() exited",_ClassName));
   return rc;
} /* end SMIS_FCRealizesProviderEnumInstances */





/******************************************************************************/
/* Function: SMIS_FCRealizesProviderGetInstance                 */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderGetInstance( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop,
            const char ** properties) 
{
   CMPIInstance * ci = NULL;
   CMPIStatus     rc = {CMPI_RC_OK, NULL};

   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() called",_ClassName));
  
   ci = _assoc_get_inst( _broker,ctx,cop,_ClassName,_RefLeft,_RefRight,&rc);

   if( ci == NULL ) { 
      if( rc.msg != NULL ) 
         _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      else
         _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed",_ClassName));

      CMReturn(CMPI_RC_ERR_NOT_FOUND); 
   }

   CMReturnInstance( rslt, ci );
   CMReturnDone(rslt);
   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() exited",_ClassName));
   return rc;
} /* end SMIS_FCRealizesProviderGetInstance */





/******************************************************************************/
/* Function: SMIS_FCRealizesProviderCreateInstance              */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderCreateInstance( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop,
            const CMPIInstance * ci) 
{
   CMPIStatus rc = {CMPI_RC_OK, NULL};

   _OSBASE_TRACE(1,("--- %s CMPI CreateInstance() called",_ClassName));

   CMSetStatusWithChars( _broker, &rc,
                         CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" );

   _OSBASE_TRACE(1,("--- %s CMPI CreateInstance() called",_ClassName));
   return rc;
}
   


/******************************************************************************/
/* Function: SMIS_FCRealizesProviderModifyInstance              */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderModifyInstance( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop,
            const CMPIInstance * ci,
            const char **properties) 
{
   CMPIStatus rc = {CMPI_RC_OK, NULL};

   _OSBASE_TRACE(1,("--- %s CMPI ModifyInstance() called",_ClassName));

   CMSetStatusWithChars( _broker, &rc,
                         CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" );

   _OSBASE_TRACE(1,("--- %s CMPI ModifyInstance() exited",_ClassName));
   return rc;
}





/******************************************************************************/
/* Function: SMIS_FCRealizesProviderDeleteInstance              */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderDeleteInstance( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop) 
{
   CMPIStatus rc = {CMPI_RC_OK, NULL};

   _OSBASE_TRACE(1,("--- %s CMPI DeleteInstance() called",_ClassName));

   CMSetStatusWithChars( _broker, &rc,
                         CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" );

   _OSBASE_TRACE(1,("--- %s CMPI DeleteInstance() exited",_ClassName));
   return rc;
}




/******************************************************************************/
/* Function: SMIS_FCRealizesProviderExecQuery                   */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderExecQuery( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref,
            const char * lang,
            const char * query) 
{
   CMPIStatus rc = {CMPI_RC_OK, NULL};

   _OSBASE_TRACE(1,("--- %s CMPI ExecQuery() called",_ClassName));

   CMSetStatusWithChars( _broker, &rc,
                         CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" );

   _OSBASE_TRACE(1,("--- %s CMPI ExecQuery() exited",_ClassName));
   return rc;
}



/* ---------------------------------------------------------------------------*/
/*                    Associator Provider Interface                           */
/* ---------------------------------------------------------------------------*/



/******************************************************************************/
/* Function: SMIS_FCRealizesProviderAssociationCleanup          */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderAssociationCleanup( 
   CMPIAssociationMI * mi,
   const CMPIContext * ctx, CMPIBoolean trm) 
{
   _OSBASE_TRACE(1,("--- %s CMPI AssociationCleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI AssociationCleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
}


/* 
 * The intention of associations is to show the relations between different
 * classes and their instances. Therefore an association has two properties. 
 * Each one representing a reference to a certain instance of the specified
 * class. We can say, that an association has a left and the right "end". 
 *
 * Linux_FCElementStatiscalData : 
 *    < role >   -> < class >
 *    Antecedent -> Linux_FCCard
 *    Dependent  -> Linux_FCPortController
 *
 */

/*
 * general API information
 *
 * cop :
 *    Specifies the CMPIObjectPath to the CMPIInstance of the known end of 
 *    the association. Its absolutelly necessary to define this anchor point,
 *    from where the evaluation of the association starts.
 * assocPath : 
 *    If not NULL, it contains the name of the association the caller is 
 *    looking for. The provider should only return values, when the assocPath 
 *    contains the name(s) of the association(s) he is responsible for !
 * role :
 *    The caller can specify that the source instance ( cop ) has to play 
 *    a certain <role> in the association. The <role> is specified by the
 *    association definition (see above). That means, role has to contain
 *    the same value as the <role> the source instance plays in this assoc.
 *    If this requirement is not true, the provider returns nothing.
 */

/*
 * specification of associators() and associatorNames()
 *
 * These methods return CMPIInstance ( in the case of associators() ) or 
 * CMPIObjectPath ( in the case of associatorNames() ) object(s) of the 
 * opposite end of the association.
 *
 * resultRole :
 *    The caller can specify that the target instance(s) has/have to play 
 *    a certain <role> in the association. The <role> is specified by the
 *    association definition (see above). That means, resultRole has to 
 *    contain the same value as the <role> the target instance(s) plays
 *    in this assoc. If this requirement is not true, the provider returns 
 *    nothing.
 * resultClass :
 *    The caller can specify that the target instance(s) has/have to be 
 *    instances of a certain <class>. The <class> is specified by the
 *    association definition (see above). That means, resultClass has to 
 *    contain the same value as the <class> of the target instance(s).
 *    If this requirement is not true, the provider returns nothing.
 */



/******************************************************************************/
/* Function: SMIS_FCRealizesProviderAssociators                 */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderAssociators( CMPIAssociationMI * mi,
           const CMPIContext * ctx,
           const CMPIResult * rslt,
           const CMPIObjectPath * cop,
           const char * assocClass,
           const char * resultClass,
           const char * role,
           const char * resultRole,
           const char ** propertyList ) 
{
   CMPIStatus       rc    = {CMPI_RC_OK, NULL};
   CMPIObjectPath * op    = NULL;                 /* An association object path */
   int              refrc = 0;

   _OSBASE_TRACE(1,("--- %s CMPI Associators() called",_ClassName));

   if( assocClass ) 
      op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
             _ClassName, &rc );

   if( ( assocClass!=NULL ) && ( CMClassPathIsA(_broker,op,assocClass,&rc) == 0 ) )
      goto exit;


   if( _assoc_check_parameter_const( _broker,cop,_RefLeft,_RefRight,
      _RefLeftClass,_RefRightClass,
      resultClass,role,resultRole, 
      &rc ) == 0 ) 
      goto exit;

   /* There are no children classes of FCPortController or FCCard, so the only resultClass that */
   /*    makes sense is the _RefRightClass or _RefLeftClass */

   refrc = _assoc_create_refs_FCRealizes(_broker, ctx, rslt, cop, 
         _ClassName,_RefLeftClass,_RefRightClass,
         _RefLeft,_RefRight,
         1, 1, &rc);    

   if( refrc != 0 ) { 
      if( rc.msg != NULL )
         _OSBASE_TRACE(1,("--- %s CMPI Associators() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      else
         _OSBASE_TRACE(1,("--- %s CMPI Associators() failed",_ClassName));

      CMReturn(CMPI_RC_ERR_FAILED); 
   }

exit:
   if(op) CMRelease(op);
   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI Associators() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
} /* end SMIS_FCRealizesProviderAssociators */





/******************************************************************************/
/* Function: SMIS_FCRealizesProviderAssociatorNames             */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderAssociatorNames( CMPIAssociationMI * mi,
           const CMPIContext * ctx,
           const CMPIResult * rslt,
           const CMPIObjectPath * cop,
           const char * assocClass,
           const char * resultClass,
           const char * role,
           const char * resultRole) 
{
   CMPIStatus       rc    = {CMPI_RC_OK, NULL};
   CMPIObjectPath * op    = NULL;                 /* An association object path */
   int              refrc = 0;

   _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() called",_ClassName));

   if( assocClass ) 
      op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
             _ClassName, &rc );

   if( ( assocClass!=NULL ) && ( CMClassPathIsA(_broker,op,assocClass,&rc) == 0 ) )
      goto exit;


   if( _assoc_check_parameter_const( _broker,cop,_RefLeft,_RefRight,
      _RefLeftClass,_RefRightClass,
      resultClass,role,resultRole, 
      &rc ) == 0 ) 
      goto exit;

   /* There are no children classes of FCPortController or FCCard, so the only resultClass that */
   /*    makes sense is the _RefRightClass or _RefLeftClass */

   refrc = _assoc_create_refs_FCRealizes(_broker, ctx, rslt, cop, 
         _ClassName,_RefLeftClass,_RefRightClass,
         _RefLeft,_RefRight,
         0, 1, &rc);    

   if( refrc != 0 ) { 
      if( rc.msg != NULL )
         _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      else
         _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() failed",_ClassName));

      CMReturn(CMPI_RC_ERR_FAILED); 
   }

exit:
   if(op) CMRelease(op);
   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
} /* end SMIS_FCRealizesProviderAssociatorNames */





/******************************************************************************/
/* Function: SMIS_FCRealizesProviderReferences                  */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderReferences( CMPIAssociationMI * mi,
           const CMPIContext * ctx,
           const CMPIResult * rslt,
           const CMPIObjectPath * cop,
           const char * assocClass,
           const char * role,
           const char ** propertyList ) 
{
   CMPIStatus       rc    = {CMPI_RC_OK, NULL};
   CMPIObjectPath * op    = NULL;
   int              refrc = 0;

   _OSBASE_TRACE(1,("--- %s CMPI References() called",_ClassName));

   if( assocClass ) 
      op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
             _ClassName, &rc );

   if( ( assocClass!=NULL ) && ( CMClassPathIsA(_broker,op,assocClass,&rc) == 0 ) )
      goto exit;

   if( _assoc_check_parameter_const( _broker,cop,_RefLeft,_RefRight,
      _RefLeftClass,_RefRightClass,
      NULL,role,NULL, 
      &rc ) == 0 ) 
      goto exit;

    
   /* There are no children classes of FCPortController or FCCard, so the only resultClass that */
   /*    makes sense is the _RefRightClass or _RefLeftClass */

   refrc = _assoc_create_refs_FCRealizes(_broker, ctx, rslt, cop, 
         _ClassName,_RefLeftClass,_RefRightClass,
         _RefLeft,_RefRight,
         1, 0, &rc);    

   if( refrc != 0 ) { 
      if( rc.msg != NULL )
         _OSBASE_TRACE(1,("--- %s CMPI References() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      else
         _OSBASE_TRACE(1,("--- %s CMPI References() failed",_ClassName));

      CMReturn(CMPI_RC_ERR_FAILED); 
   }

exit:
   if(op) CMRelease(op);
   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI References() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
} /* end SMIS_FCRealizesProviderReferences */





/******************************************************************************/
/* Function: SMIS_FCRealizesProviderReferenceNames              */
/******************************************************************************/
CMPIStatus SMIS_FCRealizesProviderReferenceNames( CMPIAssociationMI * mi,
           const CMPIContext * ctx,
           const CMPIResult * rslt,
           const CMPIObjectPath * cop,
           const char * assocClass,
           const char * role) 
{
   CMPIStatus       rc    = {CMPI_RC_OK, NULL};
   CMPIObjectPath * op    = NULL;
   int              refrc = 0;

   _OSBASE_TRACE(1,("--- %s CMPI ReferenceNames() called",_ClassName));
    
   if( assocClass ) 
      op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
             _ClassName, &rc );


   if( ( assocClass!=NULL ) && ( CMClassPathIsA(_broker,op,assocClass,&rc) == 0 ) )
      goto exit;


   if( _assoc_check_parameter_const( _broker,cop,_RefLeft,_RefRight,
      _RefLeftClass,_RefRightClass,
      NULL,role,NULL, 
      &rc ) == 0 ) 
      goto exit;

    
   /* There are no children classes of FCPortController or FCCard, so the only resultClass that */
   /*    makes sense is the _RefRightClass or _RefLeftClass */

   refrc = _assoc_create_refs_FCRealizes(_broker, ctx, rslt, cop, 
         _ClassName,_RefLeftClass,_RefRightClass,
         _RefLeft,_RefRight,
         0, 0, &rc);    

   if( refrc != 0 ) { 
      if( rc.msg != NULL )
         _OSBASE_TRACE(1,("--- %s CMPI ReferenceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      else
         _OSBASE_TRACE(1,("--- %s CMPI ReferenceNames() failed",_ClassName));

      CMReturn(CMPI_RC_ERR_FAILED); 
   }

exit:
   if(op) CMRelease(op);
   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI ReferenceNames() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
} /* end SMIS_FCRealizesProviderReferenceNames */





/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( SMIS_FCRealizesProvider,
                  SMIS_FCRealizesProvider,
                  _broker,
                  CMNoHook)

CMAssociationMIStub( SMIS_FCRealizesProvider,
                SMIS_FCRealizesProvider,
                _broker,
                CMNoHook)


/* ---------------------------------------------------------------------------*/
/*                end of cmpiSMIS_FCRealizesProvider                              */
/* ---------------------------------------------------------------------------*/
