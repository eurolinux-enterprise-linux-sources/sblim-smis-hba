/*
 * $Id: cmpiSMIS_FCSystemDeviceProvider.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 *      FC System Device Driver provider.
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

static char * _ClassName     = "Linux_FCSystemDevice";
static char * _RefLeft       = "GroupComponent";
static char * _RefRight      = "PartComponent";
static char * _RefLeftClass  = "Linux_ComputerSystem";
static char * _RefRightClass = "Linux_FCPortController";

/*
 * Usually, one ComputerSystem can contains more than one PortController.
 * The relationship between ComputerSystem and PortController may be 1:N
 */

#define COMPUTERSYSTEM_CLASS_NAME "Linux_ComputerSystem"
/* method to create a CMPIObjectPath of this class                            */
CMPIObjectPath * _SMIS_makePath_ComputerSystem_Stub( const CMPIBroker * _broker,
            const CMPIContext * ctx, 
            const CMPIObjectPath * cop,
            CMPIStatus * rc)
{
   CMPIObjectPath  * op  = NULL;
   CMPIEnumeration * en  = NULL;
   CMPIData          data ;
   
   _OSBASE_TRACE(1,("--- _SMIS_makePath_ComputerSystem_Stub() called"));
   
   op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)),
                       COMPUTERSYSTEM_CLASS_NAME, rc );
   
   if( CMIsNullObject(op) ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "_SMIS_makePath_ComputerSystem_Stub Create CMPIObjectPath failed." );
      _OSBASE_TRACE(2,("--- _SMIS_makePath_ComputerSystem_Stub() failed : %s",CMGetCharPtr(rc->msg)));
      return NULL;
   }
   
   en = CBEnumInstanceNames( _broker, ctx, op, rc);
   CMRelease(op);
   if( en == NULL ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "CBEnumInstanceNames( _broker, ctx, op, rc)" );
      _OSBASE_TRACE(1,("--- _SMIS_makePath_ComputerSystem_Stub(), en==NULL, failed"));
      return NULL;
   }
   
   while ( CMHasNext( en, rc) && rc->rc == CMPI_RC_OK ) {
      data = CMGetNext( en, rc);
      /* find the first object path and return it, there should be only one object path */
      if( data.value.ref != NULL ) {
      	_OSBASE_TRACE(1,("--- _SMIS_makePath_ComputerSystem_Stub() exited"));
      	return data.value.ref;
      }	
   }
   
   CMSetStatusWithChars( _broker, rc,
                         CMPI_RC_ERR_FAILED, "Can not find object path in CBEnumInstanceNames" );
   _OSBASE_TRACE(1,("--- _SMIS_makePath_ComputerSystem_Stub() can not find Object Path."));
   return NULL;
}

/* method to create a CMPIInstance of this class                              */
CMPIInstance * _SMIS_makeInst_ComputerSystem_Stub(const CMPIBroker * _broker,
           const CMPIContext * ctx, 
           const CMPIObjectPath * cop,
           const char ** properties,
           CMPIStatus * rc)
{
   CMPIObjectPath  * op  = NULL;
   CMPIEnumeration * en  = NULL;
   CMPIData          data ;
   
   _OSBASE_TRACE(1,("--- _SMIS_makeInst_ComputerSystem_Stub() called"));
   
   op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)),
                       COMPUTERSYSTEM_CLASS_NAME, rc );
   
   if( CMIsNullObject(op) ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "_SMIS_makeInst_ComputerSystem_Stub Create CMPIObjectPath failed." );
      _OSBASE_TRACE(2,("--- _SMIS_makeInst_ComputerSystem_Stub() failed : %s",CMGetCharPtr(rc->msg)));
      return NULL;
   }
   
   en = CBEnumInstances( _broker, ctx, op, properties, rc);
   CMRelease(op);
   if( en == NULL ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "CBEnumInstance( _broker, ctx, op, rc)" );
      _OSBASE_TRACE(1,("--- _SMIS_makeInst_ComputerSystem_Stub(), en==NULL, failed"));
      return NULL;
   }

   while ( CMHasNext( en, rc) && rc->rc == CMPI_RC_OK ) {
      data = CMGetNext( en, rc);
      /* find the first instance and return it, there should be only one instance */
      if( data.value.inst != NULL ) {
      	_OSBASE_TRACE(1,("--- _SMIS_makeInst_ComputerSystem_Stub() exited"));
      	return data.value.inst;
      }	
   }
   
   CMSetStatusWithChars( _broker, rc,
                         CMPI_RC_ERR_FAILED, "Can not find instance in CBEnumInstanceNames" );
   _OSBASE_TRACE(1,("--- _SMIS_makeInst_ComputerSystem_Stub can not find Instance."));
   return NULL;
}

/* ---------------------------------------------------------------------------*/
/*                  _makePath_FCSystemDevice()                  */
/* ---------------------------------------------------------------------------*/
/* method to create object path for Linux_FCSystemDevice       */
/*
 * ref - source object path
 * lop - object path on left end ( Linux_ComputerSystem )
 * _ClassName - "Linux_FCSystemDevice"
 * lptr - pointer to hbaPortList, only using current sptr
 * return - object path (SUCCESS) or NULL(FAILED)
 */
/* ---------------------------------------------------------------------------*/
CMPIObjectPath * _makePath_FCSystemDevice(  const CMPIBroker * _broker,
                 const CMPIContext * ctx,
                 const CMPIObjectPath * ref,
                 char * _ClassName,
                 const struct hbaPortList * lptr, /* only use current hbaPort */
                 CMPIStatus * rc)
{
   CMPIObjectPath * cop = NULL;                /* class object path */
   CMPIObjectPath * lop = NULL;                /* left end object path */
   CMPIObjectPath * rop = NULL;                /* right end object path */
   
   _OSBASE_TRACE(1,("--- _makePath_FCSystemDevice() called"));
   
   if ( lptr == NULL || lptr->sptr == NULL )
      return NULL;
   
   cop = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                          _ClassName, rc );
   if( rc->rc != CMPI_RC_OK ) cop=NULL;

   lop = _SMIS_makePath_ComputerSystem_Stub(_broker, ctx, ref, rc);
   if (rc->rc != CMPI_RC_OK) lop = NULL;
            
   rop = _makePath_FCPortController( _broker, ctx, ref, lptr->sptr, rc );
   if( rc->rc != CMPI_RC_OK ) rop=NULL;       
         
   if( cop == NULL || rop == NULL || lop == NULL) {
               
      _OSBASE_TRACE(1,("--- %s CMPI _makePath_FCSystemDevice() failed creating object paths.",
                    _ClassName));
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, 
                            "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(1,("--- %s CMPI _makePath_FCSystemDevice() failed : %s",
                    _ClassName,CMGetCharPtr(rc->msg)));
      return NULL;

   }
         
   CMAddKey( cop, _RefLeft, (CMPIValue*)&(lop), CMPI_ref ); 
   CMAddKey( cop, _RefRight, (CMPIValue*)&(rop), CMPI_ref ); 
   
   _OSBASE_TRACE(1,("--- _makePath_FCSystemDevice() exited"));
   return cop;
}

/* ---------------------------------------------------------------------------*/
/*                      _makeInst_FCSystemDevice()                            */
/* ---------------------------------------------------------------------------*/
/* method to create object path for Linux_FCSystemDevice                     */
/*
 * ref - source object path
 * lop - object path on left end ( Linux_ComputerSystem )
 * _ClassName - "Linux_FCSystemDevice"
 * lptr - pointer to hbaPortList, only using current sptr
 * return - object path (SUCCESS) or NULL(FAILED)
 */
/* ---------------------------------------------------------------------------*/
CMPIInstance * _makeInst_FCSystemDevice(  const CMPIBroker * _broker,
                 const CMPIContext * ctx,
                 const CMPIObjectPath * ref,
                 char * _ClassName,
                 const struct hbaPortList * lptr, /* only use current hbaPort */
                 CMPIStatus * rc)
{
   CMPIObjectPath * cop = NULL;                /* class object path */
   CMPIObjectPath * rop = NULL;                /* right end object path */
   CMPIObjectPath * lop = NULL;                /* left end object path */
   CMPIInstance   * cin = NULL;                /* class instance */
   
   _OSBASE_TRACE(1,("--- _makeInst_FCSystemDevice() called"));
   
   if ( lptr == NULL || lptr->sptr == NULL )
      return NULL;
   
   cop = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                          _ClassName, rc );
   if( rc->rc != CMPI_RC_OK ) cop=NULL;

   lop = _SMIS_makePath_ComputerSystem_Stub( _broker, ctx, ref, rc);
   if( rc->rc != CMPI_RC_OK ) lop = NULL;
            
   rop = _makePath_FCPortController( _broker, ctx, ref, lptr->sptr, rc );
   if( rc->rc != CMPI_RC_OK ) rop=NULL;       

   cin = CMNewInstance( _broker, cop, rc);
   if( rc->rc != CMPI_RC_OK ) cin=NULL;     
   
   if( cop == NULL || rop == NULL || lop == NULL || cin == NULL ) {
      _OSBASE_TRACE(1,("--- %s CMPI _makeInst_FCSystemDevice() failed creating object paths.",_ClassName));
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, 
                            "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(1,("--- %s CMPI _makeInst_FCSystemDevice() failed : %s",
                    _ClassName,CMGetCharPtr(rc->msg)));
      return NULL;

   }
         
   CMSetProperty( cin, _RefLeft, (CMPIValue*)&(lop), CMPI_ref ); 
   CMSetProperty( cin, _RefRight, (CMPIValue*)&(rop), CMPI_ref ); 
   
   _OSBASE_TRACE(1,("--- _makeInst_FCSystemDevice() exited"));
   return cin;
}


/* ---------------------------------------------------------------------------*/
/*                 _makeAssoc_FCSystemDeviceList()              */
/* ---------------------------------------------------------------------------*/
/* method to create path/inst list for Linux_FCSystemDevice    */
/*
 * ref - source object path
 * _ClassName - "Linux_FCSystemDevice"
 * inst - 1: return instance   0: return object path
 * lptr - pointer to hbaPortList, 
 *        will processing all list and remove duplicated key
 * return - -1 ( FAILED ) or N (>=0) number of path/inst
 */
/* ---------------------------------------------------------------------------*/
int _makeAssoc_FCSystemDeviceList( const CMPIBroker * _broker,
                 const CMPIContext * ctx,
                 const CMPIResult * rslt,
                 const CMPIObjectPath * ref,
                 char * _ClassName,
                 int inst,
                 const struct hbaPortList * lptr,
                 CMPIStatus * rc)
{
   CMPIObjectPath * cop;                   /* valid when inst == 0      */
   CMPIInstance   * cin;                   /* valid when inst == 1      */
   char *key = NULL;                       /* temporary pointer to save key of current hbaPort */
   void * keyList = NULL;
   int adapterNum = 0;
   void** resInsOrPath = NULL;             /* pointe to cop or cin to store the result */
   char * failMsg = NULL;                  /* get instance or object path failed message */
   char * failStatusChars = NULL;
   typedef void* (*InsOrPathFunc)(const CMPIBroker*, const CMPIContext*, const CMPIObjectPath*,
                    char*, const struct hbaPortList*, CMPIStatus*);
   InsOrPathFunc makeInsOrPathFunc;
   /* the function used to make instance of object path */
   
   _OSBASE_TRACE(1,("--- _makeAssoc_FCSystemDeviceList() called"));

   if ( lptr == NULL )
      return -1;

   if ( inst == 1 ) { /* return instances */
      resInsOrPath = (void**)& cin;
      makeInsOrPathFunc = (InsOrPathFunc)_makeInst_FCSystemDevice;
      failMsg = "--- %s CMPI _makeAssoc_FCSystemDeviceList() failed creating instance.";
      failStatusChars = "Create CMPIInstance failed.";
   }
   else {             /* return object paths */
      resInsOrPath = (void**)& cop;
      makeInsOrPathFunc = (InsOrPathFunc)_makePath_FCSystemDevice;
      failMsg = "--- %s CMPI _makeAssoc_FCSystemDeviceList() failed creating object paths.";
      failStatusChars = "Create CMPIObjectPath failed.";
   }
        
   /* we need to create object path between each pair of ComputerSystem and FCPortController*/
   for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next ) {
      * resInsOrPath = makeInsOrPathFunc( _broker, ctx, ref, _ClassName, lptr, rc);
      
      if ( rc->rc != CMPI_RC_OK || * resInsOrPath == NULL ) {
      
         _OSBASE_TRACE(2,( failMsg, _ClassName));
         CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, failStatusChars);
         _OSBASE_TRACE(1,("--- %s CMPI _makeAssoc_FCSystemDeviceList() failed : %s",
                       _ClassName,CMGetCharPtr(rc->msg)));
         return -1;
      }
      if (inst == 1){
         CMReturnInstance( rslt, (CMPIInstance*)(*resInsOrPath));
      }
      else {
         CMReturnObjectPath( rslt, (CMPIObjectPath*)(*resInsOrPath));
      }
      adapterNum ++;
   } /* end of for (; lptr ...) */
   _OSBASE_TRACE(1,("--- _makeAssoc_FCSystemDeviceList() exited"));
   return adapterNum;
}


/* for every port in the hbaPortList, create a object path or instance */
int _makeInstOrPathList( const CMPIBroker * _broker,
                                    const CMPIContext * ctx,
                                    const CMPIResult * rslt,
                                    const CMPIObjectPath * ref,
                                    struct hbaPortList * lptr,
                                    CMPIStatus * rc,
                                    int inst)
{
   void * result = NULL;
   struct hbaPortList * rm = lptr;
   int portNum = 0;

   _OSBASE_TRACE(1,("--- _makePathList() called"));
   // iterate port list
   if( lptr != NULL ) {
      for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next) {
         if (inst)
            result = (CMPIInstance*)_makeInst_FCPortController( _broker, ctx, ref, lptr->sptr, rc );
         else
            result = (CMPIObjectPath*)_makePath_FCPortController( _broker, ctx, ref, lptr->sptr, rc );
         if( result == NULL || rc->rc != CMPI_RC_OK ) {
            if( rc->msg != NULL ) {
               _OSBASE_TRACE(2,("--- %s _makePathList() failed : %s",
                             _ClassName,CMGetCharPtr(rc->msg)));
            }
            CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, 
                                 "Transformation from internal structure to CIM ObjectPath failed." );
            if(rm) free_hbaPortList(rm);
            _OSBASE_TRACE(2,("--- %s _makePathList() failed : %s",
                          _ClassName,CMGetCharPtr(rc->msg)));
            break;
         }
         if(inst) CMReturnInstance( rslt, (CMPIInstance*)result);
         else CMReturnObjectPath( rslt, (CMPIObjectPath*)result);
         ++portNum; 
      }
      if(rm) free_hbaPortList(rm);
   } 
   _OSBASE_TRACE(1,("--- _makePathList() exited"));
   return portNum;
}

/* ---------------------------------------------------------------------------*/
/*             _assoc_create_refs_FCSystemDevice()              */
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
int _assoc_create_refs_FCSystemDevice( const CMPIBroker * _broker,
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

   struct hbaPortList * lptr  = NULL ;
   struct hbaPortList * rm    = NULL ;
   char *key = NULL;                       /* temporary pointer to save key of current hbaPort */
   void * keyList = NULL;
   
   _OSBASE_TRACE(1,("--- _assoc_create_refs_FCSystemDevice() called"));

   targetName = _assoc_targetClass_Name(_broker,ref,_RefLeftClass,_RefRightClass,rc);
   sourceName = (targetName == _RefRightClass) ? _RefLeftClass : _RefRightClass;

   if( enum_all_hbaPorts( &lptr, 0 ) != 0 ) {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, 
                            "Could not list hba adapters." );
      _OSBASE_TRACE(2,("--- _assoc_create_refs_FCSystemDevice() exited"));
      return -1;
   }

   /* check if source instance does exist */
   /* For this association it is a one to N mapping between ComputerSystem and FCPortController*/
   /* Also this will find the hbaPort associated with the passed in object path if the sop is at the right end */ 
   /* iterate adapter list */

   rm = lptr;
   if( lptr == NULL )
      goto exit;

   if (sourceName == _RefLeftClass) {
      sop = _SMIS_makePath_ComputerSystem_Stub( _broker, ctx, ref, rc );
      
      /* can not find the object path related with ref */
      if ( sop == NULL )
         goto exit;
   }
   else {
      /* sop is at the right end, find the hbaPort associated with the ref */
      for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next) {
         sop = _makePath_FCPortController( _broker, ctx, ref, lptr->sptr, rc );

         if(matchObjectPathKeys(ref,sop) == TRUE) {
            CMRelease(sop);
            break;
         }
         CMRelease(sop);
      } /* end for lptr */

      /* can not find the hbaPort associated with the ref */
      if ( lptr == NULL )
         goto exit;
   }
   
   /*** associatorNames() && associators() ***/
   if( associators == 1 ) {
      if (targetName==_RefLeftClass) {
         if ( inst == 1 ) {
            /* only return the inst of ComputerSystem */
            tin = _SMIS_makeInst_ComputerSystem_Stub( _broker, ctx, ref, NULL, rc );
            if ( rc->rc == CMPI_RC_OK && tin != NULL )
               CMReturnInstance( rslt, tin );
         }
         else {
            /* only return the path of ComputerSystem */
            top = _SMIS_makePath_ComputerSystem_Stub( _broker, ctx, ref, rc );
            if ( rc->rc == CMPI_RC_OK && top != NULL )
               CMReturnObjectPath( rslt, top ); 
         }
      }
      else {
         _makeInstOrPathList( _broker, ctx, rslt, ref, lptr, rc, inst); 
      }
   } /* end if associators */   

   /*** referenceNames() && references() ***/
   if( associators == 0 ) {
      if (targetName==_RefLeftClass) {

         if ( inst == 1 ) {
            /* references ( instances ) */
            cin = _makeInst_FCSystemDevice( _broker, ctx, ref, _ClassName, lptr, rc);
            if ( rc->rc == CMPI_RC_OK && cin != NULL )
               CMReturnInstance( rslt, cin );
            /* errors have been logged in _makeInst_FCSystemDevice */
         }
         else {
         	/* referenceNames ( object paths ) */
            cop = _makePath_FCSystemDevice( _broker, ctx, ref, _ClassName, lptr, rc);
            if ( rc->rc == CMPI_RC_OK && cop != NULL )
               CMReturnObjectPath( rslt, cop );
            /* errors have been logged in _makeInst_FCSystemDevice */
         }
      } /* end of targetName==_RefLeftClass */
      else {
         /* we need to create object path between each pair of ComputerSystem and FCPortController */
         _makeAssoc_FCSystemDeviceList( _broker, 
               ctx, rslt, ref, _ClassName, inst, lptr, rc);
      } /* end of targetName==_RefRightClass */
   } /* end if references */   

exit:
   if(rm) free_hbaPortList(rm);
   _OSBASE_TRACE(1,("--- _assoc_create_refs_FCSystemDevice() exited"));
   return 0;
}


/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


/******************************************************************************/
/* Function: SMIS_FCSystemDeviceProviderCleanup                     */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderCleanup( CMPIInstanceMI * mi,
            const CMPIContext * ctx, CMPIBoolean trm) 
{
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
}



/******************************************************************************/
/* Function: SMIS_FCSystemDeviceProviderEnumInstanceNames           */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderEnumInstanceNames( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref) 
{
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaPortList * lptr  = NULL ;
   struct hbaPortList * rm    = NULL ;

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() called",_ClassName));

   if( enum_all_hbaPorts( &lptr, 0 ) != 0 ) {
      CMSetStatusWithChars( _broker, &rc, CMPI_RC_ERR_FAILED, 
                            "Could not list hba ports." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",
                    _ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }

   rm = lptr;
   // iterate adapter list
   if( lptr != NULL ) {
      _makeAssoc_FCSystemDeviceList( _broker, ctx, rslt, ref, 
                                     _ClassName, 0, lptr, &rc);

      /* release adapters list */   
      free_hbaPortList(rm);
   }

   if ( rc.rc == CMPI_RC_OK )
      CMReturnDone( rslt );
      
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() exited",_ClassName));
   return rc;
} 
   

/******************************************************************************/
/* Function: SMIS_FCSystemDeviceProviderEnumInstances */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderEnumInstances( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref,
            const char ** properties) 
{
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaPortList * lptr  = NULL ;
   struct hbaPortList * rm    = NULL ;
   
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() called",_ClassName));

   if( enum_all_hbaPorts( &lptr, 0 ) != 0 ) {
      CMSetStatusWithChars( _broker, &rc, CMPI_RC_ERR_FAILED, 
                            "Could not list hba ports." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",
                    _ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }

   rm = lptr;
   // iterate adapter list
   if( lptr != NULL ) {
      _makeAssoc_FCSystemDeviceList( _broker, ctx, rslt, ref, 
                                     _ClassName, 1, lptr, &rc);      
      /* release adapters list */
      free_hbaPortList(rm);
   }

   if ( rc.rc == CMPI_RC_OK )
      CMReturnDone( rslt );

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() exited",_ClassName));
   return rc;
} 





/******************************************************************************/
/* Function: SMIS_FCSystemDeviceProviderGetInstance                 */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderGetInstance( CMPIInstanceMI * mi,
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
} 

/******************************************************************************/
/* Function: SMIS_FCSystemDeviceProviderCreateInstance                        */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderCreateInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCSystemDeviceProviderModifyInstance              */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderModifyInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCSystemDeviceProviderDeleteInstance              */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderDeleteInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCSystemDeviceProviderExecQuery                   */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderExecQuery( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCSystemDeviceProviderAssociationCleanup          */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderAssociationCleanup( 
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
 *    GroupComponent -> Linux_ComputerSystem
 *    PartComponent  -> Linux
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
/* Function: SMIS_FCSystemDeviceProviderAssociators                 */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderAssociators( CMPIAssociationMI * mi,
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
      _RefLeftClass,_RefRightClass, resultClass,role,resultRole, &rc ) == 0 ) 
      goto exit;

   /* There are no children classes of FCPortController, so the only resultClass that */
   /*    makes sense is the _RefRightClass or _RefLeftClass */

   refrc = _assoc_create_refs_FCSystemDevice(_broker, ctx, rslt, cop, 
         _ClassName,_RefLeftClass,_RefRightClass, _RefLeft,_RefRight,
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
} 





/******************************************************************************/
/* Function: SMIS_FCSystemDeviceProviderAssociatorNames             */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderAssociatorNames( CMPIAssociationMI * mi,
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

   if( ( assocClass!=NULL ) && ( CMClassPathIsA(_broker,op,assocClass,&rc) == 0 ) ){
      _OSBASE_TRACE(4,("--- assocClass is NULL or object isn't an assocClass"));
      goto exit;
   }


   if( _assoc_check_parameter_const( _broker,cop,_RefLeft,_RefRight,
         _RefLeftClass,_RefRightClass,
         resultClass,role,resultRole, 
         &rc ) == 0 ) {
      goto exit;
   }

   /* There are no children classes of FCPortController or FCPhysicalPackage, so the only resultClass that */
   /*    makes sense is the _RefRightClass or _RefLeftClass */

   refrc = _assoc_create_refs_FCSystemDevice(_broker, ctx, rslt, cop, 
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
} /* end SMIS_FCSystemDeviceProviderAssociatorNames */





/******************************************************************************/
/* Function: SMIS_FCSystemDeviceProviderReferences                  */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderReferences( CMPIAssociationMI * mi,
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

    
   /* There are no children classes of FCPortController or FCPhysicalPackage, so the only resultClass that */
   /*    makes sense is the _RefRightClass or _RefLeftClass */

   refrc = _assoc_create_refs_FCSystemDevice(_broker, ctx, rslt, cop, 
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
} /* end SMIS_FCSystemDeviceProviderReferences */





/******************************************************************************/
/* Function: SMIS_FCSystemDeviceProviderReferenceNames              */
/******************************************************************************/
CMPIStatus SMIS_FCSystemDeviceProviderReferenceNames( CMPIAssociationMI * mi,
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

    
   /* There are no children classes of FCPortController, so the only resultClass that */
   /*    makes sense is the _RefRightClass or _RefLeftClass */

   refrc = _assoc_create_refs_FCSystemDevice(_broker, ctx, rslt, cop, 
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
} /* end SMIS_FCSystemDeviceProviderReferenceNames */





/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( SMIS_FCSystemDeviceProvider,
                  SMIS_FCSystemDeviceProvider,
                  _broker,
                  CMNoHook)

CMAssociationMIStub( SMIS_FCSystemDeviceProvider,
                SMIS_FCSystemDeviceProvider,
                _broker,
                CMNoHook)


/* ---------------------------------------------------------------------------*/
/*   end of cmpiSMIS_FCSystemDeviceProvider                                   */
/* ---------------------------------------------------------------------------*/
