/*
 * $Id: cmpiLinux_Common.c,v 1.1.1.1 2009/05/12 21:46:34 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2002, 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Author:       Heidi Neumann <heidineu@de.ibm.com>
 * Contributors: Adrian Schuur <schuur@de.ibm.com>
 *
 * Interface Type : Common Manageability Programming Interface ( CMPI )
 *
 * Description:
 * This library contains common methods to write CMPI providers.
 *
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

#include "cmpiOSBase_Common.h"
#include "OSBase_Common.h"

unsigned char CMPI_true=1;
unsigned char CMPI_false=0;


/* ---------------------------------------------------------------------------*/

char * CSCreationClassName = "Linux_ComputerSystem";
char * OSCreationClassName = "Linux_OperatingSystem";

/* ---------------------------------------------------------------------------*/


/* ---------------------------------------------------------------------------*/
/*                    _check_system_key_value_pairs()                         */
/* ---------------------------------------------------------------------------*/
/*     method to check the keys of ComputerSystem and OperatingSystem for     */
/*     their correctness                                                      */
/*                                                                            */
/* ---------------------------------------------------------------------------*/

void _check_system_key_value_pairs( const CMPIBroker * _broker,
				    const CMPIObjectPath * cop,
				    const char * creationClassName,
				    const char * className,
				    CMPIStatus * rc ) {
  CMPIString * name = NULL;

  _OSBASE_TRACE(4,("--- _check_system_key_value_pairs() called"));

   name = CMGetKey( cop, className, rc).value.string;
   if ( rc->rc != CMPI_RC_OK || name == NULL ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Could not get CS/OS Name of instance." );
      _OSBASE_TRACE(4,("--- _check_system_key_value_pairs() failed : %s",CMGetCharPtr(rc->msg)));
      return;
   }

   if (( strcasecmp(CMGetCharPtr(name),get_system_name()) != 0 ) &&
       ( strcasecmp(CMGetCharPtr(name),get_os_name()) != 0 )) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_NOT_FOUND, "This instance does not exist (wrong CS/OS Name)." );
      _OSBASE_TRACE(4,("--- _check_system_key_value_pairs() failed : %s",CMGetCharPtr(rc->msg)));
      return;
   }

   name = CMGetKey( cop, creationClassName, rc).value.string;
   if ( rc->rc != CMPI_RC_OK || name == NULL ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Could not get CS/OS CreationClassName of instance." );
      _OSBASE_TRACE(4,("--- _check_system_key_value_pairs() failed : %s",CMGetCharPtr(rc->msg)));
      return;
   }
   if ( (strcasecmp(CMGetCharPtr(name),CSCreationClassName) != 0) &&
        (strcasecmp(CMGetCharPtr(name),OSCreationClassName) != 0)   ) {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_NOT_FOUND, "This class name does not exist (wrong CS/OS CreationClassName)." );
      _OSBASE_TRACE(4,("--- _check_system_key_value_pairs() failed : %s",CMGetCharPtr(rc->msg)));
      return;
   }

  _OSBASE_TRACE(4,("--- _check_system_key_value_pairs() exited"));
  return;
}


/* ---------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------------*/
/*                      _assoc_create_inst_1toN()                             */
/* ---------------------------------------------------------------------------*/
/*     method to create CMPIInstance(s) / CMPIObjectPath(s) of association    */
/*                                                                            */
/* combination of int <left> and int <inst> :
 * 0 0 -> enum CMPIObjectPath(s) of right end;
 *        return CMPIObjectPath(s) of association
 * 1 0 -> enum CMPIObjectPath(s) of left end;
 *        return CMPIObjectPath(s) of association
 * 0 1 -> enum CMPIObjectPath(s) of right end;
 *        return CMPIInstance(s) of association
 * 1 1 -> enum CMPIObjectPath(s) of left end;
 *        return CMPIInstance(s) of association
 */
/* return value : SUCCESS = 0 ; FAILED = -1                                   */
/* !!! while calling _assoc_create_refs_1toN(), this method returns each      */
/* found CMPIInstance / CMPIObjectPath object to the Object Manager (OM)                                                    */
/* ---------------------------------------------------------------------------*/

int _assoc_create_inst_1toN( const CMPIBroker * _broker,
			     const CMPIContext * ctx,
			     const CMPIResult * rslt,
			     const CMPIObjectPath * cop,
			     const char * _ClassName,
			     const char * _RefLeftClass,
			     const char * _RefRightClass,
			     const char * _RefLeft,
			     const char * _RefRight,
			     int left,
			     int inst,
			     CMPIStatus * rc) {
  CMPIObjectPath  * op  = NULL;
  CMPIEnumeration * en  = NULL;
  CMPIData          data ;
  int               arc = 0;

  _OSBASE_TRACE(2,("--- _assoc_create_inst_1toN() called"));

  if( left == 0 ) {
    op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)),
			  _RefRightClass, rc );
  }
  else { /* left == 1 */
    op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)),
			  _RefLeftClass, rc );
  }
  if( CMIsNullObject(op) ) {
    CMSetStatusWithChars( _broker, rc,
			  CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
    _OSBASE_TRACE(2,("--- _assoc_create_inst_1toN() failed : %s",CMGetCharPtr(rc->msg)));
    return -1;
  }

  en = CBEnumInstanceNames( _broker, ctx, op, rc);
  if( en == NULL ) {
    CMSetStatusWithChars( _broker, rc,
			  CMPI_RC_ERR_FAILED, "CBEnumInstanceNames( _broker, ctx, op, rc)" );
    _OSBASE_TRACE(2,("--- _assoc_create_inst_1toN() failed : %s",CMGetCharPtr(rc->msg)));
    return -1;
  }

  /* this approach works only for 1 to N relations
   * int <left> contains the information, which side of the association has
   * only one instance
   */

  while( CMHasNext( en, rc) ) {

    data = CMGetNext( en, rc);
    if( data.value.ref == NULL ) {
      CMSetStatusWithChars( _broker, rc,
			    CMPI_RC_ERR_FAILED, "CMGetNext( en, rc)" );
      _OSBASE_TRACE(2,("--- _assoc_create_inst_1toN() failed : %s",CMGetCharPtr(rc->msg)));
      return -1;
    }

    //fprintf(stderr,"_assoc_create_inst_1toN(): %s\n",
    //	    CMGetCharPtr(CDToString(_broker, data.value.ref, rc)));

    arc = _assoc_create_refs_1toN( _broker,ctx,rslt,data.value.ref,
				   _ClassName,_RefLeftClass,_RefRightClass,
				   _RefLeft,_RefRight,inst,0,rc);
  }

  _OSBASE_TRACE(2,("--- _assoc_create_inst_1toN() exited"));
  return arc;
}

/* verification one reference property of a association instance */
CMPIObjectPath * verify_assoc_ref (const CMPIBroker * _broker,
				const CMPIContext * ctx,
				const CMPIObjectPath * cop,
				char * _ClassName,
				char * _RefRole,
				CMPIStatus * rc)
{
  CMPIInstance   * ci = NULL;
  CMPIData         dt;
  CMPIObjectPath * op = NULL;  /*  the referece property to be verified */

  _OSBASE_TRACE(3,("--- verify_assoc_ref() called"));

  dt = CMGetKey( cop, _RefRole, rc);
  if( dt.value.ref == NULL ) 
  {
    CMSetStatusWithChars( _broker, rc,
			  CMPI_RC_ERR_FAILED, "CMGetKey( cop, _RefRole, rc)" );
    goto exit;
  }

  if ( dt.type == CMPI_ref )
  {
    op = dt.value.ref;
  }
  else if ( dt.type == CMPI_string )
  { 
	 // if string, create a objpath and set its property
    op = CMNewObjectPath(_broker, 0, 0, rc);
    if( CMIsNullObject(op) )
    {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      goto exit;
    }
#if 0
    // REMOVE - Open Pegasus Funciton. <guocb@cn.ibm.com>
    if (parseObjectPath(CMGetCharPtr(dt.value.string), op))
    {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_NOT_FOUND, "reference not found.");
      goto exit;
    }
#endif
  }
  else
  { 
	 // fail, if input is neither ref or string
    CMSetStatusWithChars( _broker, rc,
                          CMPI_RC_ERR_NOT_FOUND, "reference not found.");
    goto exit;
  }

  CMSetNameSpace(op, CMGetCharPtr(CMGetNameSpace(cop,rc)));
  ci = CBGetInstance(_broker, ctx, op, NULL, rc);

  if( ci == NULL ) {
    if( rc->rc == CMPI_RC_ERR_FAILED ) {
    	CMSetStatusWithChars( _broker, rc,
			  CMPI_RC_ERR_FAILED, "GetInstance of left reference failed.");
    }
    if( rc->rc == CMPI_RC_ERR_NOT_FOUND ) { 
         CMSetStatusWithChars( _broker, rc, 
                               CMPI_RC_ERR_NOT_FOUND, "Left reference not found."); 
    } 
	 goto exit;
  }
exit:
  if(rc->rc != CMPI_RC_OK) 
    _OSBASE_TRACE(3, ("--- verify_assoc_ref() failed : %s",CMGetCharPtr(rc->msg))); 
  _OSBASE_TRACE(3,("--- verify_assoc_ref() exited"));
  return op;
}


/* ---------------------------------------------------------------------------*/
/*                           _assoc_get_inst()                                */
/* ---------------------------------------------------------------------------*/
/*     method to get CMPIInstance object of an association CMPIObjectPath     */
/*                                                                            */
/* ---------------------------------------------------------------------------*/

CMPIInstance * _assoc_get_inst( const CMPIBroker * _broker,
				const CMPIContext * ctx,
				const CMPIObjectPath * cop,
				const char * _ClassName,
				const char * _RefLeft,
				const char * _RefRight,
				CMPIStatus * rc ) {
  CMPIInstance   * ci = NULL;
  CMPIObjectPath * op = NULL;
  CMPIData         dtl;
  CMPIData         dtr;

  _OSBASE_TRACE(2,("--- _assoc_get_inst() called"));

  dtl = CMGetKey( cop, _RefLeft, rc);
  if( dtl.value.ref == NULL ) {
    CMSetStatusWithChars( _broker, rc,
			  CMPI_RC_ERR_FAILED, "CMGetKey( cop, _RefLeft, rc)" );
    _OSBASE_TRACE(2,("--- _assoc_get_inst() failed : %s",CMGetCharPtr(rc->msg)));
    goto exit;
  }

  CMSetNameSpace(dtl.value.ref,CMGetCharPtr(CMGetNameSpace(cop,rc)));
  ci = CBGetInstance(_broker, ctx, dtl.value.ref, NULL, rc);
  if( ci == NULL ) {
    if( rc->rc == CMPI_RC_ERR_FAILED ) {
      CMSetStatusWithChars( _broker, rc,
			    CMPI_RC_ERR_FAILED, "GetInstance of left reference failed.");
    }
    if( rc->rc == CMPI_RC_ERR_NOT_FOUND ) {
      CMSetStatusWithChars( _broker, rc,
			    CMPI_RC_ERR_NOT_FOUND, "Left reference not found.");
    }
    _OSBASE_TRACE(2,("--- _assoc_get_inst() failed : %s",CMGetCharPtr(rc->msg)));
    goto exit;
  }

  dtr = CMGetKey( cop, _RefRight, rc);
  if( dtr.value.ref == NULL ) {
    CMSetStatusWithChars( _broker, rc,
			  CMPI_RC_ERR_FAILED, "CMGetKey( cop, _RefRight, rc)" );
    _OSBASE_TRACE(2,("--- _assoc_get_inst() failed : %s",CMGetCharPtr(rc->msg)));
    goto exit;
  }

  CMSetNameSpace(dtr.value.ref,CMGetCharPtr(CMGetNameSpace(cop,rc)));
  ci = CBGetInstance(_broker, ctx, dtr.value.ref, NULL, rc);
  if( ci == NULL ) {
    if( rc->rc == CMPI_RC_ERR_FAILED ) {
      CMSetStatusWithChars( _broker, rc,
			    CMPI_RC_ERR_FAILED, "GetInstance of right reference failed.");
    }
    if( rc->rc == CMPI_RC_ERR_NOT_FOUND ) {
      CMSetStatusWithChars( _broker, rc,
			    CMPI_RC_ERR_NOT_FOUND, "Right reference not found.");
    }
    _OSBASE_TRACE(2,("--- _assoc_get_inst() failed : %s",CMGetCharPtr(rc->msg)));
    goto exit;
  }

  op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)),
			_ClassName, rc );
  if( CMIsNullObject(op) ) {
    CMSetStatusWithChars( _broker, rc,
			  CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
    _OSBASE_TRACE(2,("--- _assoc_get_inst() failed : %s",CMGetCharPtr(rc->msg)));
    goto exit;
  }

  ci = CMNewInstance( _broker, op, rc);
  if( CMIsNullObject(ci) ) {
    CMSetStatusWithChars( _broker, rc,
			  CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
    _OSBASE_TRACE(2,("--- _assoc_get_inst() failed : %s",CMGetCharPtr(rc->msg)));
    goto exit;
  }

  CMSetProperty( ci, _RefLeft, (CMPIValue*)&(dtl.value.ref), CMPI_ref );
  CMSetProperty( ci, _RefRight, (CMPIValue*)&(dtr.value.ref), CMPI_ref );

 exit:
  _OSBASE_TRACE(2,("--- _assoc_get_inst() exited"));
  return ci;
}



/* ---------------------------------------------------------------------------*/
/*                      _assoc_create_refs_1toN()                             */
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

int _assoc_create_refs_1toN( const CMPIBroker * _broker,
			     const CMPIContext * ctx,
			     const CMPIResult * rslt,
			     const CMPIObjectPath * ref,
			     const char * _ClassName,
			     const char * _RefLeftClass,
			     const char * _RefRightClass,
			     const char * _RefLeft,
			     const char * _RefRight,
			     int inst,
			     int associators,
			     CMPIStatus * rc) {

  CMPIInstance    * cis = NULL;
  CMPIInstance    * ci  = NULL;
  CMPIObjectPath  * op  = NULL;
  CMPIObjectPath  * rop = NULL;
  CMPIObjectPath  * cop = NULL;
  CMPIEnumeration * en  = NULL;
  CMPIData          data ;
  const char      * targetName = NULL;

  _OSBASE_TRACE(2,("--- _assoc_create_refs_1toN() called"));

  /* check if source instance does exist */
  cis = CBGetInstance(_broker, ctx, ref, NULL, rc);
  if( cis == NULL ) {
    if( rc->rc == CMPI_RC_ERR_FAILED ) {
      CMSetStatusWithChars( _broker, rc,
			    CMPI_RC_ERR_FAILED, "GetInstance of source object failed.");
    }
    if( rc->rc == CMPI_RC_ERR_NOT_FOUND ) {
      CMSetStatusWithChars( _broker, rc,
			    CMPI_RC_ERR_NOT_FOUND, "Source object not found.");
    }
    _OSBASE_TRACE(2,("--- _assoc_create_refs_1toN() failed : %s",CMGetCharPtr(rc->msg)));
    return -1;
  }

  op = _assoc_targetClass_OP(_broker,ref,_RefLeftClass,_RefRightClass,rc);
  if( op == NULL ) { 
    _OSBASE_TRACE(2,("--- _assoc_create_refs_1toN() exited : _assoc_targetClass_OP() returned with NULL"));
    goto exit; 
  }

  rop = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
			 _ClassName, rc );
  if( CMIsNullObject(rop) ) {
    CMSetStatusWithChars( _broker, rc,
			  CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
    _OSBASE_TRACE(2,("--- _assoc_create_refs_1toN() failed : %s",CMGetCharPtr(rc->msg)));
    return -1;
  }

  if( (associators == 1) && (inst == 1) ) {
    /* associators() */
    en = CBEnumInstances( _broker, ctx, op, NULL, rc);
    if( en == NULL ) {
      CMSetStatusWithChars( _broker, rc,
			    CMPI_RC_ERR_FAILED, "CBEnumInstances( _broker, ctx, op, rc)" );
      _OSBASE_TRACE(2,("--- _assoc_create_refs_1toN() failed : %s",CMGetCharPtr(rc->msg)));
      return -1;
    }

    while( CMHasNext( en, rc) ) {

      data = CMGetNext( en, rc);
      if( data.value.inst == NULL ) {
        CMSetStatusWithChars( _broker, rc,
			      CMPI_RC_ERR_FAILED, "CMGetNext( en, rc)" );
	_OSBASE_TRACE(2,("--- _assoc_create_refs_1toN() failed : %s",CMGetCharPtr(rc->msg)));
        return -1;
      }
      
      CMReturnInstance( rslt, data.value.inst );
    }
  }

  else {
    en = CBEnumInstanceNames( _broker, ctx, op, rc);
    if( en == NULL ) {
      CMSetStatusWithChars( _broker, rc,
			    CMPI_RC_ERR_FAILED, "CBEnumInstanceNames( _broker, ctx, op, rc)" );
      _OSBASE_TRACE(2,("--- _assoc_create_refs_1toN() failed : %s",CMGetCharPtr(rc->msg)));
      return -1;
    }

    while( CMHasNext( en, rc) ) {

      data = CMGetNext( en, rc);
      if( data.value.ref == NULL ) {
	CMSetStatusWithChars( _broker, rc,
			      CMPI_RC_ERR_FAILED, "CMGetNext( en, rc)" );
	_OSBASE_TRACE(2,("--- _assoc_create_refs_1toN() failed : %s",CMGetCharPtr(rc->msg)));
	return -1;
      }
      //    fprintf(stderr,"_assoc_create_refs_1toN(): %s\n",
      //    	    CMGetCharPtr(CDToString(_broker, data.value.ref, rc)));
      //    if( _debug) { fprintf(stderr,"data.value.ref - namespace : %s\n",
      //    			  CMGetCharPtr(CMGetNameSpace(data.value.ref,rc))); }
      
      if( associators == 0 ) {

	/* references() || referenceNames() */
	ci = CMNewInstance( _broker, rop, rc);
	if( CMIsNullObject(ci) ) {
	  CMSetStatusWithChars( _broker, rc,
				CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
	  _OSBASE_TRACE(2,("--- _assoc_create_refs_1toN() failed : %s",CMGetCharPtr(rc->msg)));
	  return -1;
	}
	
	targetName = _assoc_targetClass_Name(_broker,ref,_RefLeftClass,_RefRightClass,rc);
	
	if( strcmp( targetName,_RefRightClass) == 0 ) {
	  CMSetProperty( ci, _RefLeft, (CMPIValue*)&(ref), CMPI_ref );
	  CMSetProperty( ci, _RefRight, (CMPIValue*)&(data.value.ref), CMPI_ref );
	}
	else if( strcmp( targetName,_RefLeftClass) == 0 ) {
	  CMSetProperty( ci, _RefLeft, (CMPIValue*)&(data.value.ref), CMPI_ref );
	  CMSetProperty( ci, _RefRight, (CMPIValue*)&(ref), CMPI_ref );
	}
	
	//      fprintf(stderr,"_assoc_create_refs_1toN() inst: %s\n",
	//      	      CMGetCharPtr(CDToString(_broker, ci, rc)));
	
	if( inst == 0 ) {
	  cop = CMGetObjectPath(ci,rc);
	  if( cop == NULL ) {
	    CMSetStatusWithChars( _broker, rc,
				  CMPI_RC_ERR_FAILED, "CMGetObjectPath(ci,rc)" );
	  return -1;
	  }
	  CMSetNameSpace(cop,CMGetCharPtr(CMGetNameSpace(ref,rc)));
	  //	fprintf(stderr,"_assoc_create_refs_1toN() cop: %s\n",
	  //		CMGetCharPtr(CDToString(_broker, cop, rc)));
	  CMReturnObjectPath( rslt, cop );
	}
	else {
	  //	fprintf(stderr,"_assoc_create_refs_1toN() returns instance\n");
	  CMReturnInstance( rslt, ci );
	}
      }
      else {
	/* associatorNames() */
	if( inst == 0 ) { CMReturnObjectPath( rslt, data.value.ref ); }
      }
      
    }
  }

 exit:
  _OSBASE_TRACE(2,("--- _assoc_create_refs_1toN() exited"));
  return 0;
}


/* ---------------------------------------------------------------------------*/
/*                      _assoc_targetClass_Name()                             */
/* ---------------------------------------------------------------------------*/
/*                  method to get the name of the target class                */
/*                                                                            */
/* return value NULL : association is not responsible for this request        */
/* ---------------------------------------------------------------------------*/

const char * _assoc_targetClass_Name( const CMPIBroker * _broker,
				      const CMPIObjectPath * ref,
				      const char * _RefLeftClass,
				      const char * _RefRightClass,
				      CMPIStatus * rc) {

  CMPIString * sourceClass = NULL;

  _OSBASE_TRACE(4,("--- _assoc_targetClass_Name() called"));

  sourceClass = CMGetClassName(ref, rc);
  if( sourceClass == NULL ) { 
    CMSetStatusWithChars( _broker, rc,
			  CMPI_RC_ERR_FAILED, "Could not get classname of source object path." );
    _OSBASE_TRACE(4,("--- _assoc_targetClass_Name() failed : %s",CMGetCharPtr(rc->msg)));
    return NULL; 
  }
  _OSBASE_TRACE(4,("--- _assoc_targetClass_Name() : source class %s",CMGetCharPtr(sourceClass)));

  if( strcmp(CMGetCharPtr(sourceClass), _RefLeftClass ) == 0 ) {
    /* pathName = left end -> get right end */
    _OSBASE_TRACE(4,("--- _assoc_targetClass_Name() exited : %s",_RefRightClass));
    return _RefRightClass;
  }
  else if( strcmp(CMGetCharPtr(sourceClass), _RefRightClass ) == 0 ) {
    /* pathName = right end -> get left end */
    _OSBASE_TRACE(4,("--- _assoc_targetClass_Name() exited : %s",_RefLeftClass));
    return _RefLeftClass;
  }
  else if( CMClassPathIsA(_broker,ref,_RefLeftClass,rc) == 1 ) {
    /* pathName = left end -> get right end */
    _OSBASE_TRACE(4,("--- _assoc_targetClass_Name() exited : %s",_RefRightClass));
    return _RefRightClass;
  }
  else if( CMClassPathIsA(_broker,ref,_RefRightClass,rc) == 1 ) {
    /* pathName = right end -> get left end */
    _OSBASE_TRACE(4,("--- _assoc_targetClass_Name() exited : %s",_RefLeftClass));
    return _RefLeftClass;
  }

  CMSetStatusWithChars( _broker, rc,
			CMPI_RC_ERR_FAILED, "referenced Class is not covered by this Association." );
  _OSBASE_TRACE(4,("--- _assoc_targetClass_Name() exited : no target class found"));
  return NULL;
}


/* ---------------------------------------------------------------------------*/
/*                       _assoc_targetClass_OP()                              */
/* ---------------------------------------------------------------------------*/
/*        method to create an empty CMPIObjectPath of the target class        */
/*                                                                            */
/* return value NULL : association is not responsible for this request        */
/* ---------------------------------------------------------------------------*/

CMPIObjectPath * _assoc_targetClass_OP( const CMPIBroker * _broker,
					const CMPIObjectPath * ref,
					const char * _RefLeftClass,
					const char * _RefRightClass,
					CMPIStatus * rc ) {
  CMPIObjectPath  * op         = NULL;
  const char      * targetName = NULL;

  _OSBASE_TRACE(4,("--- _assoc_targetClass_OP() called"));

  targetName = _assoc_targetClass_Name(_broker,ref,_RefLeftClass,_RefRightClass,rc);

  if( targetName != NULL ) {
    op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
			  targetName, rc );
    if( CMIsNullObject(op) ) {
      CMSetStatusWithChars( _broker, rc,
			  CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(4,("--- _assoc_targetClass_OP() failed : %s",CMGetCharPtr(rc->msg)));
    }
  }

  _OSBASE_TRACE(4,("--- _assoc_targetClass_OP() exited"));
  return op;
}




/* ---------------------------------------------------------------------------*/
/*                    _assoc_check_parameter_const()                          */
/* ---------------------------------------------------------------------------*/
/*    method to check the input parameter resultClass, role and resultRole    */
/*           submitted to the methods of the association interface            */
/*                                                                            */
/* return value 0 : association is not responsible for this request           */
/* ---------------------------------------------------------------------------*/

int _assoc_check_parameter_const( const CMPIBroker * _broker,
				  const CMPIObjectPath * cop,
				  const char * _RefLeft,
				  const char * _RefRight,
				  const char * _RefLeftClass,
				  const char * _RefRightClass,
				  const char * resultClass,
				  const char * role,
				  const char * resultRole,
				  CMPIStatus * rc ) {
  CMPIObjectPath * op          = NULL;
  CMPIObjectPath * scop        = NULL;
  CMPIObjectPath * rcop        = NULL;
  CMPIString     * sourceClass = NULL;
  int              intrc       = 0;

  _OSBASE_TRACE(2,("--- _assoc_check_parameter_const() called"));
  //fprintf(stderr,"cop : %s\n",CMGetCharPtr(CDToString(_broker, cop, rc)));

  sourceClass = CMGetClassName(cop, rc);
  scop = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)),
			    CMGetCharPtr(sourceClass), rc );

  if( strcasecmp(CMGetCharPtr(sourceClass),_RefLeftClass) == 0 ||
      CMClassPathIsA(_broker,scop,_RefLeftClass,rc) == 1 ) {
    intrc = 1;
  }
  else if( strcasecmp(CMGetCharPtr(sourceClass),_RefRightClass) == 0 ||
	   CMClassPathIsA(_broker,scop,_RefRightClass,rc) == 1 ) {
    intrc = 1;
  }
  else {  goto exit; }

  if( resultClass || role || resultRole) {

    /* check if resultClass is parent or the class itslef of the target class */
    if( resultClass ) {

      if( strcasecmp(CMGetCharPtr(sourceClass),_RefLeftClass) == 0 ||
	  CMClassPathIsA(_broker,scop,_RefLeftClass,rc) == 1 ) {
	op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)),
			      _RefRightClass, rc );
      }
      else if( strcasecmp(CMGetCharPtr(sourceClass),_RefRightClass) == 0 ||
	       CMClassPathIsA(_broker,scop,_RefRightClass,rc) == 1 ) {
	op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)),
			      _RefLeftClass, rc );
      }
      else { goto exit; }

      rcop = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)),
			      resultClass, rc );

      if( CMClassPathIsA(_broker,op,resultClass,rc) == 1 ) { intrc = 1; }
      else if( ( CMClassPathIsA(_broker,rcop,_RefRightClass,rc) == 1 &&
		 strcasecmp(CMGetCharPtr(sourceClass),_RefLeftClass) == 0 ) ||
	       ( CMClassPathIsA(_broker,rcop,_RefRightClass,rc) == 1 &&
		 CMClassPathIsA(_broker,scop,_RefLeftClass,rc) == 1 ) ) {
	intrc = 1;
      }
      else if( ( CMClassPathIsA(_broker,rcop,_RefLeftClass,rc) == 1 &&
		 strcasecmp(CMGetCharPtr(sourceClass),_RefRightClass) == 0 ) ||
	       ( CMClassPathIsA(_broker,rcop,_RefLeftClass,rc) == 1 &&
		 CMClassPathIsA(_broker,scop,_RefRightClass,rc) == 1 ) ) {
	intrc = 1;
      }
      else { intrc = 0;  goto exit; }

    }

    /* check if the source object (cop) plays the Role ( specified in input
     * parameter 'role' within this association */
    if( role ) {
      if( strcasecmp(CMGetCharPtr(sourceClass),_RefLeftClass) == 0 ||
	  CMClassPathIsA(_broker,scop,_RefLeftClass,rc) == 1 ) {
	if( strcasecmp(role,_RefLeft) != 0 ) { intrc = 0; goto exit; }
	else { intrc = 1; }
      }
      else if( strcasecmp(CMGetCharPtr(sourceClass),_RefRightClass) == 0 ||
	       CMClassPathIsA(_broker,scop,_RefRightClass,rc) == 1 ) {
	if( strcasecmp(role,_RefRight) != 0 ) { intrc = 0; goto exit; }
	else { intrc = 1; }
      }
      else { goto exit; }
    }

    /* check if the target object plays the Role ( specified in input
     * parameter 'resultRole' within this association */
    if( resultRole ) {
      if( strcasecmp(CMGetCharPtr(sourceClass),_RefLeftClass) == 0 ||
	  CMClassPathIsA(_broker,scop,_RefLeftClass,rc) == 1 ) {
	if( strcasecmp(resultRole,_RefRight) != 0 ) { intrc = 0; goto exit; }
	else { intrc = 1; }
      }
      else if( strcasecmp(CMGetCharPtr(sourceClass),_RefRightClass) == 0 ||
	       CMClassPathIsA(_broker,scop,_RefRightClass,rc) == 1 ) {
	if( strcasecmp(resultRole,_RefLeft) != 0 ) { intrc = 0; goto exit; }
	else { intrc = 1; }
      }
      else { goto exit; }
    }
  }
  else { intrc = 1; }

 exit:
  if( intrc == 1 ) {
    _OSBASE_TRACE(2,("--- _assoc_check_parameter_const() exited : responsible"));
  }
  else {
    _OSBASE_TRACE(2,("--- _assoc_check_parameter_const() exited : not responsible"));
  }
  return intrc;
}


/******************************************************************************/
/* Function: stripLeadingTrailingSpaces                                       */
/* Purpose:  To strip leading and trailing spaces                             */
/* Inputs:   The string to act upon.                                          */
/* Outputs:  None                                                             */
/* Notes:    The inputted string is modified.                                 */
/******************************************************************************/
void stripLeadingTrailingSpaces(char *string)
{
   int index=0;
   
   if(string==NULL) return;
   
   /* strip leading spaces */
   if(isspace(string[index]))
   {
      while((string[index+1]!='\0') && (isspace(string[index+1])))
         index++;
      memmove(string,&string[index+1],(strlen(&string[index+1])+1) ); 
   }

   if(string==NULL) return;
   
   /* strip trailing spaces */
   index=strlen(string)-1;
   if(isspace(string[index]))
   {
      while((index-1>=0) && (isspace(string[index-1])))
         index--;
      string[index]='\0';
   }
}



/******************************************************************************/
/* Function: isDuplicateKey                                                   */
/* Purpose:  To see if the passed in key is a duplicate of an already passed  */
/*              in key.  If not then remember this new key.                   */
/* Inputs:   key which is the key to match against.                           */
/*           (*keylist) which is initially set to NULL that will              */
/*              hold the current pointer to the key list.                     */
/*           add_or_free is whether to add new keys or free the whole list,   */
/*              add_or_free=ADD_TO_LIST or FREE_LIST or FREE_LIST_AND_KEYS    */
/* Outputs:  Returns TRUE for duplicate else FALSE.  No real return value if  */
/*               add_or_free=FREE_LIST or FREE_LIST_AND_KEYS                  */   
/* Notes:    Example call:                                                    */
/*              void * keyList = NULL;                                        */
/*              dup=isDuplicateKey(string, &keyList, ADD_TO_LIST);            */
/******************************************************************************/
int isDuplicateKey(char *key, void **keylist, int add_or_free)
{
   struct keys 
   {
      char *key;
      struct keys *next;
   };
   struct keys *currentKey=NULL;
   struct keys *keyList=(struct keys *)(*keylist);

   _OSBASE_TRACE(3,("--- isDuplicateKey() called with key=%s",key));
   
   if( (add_or_free == FREE_LIST) || (add_or_free == FREE_LIST_AND_KEYS) )
   {
      _OSBASE_TRACE(4,("--- isDuplicateKey() freeing key list"));
      currentKey=keyList;
      while(currentKey!=NULL)
      {
	       keyList=currentKey->next;
	       if( (add_or_free == FREE_LIST_AND_KEYS) && (currentKey->key != NULL) )   
	          free(currentKey->key);
         free(currentKey);
         currentKey=(struct keys *)keyList;
	    }
      (*keylist)=NULL;
      _OSBASE_TRACE(3,("--- isDuplicateKey() exited"));
      return 0;
   }

   _OSBASE_TRACE(4,("--- isDuplicateKey() checking for duplicates in key list"));
   for(currentKey=keyList; currentKey!=NULL; currentKey=currentKey->next)
   {
      if (strcmp(key,currentKey->key)==0)
         break;      
   }

   if(currentKey==NULL) /* Did not find key in list */
   {
      currentKey=(struct keys*)malloc(sizeof(struct keys));
      memset(currentKey, 0, sizeof(struct keys));
      currentKey->key=key;
      currentKey->next=keyList;
      keyList=currentKey;
      (*keylist)=keyList;
      _OSBASE_TRACE(3,("--- isDuplicateKey() exited"));
      return FALSE;
   }

   _OSBASE_TRACE(4,("--- isDuplicateKey() duplicate key = %s",key));
   _OSBASE_TRACE(3,("--- isDuplicateKey() exited"));
   return TRUE;
} /* end isDuplicateKey */


/******************************************************************************/
/* Function: matchObjectPathKeys                                              */
/* Purpose:  To see if two passed in object paths contain the same keys       */
/* Inputs:   op1 = object path number 1                                       */
/*           op2 = object path number 2                                       */
/* Outputs:  Returns TRUE for match else FALSE.                               */
/******************************************************************************/
int matchObjectPathKeys(const CMPIObjectPath *op1, const CMPIObjectPath *op2)
{
   unsigned int numKeys;
   unsigned int i;
   char *key; 
   char *value1; 
   char *value2; 
   CMPIStatus rc;
   CMPIString *name=NULL;
   
   _OSBASE_TRACE(3,("--- matchObjectPathKeys() called."));
   numKeys=CMGetKeyCount(op1,&rc);
   if(numKeys != CMGetKeyCount(op2,&rc))
   {
      _OSBASE_TRACE(4,("--- matchObjectPathKeys() Object path keys did not match."));
      _OSBASE_TRACE(3,("--- matchObjectPathKeys() exited"));
      return FALSE;
   }
      
   for(i=0; i<numKeys; i++)
   {
      value1=CMGetCharPtr(CMGetKeyAt( op1, i, &name, &rc).value.string);
      key=CMGetCharPtr(name);
      value2=CMGetCharPtr(CMGetKey(op2, key, &rc).value.string);
      _OSBASE_TRACE(4,("--- matchObjectPathKeys() key=%s, value1=%s, value2=%s.",key,value1,value2));
      if(strcmp(value1,value2) != 0 )
      {
         _OSBASE_TRACE(4,("--- matchObjectPathKeys() Object path keys did not match."));
         _OSBASE_TRACE(3,("--- matchObjectPathKeys() exited"));
	       return FALSE;
	    }
   }
   
   _OSBASE_TRACE(4,("--- matchObjectPathKeys() Object path keys matched."));
   return TRUE;
} /* end matchObjectPathKeys */

