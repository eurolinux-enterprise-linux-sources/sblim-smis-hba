/*
 * $Id: cmpiSMIS_FCSCSIInitiatorTargetLogicalUnitPathProvider.c,v 1.1.1.1 2009/05/12 21:46:34 nsharoff Exp $
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
 *	Fibre Channel SCSI Initiator Target Logical Unit Path provider.
*/

#include <string.h>
#include "Linux_Common.h"
#include "Linux_CommonHBA.h"
#include "cmpiLinux_Common.h"
#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

#define INITIATOR "Initiator"
#define TARGET "Target"
#define LOGICALUNIT "LogicalUnit"
#define INIT_ROLE 2
#define TARGET_ROLE 3
#define BOTH_ROLE 4
#define UNKNOWN_ROLE 1

static const CMPIBroker * _broker;
static char * _RefInitiator    = INITIATOR;
static char * _RefTarget       = TARGET;
static char * _RefLU           = LOGICALUNIT;
static char * _RefInitiatorCls = "Linux_FCSCSIProtocolEndpoint";
static char * _RefTargetCls    = "Linux_FCSCSIProtocolEndpoint";
static char * _RefLUCls        = "Linux_FCLogicalDisk";
static char * _ClassName       = "Linux_FCSCSIInitiatorTargetLogicalUnitPath";
/* ---------------------------------------------------------------------------*/
/* private declarations                                                       */
/* ---------------------------------------------------------------------------*/
struct InitiatorTargetLUPath
{
   HBA_WWN initiator;
   HBA_WWN target;
   char* deviceID;
};

typedef struct initiatorTargetLUPathList
{
   struct InitiatorTargetLUPath path;
   struct initiatorTargetLUPathList* next;
}
InitiatorTargetLUPathList;

typedef struct refList
{
   CMPIObjectPath* ref;
   struct refList * next;
}
RefList;

static CMPIObjectPath* getRefByKeyValue(RefList* array, const char* key, const char* value);

static CMPIInstance * getLUInstance( const CMPIBroker * _broker,
      const CMPIContext * ctx,
      const CMPIObjectPath * cop,
      char * _ClassName,
      char * _RefInitiator,
      char * _RefTarget,
      char * _RefLU,
      CMPIStatus * rc );

static CMPIStatus getInstanceNamesList(
      const char * nameSpace, 
      const CMPIContext * ctx,
      const CMPIObjectPath * cop,
      RefList ** Initiators, 
      RefList ** Targets, 
      RefList ** LUs);

static void freeInstanceNamesList(RefList* Initiators, RefList* Targets, RefList* LUs);

static int isObjectPathIdentity(const CMPIBroker * _broker, 
      const CMPIObjectPath * lhs,
      const CMPIObjectPath * rhs);

static RefList* remvoeObjectPathFromList(RefList * Lst, const CMPIObjectPath * op); 

static CMPIObjectPath * _makePath_SCSILUPath(const char* nameSpace,
      const CMPIObjectPath * initiator, 
      const CMPIObjectPath * target,
      const CMPIObjectPath * LU);

static CMPIInstance* _makeInst_SCSILUPath(const char * nameSpace,
      const CMPIObjectPath* refInitiator,
      const CMPIObjectPath* refTarget,
      const CMPIObjectPath* refLogicalUnit);

int _assoc_create_refs_FCLUPath( const CMPIBroker * _broker,
      const CMPIContext * ctx,
      const CMPIResult * rslt,
      const CMPIObjectPath * cop,
      const char * _ClassName,
      const char * resultCls,
      const char* role,
      const char* resultRole,
      int isSCSIEndpoint,
      int assoc,
      int inst,
      CMPIStatus* rc);

static int _classes_is_parentity( const CMPIBroker * _broker,
      const char * nameSpace,
      const char * classA,
      const char * classB);

static int _check_param( const CMPIBroker * _broker,
      const CMPIObjectPath * cop,
      const CMPIContext * ctx,
      const char * _RefInit,
      const char * _RefTarget,
      const char * _RefLU,
      const char * _RefInitCls,
      const char * _RefTargetCls,
      const char * _RefLUCls,
      const char * resultClass,
      const char * role,
      const char * resultRole,
      int* isSCSIEndpoint,
      CMPIStatus * rc);

/* remvove object path op from RefList lst*/
static RefList * remvoeObjectPathFromList(RefList * lst, const CMPIObjectPath * op)
{
   RefList *head = lst;
   RefList *prev = NULL;
   while(lst)
   {
      RefList * next = lst->next;
      if(lst->ref == op)
      {
         if(prev)
            prev->next = lst->next;
         else
            head = lst->next;
         free(lst);
         break;
      }
      prev = lst;
      lst = next;
   }
   return head;
}

/* judge the two objectpath of SCSIProtocolEndpoint or LogicalDevice is identity 
   the two input objectpath is either SCSIProtocolEndpoint or LogicalDevice
   the only key of SCSIProtocolEndpoint of local system is Name, LogicalDevice is DeviceID
   return value: 
   1 - two object paths are identity
   0 - aren't identity
 */
static int isObjectPathIdentity(const CMPIBroker * _broker, 
      const CMPIObjectPath * lhs,
      const CMPIObjectPath * rhs)
{
   CMPIStatus rc = {CMPI_RC_OK, NULL};
   char * nsl = CMGetCharPtr(CMGetNameSpace(lhs, &rc));
   char * nsr = CMGetCharPtr(CMGetNameSpace(rhs, &rc));
   const char * key;
   CMPIData dtl, dtr;
   if(strcmp(nsl, nsr) != 0)
   {
      return 0;
   }
   key = strcmp(nsl, _RefInitiatorCls) == 0? "Name": "DeviceID";
   dtl = CMGetKey(lhs, key, &rc);
   dtr = CMGetKey(rhs, key, &rc);
   if(strcmp(CMGetCharPtr(dtl.value.string), CMGetCharPtr(dtr.value.string)) == 0)
   {
      return 1;
   }
}

/* judge if the classA is parent of classB or reverse */
static int _classes_is_parentity( const CMPIBroker * _broker,
      const char * nameSpace,
      const char * classA,
      const char * classB)
{
   CMPIStatus rc = {CMPI_RC_OK, NULL};
   CMPIObjectPath * opA = CMNewObjectPath(_broker, nameSpace, classA, &rc);
   CMPIObjectPath * opB = CMNewObjectPath(_broker, nameSpace, classB, &rc);
   if(CMClassPathIsA(_broker, opA, classB, &rc) == 1 ||
         CMClassPathIsA(_broker, opB, classA, &rc) == 1)
   {
      return 1;
   }
   return 0;
}
      
/* judge if the parameters for associatior/reference is legal or not 
 * return value: 0 - parameters isn't legal
 *               1 - parameters is legal 
 * isSCSIEndpoint: 1 - the input object path is a scsi protocol endpoint
 *                 0 - the input object path is logical unit
 */
static int _check_param( const CMPIBroker * _broker,
      const CMPIObjectPath * cop,
      const CMPIContext * ctx,
      const char * _RefInit,
      const char * _RefTarget,
      const char * _RefLU,
      const char * _RefInitCls,
      const char * _RefTargetCls,
      const char * _RefLUCls,
      const char * resultClass,
      const char * role,
      const char * resultRole,
      int* isSCSIEndpoint,
      CMPIStatus * rc)
{
   CMPIObjectPath * scop        = NULL;
   CMPIObjectPath * rcop        = NULL;
   CMPIString     * sourceClass = NULL;
   const char     * nameSpace   = NULL;
   CMPIInstance   * in          = NULL;
   int ret = 0;
   int resIsInit = 1, resIsTarget = 1, resIsLU = 1;
   int isInit = 0, isTarget = 0, isLU = 0;

   in = CBGetInstance(_broker, ctx, cop, NULL, rc);
   if(CMIsNullObject(in))
   {
      goto exit;
   }

   nameSpace = CMGetCharPtr(CMGetNameSpace(cop, rc));

   sourceClass = CMGetClassName(cop, rc);
   scop = CMNewObjectPath( _broker, nameSpace, CMGetCharPtr(sourceClass), rc );
   if(CMClassPathIsA(_broker, scop, _RefInitCls, rc) == 1)
   {
      /* check the role from instance property */
      CMPIData dt;
      dt = CMGetProperty(in, "Role", rc);
      switch(dt.value.uint16)
      {
      case INIT_ROLE:
         isInit = 1;
         resIsInit = 0;
         break;
      case TARGET_ROLE:
         isTarget = 1;
         resIsTarget = 0;
         break;
      case BOTH_ROLE:
         isInit = 1;
         isTarget = 1;
         break;
      case UNKNOWN_ROLE:
         break;
      default:
         goto exit;
      }
      * isSCSIEndpoint = 1;
   }
   else if(CMClassPathIsA(_broker, scop, _RefLUCls, rc) == 1)
   {
      isLU = 1;
      resIsLU = 0;
      * isSCSIEndpoint = 0;
   }
   if(resultClass|| role || resultRole)
   {
      if(role)
      {
         /* if role specified, then only one of isInit, isTarget and isLU is equal to 1 */
         if(strcasecmp(role, _RefInit) == 0 && isInit == 1)
         {
            isTarget = isLU = resIsInit = 0;
         }
         else if (strcasecmp(role, _RefTarget) == 0 && isTarget == 1)
         {
            isInit = isLU = resIsTarget = 0;
         }
         else if (strcasecmp(role, _RefLU) == 0 && isLU == 1)
         {
            isInit = isTarget = resIsLU = 0;
         }
         else
         {
            goto exit;
         }
      }
      if(resultRole)
      {
         if(strcasecmp(resultRole, _RefInit) == 0 && (isTarget == 1 || isLU == 1))
         {
            resIsTarget = resIsLU = 0; /* result class cannot be target or logical unit */
         }
         else if (strcasecmp(resultRole, _RefTarget) == 0 && (isInit == 1 || isLU == 1))
         {
            resIsInit = resIsLU = 0;
         }
         else if (strcasecmp(resultRole, _RefLU) == 0 && (isInit == 1 || isTarget == 1))
         {
            resIsInit = resIsTarget = 0;
         }
         else 
         {
            goto exit;
         }
      }
      if(resultClass)
      {
         if(resIsInit == 1 && _classes_is_parentity(_broker, nameSpace, resultClass, _RefInitCls) == 1)
         {
            if(isTarget == 1 || isLU == 1)
            {
               ret = 1; goto exit;
            }
         }
         if(resIsTarget == 1 && _classes_is_parentity(_broker, nameSpace, resultClass, _RefTargetCls) == 1)
         {
            if(isInit == 1 || isLU == 1)
            {
               ret = 1; goto exit;
            }
         }
         if(resIsLU == 1 && _classes_is_parentity(_broker, nameSpace, resultClass, _RefLUCls) == 1)
         {
            if(isInit == 1 || isTarget == 1)
            {
               ret = 1; goto exit;
            }
         }
         else
         {
            ret = 0; goto exit;
         }
      }
   }
   ret = 1;

exit:
   return ret;
}



/* 
 * enumerate all path connected with initiator, target and logical unit.
 */
void enum_all_LUPath(InitiatorTargetLUPathList** lst)
{
   struct hbaPortList        *lptr = NULL;
   struct hbaLogicalDiskList *dptr = NULL;
   InitiatorTargetLUPathList *tail = NULL;

   _OSBASE_TRACE(2, ("enum_all_LUPath called"));
   /* set a empty head node */
   *lst = (InitiatorTargetLUPathList*)malloc(sizeof(InitiatorTargetLUPathList));
   (*lst)->next = NULL;
   tail = *lst;

   enum_all_hbaLogicalDisks( &lptr, 0, &dptr);
   while(dptr)
   {
      InitiatorTargetLUPathList* node = 
         (InitiatorTargetLUPathList*)malloc(sizeof(InitiatorTargetLUPathList));
      node->path.initiator = dptr->sptr->InitiatorPortWWN;
      node->path.target = dptr->sptr->PortWWN;
      node->path.deviceID = strdup(dptr->sptr->OSDeviceName);
      node->next = NULL;
      tail->next = node;
      tail = node;

      dptr = dptr->next;
      _OSBASE_TRACE(4, ("Path:%llx<->%llx<->%s", *(unsigned long long*)node->path.initiator.wwn, *(unsigned long long*)node->path.target.wwn, node->path.deviceID));
   }
   free_hbaPortList(lptr);
   free_hbaLogicalDiskList(dptr);
   /* release the empty head node */
   tail = *lst;
   * lst = (*lst)->next;
   free(tail);

   _OSBASE_TRACE(2, ("enum_all_LUPath exited"));
}

void free_all_LUPath(InitiatorTargetLUPathList* lst)
{
   InitiatorTargetLUPathList* node = NULL;
   while(lst)
   {
      node = lst;
      lst = lst->next;
      free(node->path.deviceID);
      free(node);
   }
}

/*
 * return all instance names of Initiator, Target and Logical Unit in three separated array
 */
static CMPIStatus getInstanceNamesList(
      const char * nameSpace, 
      const CMPIContext * ctx,
      const CMPIObjectPath * cop,
      RefList ** Initiators, 
      RefList ** Targets, 
      RefList ** LUs)
{
   CMPIObjectPath * opInitiator   = NULL;                /* initiator object path */
   CMPIObjectPath * opLogicalUnit = NULL;                /* logical unit object path */
   CMPIEnumeration * enEndpoint   = NULL;
   CMPIEnumeration * enLogicalUnit= NULL;
   CMPIStatus rc                  = {CMPI_RC_OK, NULL};
   RefList * tailInit             = NULL;
   RefList * tailTarget           = NULL;
   RefList * tailLU               = NULL;
   RefList * node                 = NULL;
   RefList ** tail                = NULL;

   _OSBASE_TRACE(2,("getInstanceNamesList called"));
   opInitiator = CMNewObjectPath( _broker, nameSpace, _RefInitiatorCls, &rc);
   if (rc.rc != CMPI_RC_OK) opInitiator = NULL;

   /* object path of all SCSIProtocolEndpoint include initiator and target SCSIProtocolEndpoint */
   enEndpoint = CBEnumInstances( _broker, ctx, opInitiator, NULL, &rc); 
   if( enEndpoint == NULL || rc.rc != CMPI_RC_OK ) {
       CMSetStatusWithChars( _broker, &rc,
               CMPI_RC_ERR_FAILED, "No FC SCSI Protocol Endpoint found." );
       _OSBASE_TRACE(2,("%s, getInstanceNamesList exited", rc.msg));
       return rc;
   }

   /* categorize the returned object path to Initiator and Target according to "Role" property */
   *Initiators = (RefList*)malloc(sizeof(RefList));
   tailInit = *Initiators;
   tailInit->next = NULL;
   *Targets = (RefList*)malloc(sizeof(RefList));
   tailTarget =  *Targets;
   tailTarget->next = NULL;

   while(CMHasNext(enEndpoint, &rc))
   {
      CMPIData result;
      CMPIData data = CMGetNext(enEndpoint, &rc);
      CMPIUint16 role = 0;
      if( data.value.inst == NULL ) {
         CMSetStatusWithChars( _broker, &rc,
               CMPI_RC_ERR_FAILED, "CMGetNext( enEndpoint, &rc)" );
         _OSBASE_TRACE(2,("%s, getInstanceNamesList exited", rc.msg));
         return rc;
      }
      result = CMGetProperty(data.value.inst, "Role", &rc);
      /* 0 - unkown, 2 - initiator, 3 - target, 4 - both initiator and target */
      role = result.value.uint16;
      if(role == 2 || role == 4) {
         tail = &tailInit; /* an initiator */
         _OSBASE_TRACE(4, ("Find an Initiator"));
      }
      else if(role == 3 || role == 4) {
         tail = &tailTarget; /* a target */
         _OSBASE_TRACE(4, ("Find an Target"));
      }
      else {
         continue; /* unknown role */
      }
      node = (RefList*) malloc(sizeof(RefList));
      node->ref = CMGetObjectPath(data.value.inst, &rc);
      node->next = NULL;
      (*tail)->next = node;
      (*tail) = node;
   }
   /* free the empty head node of initiator list and target list*/
   tailInit = *Initiators;
   *Initiators = (*Initiators)->next;
   free(tailInit);
   tailTarget = *Targets;
   *Targets = (*Targets)->next;
   free(tailTarget);

   /* get list of logical unit */
   opLogicalUnit = CMNewObjectPath( _broker, nameSpace, _RefLUCls, &rc);
   if (rc.rc != CMPI_RC_OK) opLogicalUnit = NULL;

   enLogicalUnit = CBEnumInstanceNames( _broker, ctx, opLogicalUnit, &rc);
   if( enLogicalUnit == NULL ) {
      CMSetStatusWithChars( _broker, &rc,
            CMPI_RC_ERR_FAILED, "CBEnumInstanceNames( _broker, ctx, op, &rc)" );
      _OSBASE_TRACE(2,("%s, getInstanceNamesList exited", rc.msg));
      return rc;
   }
   /* convert enumeration to list*/
   *LUs = (RefList*)malloc(sizeof(RefList));
   memset(*LUs,0,sizeof(RefList));
   tailLU = *LUs;
   while(CMHasNext(enLogicalUnit, &rc))
   {
      CMPIData data = CMGetNext(enLogicalUnit, &rc);
      node = (RefList*) malloc(sizeof(RefList));
      node->ref = data.value.ref;
      node->next = NULL;
      tailLU->next = node;
      tailLU = node;
      _OSBASE_TRACE(4, ("Find an LU"));
   }
   tailLU = *LUs;
   *LUs = (*LUs)->next;
   free(tailLU);
   _OSBASE_TRACE(2,("getInstanceNamesList exited"));
   return rc;
}

/* free the list hold of initiators, targets, and Logical units */
static void freeInstanceNamesList(RefList* Initiators, RefList* Targets, RefList* LUs)
{
   RefList* node = NULL;
   while(Initiators)
   {
      node = Initiators;
      Initiators = Initiators->next;
      free(node);
   }
   while(Targets)
   {
      node = Targets;
      Targets = Targets->next;
      free(node);
   }
   while(LUs)
   {
      node = LUs;
      LUs = LUs->next;
      free(node);
   }
}

/* judge if an object path is satisfy the requirement of result role and
   result class.
   if satisfy, return 1, else return 0
 */
static int filterObjectPath(const CMPIObjectPath * path, 
      const char* myRole,
      const char * resultClass, 
      const char * resultRole)
{
   int result = 0;
   CMPIStatus rc = {CMPI_RC_OK, NULL};
   const char * className = NULL;
   const char * nameSpace = NULL;
   _OSBASE_TRACE(4,("filterObjectPath called"));
   if(path)
   {
      className = CMGetCharPtr(CMGetClassName(path, &rc));
      nameSpace = CMGetCharPtr(CMGetNameSpace(path, &rc));
      _OSBASE_TRACE(4,("classname:%s, role:%s, result class:%s, result role:%s", 
               className, myRole, resultClass, resultRole));
      if(resultRole)
      {
         if(strcasecmp(myRole, resultRole) != 0)
         {
             goto exit;
         }
      }
      if(resultClass)
      {
         CMPIObjectPath * op = CMNewObjectPath(_broker, nameSpace, className, &rc);
         if( CMIsNullObject(op) || ! CMClassPathIsA(_broker, op, resultClass, &rc))
         {
             goto exit;
         }
      }
      _OSBASE_TRACE(4,("filterObjectPath exited with 1"));
      return 1;
   }
exit:
   _OSBASE_TRACE(4,("filterObjectPath exited with 0"));
   return 0;
}
/* 
 * enum all associations, and check the key of the association is the objectpath passed in
 * assoc, inst
 * 0      0   -> reference names
 * 0      1   -> references
 * 1      0   -> associator names
 * 1      1   -> associators
 */
int _assoc_create_refs_FCLUPath( const CMPIBroker * _broker,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop,
            const char * _ClassName,
            const char * resultCls,
            const char* role,
            const char* resultRole,
            int isSCSIEndpoint,
            int assoc,
            int inst,
            CMPIStatus* rc)
{
   int ret = 0;
   /* Since the parameters has passed the validation test,
    * the input object path and role, if not NULL, is matched.
    */
   CMPIObjectPath * op = NULL;
   CMPIEnumeration * en = NULL;
   _OSBASE_TRACE(4,("_assoc_create_refs_FCLUPath called"));
   if(isSCSIEndpoint)
   {
      /* check the cop really play the role */
      CMPIData dt;
      CMPIInstance * ci = CBGetInstance(_broker, ctx, cop, NULL, rc);
      if(ci == NULL || rc->rc != CMPI_RC_OK)
      {
         ret = 1;
         goto exit;
      }
      /* get property value of "Role" */
      dt = CMGetProperty(ci, "Role", rc);
      _OSBASE_TRACE(4,("the role is %d", dt.value.uint16));
      if(dt.value.uint16 == 2)
      {
         if(role && (strcasecmp(role, _RefInitiatorCls) != 0)) /* initiator */
            goto exit;
         role = _RefInitiator;
      }
      else if(dt.value.uint16 == 3)
      {
         if(role && strcasecmp(role, _RefTarget) != 0) /* target */
            goto exit;
         role = _RefTarget;
      }
      else if(dt.value.uint16 == 4) /* both initiator and target */
      {
         /* call this funciton twice */
         _assoc_create_refs_FCLUPath( _broker, ctx, rslt, cop, 
               _ClassName, resultCls, _RefInitiator, resultRole, 
               isSCSIEndpoint, assoc, inst, rc);
         _assoc_create_refs_FCLUPath( _broker, ctx, rslt, cop, 
               _ClassName, resultCls, _RefTarget, resultRole, 
               isSCSIEndpoint, assoc, inst, rc);
         goto exit;
      }
      else /* unknow role */
      {
         goto exit;
      }
   }
   else /* not regarding the role is null or not, it must be "LogicalUnit" */
   {
      role = _RefLU;
   }
   {
      /* here need to find other two object path */
      InitiatorTargetLUPathList * lstPath, *node;
      RefList * lstInitiator, *lstTarget, *lstLogicalUnit;
      const char* nameSpace = CMGetCharPtr(CMGetNameSpace(cop, rc));
      CMPIStatus ret = {CMPI_RC_OK, NULL};
      char strWWN1[21], strWWN2[21], deviceID[255];
      const char* myKey, *yourKey, *hisKey;
      const char* myVal, *yourVal, *hisVal;
      const char* myRole, *yourRole, *hisRole;
      RefList *yourLst, *hisLst;
      const CMPIObjectPath * yourOp, * hisOp;
      const CMPIObjectPath ** opInit, ** opTarget, ** opLU;
      CMPIData dt;

      enum_all_LUPath(&lstPath);
      if(!lstPath) goto exit;
      ret = getInstanceNamesList(nameSpace, ctx, cop, &lstInitiator, &lstTarget, &lstLogicalUnit);

      if(ret.rc != CMPI_RC_OK)
      {
         goto exit;
      }
      if(strcasecmp(role, _RefInitiator) == 0)
      {
         myKey = "Name"; yourKey = "Name"; hisKey = "DeviceID";
         myVal = strWWN1; yourVal = strWWN2; hisVal = deviceID;
         myRole = _RefInitiator; yourRole = _RefTarget; hisRole = _RefLU;
         yourLst = lstTarget; hisLst = lstLogicalUnit;
         opInit = &cop; opTarget = &yourOp; opLU = &hisOp;
      }
      else if(strcasecmp(role, _RefTarget) == 0)
      {
         myKey = "Name"; yourKey = "Name"; hisKey = "DeviceID";
         myVal = strWWN2; yourVal = strWWN1; hisVal = deviceID;
         myRole = _RefTarget; yourRole = _RefInitiator; hisRole = _RefLU;
         yourLst = lstInitiator; hisLst = lstLogicalUnit;
         opInit = &yourOp; opTarget = &cop; opLU = &hisOp;
      }
      else if(strcasecmp(role, _RefLU) == 0)
      {
         myKey = "DeviceID"; yourKey = "Name"; hisKey = "Name";
         myVal = deviceID; yourVal = strWWN1; hisVal = strWWN2;
         myRole = _RefLU; yourRole = _RefInitiator; hisRole = _RefTarget;
         yourLst = lstInitiator; hisLst = lstTarget;
         opInit = &yourOp; opTarget = &hisOp; opLU = &cop;
      }
      else
      {
         _OSBASE_TRACE(4, ("%s isn't legal role", role));
         goto exit;
      }

      _OSBASE_TRACE(4, ("my key:%s . my role:%s", myKey, myRole));
      _OSBASE_TRACE(4, ("your key:%s . your role:%s", yourKey, yourRole));
      _OSBASE_TRACE(4, ("his key:%s . his role:%s", hisKey, hisRole));
      dt = CMGetKey(cop, myKey, rc);
      while(lstPath)
      {
         /* WWN and is 64bit. If formated in to decimal string and NULL char, 21 chars is enough */
         snprintf(strWWN1, sizeof(strWWN1), "%llx", *(unsigned long long*)lstPath->path.initiator.wwn);
         strWWN1[sizeof(strWWN1) - 1] = '\0';
         snprintf(strWWN2, sizeof(strWWN2), "%llx", *(unsigned long long*)lstPath->path.target.wwn);
         strWWN2[sizeof(strWWN2) - 1] = '\0';
         strcpy(deviceID, lstPath->path.deviceID);

         if(strcmp(myVal, CMGetCharPtr(dt.value.string)) == 0)
         {
            yourOp = getRefByKeyValue(yourLst, yourKey, yourVal);
            hisOp = getRefByKeyValue(hisLst, hisKey, hisVal);
            _OSBASE_TRACE(4,("assoc %d, inst %d", assoc, inst));
            if(assoc) /* associators or associatorNames */
            {
               if(inst) 
               {
                  if(filterObjectPath(yourOp, yourRole, resultCls, resultRole)) 
                  {
                     CMPIInstance* in = CBGetInstance(_broker, ctx, yourOp, NULL, rc);
                     if(!CMIsNullObject(in))
                     {
                        CMReturnInstance(rslt, in);
                        yourLst = remvoeObjectPathFromList(yourLst, yourOp); /* avoid duplicated return */
                     }
                  }
                  if(filterObjectPath(hisOp, hisRole, resultCls, resultRole))
                  {
                     CMPIInstance * in = CBGetInstance(_broker, ctx, hisOp, NULL, rc);
                     if(!CMIsNullObject(in))
                     {
                        CMReturnInstance(rslt, in);
                        hisLst = remvoeObjectPathFromList(hisLst, hisOp); /* avoid duplicated return */
                     }
                  }
               }
               else 
               {
                  if(filterObjectPath(yourOp, yourRole, resultCls, resultRole)) 
                  {
                     CMReturnObjectPath(rslt, yourOp);
                     yourLst = remvoeObjectPathFromList(yourLst, yourOp); /* avoid duplicated return */
                  }
                  if(filterObjectPath(hisOp, hisRole, resultCls, resultRole))
                  {
                     CMReturnObjectPath(rslt, hisOp);
                     hisLst = remvoeObjectPathFromList(hisLst, hisOp); /* avoid duplicated return */
                  }
               }
            }
            else /* references or referenceNames */
            {
               if(inst)
               {
                  CMPIInstance* cin = _makeInst_SCSILUPath(nameSpace, *opInit, *opTarget, *opLU);
                  if(cin)
                     CMReturnInstance(rslt, cin);
               }
               else
               {
                  CMPIObjectPath* op = _makePath_SCSILUPath(nameSpace, *opInit, *opTarget, *opLU);
                  if(op)
                     CMReturnObjectPath(rslt, op);
               }
            }
         }
         node = lstPath;
         lstPath = lstPath->next;
      }
      freeInstanceNamesList(lstInitiator, lstTarget, lstLogicalUnit);
      free_all_LUPath(lstPath);
   }
exit:
   _OSBASE_TRACE(4,("_assoc_create_refs_FCLUPath exited"));
   return ret;
}

 
/* Get an object path from a path array by a key value 
   if nothing found, return NULL
 */
static CMPIObjectPath* getRefByKeyValue(RefList* list, const char* key, const char* value)
{
   CMPIStatus rc = {CMPI_RC_OK, NULL};
   CMPIData data, keyValue;
   CMPIObjectPath* refResult;

   _OSBASE_TRACE(1,("getRefByKeyValue called, key is '%s', value is '%s'",  key, value));
   while(list)
   {
      refResult = list->ref;
      _OSBASE_TRACE(4,("ref is 0x%x", refResult));
      if(refResult != NULL){
         keyValue = CMGetKey(refResult, key, &rc);
         _OSBASE_TRACE(4,("value of '%s' is %s",key, CMGetCharPtr(keyValue.value.string)));
         if(strcmp(value, CMGetCharPtr(keyValue.value.string)) == 0)
         {
            _OSBASE_TRACE(4,("ref found 0x%x, getRefByKeyValue exited", refResult));
            return refResult;
         }
      }
      list = list->next;
   }
   _OSBASE_TRACE(1,("getRefByKeyValue exited with NULL"));
   return NULL;
}

static CMPIObjectPath * _makePath_SCSILUPath(const char* nameSpace,
      const CMPIObjectPath * initiator, 
      const CMPIObjectPath * target,
      const CMPIObjectPath * LU)
{
   CMPIStatus rc = {CMPI_RC_OK, NULL};
   if(initiator && target && LU)
   {
      /* construct object path for the association */
      CMPIObjectPath * op = CMNewObjectPath( _broker, nameSpace, _ClassName, &rc);
      if( CMIsNullObject(op) ) {
         CMSetStatusWithChars( _broker, &rc, CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
      }
      else
      {
         CMAddKey( op, _RefInitiator, &initiator, CMPI_ref);
         CMAddKey( op, _RefTarget, &target, CMPI_ref);
         CMAddKey( op, _RefLU, &LU, CMPI_ref);
         return op;
      }
   }
   return NULL;
}

static CMPIInstance* _makeInst_SCSILUPath(const char * nameSpace,
      const CMPIObjectPath* refInitiator,
      const CMPIObjectPath* refTarget,
      const CMPIObjectPath* refLogicalUnit)
{
   /* construct instance for the association */
   CMPIObjectPath * op = NULL;
   CMPIInstance* cin = NULL; 
   CMPIStatus rc = {CMPI_RC_OK, NULL};
   if(refInitiator && refTarget && refLogicalUnit)
   {
      op = CMNewObjectPath( _broker, nameSpace, _ClassName, &rc);
      if( CMIsNullObject(op) ) {
         CMSetStatusWithChars( _broker, &rc, CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." );
         goto exit;
      }
      cin = CMNewInstance( _broker, op, &rc);
      if( CMIsNullObject(cin) ) {
         CMSetStatusWithChars( _broker, &rc, CMPI_RC_ERR_FAILED, "Create CMPIInstance failed." );
         goto exit;
      }
      CMSetProperty( cin, _RefInitiator, &refInitiator, CMPI_ref);
      CMSetProperty( cin, _RefTarget, &refTarget, CMPI_ref);
      CMSetProperty( cin, _RefLU, &refLogicalUnit, CMPI_ref);
      return cin;
   }
exit:
   return NULL;
}


/* ---------------------------------------------------------------------------*/
/*                    Associator Provider Interface                           */
/* ---------------------------------------------------------------------------*/

/******************************************************************************/
/* Function: SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderAssociationCleanup */
/******************************************************************************/
CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderAssociationCleanup( 
   CMPIAssociationMI * mi,
   const CMPIContext * ctx, CMPIBoolean trm) 
{
   _OSBASE_TRACE(1,("--- %s CMPI AssociationCleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI AssociationCleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
}

/******************************************************************************/
/* Function: SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderAssociators     */
/******************************************************************************/
CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderAssociators( CMPIAssociationMI * mi,
           const CMPIContext * ctx,
           const CMPIResult * rslt,
           const CMPIObjectPath * cop,
           const char * assocClass,
           const char * resultClass,
           const char * role,
           const char * resultRole,
           const char ** propertyList ) 
{
   /* get all reference first, then set the object path of the association to the result */
   CMPIStatus       rc = {CMPI_RC_OK, NULL};
   CMPIObjectPath * op = NULL;
   int           refrc = 0;
   int  isSCSIEndpoint = 0;

   _OSBASE_TRACE(1,("--- %s CMPI Associator() called",_ClassName));
   if( assocClass ){ 
      op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
            _ClassName, &rc );
      if( CMClassPathIsA(_broker,op,assocClass,&rc) == 0 )
         goto exit;
   }
   /* the cop could be object path of initiator/target/logical unit */
   /* test the cop is initiator or target first */
   refrc = _check_param(_broker, cop, ctx, _RefInitiator, _RefTarget, _RefLU,
         _RefInitiatorCls, _RefTargetCls, _RefLUCls,
         resultClass, role, resultRole, &isSCSIEndpoint, &rc);
   if(!refrc) goto exit;

   refrc = _assoc_create_refs_FCLUPath(_broker, ctx, rslt, cop, 
         _ClassName, resultClass, role, resultRole, isSCSIEndpoint, 1, 1, &rc);

   if( refrc != 0 ) { 
      if( rc.msg != NULL )
         _OSBASE_TRACE(1,("--- %s CMPI Reference() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      else
         _OSBASE_TRACE(1,("--- %s CMPI Reference() failed",_ClassName));

      CMReturn(CMPI_RC_ERR_FAILED); 
   }

exit:
   _OSBASE_TRACE(1,("--- %s CMPI Associator() exited",_ClassName));
   CMReturnDone(rslt);
   CMReturn(CMPI_RC_OK);
}
/******************************************************************************/
/* Function: SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderAssociatorNames             */
/******************************************************************************/
CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderAssociatorNames( CMPIAssociationMI * mi,
           const CMPIContext * ctx,
           const CMPIResult * rslt,
           const CMPIObjectPath * cop,
           const char * assocClass,
           const char * resultClass,
           const char * role,
           const char * resultRole) 
{
   /* get all reference first, then set the object path of the association to the result */
   CMPIStatus       rc = {CMPI_RC_OK, NULL};
   CMPIObjectPath * op = NULL;
   int           refrc = 0;
   int  isSCSIEndpoint = 0;

   _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() called",_ClassName));
   if( assocClass ){ 
      op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
            _ClassName, &rc );
      if( CMClassPathIsA(_broker,op,assocClass,&rc) == 0 )
         goto exit;
   }
   /* the cop could be object path of initiator/target/logical unit */
   /* test the cop is initiator or target first */
   refrc = _check_param(_broker, cop, ctx, _RefInitiator, _RefTarget, _RefLU,
         _RefInitiatorCls, _RefTargetCls, _RefLUCls,
         resultClass, role, resultRole, &isSCSIEndpoint, &rc);
   if(!refrc) goto exit;

   refrc = _assoc_create_refs_FCLUPath(_broker, ctx, rslt, cop, 
         _ClassName, resultClass, role, resultRole, isSCSIEndpoint, 1, 0, &rc);

   if( refrc != 0 ) { 
      if( rc.msg != NULL )
         _OSBASE_TRACE(1,("--- %s CMPI Reference() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      else
         _OSBASE_TRACE(1,("--- %s CMPI Reference() failed",_ClassName));

      CMReturn(CMPI_RC_ERR_FAILED); 
   }

exit:
   _OSBASE_TRACE(1,("--- %s CMPI AssociatorNames() exited",_ClassName));
   CMReturnDone(rslt);
   CMReturn(CMPI_RC_OK);
}


/******************************************************************************/
/* Function: SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderReferences      */
/******************************************************************************/
CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderReferences( CMPIAssociationMI * mi,
           const CMPIContext * ctx,
           const CMPIResult * rslt,
           const CMPIObjectPath * cop,
           const char * assocClass,
           const char * role,
           const char ** propertyList ) 
{
   CMPIStatus       rc = {CMPI_RC_OK, NULL};
   CMPIObjectPath * op = NULL;
   int           refrc = 0;
   int  isSCSIEndpoint = 0;

   _OSBASE_TRACE(1,("--- %s CMPI Reference() called",_ClassName));
    
   if( assocClass ){ 
      op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
            _ClassName, &rc );
      if( CMClassPathIsA(_broker,op,assocClass,&rc) == 0 )
         goto exit;
   }
   /* the cop could be object path of initiator/target/logical unit */
   refrc = _check_param(_broker, cop, ctx, _RefInitiator, _RefTarget, _RefLU,
         _RefInitiatorCls, _RefTargetCls, _RefLUCls,
         NULL, role, NULL, &isSCSIEndpoint, &rc);
   if(!refrc) goto exit;

   refrc = _assoc_create_refs_FCLUPath(_broker, ctx, rslt, cop, 
         _ClassName, NULL, role, NULL, isSCSIEndpoint, 0, 1, &rc);

   if( refrc != 0 ) { 
      if( rc.msg != NULL )
         _OSBASE_TRACE(1,("--- %s CMPI Reference() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      else
         _OSBASE_TRACE(1,("--- %s CMPI Reference() failed",_ClassName));

      CMReturn(CMPI_RC_ERR_FAILED); 
   }

exit:
   if(op) CMRelease(op);
   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI Reference() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
}


/******************************************************************************/
/* Function: SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderReferenceNames  */
/******************************************************************************/
CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderReferenceNames( CMPIAssociationMI * mi,
           const CMPIContext * ctx,
           const CMPIResult * rslt,
           const CMPIObjectPath * cop,
           const char * assocClass,
           const char * role) 
{
   CMPIStatus       rc    = {CMPI_RC_OK, NULL};
   CMPIObjectPath * op    = NULL;
   int              refrc = 0;
   int     isSCSIEndpoint = 0;

   _OSBASE_TRACE(1,("--- %s CMPI ReferenceNames() called",_ClassName));
 
   if( assocClass ){ 
      _OSBASE_TRACE(4,("--- assoc class:%s ",assocClass));
      op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
            _ClassName, &rc );
      if( CMClassPathIsA(_broker,op,assocClass,&rc) == 0 )
         goto exit;
   }
   /* the cop could be object path of initiator/target/logical unit */
   refrc = _check_param(_broker, cop, ctx, _RefInitiator, _RefTarget, _RefLU,
         _RefInitiatorCls, _RefTargetCls, _RefLUCls,
         NULL, role, NULL, &isSCSIEndpoint, &rc);
   if(!refrc) goto exit;

   refrc = _assoc_create_refs_FCLUPath(_broker, ctx, rslt, cop, 
         _ClassName, NULL, role, NULL, isSCSIEndpoint, 0, 0, &rc);

   if( refrc != 0 ) { 
      if( rc.msg != NULL )
         _OSBASE_TRACE(1,("--- %s CMPI ReferenceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      else
         _OSBASE_TRACE(1,("--- %s CMPI ReferenceNames() failed",_ClassName));

      CMReturn(CMPI_RC_ERR_FAILED); 
   }

exit:
   if(op) CMRelease(op);
   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI ReferenceNames() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
} 



/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


/******************************************************************************/
/* Function: SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderCleanup                     */
/******************************************************************************/
CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderCleanup( CMPIInstanceMI * mi,
            const CMPIContext * ctx, CMPIBoolean trm) 
{
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
}

CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderCreateInstance( CMPIInstanceMI * mi,
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
   
CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderModifyInstance( CMPIInstanceMI * mi,
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

CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderDeleteInstance( CMPIInstanceMI * mi,
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


CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderExecQuery( CMPIInstanceMI * mi,
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

/******************************************************************************/
/* Function: SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderEnumInstanceNames*/
/******************************************************************************/
CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderEnumInstanceNames( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref) 
{
   CMPIObjectPath * opAssoc       = NULL;
   CMPIObjectPath * opTarget      = NULL;                /* target object path */
   CMPIStatus rc                  = {CMPI_RC_OK, NULL};
   RefList * lstInitiator         = NULL;
   RefList * lstTarget            = NULL;
   RefList * lstLogicalUnit       = NULL;
   char * nameSpace               = NULL; 

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() called",_ClassName));

   nameSpace = CMGetCharPtr(CMGetNameSpace(ref, &rc));

   rc = getInstanceNamesList(nameSpace, ctx, ref, &lstInitiator, &lstTarget, &lstLogicalUnit); 

   if(rc.rc == CMPI_RC_OK)
   {
      /* traverse all association, get objectpath from array, consturct the provider*/
      InitiatorTargetLUPathList* lst, *head;
      enum_all_LUPath(&lst);
      head = lst;
      while(lst)
      {
         CMPICount idx;
         CMPIData data;
         CMPIObjectPath* refInitiator, *refTarget, *refLogicalUnit, * result;
         /* WWN and is 64bit. If formated in to decimal string and NULL char, 21 chars is enough */
         char strWWN1[21], strWWN2[21];
         _OSBASE_TRACE(4, ("current path:%llx<->%llx<->%s",*(unsigned long long*)lst->path.initiator.wwn, *(unsigned long long*)lst->path.target.wwn, lst->path.deviceID));

         snprintf(strWWN1, sizeof(strWWN1), "%llx", *(unsigned long long*)lst->path.initiator.wwn);
         strWWN1[sizeof(strWWN1) - 1] = '\0';
         refInitiator = getRefByKeyValue(lstInitiator, "Name", strWWN1);
         snprintf(strWWN2, sizeof(strWWN2), "%llx", *(unsigned long long*)lst->path.target.wwn);
         strWWN2[sizeof(strWWN2) - 1] = '\0';
         refTarget = getRefByKeyValue(lstTarget, "Name", strWWN2);
         refLogicalUnit = getRefByKeyValue(lstLogicalUnit, "DeviceID", lst->path.deviceID);
         result = _makePath_SCSILUPath(nameSpace, refInitiator, refTarget, refLogicalUnit);
         if(result)
         {
            _OSBASE_TRACE(4, ("One InitTargetLU assoc with %s, %s, %s", strWWN1, strWWN2, lst->path.deviceID));
            CMReturnObjectPath( rslt, result);
         }
         lst = lst->next;
      }
      free_all_LUPath(head);
   }
exit:
   freeInstanceNamesList(lstInitiator, lstTarget, lstLogicalUnit);
   CMReturnDone(rslt);
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() exited"));
   return rc;
} 

/******************************************************************************/
/* Function: SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderEnumInstances */
/******************************************************************************/
CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderEnumInstances( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref,
            const char ** properties) 
{
   CMPIObjectPath * opAssoc       = NULL;
   CMPIObjectPath * opTarget      = NULL;                /* target object path */
   CMPIStatus rc                  = {CMPI_RC_OK, NULL};
   RefList * lstInitiator         = NULL;
   RefList * lstTarget            = NULL;
   RefList * lstLogicalUnit       = NULL;
   char * nameSpace               = NULL; 

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstance() called",_ClassName));

   nameSpace = CMGetCharPtr(CMGetNameSpace(ref, &rc));

   opAssoc = CMNewObjectPath( _broker, nameSpace, _ClassName, &rc );
   if( rc.rc != CMPI_RC_OK ) opAssoc = NULL;

   rc = getInstanceNamesList(nameSpace, ctx, ref, &lstInitiator, &lstTarget, &lstLogicalUnit); 

   if(rc.rc == CMPI_RC_OK)
   {
      /* traverse all association, get objectpath from array, consturct the provider*/
      InitiatorTargetLUPathList* lst, *head;
      enum_all_LUPath(&lst);
      head = lst;
      while(lst)
      {
         CMPICount idx;
         CMPIData data;
         CMPIObjectPath* refInitiator, *refTarget, *refLogicalUnit;
         CMPIInstance* cin = NULL;
         /* WWN and is 64bit. If formated in to decimal string and NULL char, 21 chars is enough */
         char strWWN1[21], strWWN2[21];
         _OSBASE_TRACE(4, ("current path:%llx<->%llx<->%s",*(unsigned long long*)lst->path.initiator.wwn, *(unsigned long long*)lst->path.target.wwn, lst->path.deviceID));

         snprintf(strWWN1, sizeof(strWWN1), "%llx", *(unsigned long long*)lst->path.initiator.wwn);
         strWWN1[sizeof(strWWN1) - 1] = '\0';
         refInitiator = getRefByKeyValue(lstInitiator, "Name", strWWN1);
         snprintf(strWWN2, sizeof(strWWN2), "%llx", *(unsigned long long*)lst->path.target.wwn);
         strWWN2[sizeof(strWWN2) - 1] = '\0';
         refTarget = getRefByKeyValue(lstTarget, "Name", strWWN2);
         refLogicalUnit = getRefByKeyValue(lstLogicalUnit, "DeviceID", lst->path.deviceID);

         cin = _makeInst_SCSILUPath(nameSpace, refInitiator, refTarget, refLogicalUnit);
         if(cin)
         {
            _OSBASE_TRACE(4, ("One InitTargetLU assoc with %s, %s, %s", strWWN1, strWWN2, lst->path.deviceID));
            CMReturnInstance( rslt, cin);
         }

         lst = lst->next;
      }
      free_all_LUPath(head);
   }
exit:
   freeInstanceNamesList(lstInitiator, lstTarget, lstLogicalUnit);
   CMReturnDone(rslt);
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstance() exited",_ClassName));
   return rc;
} 

/******************************************************************************/
/* Function: SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderGetInstance                 */
/******************************************************************************/
CMPIStatus SMIS_FCSCSIInitiatorTargetLogicalUnitPathProviderGetInstance( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop,
            const char ** properties) 
{
   CMPIInstance * ci = NULL;
   CMPIStatus     rc = {CMPI_RC_OK, NULL};

   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() called",_ClassName));
  
   ci = getLUInstance( _broker,ctx,cop,_ClassName,_RefInitiator, _RefTarget, _RefLU, &rc);

   if( ci == NULL ) { 
      if( rc.msg != NULL ) 
         _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      else
         _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed",_ClassName));

      CMReturn(CMPI_RC_ERR_NOT_FOUND); 
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() exited",_ClassName));
      return rc;
   }

   CMReturnInstance( rslt, ci );
   CMReturnDone(rslt);
   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() exited",_ClassName));
   return rc;
} 

/* create a SCSIInitiatorTargetLogicalUnitPath instance */
static CMPIInstance * getLUInstance( const CMPIBroker * _broker,
            const CMPIContext * ctx,
            const CMPIObjectPath * cop,
            char * _ClassName,
            char * _RefInitiator,
            char * _RefTarget,
            char * _RefLU,
            CMPIStatus * rc ) 
{
   CMPIInstance   * ci = NULL;
   CMPIObjectPath * op = NULL;

   CMPIObjectPath *opi = NULL;  /* Save initiator ref */
   CMPIObjectPath *opt = NULL;  /* Save target ref */
   CMPIObjectPath *opl = NULL;  /* Save logical unit ref */

   _OSBASE_TRACE(2,("--- getLUInstance() called"));

   /* verify the three ref of the association is valid */
   opi = verify_assoc_ref(_broker, ctx, cop, _ClassName, _RefInitiator, rc);
   if(rc->rc != CMPI_RC_OK) goto exit;

   opt = verify_assoc_ref(_broker, ctx, cop, _ClassName, _RefTarget, rc);
   if(rc->rc != CMPI_RC_OK) goto exit;

   opl = verify_assoc_ref(_broker, ctx, cop, _ClassName, _RefLU, rc);
   if(rc->rc != CMPI_RC_OK) goto exit;

   op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)),
         _ClassName, rc );
   if( CMIsNullObject(op)) 
   {
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

   CMSetProperty( ci, _RefInitiator, (CMPIValue*)&(opi), CMPI_ref );
   CMSetProperty( ci, _RefTarget, (CMPIValue*)&(opt), CMPI_ref );
   CMSetProperty( ci, _RefLU, (CMPIValue*)&(opl), CMPI_ref );

exit:
   _OSBASE_TRACE(2,("--- getLUInstance() exited"));
   return ci;
}

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( SMIS_FCSCSIInitiatorTargetLogicalUnitPathProvider,
                  SMIS_FCSCSIInitiatorTargetLogicalUnitPathProvider,
                  _broker,
                  CMNoHook)

CMAssociationMIStub( SMIS_FCSCSIInitiatorTargetLogicalUnitPathProvider,
                SMIS_FCSCSIInitiatorTargetLogicalUnitPathProvider,
                _broker,
                CMNoHook)


