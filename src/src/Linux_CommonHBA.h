/*
 * $Id: Linux_CommonHBA.h,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 *      Common HBA definitions.
*/

#ifndef _LINUX_COMMONHBA_H_
#define _LINUX_COMMONHBA_H_

/*
 * Linux_CommonHBA.h
 *
 * Description:
 * This file defines the interfaces for the common resource access
 * implementation for hba instrumentation.
 *
*/

/* ---------------------------------------------------------------------------*/

#include <time.h>
#include <sys/hbaapi.h>

#define HBA_MAX_DEV_NAME        30
#define DEVICEID_SIZE           256
#define BUFFER_SIZE             256
#define NUMBER_OF_ENTRIES       30
#ifdef __cplusplus
extern "C" {
#endif

struct cim_hbaAdapter {
  int                     adapter_number;
  char *                  InstanceID;
  HBA_HANDLE              handle;
  char                  * adapter_name;
  HBA_ADAPTERATTRIBUTES * adapter_attributes;

};

struct hbaAdapterList {
  struct cim_hbaAdapter * sptr ;
  struct hbaAdapterList * next ;
};


struct cim_hbaPort {
  int                     port_number;
  int                     adapter_number;
  char *                  InstanceID;
  HBA_PORTATTRIBUTES    * port_attributes;
  HBA_PORTSTATISTICS    * port_statistics;
  unsigned long long      bytes_received;
  unsigned long long      bytes_transmitted;
  unsigned long long      statistic_time;  
  unsigned short          role;
};

struct hbaPortList {
  struct cim_hbaPort    * sptr ;
  struct hbaPortList    * next ;
};

struct hbaAdapterPortList {
  struct cim_hbaAdapter * adapter_sptr;
  struct hbaPortList    * port_lptr;  
  struct hbaAdapterPortList * next;
};


struct cim_hbaLogicalDisk {
  char                    * OSDeviceName;
  HBA_UINT32              ScsiBusNumber;
  HBA_UINT32              ScsiTargetNumber;
  HBA_UINT32              ScsiOSLun;
  HBA_UINT32              FcId;
  HBA_WWN                 NodeWWN;
  HBA_WWN                 PortWWN;
  HBA_WWN                 InitiatorPortWWN;
  HBA_UINT64              FcpLun;
  char                    * buffer;
};

struct hbaLogicalDiskList {
  struct cim_hbaLogicalDisk * sptr;
  struct hbaLogicalDiskList * next;
};

int enum_all_hbaAdapters( struct hbaAdapterList ** lptr );
int enum_all_hbaPorts( struct hbaPortList ** lptr, int get_statistics );
int enum_all_hbaAdapterPorts( struct hbaAdapterPortList ** lptr);
int enum_all_hbaLogicalDisks( struct hbaPortList ** lptr, int get_statistics, 
                              struct hbaLogicalDiskList ** dptr); 
int enum_all_targetPorts(  struct hbaPortList **   lptr,
  int                     get_statistics);
int get_hbaAdapter_data( char * InstanceID, struct cim_hbaAdapter ** sptr );
int get_hbaPort_data( char * InstanceID, struct cim_hbaPort ** sptr, int get_statistics );
int get_hbaLogicalDisk_data( char * InstanceID, int get_statistics, 
                             struct cim_hbaLogicalDisk ** sptr);
int get_info_for_one_port(
  HBA_HANDLE            adapter_handle,
  int                   port_index,
  int                   get_statistics,
  HBA_PORTATTRIBUTES  * portattributes,
  HBA_PORTSTATISTICS  * portstatistics,
  struct cim_hbaPort  ** sptr  );
int get_info_for_one_adapter(
     int                     adapter_index,
     char                  * adapter_name,
     HBA_ADAPTERATTRIBUTES * adapterattributes,
     HBA_HANDLE            * handle,
     int                     do_close );
int get_targetPort_data(  
                        char  *InstanceID,
                        struct cim_hbaPort ** sptr,
                        int get_statistics );

void trace_LogicalDisks(struct hbaLogicalDiskList **dptr );
void trace_port_attributes(HBA_PORTATTRIBUTES * portattributes);
static void trace_port_statistics(HBA_PORTSTATISTICS *  portstatistics);
static void trace_adapter_attributes(HBA_ADAPTERATTRIBUTES *  adapterattributes);
void trace_LogicalDisks(struct hbaLogicalDiskList **dptr );



void free_hbaAdapterList( struct hbaAdapterList * lptr );
void free_hbaPortList( struct hbaPortList * lptr );
void free_hbaAdapterPortList( struct hbaAdapterPortList * lptr );
void free_hbaAdapter( struct cim_hbaAdapter * sptr );
void free_hbaPort( struct cim_hbaPort * sptr );
void free_hbaLogicalDisk( struct cim_hbaLogicalDisk * sptr);
void free_hbaLogicalDiskList(struct hbaLogicalDiskList * sptr);

int hbamutex_lock();
int hbamutex_unlock();


/* ---------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif

/* ---------------------------------------------------------------------------*/

#endif


