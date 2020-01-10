/*
 * $Id: Linux_CommonHBA.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2006, 2007, 2009
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
 *      Common HBA routines.
*/


#include "Linux_Common.h"
#include "Linux_CommonHBA.h"

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <pthread.h>

static pthread_mutex_t hba_mutex = PTHREAD_MUTEX_INITIALIZER;

int hbamutex_lock()
{
    return pthread_mutex_lock(&hba_mutex);
}

int hbamutex_unlock()
{
    return pthread_mutex_unlock(&hba_mutex);
}

/* ---------------------------------------------------------------------------*/

void trace_port_attributes(HBA_PORTATTRIBUTES * portattributes)
{
    int j;
    _OSBASE_TRACE(4,("        NodeWWN = %llx",
           *(unsigned long long*)portattributes->NodeWWN.wwn));

    _OSBASE_TRACE(4,("        PortWWN = %llx",
           *(unsigned long long*)portattributes->PortWWN.wwn));

    _OSBASE_TRACE(4,("        PortSymbolicName = %s",
           portattributes->PortSymbolicName));

    _OSBASE_TRACE(4,("        PortMaxFrameSize = %d",
           portattributes->PortMaxFrameSize));

    _OSBASE_TRACE(4,("        PortSupportedSpeed = %d",
           portattributes->PortSupportedSpeed));

    _OSBASE_TRACE(4,("        PortSupportedFc4Types:"));

    for (j = 0; j < 32; j++)
    {

        _OSBASE_TRACE(4,("            PortSupportedFc4Types.bits[j] = %02x",
             portattributes->PortSupportedFc4Types.bits[j]));
    }


    _OSBASE_TRACE(4,("        PortActiveFc4Types:"));
    for (j = 0; j < 32; j++)
    {
        _OSBASE_TRACE(4,("            PortActiveFc4Types.bits[j] = %02x",
             portattributes->PortActiveFc4Types.bits[j]));
    }

    _OSBASE_TRACE(4,("        PortState = %d",
           portattributes->PortState));

    _OSBASE_TRACE(4,("        PortType = %d",
           portattributes->PortType));

    _OSBASE_TRACE(4,("        OSDeviceName = %s",
           portattributes->OSDeviceName));

    _OSBASE_TRACE(4,("        PortSuportedClassofService = %d",
           portattributes->PortSupportedClassofService));

    _OSBASE_TRACE(4,("        PortFcId = %x",
           portattributes->PortFcId));

    _OSBASE_TRACE(4,("        PortSpeed = %d",
           portattributes->PortSpeed));

    _OSBASE_TRACE(4,("        NumberOfDiscoveredPorts = %d",
           portattributes->NumberofDiscoveredPorts));

    _OSBASE_TRACE(4,("        FabricName = %llx",
           *(unsigned long long*)portattributes->FabricName.wwn));
}

static void trace_port_statistics(
  HBA_PORTSTATISTICS *  portstatistics)
{

  _OSBASE_TRACE(4,("        SecondsSinceLastReset = %llu",
      portstatistics->SecondsSinceLastReset));

  _OSBASE_TRACE(4,("        TxFrames  = %llu",
      portstatistics->TxFrames));

  _OSBASE_TRACE(4,("        TxWords = %llu",
      portstatistics->TxWords));

  _OSBASE_TRACE(4,("        RxFrames = %llu",
      portstatistics->RxFrames));

  _OSBASE_TRACE(4,("        RxWords = %llu",
      portstatistics->RxWords));

  _OSBASE_TRACE(4,("        LIPCount  = %llu",
      portstatistics->LIPCount));

  _OSBASE_TRACE(4,("        NOSCount = %llu",
      portstatistics->NOSCount));

  _OSBASE_TRACE(4,("        ErrorFrames = %llu",
      portstatistics->ErrorFrames));

  _OSBASE_TRACE(4,("        DumpedFrames = %llu",
      portstatistics->DumpedFrames));

  _OSBASE_TRACE(4,("        LinkFailureCount = %llu",
      portstatistics->LinkFailureCount));

  _OSBASE_TRACE(4,("        LossOfSyncCount = %llu",
      portstatistics->LossOfSyncCount));

  _OSBASE_TRACE(4,("        LossOfSignalCount= %llu",
      portstatistics->LossOfSignalCount));

  _OSBASE_TRACE(4,("        PrimitiveSeqProtocolErrCount= %llu",
      portstatistics->PrimitiveSeqProtocolErrCount));

  _OSBASE_TRACE(4,("        InvalidTxWordCount = %llu",
      portstatistics-> InvalidTxWordCount));

  _OSBASE_TRACE(4,("        InvalidCRCCount = %llu",
      portstatistics-> InvalidCRCCount));

}

static void trace_adapter_attributes(
  HBA_ADAPTERATTRIBUTES *  adapterattributes)
{
  _OSBASE_TRACE(4,("        Manufacturer  = %s",
         adapterattributes->Manufacturer));

  _OSBASE_TRACE(4,("        Serial Number  = %s",
         adapterattributes->SerialNumber));

  _OSBASE_TRACE(4,("        Model  = %s",
         adapterattributes->Model));

  _OSBASE_TRACE(4,("        Model Description = %s",
         adapterattributes->ModelDescription));

  _OSBASE_TRACE(4,("        Node WWN  = %llu",
         adapterattributes->NodeWWN));

  _OSBASE_TRACE(4,("        Node Symbolic Name = %s",
         adapterattributes->NodeSymbolicName));

  _OSBASE_TRACE(4,("        Hardware Version = %s",
         adapterattributes->HardwareVersion));

  _OSBASE_TRACE(4,("        Driver Version= %s",
         adapterattributes->DriverVersion));

  _OSBASE_TRACE(4,("        Option ROM Version = %s",
         adapterattributes->OptionROMVersion));

  _OSBASE_TRACE(4,("        Firmware Version = %s",
         adapterattributes->FirmwareVersion));

  _OSBASE_TRACE(4,("        Vendor Specific ID = %d",
         adapterattributes->VendorSpecificID));

  _OSBASE_TRACE(4,("        Number Of Ports = %d",
         adapterattributes->NumberOfPorts));

  _OSBASE_TRACE(4,("        Driver Name = %s",
         adapterattributes->DriverName));

}

void trace_LogicalDisks(struct hbaLogicalDiskList **dptr )
{
   struct hbaLogicalDiskList *d = *dptr;
   int i = 0;
   for ( ; d ; d = d->next){ 
       _OSBASE_TRACE(4,(" node number = %d, node pointer = %p", i, d ));
       _OSBASE_TRACE(4,("        sptr = %p,", d->sptr));

       _OSBASE_TRACE(4,("        OSDeviceName = %s",
              d->sptr->OSDeviceName));

       _OSBASE_TRACE(4,("        ScsiBusNumber = %d",
              d->sptr->ScsiBusNumber));

       _OSBASE_TRACE(4,("        ScsiTargetNumber = %d",
              d->sptr->ScsiTargetNumber));
    
       _OSBASE_TRACE(4,("        ScsiOSLun = %s",
              d->sptr->ScsiOSLun));

       _OSBASE_TRACE(4,("        FcId = %d",
              d->sptr->FcId));
    
       _OSBASE_TRACE(4,("        NodeWWN = %llx",
              *(unsigned long long*)d->sptr->NodeWWN.wwn));
    
       _OSBASE_TRACE(4,("        PortWWN = %llx",
              *(unsigned long long*)d->sptr->PortWWN.wwn));
    
       _OSBASE_TRACE(4,("        FcpLun = %llx",
              d->sptr->FcpLun));

       _OSBASE_TRACE(4,("        buffer = %c256",
              d->sptr->buffer));

       _OSBASE_TRACE(4,("        next = %p,", d->next));
       _OSBASE_TRACE(4,("--- node number = %d,", i ));
       i++;
   }
   _OSBASE_TRACE(4,("number of entries is = %d", i));
             
}

static int _hbaAdapter_data(
  int                      adapter_number,
  char                  *  InstanceID,
  char                  *  adapter_name,
  HBA_ADAPTERATTRIBUTES *  adapterattributes,
  struct cim_hbaAdapter ** sptr )
{
  int                      rc    = 0;

  _OSBASE_TRACE(1,("--- _hbaAdapter_data() called"));

  (*sptr) = (struct cim_hbaAdapter*) calloc (1,sizeof(struct cim_hbaAdapter));
  (*sptr)->adapter_attributes = adapterattributes;
  (*sptr)->adapter_name  = adapter_name;
  (*sptr)->adapter_number  = adapter_number;
  (*sptr)->InstanceID         = InstanceID;

  _OSBASE_TRACE(1,("--- _hbaAdapter_data() exited"));
  return rc;
}

static int _hbaPort_data(
  int                   port_number,
  int                   adapter_number,
  char               *  InstanceID,
  HBA_PORTATTRIBUTES *  portattributes,
  HBA_PORTSTATISTICS *  portstatistics,
  struct cim_hbaPort ** sptr )
{
  int                   rc    = 0;

  _OSBASE_TRACE(1,("--- _hbaPort_data () called"));

  (*sptr)->port_attributes    = portattributes;
  (*sptr)->port_statistics    = portstatistics;
  (*sptr)->adapter_number     = adapter_number;
  (*sptr)->InstanceID         = InstanceID;
  (*sptr)->port_number        = port_number;

  /* the spec for this class indicates that

      bytes_transmitted  = TxWords * 4
      bytes_received     = RxWords * 4
  */
  if(NULL != portstatistics)
  {
      (*sptr)->bytes_transmitted  = (unsigned long long)
                                (portstatistics->TxWords * 4);
      (*sptr)->bytes_received = (unsigned long long)
                                (portstatistics->RxWords * 4);
 
  }

  _OSBASE_TRACE(1,("--- _hbaAdapter_data() exited"));
  return rc;
}

static int _hbaLogicalDisk_data(
  HBA_FCPTARGETMAPPINGV2    ** pMapping,
  int                       location, 
  HBA_WWN                   InitiatorPortWWN,
  struct cim_hbaLogicalDisk ** sptr,
  struct hbaLogicalDiskList ** dlp) 
{

  int                   rc = 0;
  _OSBASE_TRACE(1,("--- _hbaLogicalDisk_data () called"));
  _OSBASE_TRACE(4,("--- first tracing of dlp in _hbaLogicalDisk_data"));
  trace_LogicalDisks(dlp);


  (*sptr) = (struct cim_hbaLogicalDisk*) calloc(1,sizeof(struct cim_hbaLogicalDisk));
  (*sptr)->OSDeviceName = (char *) malloc(DEVICEID_SIZE);
  (*sptr)->buffer = (char *) malloc(BUFFER_SIZE);

  strncpy((*sptr)->OSDeviceName, 
          (*pMapping)->entry[location].ScsiId.OSDeviceName, 
          DEVICEID_SIZE);
  (*sptr)->OSDeviceName[DEVICEID_SIZE - 1] = '\0';

  _OSBASE_TRACE(4,("--- _hbaLogicalDisk_data set OSDeviceName"));
  
  (*sptr)->InitiatorPortWWN = InitiatorPortWWN;
  _OSBASE_TRACE(4,("--- _hbaLogicalDisk_data set InitiatorPortWWN"));
  
  (*sptr)->ScsiBusNumber      = (*pMapping)->entry[location].ScsiId.ScsiBusNumber;
  _OSBASE_TRACE(4,("--- _hbaLogicalDisk_data set ScsiBusNumber"));

  (*sptr)->ScsiTargetNumber   = (*pMapping)->entry[location].ScsiId.ScsiTargetNumber;
  _OSBASE_TRACE(4,("--- _hbaLogicalDisk_data set ScsiTargetNumber"));

  (*sptr)->ScsiOSLun          = (*pMapping)->entry[location].ScsiId.ScsiOSLun;
  _OSBASE_TRACE(4,("--- _hbaLogicalDisk_data set ScsiOSLun"));

  (*sptr)->FcId               = (*pMapping)->entry[location].FcpId.FcId;
  _OSBASE_TRACE(4,("--- _hbaLogicalDisk_data set FcId"));

  (*sptr)->NodeWWN            = (*pMapping)->entry[location].FcpId.NodeWWN;
  _OSBASE_TRACE(4,("--- _hbaLogicalDisk_data set NodeWWN"));

  (*sptr)->PortWWN            = (*pMapping)->entry[location].FcpId.PortWWN;
  _OSBASE_TRACE(4,("--- _hbaLogicalDisk_data set PortWWN"));

  (*sptr)->FcpLun             = (*pMapping)->entry[location].FcpId.FcpLun;
  _OSBASE_TRACE(4,("--- _hbaLogicalDisk_data set FcpLun"));
   
  strncpy((*sptr)->buffer, 
          (*pMapping)->entry[location].LUID.buffer,
          BUFFER_SIZE);
  (*sptr)->buffer[BUFFER_SIZE - 1] = '\0';
  
  _OSBASE_TRACE(4,("--- _hbaLogicalDisk_data set buffer"));

  _OSBASE_TRACE(4,("--- tracing of dlp in _hbaLogicalDisk_data"));
  trace_LogicalDisks(dlp);

  
  _OSBASE_TRACE(1,("--- _hbaLogicalDisk_data() exited"));
  return rc;
}


/* This is NOT thread safe */
int get_info_for_one_port(
  HBA_HANDLE            adapter_handle,
  int                   port_index,
  int                   get_statistics,
  HBA_PORTATTRIBUTES  * portattributes,
  HBA_PORTSTATISTICS  * portstatistics,
  struct cim_hbaPort  ** sptr  )
{
  int  rc  = HBA_STATUS_OK;
  struct timeval tv;
  struct timezone tz;
  unsigned long long msecs;
    
  (*sptr) = (struct cim_hbaPort*)calloc (1,sizeof(struct cim_hbaPort));  
    
  _OSBASE_TRACE(1,("--- _get_info_for_one_port () called"));
  _OSBASE_TRACE(4,("--- HBA_GetAdapterPortAttributes () called for port = %d",
                port_index));
  rc = HBA_GetAdapterPortAttributes(adapter_handle,
                port_index, portattributes);
  _OSBASE_TRACE(4,("--- HBA_GetAdapterPortAttributes () rc = %d",
                rc));
  if ((rc == HBA_STATUS_OK) && (get_statistics == 1))
  {

      trace_port_attributes(portattributes);
     _OSBASE_TRACE(3,("--- HBA_GetPortStatistics () called for port = %d",
                port_index));
                
     gettimeofday(&tv, &tz);
     msecs = 1000000 * ((unsigned long long)tv.tv_sec) + (unsigned long long)(tv.tv_usec);
     (*sptr)->statistic_time = msecs;                
                
     rc = HBA_GetPortStatistics(adapter_handle,
           port_index, portstatistics);
     _OSBASE_TRACE(4,("--- HBA_GetPortStatistics () rc  = %d",
                rc));
     if (rc == HBA_STATUS_OK)
     {
        trace_port_statistics(portstatistics);
     }
     else
     {
        _OSBASE_TRACE(2,
            ("--- HBA_GetPortStatistics () not working"));
     }
  }

  _OSBASE_TRACE(1,("--- _get_info_for_one_port () exited"));
  return rc;
}

/* This is NOT thread safe */
int get_info_for_one_adapter(
     int                     adapter_index,
     char                  * adapter_name,
     HBA_ADAPTERATTRIBUTES * adapterattributes,
     HBA_HANDLE            * handle,
     int                     do_close )
{

    int                      rc = 0;

    _OSBASE_TRACE(1,("--- _get_info_for_one_adapter () called"));
    _OSBASE_TRACE(3,("--- HBA_GetAdapterName () called for adapter = %d",
                adapter_index));
    rc = HBA_GetAdapterName(adapter_index, adapter_name);
    _OSBASE_TRACE(4,
          ("--- HBA_GetAdapterName ) rc = %d - adapter_name = %s",
          rc,adapter_name));
    if(rc == HBA_STATUS_OK)
    {
        _OSBASE_TRACE(3,
           ("--- HBA_OpenAdapter () called for adapter = %s",adapter_name));
        *handle = HBA_OpenAdapter(adapter_name);
        if(*handle > 0)
        {
            _OSBASE_TRACE(4,
               ("--- HBA_OpenAdapter () successful = handle = %d",*handle));
        }
        else
        {
            rc = HBA_STATUS_ERROR_INVALID_HANDLE;
            _OSBASE_TRACE(4,
               ("--- HBA_OpenAdapter () failed - setting rc = %d.",rc));
        }

    }

    if (rc == HBA_STATUS_OK)
    {
       _OSBASE_TRACE(1,("--- HBA_GetAdapterAttributes () called for handle = %d",
                *handle));
       rc = HBA_GetAdapterAttributes(*handle, adapterattributes);
       _OSBASE_TRACE(4,("--- HBA_GetAdapterAttributes () rc  = %d",
                rc));
       /* Defect 563943 */
       if (adapterattributes->NumberOfPorts > 0) {
          int rc = HBA_STATUS_OK;
          HBA_PORTATTRIBUTES  portattributes;
          char * command; 
          int len = 0;
          char ** hdout = NULL;
          char *line;
          char *version;
          int i;

          /* Gather port attributes to figure out osdevice name */
          _OSBASE_TRACE(4,("--- HBA_GetAdapterPortAttributes () called for port = %d", 0));
          rc = HBA_GetAdapterPortAttributes(*handle, 0 , &portattributes);
          _OSBASE_TRACE(4,("--- HBA_GetAdapterPortAttributes () rc = %d", rc));
          if (rc == HBA_STATUS_OK) {

            /* Use the OSDeviceName to determine the firmware version */
            _OSBASE_TRACE(4,("--- OSDeviceName = %s", portattributes.OSDeviceName));
            len = strlen("lsmcode -r -d ") + strlen(portattributes.OSDeviceName) + 1;
            command = (char*) malloc(len);
            snprintf(command, len, "lsmcode -r -d %s", portattributes.OSDeviceName);
            rc = runcommand(command,NULL,&hdout,NULL);
            free(command);
            if (hdout[0] != NULL)
            {
               line = strdup(hdout[0]);
               _OSBASE_TRACE(4,("--- output of lsmcode = %s", line));
               /* Copy from . to end */
               version = strchr(line, '.');
               if (version != NULL)
               {
                 /* Bump pass the . */
                 version++;
                 i = strlen(version) - 1;
                 /* If the last char is a newline remove it */
                 if (version[i] == '\n') 
                 {
                   version[i] = '\0'; 
                 }

                 /* Replace it */
                 _OSBASE_TRACE(4,("--- old    version = %s", adapterattributes->FirmwareVersion));
                 _OSBASE_TRACE(4,("--- parsed version = %s", version));
                 strncpy(adapterattributes->FirmwareVersion,version,256);
                 adapterattributes->FirmwareVersion[255] = '\0';
               }
              free(line);
            }
            freeresultbuf(hdout);
          }
       }
       if (rc == HBA_STATUS_OK)
       {
          trace_adapter_attributes(adapterattributes);
       }

    }
    /***********************************************
    ************** HBA_CloseAdapter ***************
    ***********************************************/
    if ((*handle != 0) && (do_close == 1))
    {
       _OSBASE_TRACE(1,("--- HBA_CloseAdapter () called for handle = %d",
                *handle));
        HBA_CloseAdapter(*handle);
    }
    else
    {
       _OSBASE_TRACE(1,("--- HBA_CloseAdapter () NOT called for handle = %d",
                *handle));
    }

    _OSBASE_TRACE(1,("--- _get_info_for_one_adapter () exited"));
    return rc;
;
}


int enum_all_targetPorts(struct hbaPortList ** lptr,
                         int get_statistics)
{
  struct hbaPortList    * hlp   = NULL;
  int                     adapter_number;
  char                  * adapter_name = NULL;
  int                     numberofadapters;
  HBA_HANDLE              handle;
  HBA_ADAPTERATTRIBUTES * adapter_attributes = NULL;
  HBA_PORTATTRIBUTES    * port_attributes = NULL;
  HBA_PORTSTATISTICS    * port_statistics = NULL;
  HBA_PORTATTRIBUTES    * targetport_attributes = NULL;
  int                     rc    = 0;
  unsigned int                     port_counter = 0;
  int                     InstanceID_len;
  char                  *InstanceID;
  unsigned int discovered_portindex = 0;
  _OSBASE_TRACE(1,("--- enum_all_targetPorts() called"));
  hbamutex_lock();
  _OSBASE_TRACE(1,("--- HBA_LoadLibary () called."));
  rc = HBA_LoadLibrary();
  _OSBASE_TRACE(4,("--- HBA_LoadLibrary () rc  = %d", rc));
  if (rc == HBA_STATUS_OK)
  {
      _OSBASE_TRACE(1,("--- HBA_GetNumberOfAdapters () called."));

      numberofadapters = HBA_GetNumberOfAdapters();

      _OSBASE_TRACE(4,("--- HBA_NumberOfAdapters () = %d", numberofadapters));
          
          if (numberofadapters > 0)
          {
                  adapter_attributes = (HBA_ADAPTERATTRIBUTES *)
                          malloc(sizeof(HBA_ADAPTERATTRIBUTES));
                  adapter_name = (char *) malloc(HBA_MAX_DEV_NAME + 1);

                  for (adapter_number=0;adapter_number<numberofadapters;adapter_number++)
                  {
                          handle = 0;
                          rc = get_info_for_one_adapter(adapter_number,
                                  adapter_name,
                                  adapter_attributes,
                                  &handle,
                                  0);
                          if ( rc != HBA_STATUS_OK )
                          {
                                  continue;  /* processing the next adapter*/
                          }

                          for (port_counter=0;
                                  port_counter<adapter_attributes->NumberOfPorts;
                                  port_counter++)
                          {
                                port_attributes = (HBA_PORTATTRIBUTES *)malloc(sizeof(HBA_PORTATTRIBUTES));
                                memset(port_attributes, 0, sizeof(HBA_PORTATTRIBUTES));
                                          
                                rc = HBA_GetAdapterPortAttributes(handle, port_counter, port_attributes);
                                _OSBASE_TRACE(4,("--- HBA_GetAdapterPortAttributes () rc = %d", rc));
                                if (rc == HBA_STATUS_OK)
                                {
                                        trace_port_attributes(port_attributes);
                                        _OSBASE_TRACE(3,("--- HBA_GetPortStatistics () called for port = %d", port_counter));
                                }
                                
                                for(discovered_portindex = 0; discovered_portindex < port_attributes->NumberofDiscoveredPorts; discovered_portindex++)
                                {
                                      HBA_PORTATTRIBUTES *targetport_attributes = (HBA_PORTATTRIBUTES *)
                                                                                            malloc(sizeof(HBA_PORTATTRIBUTES));
                                    memset(targetport_attributes, 0, sizeof(HBA_PORTATTRIBUTES));
                                    
                                    rc = HBA_GetDiscoveredPortAttributes(
                                                                         handle,
                                                                         port_counter,
                                                                         discovered_portindex,
                                                                         targetport_attributes); 
                                    if ( rc != HBA_STATUS_OK )
                                    {
                                        if ( targetport_attributes ) free(targetport_attributes);
                                        continue;  /*processing the next port*/
                                    }
                                    if ( hlp == NULL )
                                    {
                                        hlp = (struct hbaPortList *)malloc (sizeof(struct hbaPortList));
                                        memset(hlp, 0, sizeof(struct hbaPortList));
                                        *lptr = hlp;
                                    }
                                    if ( hlp->sptr != NULL)
                                    {
                                        hlp->next = (struct hbaPortList*) calloc (1,sizeof(struct hbaPortList));
                                        hlp = hlp->next;
                                    }
                                    hlp->sptr = (struct cim_hbaPort*) calloc (1,sizeof(struct cim_hbaPort));
                                    InstanceID_len = 18;
                                    InstanceID = (char *) malloc(InstanceID_len);
                                    snprintf(InstanceID, InstanceID_len, "%llx", *(unsigned long long*)targetport_attributes->PortWWN.wwn);
                                    rc = _hbaPort_data(
                                                       discovered_portindex,
                                                       adapter_number,
                                                       InstanceID,
                                                       targetport_attributes,
                                                       port_statistics,
                                                       &(hlp->sptr));
                                    hlp->sptr->role = 3;/*3 means target*/
                                    trace_port_attributes(targetport_attributes);                                                                        
                                }
                                free(port_attributes);
                          }
                          if (handle != 0)
                          {
                              _OSBASE_TRACE(1, ("--- HBA_CloseAdapter () called for handle = %d", handle));
                              HBA_CloseAdapter(handle);
                          }
                          handle = 0;

                  }

                  /* free adapter_name and adapter_attribute */
                  if ( adapter_name ) free(adapter_name);
                  if ( adapter_attributes ) free (adapter_attributes);
          }

          _OSBASE_TRACE(1,("--- HBA_FreeLibrary () called."));
          rc = HBA_FreeLibrary();
          _OSBASE_TRACE(4,("--- HBA_FreeLibrary () rc  = %d", rc));
  }

  _OSBASE_TRACE(1,("--- enum_all_targetPorts() exited"));
  hbamutex_unlock();
  return rc;

}

int enum_all_hbaPorts(
  struct hbaPortList **   lptr,
  int                     get_statistics )
{
  struct hbaPortList    * hlp   = NULL;
  int                     adapter_number;
  char                  * adapter_name = NULL;
  int                     numberofadapters;
  HBA_HANDLE              handle;
  HBA_ADAPTERATTRIBUTES * adapter_attributes = NULL;
  HBA_PORTATTRIBUTES    * port_attributes = NULL;
  HBA_PORTSTATISTICS    * port_statistics = NULL;
  int                     rc    = 0;
  unsigned int                     port_counter = 0;
  int                     InstanceID_len;
  char                  * InstanceID;

  _OSBASE_TRACE(1,("--- enum_all_hbaPorts() called"));
  hbamutex_lock();
  _OSBASE_TRACE(1,("--- HBA_LoadLibary () called."));
  rc = HBA_LoadLibrary();
  _OSBASE_TRACE(4,("--- HBA_LoadLibrary () rc  = %d",
                rc));
  if (rc == HBA_STATUS_OK)
  {
      _OSBASE_TRACE(1,("--- HBA_GetNumberOfAdapters () called."));

      numberofadapters = HBA_GetNumberOfAdapters();

      _OSBASE_TRACE(4,("--- HBA_NumberOfAdapters () = %d",numberofadapters));
          
          if (numberofadapters > 0)
          {
                  adapter_attributes = (HBA_ADAPTERATTRIBUTES *)
                          malloc(sizeof(HBA_ADAPTERATTRIBUTES));
                  adapter_name = (char *) malloc(HBA_MAX_DEV_NAME + 1);

                  for (adapter_number=0;adapter_number<numberofadapters;adapter_number++)
                  {
                          handle = 0;
                          rc = get_info_for_one_adapter(adapter_number,
                                  adapter_name,
                                  adapter_attributes,
                                  &handle,
                                  0);
                          if ( rc != HBA_STATUS_OK )
                          {
                                  continue;  // processing the next adapter
                          }

                          for (port_counter=0;
                                  port_counter<adapter_attributes->NumberOfPorts;
                                  port_counter++)
                          {
                                  port_attributes = (HBA_PORTATTRIBUTES *)
                                          malloc(sizeof(HBA_PORTATTRIBUTES));
                                  if (get_statistics == 1)
                                  {
                                          port_statistics = (HBA_PORTSTATISTICS *)
                                                  malloc(sizeof(HBA_PORTSTATISTICS));
                                  }
                                  else
                                  {
                                          port_statistics = NULL;
                                  }

                                  if ( hlp == NULL )
                                  {
                                          hlp = (struct hbaPortList *)
                                                  malloc (sizeof(struct hbaPortList));
                                          memset(hlp, 0, sizeof(struct hbaPortList));
                                          *lptr = hlp;
                                  }
                                  if ( hlp->sptr != NULL)
                                  {
                                          hlp->next = (struct hbaPortList*) calloc (1,sizeof(struct hbaPortList));
                                          hlp = hlp->next;
                                  }

                                  rc = get_info_for_one_port (handle,
                                          port_counter,
                                          get_statistics,
                                          port_attributes,
                                          port_statistics,
                                          &(hlp->sptr));
                                  if ( rc != HBA_STATUS_OK )
                                  {
                                          if ( port_attributes ) free(port_attributes);
                                          if ( port_statistics ) free(port_statistics);
                                  	  /*add by liuhongl at 5/8, 2007 */
                                  	  if (hlp->sptr) free(hlp->sptr);
                                  	  hlp->sptr = NULL;
                                          continue;  // processing the next port
                                  }

                                  /* 18 = length of portWWN + lineender */
                                  InstanceID_len = 18;
                                  InstanceID = (char *) malloc(InstanceID_len);
                                  snprintf(InstanceID,InstanceID_len,
                                          "%llx",*(unsigned long long*)port_attributes->PortWWN.wwn);

                                  rc = _hbaPort_data(port_counter,
                                          adapter_number,
                                          InstanceID,
                                          port_attributes,
                                          port_statistics,
                                          &(hlp->sptr));
                                  hlp->sptr->role = 2;  /*2 means initiator*/


                          }
                          if (handle != 0)
                          {
                                  _OSBASE_TRACE(1,
                                          ("--- HBA_CloseAdapter () called for handle = %d",
                                          handle));
                                  HBA_CloseAdapter(handle);
                          }
                          handle = 0;

                  }

                  /* free adapter_name and adapter_attribute */
                  if ( adapter_name ) free(adapter_name);
                  if ( adapter_attributes ) free (adapter_attributes);

          }

          _OSBASE_TRACE(1,("--- HBA_FreeLibrary () called."));
          rc = HBA_FreeLibrary();
          _OSBASE_TRACE(4,("--- HBA_FreeLibrary () rc  = %d", rc));
  }

  _OSBASE_TRACE(1,("--- enum_all_hbaPorts() exited"));
  hbamutex_unlock();
  return rc;

}

int enum_all_hbaAdapters( struct hbaAdapterList ** lptr )
{
  struct hbaAdapterList *  hlp   = NULL;
  HBA_HANDLE  handle;
  int adapter_number;
  char * adapter_name = NULL;
  int numberofadapters;
  HBA_ADAPTERATTRIBUTES * adapter_attributes;
  int                   rc    = 0;
  int                     InstanceID_len;
  char                  * InstanceID;
  char                  * system_name = NULL; /* save pointer returned by get_system_name */

  _OSBASE_TRACE(1,("--- enum_all_hbaAdapters() called"));

  hbamutex_lock();
  _OSBASE_TRACE(1,("--- HBA_LoadLibary () called."));
  rc = HBA_LoadLibrary();
  _OSBASE_TRACE(4,("--- HBA_LoadLibrary () rc  = %d",
                rc));
  if (rc == 0)
  {

      _OSBASE_TRACE(1,("--- HBA_GetNumberOfAdapters () called."));

      numberofadapters = HBA_GetNumberOfAdapters();

      _OSBASE_TRACE(4,("--- HBA_NumberOfAdapters () = %d",numberofadapters));

          if (numberofadapters > 0)
          {
                  for (adapter_number=0;adapter_number<numberofadapters;adapter_number++)
                  {
                          adapter_attributes = (HBA_ADAPTERATTRIBUTES *)
                                  malloc(sizeof(HBA_ADAPTERATTRIBUTES));
                          adapter_name = (char *) malloc(HBA_MAX_DEV_NAME + 1);

                          rc = get_info_for_one_adapter(adapter_number,adapter_name,adapter_attributes,&handle,1);
                          if ( rc != HBA_STATUS_OK )
                          {
                                  free(adapter_attributes);
                                  free(adapter_name);
                                  continue;    // processing the next adapter
                          }

                          if ( hlp == NULL )
                          {
                                  hlp = (struct hbaAdapterList *)
                                          malloc (sizeof(struct hbaAdapterList));
                                  memset(hlp, 0, sizeof(struct hbaAdapterList));
                                  *lptr = hlp;
                          }
                          if ( hlp->sptr != NULL)
                          {
                                  hlp->next = (struct hbaAdapterList*) calloc (1,sizeof(struct hbaAdapterList));
                                  hlp = hlp->next;
                          }
                          /* 19 = length of NodeWWN + "-" + lineender */
                          system_name = get_system_name();
                          InstanceID_len = 
                                  strlen(system_name) +  19;
                          InstanceID = (char *) malloc(InstanceID_len);
                          snprintf(InstanceID,InstanceID_len,
                                  "%s-%llx",system_name,
                                  *(unsigned long long*)adapter_attributes->NodeWWN.wwn);
                          /* free system_name memory */
                          if ( system_name ) free(system_name);

                          rc = _hbaAdapter_data(adapter_number , InstanceID,
                                  adapter_name,adapter_attributes,
                                  &(hlp->sptr));

                  }
          }

          _OSBASE_TRACE(1,("--- HBA_FreeLibrary () called."));
          rc = HBA_FreeLibrary();
          _OSBASE_TRACE(4,("--- HBA_FreeLibrary () rc  = %d", rc));
  }
  _OSBASE_TRACE(1,("--- enum_all_hbaAdapters() exited"));
  hbamutex_unlock();
  return rc;
}

int enum_all_hbaAdapterPorts( struct hbaAdapterPortList ** lptr)
{
  struct hbaAdapterPortList * hlp = NULL;
  struct hbaPortList    * port_lptr = NULL;
  int                     adapter_number;
  char                  * adapter_name = NULL;
  int                     numberofadapters;
  HBA_HANDLE              handle;
  HBA_ADAPTERATTRIBUTES * adapter_attributes = NULL;
  HBA_PORTATTRIBUTES    * port_attributes = NULL;
  int                     rc    = 0;
  unsigned int                     port_counter = 0;
  int                     InstanceID_len;
  char                  * InstanceID;
  char                  * system_name = NULL; /* save pointer returned by get_system_name */
  
  _OSBASE_TRACE(1,("--- enum_all_hbaAdapterPorts() called"));
  hbamutex_lock();
  _OSBASE_TRACE(1,("--- HBA_LoadLibary () called."));
  rc = HBA_LoadLibrary();
  _OSBASE_TRACE(4,("--- HBA_LoadLibrary () rc  = %d",
                rc));

  if (rc == HBA_STATUS_OK)
  {
      _OSBASE_TRACE(1,("--- HBA_GetNumberOfAdapters () called."));

      numberofadapters = HBA_GetNumberOfAdapters();

      _OSBASE_TRACE(4,("--- HBA_NumberOfAdapters () = %d",numberofadapters));

          if (numberofadapters > 0)
          {
                  for (adapter_number=0;adapter_number<numberofadapters;adapter_number++)
                  {
                          handle = 0;
                          adapter_attributes = (HBA_ADAPTERATTRIBUTES *)
                                  malloc(sizeof(HBA_ADAPTERATTRIBUTES));
                          adapter_name = (char *) malloc(HBA_MAX_DEV_NAME + 1);
                          //rc = get_info_for_one_adapter(adapter_number,adapter_name,adapter_attributes,&handle,1);
                          rc = get_info_for_one_adapter(adapter_number,adapter_name,adapter_attributes,&handle,0);
                          if ( rc != HBA_STATUS_OK )
                          {
                                  free(adapter_attributes);
                                  free(adapter_name);
                                  if (handle != 0)
                                  {
                                          _OSBASE_TRACE(1,
                                                  ("--- HBA_CloseAdapter () called for handle = %d",
                                                  handle));
                                          HBA_CloseAdapter(handle);
                                  }
                                  continue;  // continue the next adapter
                          }

                          /* assemble adapter data */
                          if ( hlp == NULL )
                          {
                                  hlp = (struct hbaAdapterPortList *)
                                          malloc (sizeof(struct hbaAdapterPortList));
                                  memset(hlp, 0, sizeof(struct hbaAdapterPortList));
                                  *lptr = hlp;
                          }
                          else
                          {
                                  hlp->next = (struct hbaAdapterPortList*) 
                                          calloc (1,sizeof(struct hbaAdapterPortList));
                                  hlp = hlp->next;
                          }
                          /* 19 = length of NodeWWN + "-" + lineender */
                          system_name = get_system_name();
                          InstanceID_len = 
                                  strlen(system_name) +  19;
                          InstanceID = (char *) malloc(InstanceID_len);
                          snprintf(InstanceID,InstanceID_len,
                                  "%s-%llx",system_name,
                                  *(unsigned long long*)adapter_attributes->NodeWWN.wwn);
                          /* free system_name memory */
                          if ( system_name ) free(system_name);

                          rc = _hbaAdapter_data(adapter_number , InstanceID,
                                  adapter_name,adapter_attributes,
                                  &(hlp->adapter_sptr));

                          port_lptr = NULL;
                          /* processing adapter ports */
                          for (port_counter=0;
                                  port_counter<adapter_attributes->NumberOfPorts;
                                  port_counter++)
                          {
                                  if ( port_lptr == NULL )
                                  {
                                          port_lptr = (struct hbaPortList *)
                                                  malloc (sizeof(struct hbaPortList));
                                          memset(port_lptr, 0, sizeof(struct hbaPortList));
                                          hlp->port_lptr = port_lptr;
                                  }
                                  else 
                                  {
                                          port_lptr->next = (struct hbaPortList *)
                                                  calloc (1,sizeof(struct hbaPortList));
                                          port_lptr = port_lptr->next;
                                  }

                                  port_attributes = (HBA_PORTATTRIBUTES *)
                                          malloc(sizeof(HBA_PORTATTRIBUTES));
                                  rc = get_info_for_one_port (handle,
                                          port_counter,
                                          0,
                                          port_attributes,
                                          NULL,
                                          &(port_lptr->sptr));
                                  if ( rc != HBA_STATUS_OK )
                                  {
                                          free(port_attributes);
                                          continue;  // continue the next port
                                  }


                                  /* 18 = length of portWWN + lineender */
                                  InstanceID_len = 18;
                                  InstanceID = (char *) malloc(InstanceID_len);
                                  snprintf(InstanceID,InstanceID_len,
                                          "%llx",*(unsigned long long*)port_attributes->PortWWN.wwn);

                                  rc = _hbaPort_data(port_counter,
                                          adapter_number,
                                          InstanceID,
                                          port_attributes,
                                          0,
                                          &(port_lptr->sptr));
                          }
                          if (handle != 0)
                          {
                                  _OSBASE_TRACE(1,
                                          ("--- HBA_CloseAdapter () called for handle = %d",
                                          handle));
                                  HBA_CloseAdapter(handle);
                          }
                          handle = 0;

                  }

          }

          _OSBASE_TRACE(1,("--- HBA_FreeLibrary () called."));
          rc = HBA_FreeLibrary();
          _OSBASE_TRACE(4,("--- HBA_FreeLibrary () rc  = %d", rc));
  }

  _OSBASE_TRACE(1,("--- enum_all_hbaAdapterPorts() exited"));
      hbamutex_unlock();
  return rc;

}

int enum_all_hbaLogicalDisks(
  struct hbaPortList       ** lptr,
  int                       get_statistics,
  struct hbaLogicalDiskList ** dptr
  )
{
  struct hbaPortList      * hlp   = NULL;
  struct hbaLogicalDiskList * dlp = NULL;
  int                     adapter_number;
  char                    * adapter_name = NULL;
  int                     numberofadapters;
  HBA_HANDLE              handle;
  HBA_ADAPTERATTRIBUTES   * adapter_attributes = NULL;
  HBA_PORTATTRIBUTES      * port_attributes = NULL;
  HBA_PORTSTATISTICS      * port_statistics = NULL;
  int                     rc    = 0;
  unsigned int                     port_counter = 0;
  int                     InstanceID_len;
  char                    * InstanceID;
  HBA_FCPTARGETMAPPINGV2  * tempMapping = NULL;
  unsigned int                     i = 0;
  int                     j = 0;
  

  _OSBASE_TRACE(1,("--- enum_all_hbaLogicalDisks() called"));
  hbamutex_lock();
  _OSBASE_TRACE(1,("--- HBA_LoadLibary () called."));
  rc = HBA_LoadLibrary();
  _OSBASE_TRACE(4,("--- HBA_LoadLibrary () rc  = %d",
                rc));
  if (rc == HBA_STATUS_OK)
  {
      _OSBASE_TRACE(1,("--- HBA_GetNumberOfAdapters () called."));

      numberofadapters = HBA_GetNumberOfAdapters();

      _OSBASE_TRACE(4,("--- HBA_NumberOfAdapters () = %d",numberofadapters));

          if (numberofadapters > 0)
          {
                  adapter_attributes = (HBA_ADAPTERATTRIBUTES *)
                          malloc(sizeof(HBA_ADAPTERATTRIBUTES));
                  adapter_name = (char *) malloc(HBA_MAX_DEV_NAME + 1);

                  for (adapter_number=0;adapter_number<numberofadapters;adapter_number++)
                  {
                          handle = 0;
                          rc = get_info_for_one_adapter(adapter_number,
                                  adapter_name,
                                  adapter_attributes,
                                  &handle,
                                  0);
                          if ( rc != HBA_STATUS_OK )
                          {
                                  continue;  // processing the next adapter
                          }

                          for (port_counter=0;
                                  port_counter<adapter_attributes->NumberOfPorts;
                                  port_counter++)
                          {
                                  port_attributes = (HBA_PORTATTRIBUTES *)
                                          malloc(sizeof(HBA_PORTATTRIBUTES));
                                  if (get_statistics == 1)
                                  {
                                          port_statistics = (HBA_PORTSTATISTICS *)
                                                  malloc(sizeof(HBA_PORTSTATISTICS));
                                  }
                                  else
                                  {
                                          port_statistics = NULL;
                                  }

                                  if ( hlp == NULL )
                                  {
                                          hlp = (struct hbaPortList *)
                                                  malloc (sizeof(struct hbaPortList));
                                          memset(hlp, 0, sizeof(struct hbaPortList));
                                          *lptr = hlp;
                                  }
                                  if ( hlp->sptr != NULL)
                                  {
                                          hlp->next = (struct hbaPortList*) calloc (1,sizeof(struct hbaPortList));
                                          hlp = hlp->next;
                                  }

                                  rc = get_info_for_one_port (handle,
                                          port_counter,
                                          get_statistics,
                                          port_attributes,
                                          port_statistics,
                                          &(hlp->sptr));


                                  if ( rc != HBA_STATUS_OK )
                                  {
                                          if ( port_attributes ) free(port_attributes);
                                          if ( port_statistics ) free(port_statistics);
                                          if (hlp->sptr) free(hlp->sptr);
                                          hlp->sptr = NULL;
                                          continue;  // processing the next port
                                  }



                                  /* 18 = length of portWWN + lineender */
                                  InstanceID_len = 18;
                                  InstanceID = (char *) malloc(InstanceID_len);
                                  snprintf(InstanceID,InstanceID_len,
                                          "%llx",*(unsigned long long*)port_attributes->PortWWN.wwn);

                                  rc = _hbaPort_data(port_counter,
                                          adapter_number,
                                          InstanceID,
                                          port_attributes,
                                          port_statistics,
                                          &(hlp->sptr));

                                  /* Get memory for tempMapping and fill it in */
                                  tempMapping = (HBA_FCPTARGETMAPPINGV2*) malloc(sizeof(HBA_UINT32) + NUMBER_OF_ENTRIES*sizeof(HBA_FCPSCSIENTRYV2)); 
                                  if (tempMapping == NULL)
                                  {
                                          _OSBASE_TRACE(4,("--- Not enough memory for tempMapping"));
                                          goto exit;
                                  }

                                  tempMapping->NumberOfEntries = NUMBER_OF_ENTRIES;

                                  /* Filling in tempMapping */
                                  _OSBASE_TRACE(1,("--- HBA_GetFcpTargetMappingV2 called"));
                                  rc = HBA_GetFcpTargetMappingV2(handle, port_attributes->PortWWN, tempMapping);

                                  if(rc == HBA_STATUS_ERROR_MORE_DATA){
                                          _OSBASE_TRACE(4,("--- HBA_GetFcpTargetMappingV2 failed.  Not enough space."));
                                          goto exit;
                                  }

                                  else if(rc == HBA_STATUS_ERROR_ILLEGAL_WWN){
                                          _OSBASE_TRACE(4,("--- HBA_GetFcpTargetMappingV2 failed.  Illegal WWN."));
                                          goto exit;
                                  }

                                  else if(rc == HBA_STATUS_ERROR_NOT_SUPPORTED){
                                          _OSBASE_TRACE(4,("--- HBA_GetFcpTargetMappingV2 failed.  Not Supported."));
                                          goto exit;
                                  }

                                  else if(rc != HBA_STATUS_OK)
                                  {
                                          _OSBASE_TRACE(4,("--- HBA_GetFcpTargetMappingV2 failed"));
                                          goto exit;
                                  }

                                  for(i = 0; i < tempMapping->NumberOfEntries; i++)
                                  {
                                          /* Filling in Logical Disk List */
                                          if ( dlp == NULL )
                                          {
                                                  _OSBASE_TRACE(2,("--- get space for first node"));
                                                  dlp = (struct hbaLogicalDiskList *) malloc (sizeof(struct hbaLogicalDiskList));
                                                  if (dlp == NULL)
                                                  {
                                                          _OSBASE_TRACE(4,("--- Not enough memory for dlp"));
                                                          goto exit;
                                                  }
                                                  memset(dlp, 0, sizeof(struct hbaLogicalDiskList));
                                                  _OSBASE_TRACE(4,("--- dlp->next after memset = %p", dlp->next));
                                                  *dptr = dlp;
                                          }
                                          if ( (dlp)->sptr != NULL)
                                          {
                                                  _OSBASE_TRACE(2,("--- getting memory for cim_hbaLogicalDisk"));
                                                  (dlp)->next = (struct hbaLogicalDiskList*) calloc (1,sizeof(struct hbaLogicalDiskList));
                                                  if (dlp->next == NULL)
                                                  {
                                                          _OSBASE_TRACE(4,("--- Not enough memory for dlp's next"));
                                                          goto exit;
                                                  }

                                                  dlp = (dlp)->next;
                                          }

					  rc = _hbaLogicalDisk_data(&tempMapping, i, port_attributes->PortWWN, &(dlp->sptr), &dlp);
                                          trace_LogicalDisks(dptr);

                                          if ( rc != HBA_STATUS_OK )
                                          {
                                                  _OSBASE_TRACE(4,("--- _hbaLogicalDisk_data failed.  Going to exit."));
                                                  goto exit;
                                          }
                                          _OSBASE_TRACE(4,("---"));
                                          _OSBASE_TRACE(4,("---"));
                                          _OSBASE_TRACE(4,("---"));
                                          trace_LogicalDisks(dptr);
                                  }
                          }
                          if (handle != 0)
                          {
                                  _OSBASE_TRACE(4,
                                          ("--- HBA_CloseAdapter () called for handle = %d",
                                          handle));
                                  HBA_CloseAdapter(handle);
                          }
                          handle = 0;
                  }
exit:
                  /* free adapter_name and adapter_attribute */
                  if ( adapter_name ) free(adapter_name);
                  if ( adapter_attributes ) free (adapter_attributes);
                  if ( tempMapping ) free (tempMapping);
          }

          _OSBASE_TRACE(4,("--- HBA_FreeLibrary () called."));
          rc = HBA_FreeLibrary();
          _OSBASE_TRACE(4,("--- HBA_FreeLibrary () rc  = %d", rc));
  }

  hbamutex_unlock();
  _OSBASE_TRACE(1,("--- enum_all_hbaLogicalDisks() exited"));
  return rc;

}

int get_hbaAdapter_data(
  char                   * InstanceID,
  struct cim_hbaAdapter ** sptr )
{
  struct hbaAdapterList * lptr = NULL;
  struct hbaAdapterList * lhlp = NULL;
  struct cim_hbaAdapter * hbaadapter = NULL;
  int                     rc   = 0;

  _OSBASE_TRACE(3,("--- get_hbaAdapter_data() called"));

  rc = enum_all_hbaAdapters( &lptr ) ;

  if( rc == 0 && lptr != NULL ) {
    lhlp = lptr;
    for ( ; lptr ; lptr = lptr->next)
    {
      hbaadapter = lptr->sptr;
      if( strcmp(hbaadapter->InstanceID, InstanceID) == 0)
      {
        *sptr = hbaadapter;
        hbaadapter = NULL ;
        break;
      }
    }
    lptr = lhlp;
    for ( ; lptr ; )
    {
      hbaadapter = lptr->sptr;
      if( hbaadapter != *sptr ) {
        free_hbaAdapter(hbaadapter);
      }
      lhlp = lptr;
      lptr = lptr->next;
      free(lhlp);
    }
    _OSBASE_TRACE(3,("--- get_hbaAdapter_data() exited"));
    return 0;
  }
  _OSBASE_TRACE(3,("--- get_hbaAdapter_data() failed"));
  return -1;
}
int get_targetPort_data(  
                        char  *InstanceID,
                        struct cim_hbaPort ** sptr,
                        int get_statistics )
{
    struct hbaPortList *  lptr = NULL;
    struct hbaPortList *  lhlp = NULL;
    struct cim_hbaPort *  hbaport = NULL;
    int rc = 0;

  _OSBASE_TRACE(3,("--- get_targetPort_data() called: instanceID = %s", InstanceID));

  rc = enum_all_targetPorts( &lptr, get_statistics ) ;

  if( rc == 0 && lptr != NULL )
  {
    lhlp = lptr;
    for ( ; lptr ; lptr = lptr->next)
    {
      hbaport = lptr->sptr;
      if( strcmp(hbaport->InstanceID, InstanceID) == 0)
      {
        *sptr = hbaport;
        hbaport = NULL ;
        break;
      }
    }
    lptr = lhlp;
    for ( ; lptr ; )
    {
      hbaport = lptr->sptr;
      if( hbaport != *sptr )
      {
        free_hbaPort(hbaport);
      }
      lhlp = lptr;
      lptr = lptr->next;
      free(lhlp);
    }
    _OSBASE_TRACE(3,("--- get_targetPort_data() exited"));
    return 0;
  }
  _OSBASE_TRACE(3,("--- get_targetPort_data() failed"));
  return -1;
}
int get_hbaPort_data(
  char                * InstanceID,
  struct cim_hbaPort ** sptr,
  int                   get_statistics )
{
  struct hbaPortList *  lptr = NULL;
  struct hbaPortList *  lhlp = NULL;
  struct cim_hbaPort *  hbaport = NULL;
  int                   rc   = 0;

  _OSBASE_TRACE(3,("--- get_hbaPort_data() called"));

  rc = enum_all_hbaPorts( &lptr, get_statistics ) ;

  if( rc == 0 && lptr != NULL )
  {
    lhlp = lptr;
    for ( ; lptr ; lptr = lptr->next)
    {
      hbaport = lptr->sptr;
      if( strcmp(hbaport->InstanceID, InstanceID) == 0)
      {
        *sptr = hbaport;
        hbaport = NULL ;
        break;
      }
    }
    lptr = lhlp;
    for ( ; lptr ; )
    {
      hbaport = lptr->sptr;
      if( hbaport != *sptr )
      {
        free_hbaPort(hbaport);
      }
      lhlp = lptr;
      lptr = lptr->next;
      free(lhlp);
    }
    _OSBASE_TRACE(3,("--- get_hbaPort_data() exited"));
    return 0;
  }
  _OSBASE_TRACE(3,("--- get_hbaPort_data() failed"));
  return -1;
}

int get_hbaLogicalDisk_data(
  char                * InstanceID,
  int                 get_statistics, 
  struct cim_hbaLogicalDisk ** sptr)
{
  int                   rc   = 0;
  int                   found = -1;
  struct hbaLogicalDiskList *dptr = NULL;
  struct hbaLogicalDiskList *dhlp = NULL;
  struct cim_hbaLogicalDisk *hbaLogicalDisk = NULL;
  struct hbaPortList        *lptr = NULL;

  _OSBASE_TRACE(1,("--- get_hbaLogicalDisk_data() called"));

  rc = enum_all_hbaLogicalDisks(&lptr, 0, &dptr);
  if( rc == 0 && dptr != NULL)
  {
     dhlp = dptr;
     for( ; dptr ; dptr = dptr->next )
     {
        hbaLogicalDisk = dptr->sptr;
        if ( strcmp(hbaLogicalDisk->OSDeviceName, InstanceID) == 0 )
        {
           *sptr = hbaLogicalDisk;
           hbaLogicalDisk = NULL;
           break;
        }
     }
     dptr = dhlp;
     for ( ; dptr ; )
     {
        hbaLogicalDisk = dptr->sptr;
        if (hbaLogicalDisk != *sptr )
        {
           free_hbaLogicalDisk(hbaLogicalDisk);
        }
        dhlp = dptr;
        dptr = dptr->next;
        free(dhlp);
     }
       
     _OSBASE_TRACE(1,("--- get_hbaLogicalDisk_data() exited"));
    return 0;
  }

  _OSBASE_TRACE(1,("--- get_hbaLogicalDisk_data() failed"));
  return -1;
}




void free_hbaAdapterList(
  struct hbaAdapterList * lptr )
{
  struct hbaAdapterList * ls = NULL ;

  if( lptr == NULL ) return;
  for( ; lptr ;
  )
  {
    if(lptr->sptr)
    {
       free_hbaAdapter(lptr->sptr);
    }
    ls = lptr;
    lptr = lptr->next;
    free(ls);
  }

}

void free_hbaPortList(
  struct hbaPortList * lptr )
{
  struct hbaPortList * ls = NULL ;

  if( lptr == NULL ) return;
  for( ; lptr ; )
  {
    if(lptr->sptr)
    {
      free_hbaPort(lptr->sptr);
    }
    ls = lptr;
    lptr = lptr->next;
    free(ls);
  }
}

void free_hbaAdapterPortList( struct hbaAdapterPortList * lptr )
{
  struct hbaAdapterPortList * ls = NULL;
  if(lptr == NULL) return;
  for( ; lptr ; )
  {
    if(lptr->adapter_sptr)
    {
      free_hbaAdapter(lptr->adapter_sptr);
    }
    if(lptr->port_lptr)
    {
      free_hbaPortList(lptr->port_lptr);
    }
    ls = lptr;
    lptr = lptr->next;
    free(ls);
  }
}

void free_hbaAdapter( struct cim_hbaAdapter * sptr )
{
  if(sptr == NULL) return;
  if (sptr-> adapter_attributes != NULL)
  {
     free(sptr->adapter_attributes);
  }
  if (sptr->adapter_name != NULL)
  {
     free(sptr->adapter_name);
  }
  if (sptr->InstanceID != NULL)
  {
     free(sptr->InstanceID);
  }
  free(sptr);
}

void free_hbaPort( struct cim_hbaPort * sptr )
{

  if(sptr == NULL) return;
  if (sptr-> port_attributes != NULL)
  {
     free(sptr->port_attributes);
  }
  if (sptr-> port_statistics != NULL)
  {
     free(sptr->port_statistics);
  }
  if (sptr->InstanceID != NULL)
  {
     free(sptr->InstanceID);
  }
  free(sptr);
}

void free_hbaLogicalDiskList(
  struct hbaLogicalDiskList * lptr )
{
  struct hbaLogicalDiskList * ls = NULL ;

  if( lptr == NULL ) return;
  for( ; lptr ; )
  {
    if(lptr->sptr)
    {
      free_hbaLogicalDisk(lptr->sptr);
    }
    ls = lptr;
    lptr = lptr->next;
    free(ls);
  }
}

void free_hbaLogicalDisk( struct cim_hbaLogicalDisk * sptr )
{

  if(sptr == NULL) return;
  if (sptr-> OSDeviceName != NULL)
  {
     free(sptr->OSDeviceName);
  }
  if (sptr->buffer != NULL)
  {
     free(sptr->buffer);
  }
  free(sptr);
}


