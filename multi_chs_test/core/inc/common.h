/*
 * encode.h
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2007
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#ifndef _COMMON_H
#define _COMMON_H

/* Standard Linux headers */
#include "rendezvous.h"
#include "fifoutil.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

//#define ENCODE_TEST_MODE
//#define ENCODE_TEST_MODE_TWO

typedef struct _threadEnv
{
    Rendezvous_Obj rendezvousInit;
    Rendezvous_Obj rendezvousCleanup;

}ThreadEnv;

#define LOCAL_TCP_PORT 9090
#define UART_LOCAL_TCP_PORT 9092

/* system config file path*/
#define SYS_CONFIG_FILE      "./board.cfg"

/* storage file path*/
#define STORAGE_ROOT_PATH    "/tmp/" //the root path of storage  
#define STORAGE_FILE_LEN     20      //the max len of filename
#define STORAGE_PATH_LEN     20      //the max len of full path
#define STORAGE_FULLPATH_LEN (STORAGE_FILE_LEN + STORAGE_PATH_LEN)

/* video capture and encode param const define */
#define MAX_VIDEO_CHANNEL     32  // video capture channels

#define VIDEO_SYSTEM_PAL      0   // Video System: PAL
#define VIDEO_SYSTEM_NTSC     1   // Video System: NTSC

#define VIDEO_RESOLUTION_CIF  0   // Video resolution CIF
#define VIDEO_RESOLUTION_D1   1   // Video resolution D1

#define DEFAULT_D1_BITRATE    3000000 // video bitrate D1
#define DEFAULT_CIF_BITRATE   200000  // video bitrate CIF

// calc line num of image size matrix
#define GET_LINE_NUM(sys,reslution) (sys*2+reslution)

/* Function error codes */
#define SUCCESS         0
#define FAILURE         -1

/* Thread error codes */
#define THREAD_SUCCESS  (void *) 0
#define THREAD_FAILURE  (void *) -1

// define bool type
typedef unsigned char BOOL;

/* True of false enumeration */
#define TRUE            1
#define FALSE           0

/* Global data structure */
typedef struct GlobalData 
{
    int  quit;                  // Global quit flag
    int  sysmode;               // sys mode PAL or NTSC
    int  resolution;            // Video resolution CIF or D1
    int  bitrate;               // bitrate
	int  videoChannels;

    BOOL videoFlag[MAX_VIDEO_CHANNEL];   // indicate which channel need send video
    
    // audio global data.
    int sampleRate;
    int bitsPerSample;
    int audioChannels;
    int audioBitRate;

    BOOL audioFlag[MAX_VIDEO_CHANNEL];   // indicate which channel need send audio

    pthread_mutex_t mutex;               // Mutex to protect the global data 

    char storePath[STORAGE_PATH_LEN]; // storage path

} GlobalData;

/* Global data */
extern GlobalData gbl;

// image size matrix
static int IMAGE_SIZE_MATRIX[4][2] = 
{
// {width,height}
    {352, 288}, // PAL, CIF
    {704, 576}, // PAL, D1
    {352, 240}, // NTSC,CIF
    {704, 480}  // NTSC,D1
};

/* The Process Exit State */
typedef enum _ExitState
{
    DVS_WORKING = 0,	
    DVS_EXIT_SUCCESS,
    DVS_EXIT_FAIL,
    DVS_EXIT_RESET_BOARD,

}ExitState;

extern ExitState g_exitState;

static inline void gblSetExitState( ExitState state )
{
    pthread_mutex_lock(&gbl.mutex);

    g_exitState = state;

    pthread_mutex_unlock(&gbl.mutex);
}

static inline ExitState gblGetExitState()
{
    ExitState state;

    pthread_mutex_lock(&gbl.mutex);

    state = g_exitState;

    pthread_mutex_unlock(&gbl.mutex);

    return state;
}

static inline void gblSetSysMode( int sysmode )
{
    pthread_mutex_lock(&gbl.mutex);

    gbl.sysmode = sysmode;
    
    pthread_mutex_unlock(&gbl.mutex);    
}

static inline void gblSetResolution( int resolution )
{
    pthread_mutex_lock(&gbl.mutex);
    
    gbl.resolution = resolution;

    pthread_mutex_unlock(&gbl.mutex); 
}

static inline int gblGetSysMode()
{
    return gbl.sysmode;
}

static inline int gblGetResolution()
{
    return gbl.resolution;
}

static inline int gblGetWidth()
{
    int sizeLineNum;
    
    sizeLineNum = GET_LINE_NUM(gbl.sysmode,gbl.resolution);

    return IMAGE_SIZE_MATRIX[sizeLineNum][0];
}

static inline int gblGetHeight()
{
    int sizeLineNum;
    
    sizeLineNum = GET_LINE_NUM(gbl.sysmode,gbl.resolution);

    return IMAGE_SIZE_MATRIX[sizeLineNum][1];
}

static inline void gblSetBitRate( int bitrate )
{
    pthread_mutex_lock(&gbl.mutex);

    gbl.bitrate = bitrate;
    
    pthread_mutex_unlock(&gbl.mutex);
}

static inline int gblGetBitRate()
{
    return gbl.bitrate;
}

/* Functions to protect the global data */
static inline int gblGetQuit(void)
{
    int quit;

    pthread_mutex_lock(&gbl.mutex);
    
    quit = gbl.quit;
    
    pthread_mutex_unlock(&gbl.mutex);

    return quit;
}

static inline void gblSetQuit(void)
{
    pthread_mutex_lock(&gbl.mutex);

    gbl.quit = 1;
    
    pthread_mutex_unlock(&gbl.mutex);
}

static inline void gblClearQuit(void)
{
     pthread_mutex_lock(&gbl.mutex);

     gbl.quit = 0;

     pthread_mutex_unlock(&gbl.mutex);
}

static inline void gblOpenVideoChannel( int chId )
{
    pthread_mutex_lock(&gbl.mutex);   

    if( chId >= MAX_VIDEO_CHANNEL)
    {
        memset(gbl.videoFlag, TRUE, MAX_VIDEO_CHANNEL);
    }
    else
    {
        gbl.videoFlag[chId] = TRUE;
    }
	
    pthread_mutex_unlock(&gbl.mutex);
}

static inline void gblCloseVideoChannel( int chId )
{
    pthread_mutex_lock(&gbl.mutex);

    if( chId >= MAX_VIDEO_CHANNEL)
    {
        memset(gbl.videoFlag, FALSE, MAX_VIDEO_CHANNEL);
    }
    else
    {
        gbl.videoFlag[chId] = FALSE;
    }
 
    pthread_mutex_unlock(&gbl.mutex);
}

static inline BOOL gblCheckVideoChannel( int chId )
{
    BOOL bOpen = FALSE;

    pthread_mutex_lock(&gbl.mutex);

    bOpen = gbl.videoFlag[chId];

    pthread_mutex_unlock(&gbl.mutex);

    return bOpen;
}

/*
 * AUDIO GLOBAL FUNC
 */
static  inline void gblSetAudioParam(int aud_sampleRate,int aud_bitsPerSample,int aud_channels,int aud_bitRate)
{
	
    pthread_mutex_lock(&gbl.mutex);

    gbl.sampleRate = aud_sampleRate;
    gbl.audioChannels = aud_channels;
    gbl.bitsPerSample = aud_bitsPerSample;
    gbl.audioBitRate = aud_bitRate;

    pthread_mutex_unlock(&gbl.mutex);
}


static inline int gblGetAudSampleRate()
{
	int sampleRate;
    pthread_mutex_lock(&gbl.mutex);

	sampleRate =  gbl.sampleRate;
	
	pthread_mutex_unlock(&gbl.mutex);
	return sampleRate;
}

static inline int gblGetAudChannels()
{
	int audioChannels;
    pthread_mutex_lock(&gbl.mutex);

	audioChannels =  gbl.audioChannels;
	
	pthread_mutex_unlock(&gbl.mutex);
	return audioChannels;
}

static inline int gblGetAudBitsPerSample()
{
	int bitsPerSample;
    pthread_mutex_lock(&gbl.mutex);

	bitsPerSample = gbl.bitsPerSample;
	
	pthread_mutex_unlock(&gbl.mutex);
	return bitsPerSample;
}
	
static inline int gblGetAudBitRate()
{
	int audioBitRate;
    pthread_mutex_lock(&gbl.mutex);

	audioBitRate = gbl.audioBitRate;
	
	pthread_mutex_unlock(&gbl.mutex);

	return audioBitRate;
}
	
static inline void gblOpenAudioChannel( int chId )
{
	pthread_mutex_lock(&gbl.mutex);   

	if( chId >= MAX_VIDEO_CHANNEL)
	{
		memset(gbl.audioFlag, TRUE, MAX_VIDEO_CHANNEL);
	}
	else
	{
		gbl.audioFlag[chId] = TRUE;
	}
	
	pthread_mutex_unlock(&gbl.mutex);
}

static inline void gblCloseAudioChannel( int chId )
{
	pthread_mutex_lock(&gbl.mutex);

	if( chId >= MAX_VIDEO_CHANNEL)
	{
		memset(gbl.audioFlag, FALSE, MAX_VIDEO_CHANNEL);
	}
	else
	{
		gbl.audioFlag[chId] = FALSE;
	}
 
	pthread_mutex_unlock(&gbl.mutex);
}

static inline BOOL gblCheckAudioChannel( int chId )
{
	BOOL bOpen = FALSE;

	pthread_mutex_lock(&gbl.mutex);

	bOpen = gbl.audioFlag[chId];

	pthread_mutex_unlock(&gbl.mutex);

	return bOpen;
}

static inline int gblSetStorePath( char *path )
{
    pthread_mutex_lock(&gbl.mutex);

    memcpy(gbl.storePath,path,STORAGE_PATH_LEN);

    pthread_mutex_unlock(&gbl.mutex);

    return 0;    
}

static inline int gblSetVideoChannels( int num)
{
	gbl.videoChannels = num;
	return num;
}

static inline int gblGetVideoChannels()
{
	return gbl.videoChannels;
}

static inline char *gblGetStorePath()
{
    return gbl.storePath;
}

int save_sys_config();
int read_sys_config();

#endif /* COMMON_H */
