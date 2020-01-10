/*
 * $Id: cmpiSMIS_FCPortStatisticsProvider.c,v 1.1.1.1 2009/05/12 21:46:34 nsharoff Exp $
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
 *      Provider for Fibre Channel Port statistics.
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

static char * _ClassName = "Linux_FCPortStatistics";


/* ---------------------------------------------------------------------------*/


/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


CMPIStatus SMIS_FCPortStatisticsProviderCleanup( CMPIInstanceMI * mi,
           const CMPIContext * ctx, CMPIBoolean trm)
{
  _OSBASE_TRACE(1,("--- %s CMPI Cleanup() called",_ClassName));
  _OSBASE_TRACE(1,("--- %s CMPI Cleanup() exited",_ClassName));
  CMReturn(CMPI_RC_OK);
}

CMPIStatus SMIS_FCPortStatisticsProviderEnumInstanceNames( CMPIInstanceMI * mi,
           const CMPIContext * ctx,
           const CMPIResult * rslt,
           const CMPIObjectPath * ref)
{
  CMPIObjectPath     * op    = NULL;
  CMPIStatus           rc    = {CMPI_RC_OK, NULL};
  struct hbaPortList * lptr  = NULL ;
  struct hbaPortList * rm    = NULL ;

  _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() called",_ClassName));

  if( enum_all_hbaPorts( &lptr, 1 ) != 0 )
  {
    CMSetStatusWithChars( _broker, &rc,
                          CMPI_RC_ERR_FAILED, "Could not list hba ports." );
    _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
    return rc;
  }

  rm = lptr;
  // iterate port list
  if( lptr != NULL )
  {
    for ( ; lptr && rc.rc == CMPI_RC_OK ; lptr = lptr->next)
    {
        op = _makePath_FCPortStatistics( _broker, ctx, ref,lptr->sptr, &rc );
        if( op == NULL || rc.rc != CMPI_RC_OK )
        {
          if( rc.msg != NULL )
          {
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
          }
          CMSetStatusWithChars( _broker, &rc,
                              CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM ObjectPath failed." );
          if(rm)
          {
             free_hbaPortList(rm);
          }
          _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
          return rc;
        }
        else
        {
           CMReturnObjectPath( rslt, op );
        }
    }
    if (rm)
    {
       free_hbaPortList(rm);
    }
  }

  CMReturnDone( rslt );
  _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() exited",_ClassName));
  return rc;
}

CMPIStatus SMIS_FCPortStatisticsProviderEnumInstances( CMPIInstanceMI * mi,
           const CMPIContext * ctx,
           const CMPIResult * rslt,
           const CMPIObjectPath * ref,
           const char ** properties)
{
  CMPIInstance       * ci    = NULL;
  CMPIStatus           rc    = {CMPI_RC_OK, NULL};
  struct hbaPortList * lptr  = NULL ;
  struct hbaPortList * rm    = NULL ;
  int                  cmdrc = 0;

  _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() called",_ClassName));

  cmdrc = enum_all_hbaPorts( &lptr, 1 );
  if( cmdrc != 0 )
  {
    CMSetStatusWithChars( _broker, &rc,
                          CMPI_RC_ERR_FAILED, "Could not list hba ports." );
    _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
    return rc;
  }

  rm = lptr;
  // iterate port list
  if( lptr != NULL )
  {
    for ( ; lptr && rc.rc == CMPI_RC_OK ; lptr = lptr->next)
    {
        ci = _makeInst_FCPortStatistics( _broker, ctx, ref, lptr->sptr, &rc );
        if( ci == NULL || rc.rc != CMPI_RC_OK )
        {
          if( rc.msg != NULL )
          {
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
          }
          CMSetStatusWithChars( _broker, &rc,
                                CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM Instance failed." );
          if(rm)
          {
             free_hbaPortList(rm);
          }
          _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
          return rc;
        }
        else
        {
           CMReturnInstance( rslt, ci );
        }
    }
    if(rm)
    {
       free_hbaPortList(rm);
    }
  }

  CMReturnDone( rslt );
  _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() exited",_ClassName));
  return rc;
}

CMPIStatus SMIS_FCPortStatisticsProviderGetInstance( CMPIInstanceMI * mi,
           const CMPIContext * ctx,
           const CMPIResult * rslt,
           const CMPIObjectPath * cop,
           const char ** properties)
{
  CMPIInstance       * ci    = NULL;
  CMPIString         * id    = NULL;
  struct cim_hbaPort * sptr  = NULL;
  CMPIStatus           rc    = {CMPI_RC_OK, NULL};
  int                  cmdrc = 0;

  _OSBASE_TRACE(1,("--- %s CMPI GetInstance() called",_ClassName));

  _check_system_key_value_pairs( _broker, cop,
        "SystemCreationClassName", "SystemName", &rc );
  if( rc.rc != CMPI_RC_OK )
  {
    _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
    return rc;
  }

  id = CMGetKey( cop, "InstanceID", &rc).value.string;
  if( id == NULL )
  {
    CMSetStatusWithChars( _broker, &rc,
                          CMPI_RC_ERR_FAILED, "Could not get FCPortStatisticsID." );
    _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
    return rc;
  }

  cmdrc = get_hbaPort_data( CMGetCharPtr(id) , &sptr, 1 );
  if( cmdrc != 0 || sptr == NULL )
  {
    CMSetStatusWithChars( _broker, &rc,
                          CMPI_RC_ERR_NOT_FOUND, "FCPortStatistics does not exist." );
    _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
    return rc;
  }

  ci = _makeInst_FCPortStatistics( _broker, ctx, cop, sptr, &rc );
  if(sptr)
  {
     free_hbaPort(sptr);
  }

  if( ci == NULL )
  {
    if( rc.msg != NULL )
    {
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
    }
    else
    {
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed",_ClassName));
    }
    return rc;
  }

  CMReturnInstance( rslt, ci );
  CMReturnDone(rslt);
  _OSBASE_TRACE(1,("--- %s CMPI GetInstance() exited",_ClassName));
  return rc;
}

CMPIStatus SMIS_FCPortStatisticsProviderCreateInstance( CMPIInstanceMI * mi,
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

CMPIStatus SMIS_FCPortStatisticsProviderModifyInstance( CMPIInstanceMI * mi,
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

CMPIStatus SMIS_FCPortStatisticsProviderDeleteInstance( CMPIInstanceMI * mi,
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

CMPIStatus SMIS_FCPortStatisticsProviderExecQuery( CMPIInstanceMI * mi,
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
/*                        Method Provider Interface                           */
/* ---------------------------------------------------------------------------*/


CMPIStatus SMIS_FCPortStatisticsProviderMethodCleanup( CMPIMethodMI * mi,
           const CMPIContext * ctx, CMPIBoolean trm)
{
  _OSBASE_TRACE(1,("--- %s CMPI MethodCleanup() called",_ClassName));
  _OSBASE_TRACE(1,("--- %s CMPI MethodCleanup() exited",_ClassName));
  CMReturn(CMPI_RC_OK);
}

CMPIStatus SMIS_FCPortStatisticsProviderInvokeMethod( CMPIMethodMI * mi,
           const CMPIContext * ctx,
           const CMPIResult * rslt,
           const CMPIObjectPath * ref,
           const char * methodName,
           const CMPIArgs * in,
           CMPIArgs * out) {
  CMPIString * Class = NULL;
  CMPIStatus   rc    = {CMPI_RC_OK, NULL};

  _OSBASE_TRACE(1,("--- %s CMPI InvokeMethod() called",_ClassName));

  Class = CMGetClassName(ref, &rc);


  if( strcasecmp(CMGetCharPtr(Class), _ClassName) == 0 )
  {
    CMSetStatusWithChars( _broker, &rc,
                          CMPI_RC_ERR_NOT_SUPPORTED, methodName );
  }

  _OSBASE_TRACE(1,("--- %s CMPI InvokeMethod() exited",_ClassName));
  return rc;
}


/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( SMIS_FCPortStatisticsProvider,
                  SMIS_FCPortStatisticsProvider,
                  _broker,
                  CMNoHook)

CMMethodMIStub( SMIS_FCPortStatisticsProvider,
                SMIS_FCPortStatisticsProvider,
                _broker,
                CMNoHook)


/* ---------------------------------------------------------------------------*/
/*                end of cmpiSMIS_FCPortStatisticsProvider                      */
/* ---------------------------------------------------------------------------*/
