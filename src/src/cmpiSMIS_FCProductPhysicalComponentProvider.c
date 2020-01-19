/*
 * $Id: cmpiSMIS_FCProductPhysicalComponentProvider.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 *      Fibre Channel Product Physical Component Provider.
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

static char * _ClassName     = "Linux_FCProductPhysicalComponent";
static char * _RefLeft       = "GroupComponent";
static char * _RefRight      = "PartComponent";
static char * _RefLeftClass  = "Linux_FCProduct";
static char * _RefRightClass = "Linux_FCCard";

/*
 * Usually, one product can contains one or more than one components.
 * The relationship may be 1:1 or 1:N
 * for HBA on linux, it should be 1:1.
 * One FC HBA product contains one card
 */

/* ---------------------------------------------------------------------------*/
/*                    _assoc_create_refs_FCProductPhysicalComponent()         */
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

int _assoc_create_refs_FCProductPhysicalComponent( const CMPIBroker * _broker,
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

   CMPIInstance    * cin = NULL;          /* The association class instance */
   CMPIObjectPath  * cop = NULL;           /* The association class object path */
   char            * sourceName = NULL;    /* class name of the source side of association */
   char            * targetName = NULL;    /* class name of other side of association */
   CMPIObjectPath  * sop    = NULL;        /* The source side object path */
   CMPIObjectPath  * top    = NULL;        /* The target side object path, not the association class op */
   CMPIInstance    * tin    = NULL;        /* The target side instance, not the association class inst */
   CMPIObjectPath  * lop    = NULL;        /* The left side object path */
   CMPIObjectPath  * rop    = NULL;        /* The right side object path */

   struct hbaAdapterList * lptr  = NULL ;
   struct hbaAdapterList * rm    = NULL ;

   _OSBASE_TRACE(1,("--- _assoc_create_refs_FCProductPhysicalComponent() called"));

   targetName = _assoc_targetClass_Name(_broker,ref,_RefLeftClass,_RefRightClass,rc);
   if (targetName == _RefRightClass)
      sourceName=_RefLeftClass;
   else
      sourceName=_RefRightClass;

   if( enum_all_hbaAdapters( &lptr ) != 0 ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Could not list hba adapters." );
      _OSBASE_TRACE(2,("--- _assoc_create_refs_FCProductPhysicalComponent() exited"));
      return -1;
   }

   /* check if source instance does exist */
   /* For this association it is a one to one mapping between FCProduct and FCCard */
   /* Also this will find the hbaAdapter associated with the passed in object path */ 
   rm = lptr;
   if( lptr == NULL )
      goto exit;
       
   // iterate adapter list
   for ( ; lptr && rc->rc == CMPI_RC_OK ; lptr = lptr->next) {
      if (sourceName==_RefLeftClass)
         sop = _makePath_FCProduct( _broker, ctx, ref,lptr->sptr, rc );
      else
         sop = _makePath_FCCard( _broker, ctx, ref,lptr->sptr, rc );

      if(matchObjectPathKeys(ref,sop) == TRUE) {
         CMRelease(sop);
	      break;
	   }

      CMRelease(sop);
   } /* end for lptr */
   
   /* can not find the hdaAdapter associated with the passed in object path */   
   if ( lptr == NULL )
      goto exit;

   /*** associatorNames() ***/
   if( (associators == 1) && (inst == 0) ) {
      if (targetName==_RefLeftClass)
         top = _makePath_FCProduct( _broker, ctx, ref, lptr->sptr, rc );
      else
         top = _makePath_FCCard( _broker, ctx, ref, lptr->sptr, rc );

      CMReturnObjectPath( rslt, top ); 
   } /* end if associatorNames */   


   /*** associators() ***/
   if( (associators == 1) && (inst == 1) ) {
      if (targetName==_RefLeftClass)
         tin = _makeInst_FCProduct( _broker, ctx, ref, lptr->sptr, rc );
      else
         tin = _makeInst_FCCard( _broker, ctx, ref, lptr->sptr, rc );

      CMReturnInstance( rslt, tin ); 
   } /* end if associators */   


   /*** referenceNames() ***/
   if( (associators == 0) && (inst == 0) ) {
      cop = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
            _ClassName, rc );
      if( rc->rc != CMPI_RC_OK ) cop=NULL;
      		 
      lop = _makePath_FCProduct( _broker, ctx, ref,lptr->sptr, rc );
      if( rc->rc != CMPI_RC_OK ) lop=NULL;		 
      
      rop = _makePath_FCCard( _broker, ctx, ref,lptr->sptr, rc );
      if( rc->rc != CMPI_RC_OK ) rop=NULL;		 
	    
      if( cop == NULL || lop == NULL || rop == NULL) {
         _OSBASE_TRACE(1,("--- %s CMPI _assoc_create_refs_FCProductPhysicalComponent() failed creating object paths.",_ClassName));
         CMSetStatusWithChars( _broker, rc,
                               CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );

         if(rm) free_hbaAdapterList(rm);

         _OSBASE_TRACE(1,("--- %s CMPI _assoc_create_refs_FCProductPhysicalComponent() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
         return -1;
      }
	 
      CMAddKey( cop, _RefLeft, (CMPIValue*)&(lop), CMPI_ref ); 
      CMAddKey( cop, _RefRight, (CMPIValue*)&(rop), CMPI_ref ); 

      CMReturnObjectPath( rslt, cop ); 
   } /* end if referenceNames */   


   /*** references() ***/
   if( (associators == 0) && (inst == 1) ) {
      cop = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
            _ClassName, rc );
      if( rc->rc != CMPI_RC_OK ) cop=NULL;
      		 
      lop = _makePath_FCProduct( _broker, ctx, ref,lptr->sptr, rc );
      if( rc->rc != CMPI_RC_OK ) lop=NULL;		 
      
      rop = _makePath_FCCard( _broker, ctx, ref,lptr->sptr, rc );
      if( rc->rc != CMPI_RC_OK ) rop=NULL;		 
      
      cin = CMNewInstance( _broker, cop, rc);
      if( rc->rc != CMPI_RC_OK ) cin=NULL;		 
	    
      if( cop == NULL || lop == NULL || rop == NULL || cin == NULL ) {
         _OSBASE_TRACE(1,("--- %s CMPI _assoc_create_refs_FCProductPhysicalComponent() failed creating object paths or instances.",_ClassName));
         CMSetStatusWithChars( _broker, rc,
                               CMPI_RC_ERR_FAILED, "Create CMPINewInstance failed." );
                               
         if(rm) free_hbaAdapterList(rm);
         
         _OSBASE_TRACE(1,("--- %s CMPI _assoc_create_refs_FCProductPhysicalComponent() failed : %s",_ClassName,CMGetCharPtr(rc->msg)));
         return -1;
      }
	 
      CMSetProperty( cin, _RefLeft, (CMPIValue*)&(lop), CMPI_ref ); 
      CMSetProperty( cin, _RefRight, (CMPIValue*)&(rop), CMPI_ref ); 

      CMReturnInstance( rslt, cin ); 
      } /* end if references */   


 exit:
  if(rm) free_hbaAdapterList(rm);
  _OSBASE_TRACE(1,("--- _assoc_create_refs_FCProductPhysicalComponent() exited"));
  return 0;
}




/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


/******************************************************************************/
/* Function: SMIS_FCProductPhysicalComponentProviderCleanup                     */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderCleanup( CMPIInstanceMI * mi,
            const CMPIContext * ctx, CMPIBoolean trm) 
{
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
}



/******************************************************************************/
/* Function: SMIS_FCProductPhysicalComponentProviderEnumInstanceNames           */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderEnumInstanceNames( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref) 
{
   CMPIObjectPath     * op    = NULL;    /* An association object path */
   CMPIObjectPath     * lop    = NULL;   /* The left side object path */
   CMPIObjectPath     * rop    = NULL;   /* The right side object path */
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   
   struct hbaAdapterList * lptr  = NULL ;
   struct hbaAdapterList * rm    = NULL ;
   char *key = NULL;		/* temporary pointer for each key of adapters */
   void * keyList = NULL;


   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() called",_ClassName));
  
   if( enum_all_hbaAdapters( &lptr ) != 0 ) {
      CMSetStatusWithChars( _broker, &rc,
                            CMPI_RC_ERR_FAILED, "Could not list hba adapters." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }

   rm = lptr;
   // iterate adapter list
   if( lptr != NULL ) {
   	
      for ( ; lptr && rc.rc == CMPI_RC_OK ; lptr = lptr->next) {
      	 
         key = _makeKey_FCProduct(lptr->sptr);
      	if ( isDuplicateKey(key, &keyList, ADD_TO_LIST) != FALSE ) {
            /* This is a duplicated key, release the buffer of key and search the next adapter */
            /* If not, free the key in isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);  */
      	   free( key );
      	   continue;
      	}     	
      	
         op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,&rc)),
                               _ClassName, &rc );
         if( rc.rc != CMPI_RC_OK ) op=NULL;		 
         
         lop = _makePath_FCProduct( _broker, ctx, ref,lptr->sptr, &rc );
	      if( rc.rc != CMPI_RC_OK ) lop=NULL;		 
         
         rop = _makePath_FCCard( _broker, ctx, ref,lptr->sptr, &rc );
	      if( rc.rc != CMPI_RC_OK ) rop=NULL;		 

         if( op == NULL || lop == NULL || rop == NULL) {
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed creating object paths.",_ClassName));
            CMSetStatusWithChars( _broker, &rc,
                               CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );

            /* release key list */
            isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
            /* release adapters list */
            if(rm) free_hbaAdapterList(rm);                               

            _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            return rc;
         }
	 
         CMAddKey( op, _RefLeft, (CMPIValue*)&(lop), CMPI_ref ); 
         CMAddKey( op, _RefRight, (CMPIValue*)&(rop), CMPI_ref ); 

         CMReturnObjectPath( rslt, op ); 
      }
      
      /* release the key list and all keys */
      isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
      /* release adapters list */   
      if(rm) free_hbaAdapterList(rm);
   }

   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() exited",_ClassName));
   return rc;
} /* end SMIS_FCProductPhysicalComponentProviderEnumInstanceNames */
   

/******************************************************************************/
/* Function: SMIS_FCProductPhysicalComponentProviderEnumInstances               */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderEnumInstances( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref,
            const char ** properties) 
{
   CMPIInstance       * ci    = NULL;
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaAdapterList * lptr  = NULL ;
   struct hbaAdapterList * rm    = NULL ;
   int                  refrc = 0;
   CMPIObjectPath     * op    = NULL;    /* An association object path */
   CMPIObjectPath     * lop    = NULL;   /* The left side object path */
   CMPIObjectPath     * rop    = NULL;   /* The right side object path */
   char *key = NULL;		/* temporary pointer for each key of adapters */
   void * keyList = NULL;

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() called",_ClassName));

   if( enum_all_hbaAdapters( &lptr ) != 0 ) {
      CMSetStatusWithChars( _broker, &rc,
                            CMPI_RC_ERR_FAILED, "Could not list hba adapters." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }

   rm = lptr;
   // iterate adapter list
   if( lptr != NULL ) {
      for ( ; lptr && rc.rc == CMPI_RC_OK ; lptr = lptr->next) {
         
         key = _makeKey_FCProduct(lptr->sptr);
         if ( isDuplicateKey(key, &keyList, ADD_TO_LIST) != FALSE ) {
            /* This is a duplicated key, release the buffer of key and search the next adapter */
            /* If not, free the key in isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);  */
            free(key);
            continue;
         }
      	       	
         op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,&rc)),
            _ClassName, &rc );
         if( rc.rc != CMPI_RC_OK ) op=NULL;		 
         
         lop = _makePath_FCProduct( _broker, ctx, ref,lptr->sptr, &rc );
	      if( rc.rc != CMPI_RC_OK ) lop=NULL;		 
         
         rop = _makePath_FCCard( _broker, ctx, ref,lptr->sptr, &rc );
	      if( rc.rc != CMPI_RC_OK ) rop=NULL;		 

         if( op == NULL || lop == NULL || rop == NULL) {
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed creating object paths.",_ClassName));
            CMSetStatusWithChars( _broker, &rc,
                               CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
            
            /* release key list */
            isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
            /* release adapters list */
            if(rm) free_hbaAdapterList(rm);
            
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            return rc;
         }
	 
         ci = CMNewInstance( _broker, op, &rc);
         if( CMIsNullObject(ci) ) {
            CMSetStatusWithChars( _broker, &rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed creating instance.",_ClassName));
            
            /* release key list */
            isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
            /* release adapters list */
            if(rm) free_hbaAdapterList(rm);
            
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            return rc;
         }
	    
         CMSetProperty( ci, _RefLeft, (CMPIValue*)&(lop), CMPI_ref ); 
         CMSetProperty( ci, _RefRight, (CMPIValue*)&(rop), CMPI_ref ); 

         CMReturnInstance( rslt, ci ); 
      }
      
      /* release key list */
      isDuplicateKey(NULL, &keyList, FREE_LIST_AND_KEYS);
      /* release adapters list */
      if(rm) free_hbaAdapterList(rm);
   }

   CMReturnDone( rslt );

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() exited",_ClassName));
   return rc;
} /* end SMIS_FCProductPhysicalComponentProviderEnumInstances */





/******************************************************************************/
/* Function: SMIS_FCProductPhysicalComponentProviderGetInstance                 */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderGetInstance( CMPIInstanceMI * mi,
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
} /* end SMIS_FCProductPhysicalComponentProviderGetInstance */





/******************************************************************************/
/* Function: SMIS_FCProductPhysicalComponentProviderCreateInstance              */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderCreateInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCProductPhysicalComponentProviderModifyInstance              */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderModifyInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCProductPhysicalComponentProviderDeleteInstance              */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderDeleteInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCProductPhysicalComponentProviderExecQuery                   */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderExecQuery( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCProductPhysicalComponentProviderAssociationCleanup          */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderAssociationCleanup( 
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
 *    GroupComponent -> Linux_FCProduct
 *    PartComponent  -> Linux_FCCard
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
/* Function: SMIS_FCProductPhysicalComponentProviderAssociators                 */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderAssociators( CMPIAssociationMI * mi,
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

   /* There are no children classes of FCProduct or FCCard, so the only resultClass that */
   /*    makes sense is the _RefRightClass or _RefLeftClass */

   refrc = _assoc_create_refs_FCProductPhysicalComponent(_broker, ctx, rslt, cop, 
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
} /* end SMIS_FCProductPhysicalComponentProviderAssociators */





/******************************************************************************/
/* Function: SMIS_FCProductPhysicalComponentProviderAssociatorNames             */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderAssociatorNames( CMPIAssociationMI * mi,
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

   /* There are no children classes of FCProduct or FCCard, so the only resultClass that */
   /*    makes sense is the _RefRightClass or _RefLeftClass */

   refrc = _assoc_create_refs_FCProductPhysicalComponent(_broker, ctx, rslt, cop, 
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
} /* end SMIS_FCProductPhysicalComponentProviderAssociatorNames */





/******************************************************************************/
/* Function: SMIS_FCProductPhysicalComponentProviderReferences                  */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderReferences( CMPIAssociationMI * mi,
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

    
   /* There are no children classes of FCProduct or FCCard, so the only resultClass that */
   /*    makes sense is the _RefRightClass or _RefLeftClass */

   refrc = _assoc_create_refs_FCProductPhysicalComponent(_broker, ctx, rslt, cop, 
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
} /* end SMIS_FCProductPhysicalComponentProviderReferences */





/******************************************************************************/
/* Function: SMIS_FCProductPhysicalComponentProviderReferenceNames              */
/******************************************************************************/
CMPIStatus SMIS_FCProductPhysicalComponentProviderReferenceNames( CMPIAssociationMI * mi,
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

    
   /* There are no children classes of FCProduct or FCCard, so the only resultClass that */
   /*    makes sense is the _RefRightClass or _RefLeftClass */

   refrc = _assoc_create_refs_FCProductPhysicalComponent(_broker, ctx, rslt, cop, 
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
} /* end SMIS_FCProductPhysicalComponentProviderReferenceNames */





/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( SMIS_FCProductPhysicalComponentProvider,
                  SMIS_FCProductPhysicalComponentProvider,
                  _broker,
                  CMNoHook)

CMAssociationMIStub( SMIS_FCProductPhysicalComponentProvider,
                     SMIS_FCProductPhysicalComponentProvider,
                     _broker,
                     CMNoHook)


/* ---------------------------------------------------------------------------*/
/*                end of cmpiSMIS_FCProductPhysicalComponentProvider                              */
/* ---------------------------------------------------------------------------*/
