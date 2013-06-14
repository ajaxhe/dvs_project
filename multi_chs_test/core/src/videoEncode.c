#include "videoEncode.h"
#include "common.h"
#include "session.h"
#include "rtpsend.h"
#include "osa_thr.h"

#include <stdio.h>

#define MAX_VIDEO_BUFFER_LEN 1000000
#define MAX_FILENAME_LEN 256


static unsigned char* video_encoded_frame_buf[MAX_VIDEO_CHANNEL] = {NULL};
static FILE* video_file_fp[MAX_VIDEO_CHANNEL] = {NULL};

static int videoEncodeInit();
static int videoEnocdeDestroy();
static int readEncodedFrameInFile(FILE *fp, int chId);

void* videoEncodeThrFxn(void *arg)
{
	ThreadEnv* envp = arg;
	//printf("In %s\n", __func__);
	if (videoEncodeInit() < 0)
	{
		printf("[video-encode-err] videoEncodeInit failed!\n");
		return THREAD_FAILURE;
	}
	Rendezvous_meet(&envp->rendezvousInit);	

	printf("[video-encode] start\n");

	int i, chs, chId;
	int frame_len;
	FrameHeader* hdr_ptr;
	SessionElement s_elt;
	chs = gblGetVideoChannels();

	s_elt.command = CMD_SESSION_BUFFER;
	s_elt.dataType = SESSION_TYPE_VIDEO;
	int frame_count = 0;

	while ( !gblGetQuit() )
	{
		chId = 2;
		frame_len = readEncodedFrameInFile(video_file_fp[chId], chId);
		if (frame_len <= 0)
			continue;

		if (frame_count % 1000 == 0)
			printf("frame_count: %d, frame_len: %d\n", frame_count, frame_len);
		frame_count++;

		s_elt.len = frame_len + FRAME_HEADER_LEN;
		
		for (i = 0; i < chs; i++)
		{
			if (i != chId)
			{
				memcpy(video_encoded_frame_buf[i], video_encoded_frame_buf[chId], s_elt.len);
				hdr_ptr = (FrameHeader*)video_encoded_frame_buf[i];
				hdr_ptr->chId = i;
			}

			s_elt.chId = i;
			s_elt.buffer = video_encoded_frame_buf[i];
			FifoUtil_put(&g_NetSessionFifos[i / CHS_PER_SESSION], &s_elt);	
		}

		usleep(40000);
	}

	s_elt.command = CMD_SESSION_FLUSH;
	for (i = 0; i < chs / CHS_PER_SESSION; i++)
	{
		FifoUtil_put(&g_NetSessionFifos[i], &s_elt);	
	}

	printf("[video-encode] exit\n");
	OSA_thrExit(NULL);
}

int videoEncodeInit()
{
	int i, chs;
	char* file_prefix = "video_channel_";
	char filename[256];

	chs = gblGetVideoChannels();
	for (i = 0; i < chs; i++)
	{
		video_encoded_frame_buf[i] = (unsigned char*)malloc(MAX_VIDEO_BUFFER_LEN);
		if (video_encoded_frame_buf[i] == NULL)
		{
			printf("malloc frame buf failed!\n");
			return -1;
		}
		snprintf(filename, MAX_FILENAME_LEN-1, "%s%d.h264", file_prefix, i);
		video_file_fp[i] = fopen(filename, "rb");	
		if (video_file_fp[i] == NULL)
		{
			printf("open %s failed!\n", filename);
			//return -1;
		}
	}
	return 0;
}
int videoEnocdeDestroy()
{
	int i, chs;
	chs = gblGetVideoChannels();
	for (i = 0; i < chs; i++)
	{
		if (video_encoded_frame_buf[i])
			free(video_encoded_frame_buf[i]);

		if (video_file_fp[i])
			fclose(video_file_fp[i]);
	}
	return 0;
}

int readEncodedFrameInFile(FILE *fp, int chId)
{
	int len, ret;

	// read frame header
	ret = fread(video_encoded_frame_buf[chId], 1, sizeof(FrameHeader), fp);
	//printf("header len: %d\n");
	if (ret == 0)
	{
		// jump to file head
		fseek(fp, 0, SEEK_SET);
		return 0;
	}
	if (ret < 0) 
		return -1;

	FrameHeader* header = (FrameHeader *)video_encoded_frame_buf[chId];
	header->chId = chId;
	len = header->len;
	//printf("frame len: %d\n", len);

	// read raw frame data
	ret = fread(video_encoded_frame_buf[chId] + sizeof(FrameHeader), 1, len, fp);

	return ret;
}
