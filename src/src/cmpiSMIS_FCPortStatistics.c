/*
 * $Id: cmpiSMIS_FCPortStatistics.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 * This is the factory implementation for creating instances of CIM
 * class Linux_FCPortStatistics.
 *
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
/* private declarations                                                       */

static char * _ClassName = "Linux_FCPortStatistics";

/* ---------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------------*/
/*                            Factory functions                               */
/* ---------------------------------------------------------------------------*/

/* ---------- method to create a CMPIObjectPath of this class ----------------*/

CMPIObjectPath * _makePath_FCPortStatistics( const CMPIBroker * _broker,
                 const CMPIContext * ctx,
                 const CMPIObjectPath * ref,
                 const struct cim_hbaPort * sptr,
                 CMPIStatus * rc) {
  CMPIObjectPath * op = NULL;
  char           * system_name = NULL; /* save pointer returned by get_system_name */
  
  _OSBASE_TRACE(2,("--- _makePath_FCPortStatistics() called"));

  /* the sblim-cmpi-base package offers some tool methods to get common
   * system datas
   * CIM_HOST_NAME contains the unique hostname of the local system
  */
  system_name = get_system_name();
  if( ! system_name ) {
    CMSetStatusWithChars( _broker, rc,
                          CMPI_RC_ERR_FAILED, "no host name found" );
    _OSBASE_TRACE(2,("--- _makePath_FCPortStatistics() failed : %s",CMGetCharPtr(rc->msg)));
    goto exit;
  }
  /* free system_name */
  free( system_name );

  op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                        _ClassName, rc );
  if( CMIsNullObject(op) ) {
    CMSetStatusWithChars( _broker, rc,
                          CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
    _OSBASE_TRACE(2,("--- _makePath_FCPortStatistics() failed : %s",CMGetCharPtr(rc->msg)));
    goto exit;
  }
  if(sptr)
  CMAddKey(op, "InstanceID", sptr->InstanceID, CMPI_chars);

 exit:
  _OSBASE_TRACE(2,("--- _makePath_FCPortStatistics() exited"));
  return op;
}

/* ----------- method to create a CMPIInstance of this class ----------------*/

CMPIInstance * _makeInst_FCPortStatistics( const CMPIBroker * _broker,
               const CMPIContext * ctx,
               const CMPIObjectPath * ref,
               const struct cim_hbaPort * sptr,
               CMPIStatus * rc) {
  CMPIObjectPath * op     = NULL;
  CMPIInstance   * ci     = NULL;
  CMPIDateTime   * dt     = NULL;
  char           * system_name = NULL; /* save pointer returned by get_system_name */
  unsigned long long      temp_uint64; /* temporary variable for setting cim properties */               	
  
  _OSBASE_TRACE(2,("--- _makeInst_FCPortStatistics() called"));

  system_name = get_system_name();
  if( ! system_name ) {
    CMSetStatusWithChars( _broker, rc,
                          CMPI_RC_ERR_FAILED, "no host name found" );
    _OSBASE_TRACE(2,("--- _makeInst_FCPortStatistics() failed : %s",CMGetCharPtr(rc->msg)));
    goto exit;
  }
  /* free system_name */
  free( system_name );

  op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                        _ClassName, rc );
  if( CMIsNullObject(op) ) {
    CMSetStatusWithChars( _broker, rc,
                          CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
    _OSBASE_TRACE(2,("--- _makeInst_FCPortStatistics() failed : %s",CMGetCharPtr(rc->msg)));
    goto exit;
  }

  ci = CMNewInstance( _broker, op, rc);
  if( CMIsNullObject(ci) ) {
    CMSetStatusWithChars( _broker, rc,
                          CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
    _OSBASE_TRACE(2,("--- _makeInst_FCPortStatistics() failed : %s",CMGetCharPtr(rc->msg)));
    goto exit;
  }
  if(sptr) {
  CMSetProperty( ci, "InstanceID", sptr->InstanceID, CMPI_chars );
  CMSetProperty( ci, "ElementName", sptr->InstanceID, CMPI_chars );
  CMSetProperty( ci, "Caption", "Linux HBA Port Statistics", CMPI_chars);
  CMSetProperty( ci, "Description", "This class represents instances of the statistics for HBA Ports.", CMPI_chars);
  
  temp_uint64 = sptr->bytes_received;
  CMSetProperty( ci, "BytesReceived", (CMPIValue*)&(temp_uint64), CMPI_uint64);  

  temp_uint64 = sptr->bytes_transmitted;
  CMSetProperty( ci, "BytesTransmitted", (CMPIValue*)&(temp_uint64), CMPI_uint64);  
  if(sptr->port_statistics) {
  temp_uint64 = sptr->port_statistics->TxFrames;
  CMSetProperty( ci, "PacketsTransmitted", (CMPIValue*)&(temp_uint64), CMPI_uint64);  
  temp_uint64 = sptr->port_statistics->RxFrames;  
  CMSetProperty( ci, "PacketsReceived", (CMPIValue*)&(temp_uint64), CMPI_uint64);  
  temp_uint64 = sptr->port_statistics->InvalidCRCCount;    
  CMSetProperty( ci, "CRCErrors", (CMPIValue*)&(temp_uint64), CMPI_uint64);
  temp_uint64 = sptr->port_statistics->LinkFailureCount;  
  CMSetProperty( ci, "LinkFailures", (CMPIValue*)&(temp_uint64), CMPI_uint64);  
  temp_uint64 = sptr->port_statistics->PrimitiveSeqProtocolErrCount;
  CMSetProperty( ci, "PrimitiveSeqProtocolErrCount", (CMPIValue*)&(temp_uint64), CMPI_uint64); 
  temp_uint64 = sptr->port_statistics->LossOfSignalCount;
  CMSetProperty( ci, "LossOfSignalCounter", (CMPIValue*)&(temp_uint64), CMPI_uint64);  
  temp_uint64 = sptr->port_statistics->InvalidTxWordCount;  
  CMSetProperty( ci, "InvalidTransmissionWords", (CMPIValue*)&(temp_uint64), CMPI_uint64);
  temp_uint64 = sptr->port_statistics->LIPCount;
  CMSetProperty( ci, "LIPCount", (CMPIValue*)&(temp_uint64), CMPI_uint64);
  temp_uint64 = sptr->port_statistics->NOSCount;  
  CMSetProperty( ci, "NOSCount", (CMPIValue*)&(temp_uint64), CMPI_uint64);
  temp_uint64 = sptr->port_statistics->ErrorFrames;  
  CMSetProperty( ci, "ErrorFrames", (CMPIValue*)&(temp_uint64), CMPI_uint64);  
  temp_uint64 = sptr->port_statistics->DumpedFrames;
  CMSetProperty( ci, "DumpedFrames", (CMPIValue*)&(temp_uint64), CMPI_uint64);  
  temp_uint64 = sptr->port_statistics->LossOfSyncCount;
  CMSetProperty( ci, "LossOfSyncCounter", (CMPIValue*)&(temp_uint64), CMPI_uint64);

  temp_uint64 = sptr->statistic_time;   
  dt = CMNewDateTimeFromBinary(_broker, temp_uint64, 0, rc);
  CMSetProperty( ci, "StatisticTime", (CMPIValue*)&dt, CMPI_dateTime);	
 
  temp_uint64 = sptr->statistic_time - 1000000*((unsigned long long)(sptr->port_statistics->SecondsSinceLastReset)); 
  dt = CMNewDateTimeFromBinary(_broker, temp_uint64, 0, rc);	
  CMSetProperty( ci, "StartStatisticTime", (CMPIValue*)&dt, CMPI_dateTime);
   }
  }	

 exit:
  _OSBASE_TRACE(2,("--- _makeInst_FCPortStatistics() exited"));
  return ci;
}



/* ---------------------------------------------------------------------------*/
/*                    end of cmpiSMIS_FCPortStatistics.c                        */
/* ---------------------------------------------------------------------------*/

