/*
 * $Id: cmpiOSBase_ComputerSystemProvider.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2002, 2008, 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Author:       Heidi Neumann <heidineu@de.ibm.com>
 *
 * Contributors: IBM 
 *
 * Description:
 *	Base computer system provider
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"
#include "Linux_Common.h"
#include "cmpiLinux_Common.h"
#include "cmpiOSBase_ComputerSystem.h"

static const CMPIBroker * _broker;

/* ---------------------------------------------------------------------------*/
/* private declarations                                                       */


/* ---------------------------------------------------------------------------*/


/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


CMPIStatus OSBase_ComputerSystemProviderCleanup( CMPIInstanceMI * mi, 
                                                 const CMPIContext * ctx, CMPIBoolean trm) { 
  _OSBASE_TRACE(1,("--%s CMPI Cleanup() called", _ClassName));
  _OSBASE_TRACE(1,("--%s CMPI Cleanup() exited", _ClassName));
  CMReturn(CMPI_RC_OK);
}

CMPIStatus OSBase_ComputerSystemProviderEnumInstanceNames( CMPIInstanceMI * mi, 
                                                           const CMPIContext * ctx, 
                                                           const CMPIResult * rslt, 
                                                           const CMPIObjectPath * ref) {
  CMPIObjectPath * op = NULL;
  CMPIStatus       rc = {CMPI_RC_OK, NULL};
  
  _OSBASE_TRACE(1,("--%s CMPI EnumInstanceNames() called", _ClassName));

  op = _makePath_ComputerSystem( _broker, ctx, ref, &rc );

  if( op == NULL ) {
    if( rc.msg != NULL )
      {_OSBASE_TRACE(1,("--%s CMPI EnumInstanceNames() failed : %s", _ClassName,CMGetCharPtr(rc.msg)));}
    else
      {_OSBASE_TRACE(1,("--%s CMPI EnumInstanceNames() failed", _ClassName));}
    return rc;
  }

  CMReturnObjectPath( rslt, op );
  CMReturnDone( rslt );
  return rc;
}

CMPIStatus OSBase_ComputerSystemProviderEnumInstances( CMPIInstanceMI * mi, 
                                                       const CMPIContext * ctx, 
                                                       const CMPIResult * rslt, 
                                                       const CMPIObjectPath * ref, 
                                                       const char ** properties) { 
  CMPIInstance * ci = NULL;
  CMPIStatus     rc = {CMPI_RC_OK, NULL};

  _OSBASE_TRACE(1,("--%s CMPI EnumInstances() called", _ClassName));

  ci = _makeInst_ComputerSystem( _broker, ctx, ref, properties, &rc );

  if( ci == NULL ) {
    if( rc.msg != NULL )
      {_OSBASE_TRACE(1,("--%s CMPI EnumInstances() failed : %s", _ClassName,CMGetCharPtr(rc.msg)));}
    else
      {_OSBASE_TRACE(1,("--%s CMPI EnumInstances() failed", _ClassName));}

    return rc;
  }

  CMReturnInstance( rslt, ci );
  CMReturnDone( rslt );
  _OSBASE_TRACE(1,("--%s CMPI EnumInstances() exited", _ClassName));
  return rc;
}

CMPIStatus OSBase_ComputerSystemProviderGetInstance( CMPIInstanceMI * mi, 
                                                     const CMPIContext * ctx, 
                                                     const CMPIResult * rslt, 
                                                     const CMPIObjectPath * cop, 
                                                     const char ** properties) {
  CMPIInstance * ci = NULL;
  CMPIStatus     rc = {CMPI_RC_OK, NULL};

  _OSBASE_TRACE(1,("--%s CMPI GetInstance() called", _ClassName));


  if( rc.rc != CMPI_RC_OK ) { 
    _OSBASE_TRACE(1,("--%s CMPI GetInstance() failed : %s",_ClassName, CMGetCharPtr(rc.msg)));
    return rc; }

  ci = _makeInst_ComputerSystem( _broker, ctx, cop, properties, &rc );

  if( ci == NULL ) {
    if( rc.msg != NULL )
      {_OSBASE_TRACE(1,("--%s CMPI GetInstance() failed : %s", _ClassName,CMGetCharPtr(rc.msg)));}
    else
      {_OSBASE_TRACE(1,("--%s CMPI GetInstance() failed", _ClassName));}
    return rc;
  }

  CMReturnInstance( rslt, ci );
  CMReturnDone(rslt);
  _OSBASE_TRACE(1,("--%s CMPI GetInstance() exited", _ClassName));
  return rc;
}

CMPIStatus OSBase_ComputerSystemProviderCreateInstance( CMPIInstanceMI * mi, 
                                                        const CMPIContext * ctx, 
                                                        const CMPIResult * rslt, 
                                                        const CMPIObjectPath * cop, 
                                                        const CMPIInstance * ci) {
  CMPIStatus rc = {CMPI_RC_OK, NULL};
 
  _OSBASE_TRACE(1,("--%s CMPI CreateInstance() called", _ClassName));

  CMSetStatusWithChars( _broker, &rc, 
                        CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" ); 
  _OSBASE_TRACE(1,("--%s CMPI CreateInstance() exited", _ClassName));
  return rc;
}

CMPIStatus OSBase_ComputerSystemProviderModifyInstance( CMPIInstanceMI * mi, 
                                                        const CMPIContext * ctx, 
                                                        const CMPIResult * rslt, 
                                                        const CMPIObjectPath * cop,
                                                        const CMPIInstance * ci, 
                                                        const char **properties) {
  CMPIStatus rc = {CMPI_RC_OK, NULL};

  _OSBASE_TRACE(1,("--%s CMPI ModifyInstance() called", _ClassName));
  CMSetStatusWithChars( _broker, &rc, 
                        CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" ); 
  _OSBASE_TRACE(1,("--%s CMPI ModifyInstance() exited", _ClassName)); 
  return rc;
}

CMPIStatus OSBase_ComputerSystemProviderDeleteInstance( CMPIInstanceMI * mi, 
                                                        const CMPIContext * ctx, 
                                                        const CMPIResult * rslt, 
                                                        const CMPIObjectPath * cop) {
  CMPIStatus rc = {CMPI_RC_OK, NULL}; 

  _OSBASE_TRACE(1,("--%s CMPI DeleteInstance() called", _ClassName));
  CMSetStatusWithChars( _broker, &rc, 
                        CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" ); 
  _OSBASE_TRACE(1,("--%s CMPI DeleteInstance() exited", _ClassName));
  return rc;
}

CMPIStatus OSBase_ComputerSystemProviderExecQuery( CMPIInstanceMI * mi, 
                                                   const CMPIContext * ctx, 
                                                   const CMPIResult * rslt, 
                                                   const CMPIObjectPath * ref, 
                                                   const char * lang, 
                                                   const char * query) {
  CMPIStatus rc = {CMPI_RC_OK, NULL};

  _OSBASE_TRACE(1,("--%s CMPI ExecQuery() called", _ClassName));
  CMSetStatusWithChars( _broker, &rc, 
                        CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" ); 
  _OSBASE_TRACE(1,("--%s CMPI ExecQuery() exited", _ClassName));
  return rc;
}


/* ---------------------------------------------------------------------------*/
/*                        Method Provider Interface                           */
/* ---------------------------------------------------------------------------*/


CMPIStatus OSBase_ComputerSystemProviderMethodCleanup( CMPIMethodMI * mi, 
                                                       const CMPIContext * ctx, CMPIBoolean trm) {
  _OSBASE_TRACE(1,("--%s CMPI MethodCleanup() called", _ClassName));
  _OSBASE_TRACE(1,("--%s CMPI MethodCleanup() exited", _ClassName));
  CMReturn(CMPI_RC_OK);
}

CMPIStatus OSBase_ComputerSystemProviderInvokeMethod( CMPIMethodMI * mi,
                                                      const CMPIContext * ctx,
                                                      const CMPIResult * rslt,
                                                      const CMPIObjectPath * ref,
                                                      const char * methodName,
                                                      const CMPIArgs * in,
                                                      CMPIArgs * out) 
{
  CMPIString * Class = NULL;
  CMPIStatus   rc    = {CMPI_RC_OK, NULL};

  _OSBASE_TRACE(1,("--%s CMPI InvokeMethod() called", _ClassName));

  Class = CMGetClassName(ref, &rc);

  if( strcasecmp(CMGetCharPtr(Class), _ClassName) == 0 &&
      strcasecmp("SetPowerState",methodName) == 0 ) {
    CMSetStatusWithChars( _broker, &rc,
                          CMPI_RC_ERR_NOT_SUPPORTED, methodName );
  }
  else {
    CMSetStatusWithChars( _broker, &rc,
                          CMPI_RC_ERR_NOT_FOUND, methodName );
  }
  _OSBASE_TRACE(1,("--%s CMPI InvokeMethod() exited", _ClassName)); 
  return rc;
}

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( OSBase_ComputerSystemProvider, 
                  OSBase_ComputerSystemProvider, 
                  _broker, 
                  CMNoHook)

CMMethodMIStub( OSBase_ComputerSystemProvider, 
                OSBase_ComputerSystemProvider, 
                _broker, 
                CMNoHook)


/* ---------------------------------------------------------------------------*/
/*              end of cmpiOSBase_ComputerSystemProvider                      */
/* ---------------------------------------------------------------------------*/

    
     
