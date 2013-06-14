#ifndef _SESSION_H_
#define _SESSION_H_

#include "rtpsend.h"
#include "fifoutil.h"

#define CMD_SESSION_BUFFER 0
#define CMD_SESSION_FLUSH -1

extern FifoUtil_Obj g_NetSessionFifos[];
extern FifoUtil_Obj g_NetBackSessionFifo;

typedef struct _SessionElement
{
    int command;

    unsigned char chId;
    unsigned char dataType; 

    unsigned char *buffer;
    uint32_t len;

}SessionElement;

/** add_client 
 *  
 *  add a new rtp client
 *  @destIp:    client ip addr
 *  @videoPort: client video rtp port,if = -1, then will not send video
 *  @audioPort: client audio rtp port,if = -1, then will not send audio
 *  @return:    clientID
 *  @see:       
 */
int add_client( char *destIp, int videoPort, int audioPort );

/** del_client 
 *  
 *  delete a client obj
 *  @clientID: clientID
 *  @return:   0:success,1:fail
 *  @see:      
 */

int del_client( int clientId );

/** open_channel/close_channel
 *  
 *  after open, dvs will send the chId's video and audio to client
 *  after close,dvs will stop send the chId's video and audio to client
 *  @clientID: clientID
 *  @chId:     channel id
 *  @return:   0:success,1:fail
 *  @see:      
 */
int open_channel( int clientID, int chId );
int close_channel( int clientID, int chId );

/* net session func interface*/
void *sessionThrFxn(void *arg);

extern FifoUtil_Obj g_NetSessionFifo;

#endif //session.h
