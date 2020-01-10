/*
 * $Id: cmpiSMIS_FCPort.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 *	Provider for logical representation of Fibre Channel disk.
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

static char * _ClassName = "Linux_FCPort";


/* ---------------------------------------------------------------------------*/
/* private functions                                                          */
/* ---------------------------------------------------------------------------*/


/******************************************************************************/
/* Function: _convertSpeed                                                    */
/* Purpose:  To convert the hba speed value to Gbits per second.              */
/* Inputs:   The HBA value for the speed.                                     */
/* Outputs:  The speed in Gbits per second.                                   */
/*              If speed is unknown return value will be 0.                   */
/******************************************************************************/
unsigned long long _convertSpeed(int hbaSpeedValue)
   {
   unsigned long long          speed;
   static unsigned long long   Gbits=1000000000;
    
   switch (hbaSpeedValue)
      {
      case 1: speed= Gbits;
              break;
      case 2: speed= 2 * Gbits;
              break;
      case 4: speed= 10 * Gbits;
              break;
      case 8: speed= 4 * Gbits;
              break;
      default:speed= 0;
              break;
      }
   return(speed);
   } /* end _convertSpeed */



/* ---------------------------------------------------------------------------*/
/* end private functions                                                      */
/* ---------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------------*/
/*                            Factory functions                               */
/* ---------------------------------------------------------------------------*/

/******************************************************************************/
/* Function: _makePath_FCPort                                                 */
/* Purpose:  method to create a CMPIObjectPath of this class                  */
/******************************************************************************/
CMPIObjectPath * _makePath_FCPort( const CMPIBroker * _broker,
                  const CMPIContext * ctx,
                  const CMPIObjectPath * ref,
                  const struct cim_hbaPort * sptr,
                  CMPIStatus * rc) 
   {
   CMPIObjectPath *op=NULL;
   char           deviceID[17];   /* to hold FCPort id */
   char         * system_name = NULL; /* save pointer returned by get_system_name() */

   _OSBASE_TRACE(2,("--- _makePath_FCPort() called"));

   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   system_name = get_system_name();
   if( ! system_name ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(2,("--- _makePath_FCPort() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   op=CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                       _ClassName, rc );
   if( CMIsNullObject(op) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(2,("--- _makePath_FCPort() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   CMAddKey(op, "SystemCreationClassName", CSCreationClassName, CMPI_chars);  
   CMAddKey(op, "SystemName", system_name, CMPI_chars);
   CMAddKey(op, "CreationClassName", _ClassName, CMPI_chars);  

   snprintf(deviceID,17,"%llx",*(unsigned long long*)sptr->port_attributes->PortWWN.wwn);
   deviceID[16]='\0';
   CMAddKey(op, "DeviceID", deviceID, CMPI_chars); 
     
   exit:
   /* free system_name */
   if ( system_name ) free(system_name);

   _OSBASE_TRACE(2,("--- _makePath_FCPort() exited"));
   return op;
   } /* end _makePath_FCPort */





/******************************************************************************/
/* Function: _makeInst_FCPort                                                 */
/* Purpose:  method to create a CMPIInstance of this class                    */
/******************************************************************************/
CMPIInstance * _makeInst_FCPort( const CMPIBroker * _broker,
                const CMPIContext * ctx,
                const CMPIObjectPath * ref,
                const struct cim_hbaPort * sptr,
                CMPIStatus * rc) 
   {
   CMPIObjectPath    * op     = NULL;
   CMPIInstance      * ci     = NULL;
   
   /* Variables for settting fibre attributes */
   char                     deviceID[17];        /* to hold FCPort id */
   static unsigned short    linkTechnology=4; /* LinkTechnology=1 for other(loopback), 2 for ethernet, 7 for token ring, 4 for fibre */
   static char              *status_states[] = {"Unknown",   /* Blank to skip index 0 */  
                                                "Unknown",   /* from HBA PortState Unknown */
                                                "OK",        /* from HBA PortState Online */ 
                                                "Stopped",   /* from HBA PortState Offline */ 
                                                "Unknown",   /* from HBA PortState Bypassed */ 
                                                "Service",   /* from HBA PortState Diagnostics */ 
                                                "Error",     /* from HBA PortState Linkdown */ 
                                                "Error",     /* from HBA PortState Error */ 
                                                "Unknown"    /* from HBA PortState Loopback */                                             
                                               };
   static unsigned short    opStatus_states[9] = {0,   /* Blank to skip index 0 */ 
                                                  0,   /* from HBA PortState Unknown to Cim opState Unknown */
                                                  2,   /* from HBA PortState Online to Cim opState OK */
                                                  15,  /* from HBA PortState Offline to Cim opState Dormant */
                                                  1,   /* from HBA PortState Bypassed to Cim opState Other */
                                                  11,  /* from HBA PortState Diagnostics to Cim opState In Service */
                                                  1,   /* from HBA PortState Linkdown to Cim opState Other */
                                                  6,   /* from HBA PortState Error to Cim opState Error */
                                                  1,   /* from HBA PortState Loopback to Cim opState Other */
                                                 };
   static unsigned short    port_types[13] = {0,   /* Blank to skip index 0 */ 
                                              0,   /* from HBA PortType Unknown */
                                              1,   /* from HBA PortType Other */
                                              1,   /* from HBA PortType Not Present to Cim Other */
                                              0,   /* undefined */
                                              10,  /* from HBA PortType Fabric (Nport) */
                                              11,  /* from HBA PortType Public Loop (NLport) */
                                              16,  /* from HBA PortType Fabric on a Loop (FLport) */
                                              15,  /* from HBA PortType Fabric Port (Fport) */
                                              14,  /* from HBA PortType Fabric Expansion Port (Eport) */
                                              18,  /* from HBA PortType Generic Fabric Port (Gport) */
                                              1,   /* from HBA PortType Private Loop to Cim Other (Lport) */
                                              1,   /* from HBA PortType Point to Point to Cim Other */
                                             };
   static char              *otherNetworkPort_types[] = {NULL,     /* Blank to skip index 0 */  
                                               NULL,               /* from HBA PortType Unknown */
                                               "Other",            /* from HBA PortType Other */ 
                                               "Not Present",      /* from HBA PortType Not Present to Cim Other */ 
                                               NULL,               /* undefined */
                                               NULL,               /* from HBA PortType Fabric (Nport) */ 
                                               NULL,               /* from HBA PortType Public Loop (NLport) */ 
                                               NULL,               /* from HBA PortType Fabric on a Loop (FLport) */ 
                                               NULL,               /* from HBA PortType Fabric Port (Fport) */
                                               NULL,               /* from HBA PortType Fabric Expansion Port (Eport) */ 
                                               NULL,               /* from HBA PortType Generic Fabric Port (Gport) */ 
                                               "Private Loop",     /* from HBA PortType Private Loop to Cim Other (Lport) */ 
                                               "Point to Point"    /* from HBA PortType Point to Point to Cim Other */                                             
                                               };
   CMPIArray                   *opStatus=NULL; /* OperationalStatus is an array of mulitple statuses, like OK and Completed */
   CMPIArray                   *opStatusDescs=NULL; /* StatusDescriptions is an array of mulitple status descriptions */
   static char                 *portStates[] = {"","Unknown","Online","Offline","Bypassed","Diagnostics","Linkdown","Error","Loopback"}; 
   unsigned short              temp_uint16;           /* temporary variable for setting cim properties */
   unsigned long               temp_uint32;           /* temporary variable for setting cim properties */
   unsigned long long          temp_uint64;           /* temporary variable for setting cim properties */
   CMPIArray                   *tempCIMArray=NULL; /* temporary variable for holding a cmpi array  */
   char                        *temp_string;           /* temporary pointer to a string for setting cim properties */
   char                        *system_name = NULL;  /* save pointer returned by get_system_name() */

   _OSBASE_TRACE(2,("--- _makeInst_FCPort() called"));

   /* the sblim-cmpi-base package offers some tool methods to get common */
   /* system datas.                                                      */  
   /* CIM_HOST_NAME contains the unique hostname of the local system     */
   system_name = get_system_name();
   if( ! system_name ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "no host name found" );
      _OSBASE_TRACE(2,("--- _makeInst_FCPort() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)),
                         _ClassName, rc );
   if( CMIsNullObject(op) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      _OSBASE_TRACE(2,("--- _makeInst_FCPort() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }

   ci = CMNewInstance( _broker, op, rc);
   if( CMIsNullObject(ci) ) 
      {
      CMSetStatusWithChars( _broker, rc,
                            CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
      _OSBASE_TRACE(2,("--- _makeInst_FCPort() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   
   
   /* Set general class attributes */
   CMSetProperty( ci, "SystemCreationClassName",CSCreationClassName , CMPI_chars ); 
   CMSetProperty( ci, "SystemName", system_name, CMPI_chars );
   CMSetProperty( ci, "CreationClassName", _ClassName, CMPI_chars ); 

   /************************/
   /* Set Fibre attributes */
   /************************/
   /* Set attributes from PortWWN */
   snprintf(deviceID,17,"%llx",*(unsigned long long*)sptr->port_attributes->PortWWN.wwn);
   deviceID[16]='\0';
   CMSetProperty( ci, "DeviceID", deviceID, CMPI_chars); 
   CMSetProperty( ci, "PermanentAddress", deviceID, CMPI_chars);

   /* Set Name to be InstanceID(PortWWN) */
   CMSetProperty( ci, "Name", sptr->InstanceID, CMPI_chars);  

   /* Set misc hardcoded attributes */
   CMSetProperty( ci, "Caption", "Linux FCPort", CMPI_chars);
   CMSetProperty( ci, "Description", "This class represents instances of available Fibre Channel Ports.", CMPI_chars);
   CMSetProperty( ci, "LinkTechnology", (CMPIValue*)&(linkTechnology), CMPI_uint16);

   /* Set misc attributes */
   temp_uint16=sptr->port_number;
   CMSetProperty( ci, "PortNumber", (CMPIValue*)&(temp_uint16), CMPI_uint16);
   temp_uint64=sptr->port_attributes->PortMaxFrameSize;
   CMSetProperty( ci, "SupportedMaximumTransmissionUnit", (CMPIValue*)&(temp_uint64), CMPI_uint64);
   CMSetProperty( ci, "ElementName", sptr->port_attributes->OSDeviceName, CMPI_chars);

   /* set status attributes */
   CMSetProperty( ci, "Status", status_states[sptr->port_attributes->PortState] , CMPI_chars);
   
   opStatus = CMNewArray(_broker,1,CMPI_uint16,rc);
   if( opStatus == NULL ) 
      {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,1,CMPI_uint16,rc)" );
      _OSBASE_TRACE(2,("--- _makeInst_FCPort() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   else 
      {
      unsigned short opStatus_state;
      opStatus_state=opStatus_states[sptr->port_attributes->PortState];
      CMSetArrayElementAt(opStatus,0,(CMPIValue*)&(opStatus_state),CMPI_uint16);
      CMSetProperty( ci, "OperationalStatus", (CMPIValue*)&(opStatus), CMPI_uint16A);
      }

   opStatusDescs = CMNewArray(_broker,19,CMPI_string,rc);
   if( opStatusDescs == NULL ) 
      {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,19,CMPI_string,rc)" );
      _OSBASE_TRACE(2,("--- _makeInst_FCPort() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   else 
      {
      CMPIString *val=NULL;
      char *opStatusDesc;
      int index=0;
      int match; /* the OperationalStatus single value */

      opStatusDesc=portStates[sptr->port_attributes->PortState];
      match=opStatus_states[sptr->port_attributes->PortState];
      for(index=0; index<19; index++)
         {
         if(index==match)
           val = CMNewString(_broker,opStatusDesc,rc);     
        else
           val = CMNewString(_broker,"",rc);     
         CMSetArrayElementAt(opStatusDescs,index,(CMPIValue*)&(val),CMPI_string); 
        }
      CMSetProperty( ci, "StatusDescriptions", (CMPIValue*)&(opStatusDescs), CMPI_stringA);        
      }
  
   /* Set n/a attributes */
   temp_uint16=5; /* 5=n/a for EnabledState */
   CMSetProperty( ci, "EnabledState", (CMPIValue*)&(temp_uint16), CMPI_uint16);
   temp_uint16=6; /* 6=n/a for Availability and AdditionalAvailability */
   CMSetProperty( ci, "Availability", (CMPIValue*)&(temp_uint16), CMPI_uint16);
   CMSetProperty( ci, "AdditionalAvailability", (CMPIValue*)&(temp_uint16), CMPI_uint16);

   /* Set speeds */
   temp_uint64=_convertSpeed(sptr->port_attributes->PortSpeed);
   if(temp_uint64 != 0) /* Skip setting speed if unknown */
      CMSetProperty( ci, "Speed", (CMPIValue*)&(temp_uint64), CMPI_uint64);
   temp_uint64=_convertSpeed(sptr->port_attributes->PortSupportedSpeed);
   if(temp_uint64 != 0) /* Skip setting speed if unknown */
      CMSetProperty( ci, "MaxSpeed", (CMPIValue*)&(temp_uint64), CMPI_uint64);
      
   /* Set Restrictions */
   /* 0=Unknown, 2=front end only, 3=back end only, 4=not restricted */
   temp_uint16=0;
   CMSetProperty( ci, "UsageRestriction", (CMPIValue*)&(temp_uint16), CMPI_uint16);

   /* Set PortType and OtherNetworkPortType */  
   temp_uint16=sptr->port_attributes->PortType;
   if((temp_uint16==11) || (temp_uint16==12)) temp_uint16=0;   /* Remap hba undefined values 11 and 12 to index 0 */
   if(temp_uint16==20) temp_uint16=11;   /* Remap value 20 to value/index 11 for easy array mapping */
   if(temp_uint16==21) temp_uint16=12;   /* Remap value 21 to value/index 12 for easy array mapping */
   if(temp_uint16>12) temp_uint16=0;   /* Remap out of bounds to index 0 */
   CMSetProperty( ci, "PortType", (CMPIValue*)&(port_types[temp_uint16]), CMPI_uint16);
   temp_string=otherNetworkPort_types[temp_uint16];
   if (temp_string != NULL)
      CMSetProperty( ci, "OtherNetworkPortType", temp_string, CMPI_chars); 

   /* Set SupportedCOS */
   {
   int arraySize=0;            /* array size */
   unsigned long COS_bitfield; /* holding HBA COS bitfield */
   unsigned short COS;         /* an individual class of service */
   int cim_arrayIndex=0;       /* cim array index */
      
   
   COS_bitfield=sptr->port_attributes->PortSupportedClassofService;
   COS_bitfield &= 0177; /* block out out of range bits, over bit 7 */
   for(COS=0; COS_bitfield != 0; COS_bitfield>>=1) /* rightshift through the bitfield */
      {
      if(COS_bitfield & 01) arraySize++; /* if lowest bit set */
      }
   if(arraySize==0) arraySize=1;
      
   tempCIMArray=CMNewArray(_broker,arraySize,CMPI_uint16,rc);
   if( tempCIMArray == NULL ) 
      {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,arraySize,CMPI_uint16,rc)" );
      _OSBASE_TRACE(2,("--- _makeInst_FCPort() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   else 
      {
      COS_bitfield=sptr->port_attributes->PortSupportedClassofService;
      COS_bitfield &= 0177; /* block out out of range bits, over bit 7 */
      for(COS=0; COS_bitfield != 0; COS_bitfield>>=1) /* rightshift through the bitfield */
         {
        if(COS_bitfield & 01) /* if lowest bit set */
           {
           if(COS==0)
              {
              static unsigned short COS_F=7; /* HBA bit 0 is class F which is cmpi value 7 */
              CMSetArrayElementAt(tempCIMArray,cim_arrayIndex,(CMPIValue*)&(COS_F),CMPI_uint16);
              }
           else
              {
              CMSetArrayElementAt(tempCIMArray,cim_arrayIndex,(CMPIValue*)&(COS),CMPI_uint16);
              }
           cim_arrayIndex++;
           }
        COS++;
         } /* end for COS */
      if(cim_arrayIndex==0) /* If no HBA bits set then set cim value unknown=0 */
        {
        static unsigned short COS_Unknown=0; 
        CMSetArrayElementAt(tempCIMArray,cim_arrayIndex,(CMPIValue*)&(COS_Unknown),CMPI_uint16);
        }
      CMSetProperty( ci, "SupportedCOS", (CMPIValue*)&(tempCIMArray), CMPI_uint16A);
      } /* end else */  
   }  /* end Set SupportedCOS */ 
      
   /* Set SupportedFC4Types */
   /* The bit field is 32 bytes long, broken up into 4 byte chunks named words */
   {
   int                         arraySize=0;        /* number of fc4 types that are set */
   unsigned char               byte;                   /* for holding 1 byte of the bit field */
   int                         cim_arrayIndex=0;   /* cim array index */
   unsigned short              wordNumber=0;       /* for holding what word number we are on. 0-7 */
   short                       byteNumber=0;       /* for holding what byte number we are on. 0-3 */
   unsigned short              bitNumber=0;        /* for holding what bit number inside the byte we are on. 0-7 */
   unsigned short              fcType=0;           /* for holding current fc4Type */
   
   for(wordNumber=0; wordNumber<8; wordNumber++)
      {
      for(byteNumber=3; byteNumber>=0; byteNumber--)
         {
        byte=sptr->port_attributes->PortSupportedFc4Types.bits[byteNumber+wordNumber*4];
         /* printf("%02x ", byte); */
         for(bitNumber=0; bitNumber<8; bitNumber++)
            {
           if(byte & 01) arraySize++; /* if lowest bit set */
           byte>>=1;                  /* rightshift the byte 1 bit */
           } /* end for bit */
        } /* end for byte */
      } /* end for word */
   if(arraySize==0) arraySize=1;
   /* printf("\n arraySize=%d \n\n",arraySize ); */

   tempCIMArray=CMNewArray(_broker,arraySize,CMPI_uint16,rc);
   if( tempCIMArray == NULL ) 
      {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,arraySize,CMPI_uint16,rc)" );
      _OSBASE_TRACE(2,("--- _makeInst_FCPort() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   else 
      {
      for(wordNumber=0; wordNumber<8; wordNumber++)
         {
         for(byteNumber=3; byteNumber>=0; byteNumber--)
            {
           byte=sptr->port_attributes->PortSupportedFc4Types.bits[byteNumber+wordNumber*4];
            for(bitNumber=0; bitNumber<8; bitNumber++)
               {
              if(byte & 01)  /* if lowest bit set */
                 {
                 CMSetArrayElementAt(tempCIMArray,cim_arrayIndex,(CMPIValue*)&(fcType),CMPI_uint16);
                 cim_arrayIndex++;
                 }
              byte>>=1;                  /* rightshift the byte 1 bit */
              fcType++;
              } /* end for bit */
           } /* end for byte */
         } /* end for word */
      if(cim_arrayIndex==0) /* If no HBA bits set then set cim value unknown=0 */
        {
        static unsigned short fcType_Unknown=0; 
        CMSetArrayElementAt(tempCIMArray,cim_arrayIndex,(CMPIValue*)&(fcType_Unknown),CMPI_uint16);
        }
      CMSetProperty( ci, "SupportedFC4Types", (CMPIValue*)&(tempCIMArray), CMPI_uint16A);
      } /* end else */  
   } /* end Set SupportedFC4Types */

   /* Set ActiveFC4Types */
   /* The bit field is 32 bytes long, broken up into 4 byte chunks named words */
   {
   int                         arraySize=0;        /* number of fc4 types that are set */
   unsigned char               byte;                   /* for holding 1 byte of the bit field */
   int                         cim_arrayIndex=0;   /* cim array index */
   unsigned short              wordNumber=0;       /* for holding what word number we are on. 0-7 */
   short                       byteNumber=0;       /* for holding what byte number we are on. 0-3 */
   unsigned short              bitNumber=0;        /* for holding what bit number inside the byte we are on. 0-7 */
   unsigned short              fcType=0;           /* for holding current fc4Type */
   
   for(wordNumber=0; wordNumber<8; wordNumber++)
      {
      for(byteNumber=3; byteNumber>=0; byteNumber--)
         {
        byte=sptr->port_attributes->PortActiveFc4Types.bits[byteNumber+wordNumber*4];
         /* printf("%02x ", byte); */
         for(bitNumber=0; bitNumber<8; bitNumber++)
            {
           if(byte & 01) arraySize++; /* if lowest bit set */
           byte>>=1;                  /* rightshift the byte 1 bit */
           } /* end for bit */
        } /* end for byte */
      } /* end for word */
   if(arraySize==0) arraySize=1;
   /* printf("\n arraySize=%d \n\n",arraySize ); */

   tempCIMArray=CMNewArray(_broker,arraySize,CMPI_uint16,rc);
   if( tempCIMArray == NULL ) 
      {
      CMSetStatusWithChars( _broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker,arraySize,CMPI_uint16,rc)" );
      _OSBASE_TRACE(2,("--- _makeInst_FCPort() failed : %s",CMGetCharPtr(rc->msg)));
      goto exit;
      }
   else 
      {
      for(wordNumber=0; wordNumber<8; wordNumber++)
         {
         for(byteNumber=3; byteNumber>=0; byteNumber--)
            {
           byte=sptr->port_attributes->PortActiveFc4Types.bits[byteNumber+wordNumber*4];
            for(bitNumber=0; bitNumber<8; bitNumber++)
               {
              if(byte & 01)  /* if lowest bit set */
                 {
                 CMSetArrayElementAt(tempCIMArray,cim_arrayIndex,(CMPIValue*)&(fcType),CMPI_uint16);
                 cim_arrayIndex++;
                 }
              byte>>=1;                  /* rightshift the byte 1 bit */
              fcType++;
              } /* end for bit */
           } /* end for byte */
         } /* end for word */
      if(cim_arrayIndex==0) /* If no HBA bits set then set cim value unknown=0 */
        {
        static unsigned short fcType_Unknown=0; 
        CMSetArrayElementAt(tempCIMArray,cim_arrayIndex,(CMPIValue*)&(fcType_Unknown),CMPI_uint16);
        }
      CMSetProperty( ci, "ActiveFC4Types", (CMPIValue*)&(tempCIMArray), CMPI_uint16A);
      } /* end else */  
   } /* end Set ActiveFC4Types */

   /****************************/
   /* End Set Fibre attributes */
   /****************************/


  exit:
   /* free system_name */
   if ( system_name ) free(system_name);

   _OSBASE_TRACE(2,("--- _makeInst_FCPort() exited"));
   return ci;
} /* end _makeInst_FCPort */


/* ---------------------------------------------------------------------------*/
/*                    end of cmpiSMIS_FCPort.c                                */
/* ---------------------------------------------------------------------------*/

