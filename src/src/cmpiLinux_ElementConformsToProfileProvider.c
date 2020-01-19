/*
 * $Id: cmpiLinux_ElementConformsToProfileProvider.c,v 1.1.1.1 2009/05/12 21:46:34 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2008, 2009
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
 *	ECTP Provider
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

#include "Linux_Common.h"
#include "cmpiLinux_Common.h"


static const CMPIBroker * _broker;

/* ---------------------------------------------------------------------------*/
/* private declarations                                                       */


/* ---------------------------------------------------------------------------*/

#define MapN 4

static char *Map[MapN][4] = {
    {"Linux_ElementConformsToProfile", "CIM_RegisteredProfile", "Linux_ComputerSystem", "HBA"},
    {"Linux_ElementConformsToProfile", "CIM_RegisteredProfile", "Linux_ComputerSystem", "HDR"},
    {"Linux_ElementConformsToProfile", "CIM_RegisteredProfile", "Linux_FCPort", "FCInitiatorPorts"},
    {"Linux_ElementConformsToProfile", "CIM_RegisteredProfile", "Linux_RecordLog", "RecordLog"}
};

static char * _ClassName     = "Linux_ElementConformsToProfile";
static char * _RefLeft       = "ConformantStandard";
static char * _RefRight      = "ManagedElement";
static char * _RefLeftClass  = NULL;
static char * _RefLeftNS      = "root/PG_InterOp";
static char * _RefRightClass = NULL;
static char * _RefRightNS    = "root/cimv2";
static char * InstanceID     = NULL;

static int ExtractName(const CMPIObjectPath * ref, int m)
{
    CMPIStatus rc    = {CMPI_RC_OK, NULL};
    CMPIData   data;


    if (m == MapN) {
        return 1;
    }

    if (CMClassPathIsA(_broker, ref, Map[m][1], &rc) == 1) {
        data = CMGetKey(ref, "InstanceID", &rc);
        _OSBASE_TRACE(1,("--- rc.rc: %d", rc.rc));
        _OSBASE_TRACE(1,("--- data.value.string: %s", CMGetCharPtr(data.value.string)));
        _OSBASE_TRACE(1,("--- InstanceID: %s", Map[m][3]));
        if (strncmp(CMGetCharPtr(data.value.string), Map[m][3], strlen(Map[m][3])) == 0) {
            _RefLeftClass = Map[m][1];
            _RefRightClass = Map[m][2];
            InstanceID = Map[m][3];
        } else {
            _RefLeftClass = NULL;
            _RefRightClass = NULL;
            InstanceID = NULL;
        }
    } else if (CMClassPathIsA(_broker, ref, Map[m][2], &rc) == 1) {
        _RefLeftClass = Map[m][1];
        _RefRightClass = Map[m][2];
        InstanceID = Map[m][3];
    } else if (CMClassPathIsA(_broker, ref, Map[m][0], &rc) == 1) {
        _RefLeftClass = Map[m][1];
        _RefRightClass = Map[m][2];
        InstanceID = Map[m][3];
    } else {
        _RefLeftClass = NULL;
        _RefRightClass = NULL;
        InstanceID = NULL;
    }

    _OSBASE_TRACE(1,("--- rc.rc: %d", rc.rc));

    return 0;
}


/* ---------------------------------------------------------------------------*/
/*                      _my_assoc_create_refs_1toN()                             */
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

static int _my_assoc_create_refs_1toN( const CMPIBroker * _broker,
                                       const CMPIContext * ctx,
                                       const CMPIResult * rslt,
                                       const CMPIObjectPath * ref,
                                       char * _RClass,
                                       char * _RefLeft,
                                       char * _RefRight,
                                       int inst,
                                       int associators,
                                       CMPIStatus * rc)
{

    CMPIInstance    * ci  = NULL;
    CMPIInstance    * cis  = NULL;
    CMPIObjectPath  * op  = NULL;
    CMPIObjectPath  * rop = NULL;
    CMPIObjectPath  * cop = NULL;
    CMPIObjectPath  * tcop = NULL;
    CMPIEnumeration * en  = NULL;
    CMPIData          data ;
    char            * targetName = NULL;
    CMPIData          data1;


    _OSBASE_TRACE(2,("--- _my_assoc_create_refs_1toN() called"));

    if (_RClass != NULL)
        _RefRightClass = _RClass;
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
        _OSBASE_TRACE(2,("--- _my_assoc_create_refs_1toN() failed : %s",CMGetCharPtr(rc->msg))); 
        return -1; 
    } 


    targetName = _assoc_targetClass_Name(_broker,ref,_RefLeftClass,_RefRightClass,rc);
    if (strncmp(targetName, _RefLeftClass, strlen(_RefLeftClass)) == 0)
        op = CMNewObjectPath( _broker, "root/PG_InterOp", targetName, rc );
    if (strncmp(targetName, _RefRightClass, strlen(_RefRightClass)) == 0)
        op = CMNewObjectPath( _broker, "root/cimv2", targetName, rc );
    if( op == NULL ) { goto exit; }

    rop = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                           _ClassName, rc );
    if( CMIsNullObject(rop) ) {
        CMSetStatusWithChars( _broker, rc,
                              CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
        return -1;
    }

    if( (associators == 1) && (inst == 1) ) {
        /* associators() */
        en = CBEnumInstances( _broker, ctx, op, NULL, rc);
        if( en == NULL ) {
            CMSetStatusWithChars( _broker, rc,
                                  CMPI_RC_ERR_FAILED, "CBEnumInstances( _broker, ctx, op, rc)" );
            return -1;
        }

        while( CMHasNext( en, rc) ) {

            data = CMGetNext( en, rc);
            if( data.value.inst == NULL ) {
                CMSetStatusWithChars( _broker, rc,
                                      CMPI_RC_ERR_FAILED, "CMGetNext( en, rc)" );
                return -1;
            }


            tcop = CMGetObjectPath(data.value.inst,rc);
            _OSBASE_TRACE(1,("--- Namespace: %s",CMGetCharPtr(CMGetNameSpace(tcop,rc))));
            _OSBASE_TRACE(1,("--- data.value.ref: %s",CMGetCharPtr(CDToString(_broker, tcop, rc))));
            if( tcop == NULL ) {
                CMSetStatusWithChars( _broker, rc,
                                      CMPI_RC_ERR_FAILED, "CMGetObjectPath(ci,rc)" );
                return -1;
            }
            if (CMClassPathIsA(_broker, tcop, "CIM_RegisteredProfile", rc) == 1) {
                data1 = CMGetKey(tcop, "InstanceID", rc);
                _OSBASE_TRACE(1,("--- rc->rc: %d", rc->rc));
                _OSBASE_TRACE(1,("--- data1.value.string: %s", CMGetCharPtr(data1.value.string)));
                _OSBASE_TRACE(1,("--- InstanceID: %s", InstanceID));
                if (strncmp(CMGetCharPtr(data1.value.string), InstanceID, strlen(InstanceID)) != 0)
                    continue;
            }
            _OSBASE_TRACE(1,("--- rc->rc: %d", rc->rc));

            CMReturnInstance( rslt, data.value.inst );
        }
    }

    else {
        en = CBEnumInstanceNames( _broker, ctx, op, rc);
        if( en == NULL ) {
            CMSetStatusWithChars( _broker, rc,
                                  CMPI_RC_ERR_FAILED, "CBEnumInstanceNames( _broker, ctx, op, rc)" );
            return -1;
        }

        while( CMHasNext( en, rc) ) {

            data = CMGetNext( en, rc);
            if( data.value.ref == NULL ) {
                CMSetStatusWithChars( _broker, rc,
                                      CMPI_RC_ERR_FAILED, "CMGetNext( en, rc)" );
                return -1;
            }

            if (strlen(CMGetCharPtr(CMGetNameSpace(data.value.ref,rc))) == 0)
                CMSetNameSpace(data.value.ref, "root/PG_InterOp");
      
            if( associators == 0 ) {

                /* references() || referenceNames() */
                ci = CMNewInstance( _broker, rop, rc);
                if( CMIsNullObject(ci) ) {
                    CMSetStatusWithChars( _broker, rc,
                                          CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
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
        
                if( inst == 0 ) {
                    cop = CMGetObjectPath(ci,rc);
                    if( cop == NULL ) {
                        CMSetStatusWithChars( _broker, rc,
                                              CMPI_RC_ERR_FAILED, "CMGetObjectPath(ci,rc)" );
                        return -1;
                    }

                    if (CMClassPathIsA(_broker, data.value.ref, "CIM_RegisteredProfile", rc) == 1) {
                        data1 = CMGetKey(data.value.ref, "InstanceID", rc);
                        _OSBASE_TRACE(1,("--- rc->rc: %d", rc->rc));
                        _OSBASE_TRACE(1,("--- data1.value.string: %s", CMGetCharPtr(data1.value.string)));
                        _OSBASE_TRACE(1,("--- InstanceID: %s", InstanceID));
                        if (strncmp(CMGetCharPtr(data1.value.string), InstanceID, strlen(InstanceID)) != 0)
                            continue;
                    }
                    _OSBASE_TRACE(1,("--- rc->rc: %d", rc->rc));

                    CMReturnObjectPath( rslt, cop );
                }
                else {
                    if (CMClassPathIsA(_broker, data.value.ref, "CIM_RegisteredProfile", rc) == 1) {
                        data1 = CMGetKey(data.value.ref, "InstanceID", rc);
                        _OSBASE_TRACE(1,("--- rc->rc: %d", rc->rc));
                        _OSBASE_TRACE(1,("--- data1.value.string: %s", CMGetCharPtr(data1.value.string)));
                        _OSBASE_TRACE(1,("--- InstanceID: %s", InstanceID));
                        if (strncmp(CMGetCharPtr(data1.value.string), InstanceID, strlen(InstanceID)) != 0)
                            continue;
                    }
                    _OSBASE_TRACE(1,("--- rc->rc: %d", rc->rc));

                    CMReturnInstance( rslt, ci );
                }
            }
            else {
                /* associatorNames() */
                if( inst == 0 ) {
                    _OSBASE_TRACE(1,("--- Namespace: %s",CMGetCharPtr(CMGetNameSpace(data.value.ref,rc))));
                    _OSBASE_TRACE(1,("--- data.value.ref: %s",CMGetCharPtr(CDToString(_broker, data.value.ref, rc))));
                    if (CMClassPathIsA(_broker, data.value.ref, "CIM_RegisteredProfile", rc) == 1) {
                        data1 = CMGetKey(data.value.ref, "InstanceID", rc);
                        _OSBASE_TRACE(1,("--- rc->rc: %d", rc->rc));
                        _OSBASE_TRACE(1,("--- data1.value.string: %s", CMGetCharPtr(data1.value.string)));
                        _OSBASE_TRACE(1,("--- InstanceID: %s", InstanceID));
                        if (strncmp(CMGetCharPtr(data1.value.string), InstanceID, strlen(InstanceID)) != 0)
                            continue;
                    }
                    _OSBASE_TRACE(1,("--- rc->rc: %d", rc->rc));

                    CMReturnObjectPath( rslt, data.value.ref ); 
                }
            }
      
        }
    }

 exit:
    return 0;
}


/* ---------------------------------------------------------------------------*/
/*                      _my_assoc_create_inst_1toN()                             */
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
/* found CMPIInstance / CMPIObjectPath object to the Object Manager (OM)      */
/* ---------------------------------------------------------------------------*/

static int _my_assoc_create_inst_1toN( const CMPIBroker * _broker,
                                       const CMPIContext * ctx,
                                       const CMPIResult * rslt,
                                       const CMPIObjectPath * cop,
                                       char * _ClassName,
                                       char * _RefLeftClass,
                                       char * _RefRightClass,
                                       char * _RefLeft,
                                       char * _RefRight,
                                       int left,
                                       int inst,
                                       CMPIStatus * rc)
{
    CMPIObjectPath  * op  = NULL;
    CMPIEnumeration * en  = NULL;
    CMPIData          data ;
    int               arc = -1;
    CMPIData          data1;


    _OSBASE_TRACE(2,("--- _my_assoc_create_inst_1toN() called"));

    if( left == 0 ) {
        op = CMNewObjectPath( _broker, "root/cimv2", _RefRightClass, rc );
    }
    else { /* left == 1 */
        op = CMNewObjectPath( _broker, "root/PG_InterOp", _RefLeftClass, rc );
       
    }
    if( CMIsNullObject(op) ) {
        CMSetStatusWithChars( _broker, rc,
                              CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
        return -1;
    }

    en = CBEnumInstanceNames( _broker, ctx, op, rc);
    if( en == NULL ) {
        CMSetStatusWithChars( _broker, rc,
                              CMPI_RC_ERR_FAILED, "CBEnumInstanceNames( _broker, ctx, op, rc)" );
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
            return -1;
        }

        if (left) {
            CMSetNameSpace(data.value.ref, "root/PG_InterOp");
            if (CMClassPathIsA(_broker, data.value.ref, "CIM_RegisteredProfile", rc) == 1) {
                data1 = CMGetKey(data.value.ref, "InstanceID", rc);
                _OSBASE_TRACE(1,("--- rc->rc: %d", rc->rc));
                _OSBASE_TRACE(1,("--- data1.value.string: %s", CMGetCharPtr(data1.value.string)));
                _OSBASE_TRACE(1,("--- InstanceID: %s", InstanceID));
                if (strncmp(CMGetCharPtr(data1.value.string), InstanceID, strlen(InstanceID)) != 0) 
                    continue;
            }
            _OSBASE_TRACE(1,("--- rc->rc: %d", rc->rc));
        }

        arc = _my_assoc_create_refs_1toN( _broker,ctx,rslt,data.value.ref,
                                          NULL, _RefLeft,_RefRight,inst,0,rc);
        break;
    }

    return arc;
}


/* ---------------------------------------------------------------------------*/
/*                           _my_assoc_get_inst()                                */
/* ---------------------------------------------------------------------------*/
/*     method to get CMPIInstance object of an association CMPIObjectPath     */
/*                                                                            */
/* ---------------------------------------------------------------------------*/

static CMPIInstance * _my_assoc_get_inst( const CMPIBroker * _broker,
                                          const CMPIContext * ctx,
                                          const CMPIObjectPath * cop,
                                          char * _ClassName,
                                          char * _RefLeft,
                                          char * _RefRight,
                                          CMPIStatus * rc ) {
    CMPIInstance   * ci = NULL;
    CMPIObjectPath * op = NULL;
    CMPIData         dtl;
    CMPIData         dtr;
    int              m = 0;

    CMPIObjectPath *opl = NULL;  // Save left ref
    CMPIObjectPath *opr = NULL;  // Save right ref

    _OSBASE_TRACE(2,("--- _my_assoc_get_inst() called"));

    dtl = CMGetKey( cop, _RefLeft, rc);
    if( dtl.value.ref == NULL ) {
        CMSetStatusWithChars( _broker, rc,
                              CMPI_RC_ERR_FAILED, "CMGetKey( cop, _RefLeft, rc)" );
        goto exit;
    }

    if ( dtl.type == CMPI_ref )
    {
        opl = dtl.value.ref;
    }
    else if ( dtl.type == CMPI_string )
    { // if string, create a objpath and set its property
        opl = CMNewObjectPath(_broker, 0, 0, rc);
        if( CMIsNullObject(opl) )
        {
            CMSetStatusWithChars( _broker, rc,
                                  CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
            goto exit;
        }
    }
    else
    { // fail, if input is neither ref or string
        CMSetStatusWithChars( _broker, rc,
                              CMPI_RC_ERR_NOT_FOUND, "Left reference not found.");
        goto exit;
    }

    CMSetNameSpace(opl, "root/PG_InterOp");
    ci = CBGetInstance(_broker, ctx, opl, NULL, rc);

    if( ci == NULL ) {
        if( rc->rc == CMPI_RC_ERR_FAILED ) {
            CMSetStatusWithChars( _broker, rc,
                                  CMPI_RC_ERR_FAILED, "GetInstance of left reference failed.");
        }
        if( rc->rc == CMPI_RC_ERR_NOT_FOUND ) { 
            CMSetStatusWithChars( _broker, rc, 
                                  CMPI_RC_ERR_NOT_FOUND, "Left reference not found."); 
        } 
        _OSBASE_TRACE(2,("--- _my_assoc_get_inst() failed : %s",CMGetCharPtr(rc->msg))); 
        goto exit;
    }

    dtr = CMGetKey( cop, _RefRight, rc);
    if( dtr.value.ref == NULL ) {
        CMSetStatusWithChars( _broker, rc,
                              CMPI_RC_ERR_FAILED, "CMGetKey( cop, _RefRight, rc)" );
        goto exit;
    }

    if ( dtr.type == CMPI_ref )
    {
        opr = dtr.value.ref;
    }
    else if ( dtr.type == CMPI_string )
    { // if string, create a objpath and set its property
        opr = CMNewObjectPath(_broker, 0, 0, rc);
        if( CMIsNullObject(opr) )
        {
            CMSetStatusWithChars( _broker, rc,
                                  CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
            goto exit;
        }
    }
    else
    { // fail, if input is neither ref or string
        CMSetStatusWithChars( _broker, rc,
                              CMPI_RC_ERR_NOT_FOUND, "Right reference not found.");
        goto exit;
    }

    CMSetNameSpace(opr, "root/cimv2");
    ci = CBGetInstance(_broker, ctx, opr, NULL, rc);

    if( ci == NULL ) {
        if( rc->rc == CMPI_RC_ERR_FAILED ) { 
            CMSetStatusWithChars( _broker, rc,
                                  CMPI_RC_ERR_FAILED, "GetInstance of right reference failed.");
        }
        if( rc->rc == CMPI_RC_ERR_NOT_FOUND ) { 
            CMSetStatusWithChars( _broker, rc, 
                                  CMPI_RC_ERR_NOT_FOUND, "Right reference not found."); 
        } 
        _OSBASE_TRACE(2,("--- _my_assoc_get_inst() failed : %s",CMGetCharPtr(rc->msg)));
        goto exit;
    }


    while (m < MapN) {
        ExtractName(opl, m);
        if(_RefLeftClass == NULL) {
            m++;
            if (m == MapN)
                return NULL;
            continue;
        }

        ExtractName(opr, m);
        if(_RefLeftClass == NULL) {
            m++;
            if (m == MapN)
                return NULL;
            continue;
        }

        op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)),
                              _ClassName, rc );
        if( CMIsNullObject(op) ) {
            CMSetStatusWithChars( _broker, rc,
                                  CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
            goto exit;
        }

        ci = CMNewInstance( _broker, op, rc);
        if( CMIsNullObject(ci) ) {
            CMSetStatusWithChars( _broker, rc,
                                  CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
            goto exit;
        }

        CMSetProperty( ci, _RefLeft, (CMPIValue*)&(opl), CMPI_ref );
        CMSetProperty( ci, _RefRight, (CMPIValue*)&(opr), CMPI_ref );
        break;
    }

 exit:
    return ci;
}


/* ---------------------------------------------------------------------------*/


/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


CMPIStatus Linux_ElementConformsToProfileProviderCleanup( CMPIInstanceMI * mi, 
                                                          const CMPIContext * ctx, CMPIBoolean trm)
{ 
    _OSBASE_TRACE(1,("--- %s CMPI Cleanup() called",_ClassName));
    _OSBASE_TRACE(1,("--- %s CMPI Cleanup() exited",_ClassName));
    CMReturn(CMPI_RC_OK);
}

CMPIStatus Linux_ElementConformsToProfileProviderEnumInstanceNames( CMPIInstanceMI * mi, 
                                                                    const CMPIContext * ctx, 
                                                                    const CMPIResult * rslt, 
                                                                    const CMPIObjectPath * ref)
{ 
    CMPIStatus rc    = {CMPI_RC_OK, NULL};
    CMPIObjectPath  * cop = NULL;
    int        n = 0;
    int        refrc = 0;
  

    _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() called","Linux_ElementConformsToProfile"));
    
    while (ExtractName(ref, n) != 1) {
        n++;
        if (InstanceID == NULL)
            continue;

        refrc = _my_assoc_create_inst_1toN( _broker,ctx,rslt,ref,
                                            _ClassName,_RefLeftClass,_RefRightClass,
                                            _RefLeft,_RefRight,
                                            1,0,&rc);
        if( refrc != 0 ) { 
            if( rc.msg != NULL ) {
                _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            }
            else {
                _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed",_ClassName));
            }
            CMReturn(CMPI_RC_ERR_FAILED); 
        }

        CMReturnObjectPath( rslt, cop );
    }

    _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() exited",_ClassName));
    return rc;
}

CMPIStatus Linux_ElementConformsToProfileProviderEnumInstances( CMPIInstanceMI * mi, 
                                                                const CMPIContext * ctx, 
                                                                const CMPIResult * rslt, 
                                                                const CMPIObjectPath * ref, 
                                                                const char ** properties)
{ 
    CMPIStatus rc    = {CMPI_RC_OK, NULL};
    CMPIInstance    * ci  = NULL;
    int        n = 0;
    int        refrc = 0;


    _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() called", "Linux_ElementConformsToProfile"));

    while (ExtractName(ref, n) != 1) {
        n++;
        if (InstanceID == NULL)
            continue;

        refrc = _my_assoc_create_inst_1toN( _broker,ctx,rslt,ref,
                                            _ClassName,_RefLeftClass,_RefRightClass,
                                            _RefLeft,_RefRight,
                                            1,1,&rc);
        if( refrc != 0 ) {  
            if( rc.msg != NULL ) {
                _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            }
            else {
                _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed",_ClassName));
            }
            CMReturn(CMPI_RC_ERR_FAILED); 
        }

        CMReturnInstance( rslt, ci );
    }

    _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() exited",_ClassName));
    return rc;
}

CMPIStatus Linux_ElementConformsToProfileProviderGetInstance( CMPIInstanceMI * mi, 
                                                              const CMPIContext * ctx, 
                                                              const CMPIResult * rslt, 
                                                              const CMPIObjectPath * cop, 
                                                              const char ** properties)
{
    CMPIInstance * ci = NULL;
    CMPIStatus     rc = {CMPI_RC_OK, NULL};


    _OSBASE_TRACE(1,("--- %s CMPI GetInstance() called",_ClassName));
  
    ci = _my_assoc_get_inst( _broker,ctx,cop,_ClassName,_RefLeft,_RefRight,&rc);

    if( ci == NULL ) { 
        if( rc.msg != NULL ) {
            _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
        }
        else {
            _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed",_ClassName));
        }
        CMReturn(CMPI_RC_ERR_NOT_FOUND); 
    }


    CMReturnInstance( rslt, ci );
    CMReturnDone(rslt);
    _OSBASE_TRACE(1,("--- %s CMPI GetInstance() exited",_ClassName));
    return rc;
}

CMPIStatus Linux_ElementConformsToProfileProviderCreateInstance( CMPIInstanceMI * mi, 
                                                                 const CMPIContext * ctx, 
                                                                 const CMPIResult * rslt, 
                                                                 const CMPIObjectPath * cop, 
                                                                 const CMPIInstance * ci)
{
    CMPIStatus rc = {CMPI_RC_OK, NULL};

    _OSBASE_TRACE(1,("--- %s CMPI CreateInstance() called",_ClassName));

    CMSetStatusWithChars( _broker, &rc, 
                          CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" ); 

    _OSBASE_TRACE(1,("--- %s CMPI CreateInstance() exited",_ClassName));
    return rc;
}

CMPIStatus Linux_ElementConformsToProfileProviderModifyInstance( CMPIInstanceMI * mi, 
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

CMPIStatus Linux_ElementConformsToProfileProviderDeleteInstance( CMPIInstanceMI * mi, 
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

CMPIStatus Linux_ElementConformsToProfileProviderExecQuery( CMPIInstanceMI * mi, 
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


CMPIStatus Linux_ElementConformsToProfileProviderAssociationCleanup( CMPIAssociationMI * mi,
                                                                     const CMPIContext * ctx,
                                                                     CMPIBoolean trm)
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
 * Linux_HBAElementConformsToProfile: 
 *    < role >   -> < class >
 *    ConformantStandard -> Linux_FCRegisteredProfile
 *    ManagedElement -> Linux_ComputerSystem
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

CMPIStatus Linux_ElementConformsToProfileProviderAssociators( CMPIAssociationMI * mi,
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
    CMPIObjectPath * op    = NULL;
    CMPIObjectPath * rcop  = NULL;
    char * resultNS = NULL;
    char * sourceNS = NULL;
    int              refrc = 0;
    int              n = 0;

    // get result namespace form the source namespace
    sourceNS = CMGetCharPtr(CMGetNameSpace(cop, &rc));
    if(strcasecmp(sourceNS, _RefLeftNS) == 0) {
        resultNS = _RefRightNS;
    }
    else if(strcasecmp(sourceNS, _RefRightNS) == 0) {
        resultNS = _RefLeftNS;
    }
    else {
        goto exit;
    }
    
    _OSBASE_TRACE(1,("--- %s CMPI Associators() called",_ClassName));
    
    if( assocClass ) {
        op = CMNewObjectPath( _broker, sourceNS, _ClassName, &rc );
    }

    while (ExtractName(cop, n) != 1) {
        n++;
        if (InstanceID == NULL)
            continue;


        if( ( assocClass==NULL ) || ( CMClassPathIsA(_broker,op,assocClass,&rc) == 1 ) ) {

            if( _assoc_check_parameter_const( _broker,cop,_RefLeft,_RefRight,
                                              _RefLeftClass,_RefRightClass,
                                              resultClass,role,resultRole, 
                                              &rc ) == 0 ) { goto exit; }

            _OSBASE_TRACE(1,("--- %s _assoc_check_paramete_const opassesd",_ClassName));

            if( resultClass ) {
                rcop = CMNewObjectPath( _broker, resultNS, resultClass,&rc );
                if(  CMClassPathIsA(_broker,rcop,_RefRightClass,&rc) == 1  ) {
                    refrc = _my_assoc_create_refs_1toN(_broker, ctx, rslt, cop, 
                                                       (char *)resultClass,
                                                       _RefLeft,_RefRight,
                                                       1, 1, &rc);
                    if( refrc != 0 ) { 
                        if( rc.msg != NULL ) {
                            _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
                        }
                        else {
                            _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() failed",_ClassName));
                        }
                        CMReturn(CMPI_RC_ERR_FAILED); 
                    }
                    goto exit;
                }
            }

            refrc = _my_assoc_create_refs_1toN(_broker, ctx, rslt, cop, 
                                               NULL, _RefLeft,_RefRight,
                                               1, 1, &rc);
            if( refrc != 0 ) { 
                if( rc.msg != NULL )  {
                    _OSBASE_TRACE(1,("--- %s CMPI Associators() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
                }
                else {
                    _OSBASE_TRACE(1,("--- %s CMPI Associators() failed",_ClassName));
                }
                CMReturn(CMPI_RC_ERR_FAILED); 
            }
        }
    }


 exit:
    CMReturnDone( rslt );
    _OSBASE_TRACE(1,("--- %s CMPI Associators() exited",_ClassName));
    CMReturn(CMPI_RC_OK);
}

CMPIStatus Linux_ElementConformsToProfileProviderAssociatorNames( CMPIAssociationMI * mi,
                                                                  const CMPIContext * ctx,
                                                                  const CMPIResult * rslt,
                                                                  const CMPIObjectPath * cop,
                                                                  const char * assocClass,
                                                                  const char * resultClass,
                                                                  const char * role,
                                                                  const char * resultRole)
{
    CMPIStatus       rc    = {CMPI_RC_OK, NULL};
    CMPIObjectPath * op    = NULL;
    CMPIObjectPath * rcop  = NULL;
    char * resultNS = NULL;
    char * sourceNS = NULL;
    int              refrc = 0;
    int              n = 0;

    
    // get result namespace form the source namespace
    sourceNS = CMGetCharPtr(CMGetNameSpace(cop, &rc));
    if(strcasecmp(sourceNS, _RefLeftNS) == 0) {
        resultNS = _RefRightNS;
    }
    else if(strcasecmp(sourceNS, _RefRightNS) == 0) {
        resultNS = _RefLeftNS;
    }
    else {
        goto exit;
    }

    _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() called",_ClassName));
    
    if( assocClass ) {
        op = CMNewObjectPath( _broker, sourceNS, _ClassName, &rc );
    }

    while (ExtractName(cop, n) != 1) {
        n++;
        if (InstanceID == NULL)
            continue;

        if( ( assocClass==NULL ) || ( CMClassPathIsA(_broker,op,assocClass,&rc) == 1 ) ) {

            if( _assoc_check_parameter_const( _broker,cop,_RefLeft,_RefRight,
                                              _RefLeftClass,_RefRightClass,
                                              resultClass,role,resultRole, 
                                              &rc ) == 0 ) { goto exit; }
            _OSBASE_TRACE(1,("--- %s _assoc_check_paramete_const opassesd",_ClassName));
            if( resultClass ) {
                rcop = CMNewObjectPath( _broker, resultNS, resultClass, &rc );
                if(  CMClassPathIsA(_broker,rcop,_RefRightClass,&rc) == 1  ) {
                    refrc = _my_assoc_create_refs_1toN(_broker, ctx, rslt, cop, 
                                                       (char *)resultClass,
                                                       _RefLeft,_RefRight,
                                                       0, 1, &rc);
                    if( refrc != 0 ) { 
                        if( rc.msg != NULL ) {
                            _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
                        }
                        else {
                            _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() failed",_ClassName));
                        }
                        CMReturn(CMPI_RC_ERR_FAILED); 
                    }

                    goto exit;
                }
            }

            refrc = _my_assoc_create_refs_1toN(_broker, ctx, rslt, cop, 
                                               NULL, _RefLeft,_RefRight,
                                               0, 1, &rc);    
            if( refrc != 0 ) { 
                if( rc.msg != NULL ) {
                    _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
                }
                else {
                    _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() failed",_ClassName));
                }
                CMReturn(CMPI_RC_ERR_FAILED); 
            }
        }
    }

 exit:
    CMReturnDone( rslt );
    _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() exited",_ClassName));
    CMReturn(CMPI_RC_OK);
}


/*
 * specification of references() and referenceNames()
 *
 * These methods return CMPIInstance ( in the case of references() ) or 
 * CMPIObjectPath ( in the case of referenceNames() ) object(s) of th 
 * association itself.
 */

CMPIStatus Linux_ElementConformsToProfileProviderReferences( CMPIAssociationMI * mi,
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
    int              n = 0;

    _OSBASE_TRACE(1,("--- %s CMPI References() called",_ClassName));
    
    if( assocClass ) {
        op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
                              _ClassName, &rc );
    }

    while (ExtractName(cop, n) != 1) {
        n++;
        if (InstanceID == NULL)
            continue;

        if( ( assocClass==NULL ) || ( CMClassPathIsA(_broker,op,assocClass,&rc) == 1 ) ) {
  
            if( _assoc_check_parameter_const( _broker,cop,_RefLeft,_RefRight,
                                              _RefLeftClass,_RefRightClass,
                                              NULL,role,NULL, 
                                              &rc ) == 0 ) { goto exit; }

            refrc = _my_assoc_create_refs_1toN(_broker, ctx, rslt, cop, 
                                               NULL, _RefLeft,_RefRight,
                                               1, 0, &rc);
            if( refrc != 0 ) { 
                if( rc.msg != NULL ) {
                    _OSBASE_TRACE(1,("--- %s CMPI References() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
                }
                else {
                    _OSBASE_TRACE(1,("--- %s CMPI References() failed",_ClassName));
                }
                CMReturn(CMPI_RC_ERR_FAILED); 
            }
        }
    }

 exit:
    CMReturnDone( rslt );
    _OSBASE_TRACE(1,("--- %s CMPI References() exited",_ClassName));
    CMReturn(CMPI_RC_OK);
}

CMPIStatus Linux_ElementConformsToProfileProviderReferenceNames( CMPIAssociationMI * mi,
                                                                 const CMPIContext * ctx,
                                                                 const CMPIResult * rslt,
                                                                 const CMPIObjectPath * cop,
                                                                 const char * assocClass,
                                                                 const char * role)
{
    CMPIStatus       rc    = {CMPI_RC_OK, NULL};
    CMPIObjectPath * op    = NULL;
    int              refrc = 0;
    int              n = 0;

    _OSBASE_TRACE(1,("--- %s CMPI ReferenceNames() called",_ClassName));
    
    if( assocClass ) {
        op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
                              _ClassName, &rc );
    }

    while (ExtractName(cop, n) != 1) {
        n++;
        if (InstanceID == NULL)
            continue;

        if( ( assocClass==NULL ) || ( CMClassPathIsA(_broker,op,assocClass,&rc) == 1 ) ) {
            if( _assoc_check_parameter_const( _broker,cop,_RefLeft,_RefRight,
                                              _RefLeftClass,_RefRightClass,
                                              NULL,role,NULL, 
                                              &rc ) == 0 ) { goto exit; }

            refrc = _my_assoc_create_refs_1toN(_broker, ctx, rslt, cop, 
                                               NULL, _RefLeft,_RefRight,
                                               0, 0, &rc);
            if( refrc != 0 ) { 
                if( rc.msg != NULL ) {
                    _OSBASE_TRACE(1,("--- %s CMPI ReferenceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
                }
                else {
                    _OSBASE_TRACE(1,("--- %s CMPI ReferenceNames() failed",_ClassName));
                }
                CMReturn(CMPI_RC_ERR_FAILED); 
            }
        }
    }

 exit:
    CMReturnDone( rslt );
    _OSBASE_TRACE(1,("--- %s CMPI ReferenceNames() exited",_ClassName));
    CMReturn(CMPI_RC_OK);
}


/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( Linux_ElementConformsToProfileProvider, 
                  Linux_ElementConformsToProfileProvider, 
                  _broker, 
                  CMNoHook)

CMAssociationMIStub( Linux_ElementConformsToProfileProvider, 
                     Linux_ElementConformsToProfileProvider, 
                     _broker, 
                     CMNoHook)


/* ---------------------------------------------------------------------------*/
/*               end of cmpiLinux_ElementConformsToProfileProvider                      */
/* ---------------------------------------------------------------------------*/

     
     
     
     
    
    
    
