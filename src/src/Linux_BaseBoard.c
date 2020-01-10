/*
 * $Id: Linux_BaseBoard.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2005, 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Author:       Heidi Neumann <heidineu@de.ibm.com>
 * Contributors:
 *
 * Description:
 * This shared library provides resource access functionality for the class
 * Linux_BaseBoard.
 * It is independent from any specific CIM technology.
 *
*/

#include "Linux_Common.h"
#include "Linux_BaseBoard.h"
#include <unistd.h>

#if defined (INTEL) || defined (X86_64) || defined (IA64) || defined (GENERIC)
#include "dmiinfo.h"
#endif

/* ---------------------------------------------------------------------------*/
// private declarations

#if defined (S390)
#define SYSINFO   "/proc/sysinfo"
#endif

#if defined (PPC)
#define MODEL     "/proc/device-tree/model"
#define SYSTEMID  "/proc/device-tree/system-id"
#endif

/* ---------------------------------------------------------------------------*/

int _create_tag(struct cim_baseboard *data) {

  int size = 10;
  if(!data) { return -1; }
  if(data->UUID) {
    data->tag = strdup(data->UUID);
  } else {
    if(!data->vendor && !data->model && !data->serialNumber) { return -1; }
    if(data->vendor) size = size + strlen(data->vendor);
    if(data->model) size = size + strlen(data->model);
    if(data->serialNumber) size = size + strlen(data->serialNumber);
    data->tag = (char *)calloc(1,size);
    if(data->vendor)
      strcpy(data->tag,data->vendor);
    strcat(data->tag,":");
    if(data->model)
      strcat(data->tag,data->model);
    strcat(data->tag,":");
    if(data->serialNumber)
      strcat(data->tag,data->serialNumber);
  }
  return 0;
}

/* ---------------------------------------------------------------------------*/

int get_baseboard_tag(char *tag, size_t size) {
  struct cim_baseboard data;

  if(!tag) { return -1; }
  if(get_baseboard_data(&data)) { return -1; }
  if( (strlen(data.tag)+1) > size ) { return -1; }
  strcpy(tag,data.tag);
  return 0;
}

int get_baseboard_data(struct cim_baseboard *data) {

  memset(data,0,sizeof(struct cim_baseboard));

#if defined (INTEL) || defined (X86_64) || defined (IA64) || defined (GENERIC)

  DMI_BIOSPRODUCT *biosproduct;

  cimdmi_init();
  biosproduct=cimdmi_getBiosProduct();
  if (biosproduct) {
    data->vendor = strdup(biosproduct->dmi_Vendor);
    if(strcasecmp(biosproduct->dmi_Vendor,"IBM") == 0) {
      data->model = (char *)calloc(1,5);
      strncpy(data->model,biosproduct->dmi_Name,4);
      data->type = (char *)calloc(1,strlen(biosproduct->dmi_Name+4)+1);
      strcpy(data->type,biosproduct->dmi_Name+4);
    }
    data->serialNumber = strdup(biosproduct->dmi_IdentifyingNumber);
  }
  else { return -1; }
  cimdmi_term();

#endif

#if defined (S390)

  FILE *fhd = NULL;
  char vendor[255];
  char model[255];
  char type[255];
  char serial[255];

  if( (fhd=fopen(SYSINFO,"r")) != NULL ) {
    fscanf(fhd,"%*s %s %*s %s %*s %s %*s %*s %s",vendor,type,model,serial);
    data->vendor = strdup(vendor);
    data->model = strdup(model);
    data->type = strdup(type);
    data->serialNumber = strdup(serial);
    fclose(fhd);
  } else { return -1; }

#endif

#if defined (PPC)

  FILE *fhd = NULL;
  char *ptr = NULL;
  char *str = NULL;
  char buffer[255];

  if( (fhd=fopen(MODEL,"r")) != NULL ) {
    fscanf(fhd,"%s",buffer);
    if( !(ptr=strchr(buffer,',')) ) {
      data->model = (char *)calloc(1,strlen(buffer)+1);
      strcpy(data->model,buffer);
    } else {
      data->vendor = (char *)calloc(1,strlen(buffer)-strlen(ptr)+1);
      strncpy(data->vendor,buffer,strlen(buffer)-strlen(ptr));
      ptr+=1;
      if( !(str=strchr(ptr,'-')) ) {
	data->type = (char *)calloc(1,5);
	strncpy(data->type,ptr,4);
	ptr+=4;
	data->model = (char *)calloc(1,strlen(ptr)+1);
	strcpy(data->model,ptr);
      } else {
	data->type = (char *)calloc(1,strlen(ptr)-strlen(str)+1);
	strncpy(data->type,ptr,strlen(ptr)-strlen(str));
	str+=1;
	data->model = (char *)calloc(1,strlen(str)+1);
	strcpy(data->model,str);
      }
    }
    fclose(fhd);
    memset(buffer,0,sizeof(buffer));

    if( (fhd=fopen(SYSTEMID,"r")) != NULL ) {
      fscanf(fhd,"%s",buffer);
      if( !(ptr=strchr(buffer,',')) ) {
	data->serialNumber = (char *)calloc(1,strlen(buffer)+1);
	strcpy(data->serialNumber,buffer);
      } else {
	ptr+=1;
	data->serialNumber = (char *)calloc(1,strlen(ptr)+1);
	strcpy(data->serialNumber,ptr+2); /* +2 since 1st two chars are for something else(?) */
      }
      fclose(fhd);
    }
  } else { return -1; }

#endif

  if(_create_tag(data)) { return -1; }

  return 0;
}

/* ---------------------------------------------------------------------------*/

void free_baseboard_data(struct cim_baseboard *data) {
  if(!data) return;
  if(data->tag) free(data->tag);
  if(data->vendor) free(data->vendor);
  if(data->type) free(data->type);
  if(data->model) free(data->model);
  if(data->partNumber) free(data->partNumber);
  if(data->serialNumber) free(data->serialNumber);
  if(data->UUID) free(data->UUID);
}

/* ---------------------------------------------------------------------------*/
/*                       end of OSBase_BaseBoard.c                            */
/* ---------------------------------------------------------------------------*/

