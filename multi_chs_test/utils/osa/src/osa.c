

#include <osa.h>
#include <sys/time.h>

int OSA_attachSignalHandler(int sigId, void (*handler)(int ) )
{
  struct sigaction sigAction;

  /* insure a clean shutdown if user types ctrl-c */
  sigAction.sa_handler = handler;
  sigemptyset(&sigAction.sa_mask);
  sigAction.sa_flags = 0;
  sigaction(sigId, &sigAction, NULL);
  
  return OSA_SOK;
}

Uint32 OSA_getCurTimeInMsec()
{
  struct timeval tv;

  if (gettimeofday(&tv, NULL) < 0) 
    return 0;

  return tv.tv_sec * 1000 + tv.tv_usec/1000;
}

void   OSA_waitMsecs(Uint32 msecs)
{
  struct timespec delayTime, elaspedTime;
  
  delayTime.tv_sec  = msecs/1000;
  delayTime.tv_nsec = (msecs%1000)*1000000;

  nanosleep(&delayTime, &elaspedTime);
}

static char xtod(char c) {
  if (c>='0' && c<='9') return c-'0';
  if (c>='A' && c<='F') return c-'A'+10;
  if (c>='a' && c<='f') return c-'a'+10;
  return c=0;        // not Hex digit
}
  
static int HextoDec(char *hex, int l)
{
  if (*hex==0) 
    return(l);

  return HextoDec(hex+1, l*16+xtod(*hex)); // hex+1?
}
  
int xstrtoi(char *hex)      // hex string to integer
{
  return HextoDec(hex,0);
}
