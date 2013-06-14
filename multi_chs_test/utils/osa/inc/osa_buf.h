

#ifndef _OSA_BUF_H_
#define _OSA_BUF_H_

#include <osa_que.h>
#include <osa_mutex.h>

#define OSA_BUF_NUM_MAX       16

#define OSA_BUF_ID_INVALID    (-1)

typedef struct {

  int size;
  int count;
  int flags;
  int timestamp;
  void *physAddr;
  void *virtAddr;

} OSA_BufInfo;

typedef struct {

  OSA_BufInfo bufInfo[OSA_BUF_NUM_MAX];
  int curReaders[OSA_BUF_NUM_MAX];
  
  int numReaders;

  OSA_QueHndl emptyQue;
  OSA_QueHndl fullQue;
  
  OSA_MutexHndl readerLock;
      
  int numBuf;

} OSA_BufHndl;

typedef struct {

  void *bufPhyAddr[OSA_BUF_NUM_MAX];
  void *bufVirtAddr[OSA_BUF_NUM_MAX];  
  
  int numBuf;
  int numReaders;

} OSA_BufCreate;

int  OSA_bufCreate(OSA_BufHndl *hndl, OSA_BufCreate *bufInit);
int  OSA_bufDelete(OSA_BufHndl *hndl);

int  OSA_bufGetFull(OSA_BufHndl *hndl, int *bufId, Uint32 timeout);
int  OSA_bufPutEmpty(OSA_BufHndl *hndl, int bufId);

int  OSA_bufGetEmpty(OSA_BufHndl *hndl, int *bufId, Uint32 timeout);
int  OSA_bufPutFull(OSA_BufHndl *hndl, int bufId);

int  OSA_bufSwitchFull (OSA_BufHndl *hndl, int *bufId);
int  OSA_bufSwitchEmpty(OSA_BufHndl *hndl, int *bufId);

OSA_BufInfo *OSA_bufGetBufInfo(OSA_BufHndl *hndl, int bufId);

#endif /* _OSA_BUF_H_ */


