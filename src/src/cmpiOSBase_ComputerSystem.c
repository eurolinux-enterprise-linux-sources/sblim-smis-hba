/*
 * $Id: cmpiOSBase_ComputerSystem.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 * Author: IBM
 *
 * Contributors: IBM 
 *
 * Description:
 *	ComputerSystem routines.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

#include "cmpiLinux_Common.h"
#include "Linux_Common.h"

#include "cmpiOSBase_ComputerSystem.h"
#include "Linux_ComputerSystem.h"

/* for the get_baseboard_data to get system Type/Model/SerialNo */
#include "Linux_BaseBoard.h"

//extern char * CSCreationClassName = "Linux_ComputerSystem";
//extern char * OSCreationClassName = "Linux_OperatingSystem";

/* -------- private function for HostingSystemName property-----------------*/
static char* getHostingSystemName();
static int getIdentifyingInfo(char ** identifyingInfo, char ** identifyingInfoDesc);
static char* getElementName();

// judge the system is either VIOS or Linux 
static int isVIOS();
/* ----------end of private functin declaration -------------- */


/* ---------------------------------------------------------------------------*/
/*                            Factory functions                               */
/* ---------------------------------------------------------------------------*/

/* ---------- method to create a CMPIObjectPath of this class ----------------*/

CMPIObjectPath * _makePath_ComputerSystem( const CMPIBroker * _broker,
                                           const CMPIContext * ctx, 
                                           const CMPIObjectPath * ref,
                                           CMPIStatus * rc) {
    CMPIObjectPath * op = NULL;
    char * system_name = get_system_name();

    _OSBASE_TRACE(2,("--_makePath_ComputerSystem() called"));
 
    /* the sblim-cmpi-base package offers some tool methods to get common
     * system datas 
     * CIM_HOST_NAME contains the unique hostname of the local system 
     */
    if( !system_name ) {   
        CMSetStatusWithChars( _broker, rc, 
                              CMPI_RC_ERR_FAILED, "no host name found" );
        _OSBASE_TRACE(2,("--_makePath_ComputerSystem() failed : %s", CMGetCharPtr(rc->msg)));
        goto exit;
    }

    op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)), 
                          _ClassName, rc );
    if( CMIsNullObject(op) ) { 
        CMSetStatusWithChars( _broker, rc, 
                              CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." ); 
        _OSBASE_TRACE(2,("--_makePath_ComputerSystem() failed : %s", CMGetCharPtr(rc->msg)));
        goto exit; 
    }

    CMAddKey(op, "CreationClassName", _ClassName, CMPI_chars);
    CMAddKey(op, "Name", system_name, CMPI_chars);
    
 exit:
    free(system_name);
    _OSBASE_TRACE(2,("--_makePath_ComputerSystem() exited" ));
    return op;                
}

/* ----------- method to create a CMPIInstance of this class ----------------*/

CMPIInstance * _makeInst_ComputerSystem( const CMPIBroker * _broker,
                                         const CMPIContext * ctx, 
                                         const CMPIObjectPath * ref,
                                         const char ** properties, 
                                         CMPIStatus * rc) {
    CMPIObjectPath * op        = NULL;
    CMPIInstance   * ci        = NULL;
    char * owner               = NULL;
    char * contact             = NULL;
    char **keys 	     = NULL;
    int 	keyCount	     = 0;
#ifndef CIM26COMPAT
    CMPIArray      * dedic     = NULL;
    unsigned short   status    = 2; /* Enabled */
    unsigned short   dedicated = 0; /* Not Dedicated */
#endif
    char lparid[255];
    char * system_name = get_system_name();

    _OSBASE_TRACE(2,("--_makeInst_ComputerSystem() called" ));

    /* the sblim-cmpi-base package offers some tool methods to get common
     * system datas 
     * CIM_HOST_NAME contains the unique hostname of the local system 
     */
    if( !system_name ) {   
        CMSetStatusWithChars( _broker, rc, 
                              CMPI_RC_ERR_FAILED, "no host name found" );
        _OSBASE_TRACE(2,("--_makeInst_ComputerSystem() failed : %s", CMGetCharPtr(rc->msg)));
        goto exit;
    }

    op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(ref,rc)), 
                          _ClassName, rc );
    if( CMIsNullObject(op) ) { 
        CMSetStatusWithChars( _broker, rc, 
                              CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." ); 
        _OSBASE_TRACE(2,("--_makeInst_ComputerSystem() failed : %s", CMGetCharPtr(rc->msg)));
        goto exit; 
    }

    ci = CMNewInstance( _broker, op, rc);
    if( CMIsNullObject(ci) ) { 
        CMSetStatusWithChars( _broker, rc, 
                              CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." ); 
        _OSBASE_TRACE(2,("--_makeInst_ComputerSystem() failed : %s", CMGetCharPtr(rc->msg)));
        goto exit; 
    }

    /* Added New Code for Property List Support */
    /* set property filter */ 
    /*
      keys = calloc(3,sizeof(char*)); 
      keys[0] = strdup("CreationClassName"); 
      keys[1] = strdup("Name"); 
      CMSetPropertyFilter(ci,properties,keys); 
      for( ;keys[keyCount]!=NULL;keyCount++) { free((char*)keys[keyCount]); } 
      free(keys); 
    */

    CMSetProperty( ci, "CreationClassName", _ClassName, CMPI_chars );
    CMSetProperty( ci, "Name", system_name, CMPI_chars );
    CMSetProperty( ci, "Status", "NULL", CMPI_chars);
    CMSetProperty( ci, "NameFormat", "IP", CMPI_chars);
    CMSetProperty( ci, "Caption", "Computer System", CMPI_chars);
    CMSetProperty( ci, "Description", "A class derived from ComputerSystem that represents the single node container of the Linux OS.", CMPI_chars);

    if( (owner=get_cs_primownername()) != NULL ) {
        CMSetProperty( ci, "PrimaryOwnerName", owner, CMPI_chars);
        free(owner);
    }
    if( (contact=get_cs_primownercontact()) != NULL) {
        CMSetProperty( ci, "PrimaryOwnerContact", contact, CMPI_chars);
        free(contact);
    }

    if(get_cs_lparid(lparid, sizeof(lparid)) == 0) {
        CMSetProperty( ci, "LPARID", lparid, CMPI_chars);
    }


    /* 2.7 */
#ifndef CIM26COMPAT
    CMSetProperty( ci, "EnabledState", (CMPIValue*)&(status), CMPI_uint16);
    CMSetProperty( ci, "OtherEnabledState", "NULL", CMPI_chars);
    CMSetProperty( ci, "RequestedState", (CMPIValue*)&(status), CMPI_uint16);
    CMSetProperty( ci, "EnabledDefault", (CMPIValue*)&(status), CMPI_uint16);

    if(isVIOS())
    {
        dedicated = 32568; // it's a VIOS system
    }
    dedic = CMNewArray(_broker,1,CMPI_uint16A,rc);
    CMSetArrayElementAt(dedic,0,(CMPIValue*)&(dedicated),CMPI_uint16);
    CMSetProperty( ci, "Dedicated", (CMPIValue*)&(dedic), CMPI_uint16A);

    {
        /* new added and modified properties for smis */
        char * hostingName = getHostingSystemName();
        char * strOtherIdenInfo = NULL;
        char * strIdenDesc = NULL;
        char * elementName = getElementName();

        CMSetProperty( ci, "ElementName", elementName, CMPI_chars);
        CMSetProperty( ci, "HostingSystemNameFormat", "Other", CMPI_chars);
        CMSetProperty( ci, "HostingSystemName", hostingName, CMPI_chars);
        free(elementName);
        free(hostingName);
    
        if( 0 == getIdentifyingInfo(&strOtherIdenInfo, &strIdenDesc)){
            CMPIArray * otherIdentifyingInfo = NULL;
            CMPIArray * identifyingDesc = NULL;
            CMPIString * str = NULL;
            /* set OtherIdentifyingInfo and IdentifyingDescription properties */
            otherIdentifyingInfo = CMNewArray(_broker, 1, CMPI_string, rc);
            identifyingDesc = CMNewArray(_broker, 1, CMPI_string, rc);
            if ( NULL == otherIdentifyingInfo || NULL == identifyingDesc ){
                CMSetStatusWithChars(_broker, rc, CMPI_RC_ERR_FAILED, "CMNewArray(_broker, 1, CMPI_String, rc)");
                _OSBASE_TRACE(2, ("---_makeInst_ComputerSystem() failed: %s",CMGetCharPtr(rc->msg)));
                goto exit;
            }

            str = CMNewString(_broker, strOtherIdenInfo, rc);
            CMSetArrayElementAt(otherIdentifyingInfo, 0, (CMPIValue*)&str, CMPI_string);
            CMSetProperty(ci, "OtherIdentifyingInfo", (CMPIValue*)&otherIdentifyingInfo, CMPI_stringA);
   
            str = CMNewString(_broker, strIdenDesc, rc);
            CMSetArrayElementAt(identifyingDesc, 0, (CMPIValue*)&str, CMPI_string);
            CMSetProperty(ci, "IdentifyingDescriptions", (CMPIValue*)&identifyingDesc, CMPI_stringA);
            free(strOtherIdenInfo);
            /* the identifying description returned by getIdentifyingInfo is a const string, no need to free */
        }
    }
#endif

 exit:
    free(system_name);
    _OSBASE_TRACE(2,("--_makeInst_ComputerSystem() exited" ));
    return ci;
}

/* Get the identifying information and description.
 * The format of them is:
 * For a ComputerSystem instance for a LPAR:
 *   OtherIdentifyingInfo[i]: string value = IBM-Type-Model-SerialNumber-LPARID
 *   IdentifyingDescriptions[i]: string = "Vendor-Type-Model-SerialNumber-LPARID"
 * For a ComputerSystem instance for a non-LPAR system (also applies to the hosting system):
 *   OtherIdentifyingInfo[i]: string value = IBM-Type-Model-SerialNumber
 *   IdentifyingDescriptions[i]: string = "Vendor-Type-Model-SerialNumber"
 */ 
static int getIdentifyingInfo(char ** identifyingInfo, char ** identifyingInfoDesc)
{
    char * idenInfo;
    struct cim_baseboard data;
    int returnval = -1;
    _OSBASE_TRACE(3,("--getIdentifyingInfo() called"));
    if(get_baseboard_data(&data)){
        _OSBASE_TRACE(3,("--get_baseboard_data() failed"));
    }
    else if(data.type && data.model && data.serialNumber ){
        int len = 4 + strlen(data.type) + strlen(data.model) + strlen(data.serialNumber)
            + 10 + 4; /* the maximum int has 10 digits, 4 for "IBM-", and another 3 for hyphen, 1 for NULL */
        char lparid[255];

        * identifyingInfo = (char*) malloc(len);
        if( 0 == get_cs_lparid(lparid, sizeof(lparid)) ){
            * identifyingInfoDesc = "Vendor-Type-Model-SerialNumber-LPARID";
            snprintf(*identifyingInfo, len, "IBM-%s-%s-%s-%s",data.type, data.model, data.serialNumber, lparid);
        }
        else{
            * identifyingInfoDesc = "Vendor-Type-Model-SerialNumber";
            snprintf(*identifyingInfo, len, "IBM-%s-%s-%s",data.type, data.model, data.serialNumber);
        }
        returnval = 0;
    }
    free_baseboard_data(&data);
    _OSBASE_TRACE(3,("--getIdentifyingInfo() exited"));
    return returnval;
}

/*
 * judge the system is an Linux system or VIOS system.
 * if the command 'ioscli ioslevel' exists, then regard it as VIOS, else
 * regard it as an Linux system.
 * the 'ioslevel' command on VIOS is a alias of 'ioscli ioslevel'!
 * parameter: char** version[input/output]: if it's VIOS system and the value of version isn't NULL,
 *            then the pointer will point to a buffer that hold the version of VIOS after the function running. 
 * return value: non zero - the system is VIOS
 *               0 - the system is an Linux 
 */
static int isVIOS()
{
    return !exec_command("ioscli ioslevel 2>/dev/null", NULL, NULL, NULL);
}

/*
 * return the buf hold Hosting Name if the system with LPAR, otherwise NULL
 * the Hosting Name format is SystemType-SystemModel-SystemSerialNubmer
 */
static char* getHostingSystemName()
{
    char lparid[255];

    _OSBASE_TRACE(4,("--getHostingSystemName() called."));
    if ( 0 == get_cs_lparid(lparid, sizeof(lparid)) ){
        char* hostingName = NULL;
        int namesz = 0; /* the size of hosting system name */
        struct cim_baseboard data;
        if( get_baseboard_data( &data)) {
            _OSBASE_TRACE(4,("--getHostingSystemName() exited."));
            return NULL; 
        }
        if (data.type && data.model && data.serialNumber ){
            namesz = strlen( data.type) + strlen( data.model) + strlen( data.serialNumber) + 7;
            /* 3 for 'IBM', 3 for '-', 1 for NULL */
            hostingName = ( char* ) malloc( namesz);
            snprintf( hostingName, namesz, "IBM-%s-%s-%s", data.type, data.model, data.serialNumber);
            _OSBASE_TRACE(4,("--getHostingSystemName() exited."));
            return hostingName;
        }
        _OSBASE_TRACE(4,("--getHostingSystemName() exited."));
        return NULL;
    }
    /* the system withour LPAR or some error occured */
    _OSBASE_TRACE(4,("--getHostingSystemName() exited."));
    return NULL;
}

/* the format of ElementName is 'IBM,<model>-<type>'
 * the information can be get from ODM CuAt of sys0
 */
static char * getElementName()
{
    char lparid[255];

    _OSBASE_TRACE(4,("--getElementName() called."));
    if ( 0 == get_cs_lparid(lparid, sizeof(lparid)) ){
        char* retval = NULL;
        int namesz = 0; /* the size of hosting system name */
        struct cim_baseboard data;
        if( get_baseboard_data( &data)) {
            _OSBASE_TRACE(4,("--getElementName() exited."));
            return NULL; 
        }
        if (data.type && data.model && data.serialNumber ){
            namesz = strlen( data.type) + strlen( data.model) + 6;
            /* 4 for 'IBM,', 1 for '-', 1 for NULL */
            retval = ( char* ) malloc( namesz);
            snprintf( retval, namesz, "IBM,%s-%s", data.model, data.type);
            _OSBASE_TRACE(4,("--getElementName() exited."));
            return retval;
        }
        _OSBASE_TRACE(4,("--getElementName() exited."));
        return NULL;
    }
    /* the system withour LPAR or some error occured */
    _OSBASE_TRACE(4,("--getElementName() exited."));
    return NULL;
}
/* ---------------------------------------------------------------------------*/
/*          end of cmpiOSBase_ComputerSystem.c                      */
/* ---------------------------------------------------------------------------*/
