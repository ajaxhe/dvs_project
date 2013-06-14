ifndef $(COMMON_HEADER_MK)
COMMON_HEADER_MK = 1

CC := gcc
AR := ar
LD := gcc

LIB_BASE_DIR=$(BASE_DIR)/../lib
OBJ_BASE_DIR=$(BASE_DIR)/../lib/obj
EXE_BASE_DIR=$(BASE_DIR)/../bin

LIB_DIR=$(LIB_BASE_DIR)/$(CONFIG)

CC_OPTS=-c -Wall

ifeq ($(CONFIG), debug)

CC_OPTS+=-g 
OPTI_OPTS=
DEFINE=-DDEBUG

else

CC_OPTS+=
OPTI_OPTS=-O3
DEFINE=

endif

AR_OPTS=-rc
# LD_OPTS=-lpthread
LD_OPTS=-pthread

FILES=$(subst ./, , $(foreach dir,.,$(wildcard $(dir)/*.c)) ) 

vpath %.a $(LIB_DIR) 

include $(BASE_DIR)/INCLUDES.MK

INCLUDE=
INCLUDE+=$(OSA_INC) $(COM_INC) $(ORTP_INC) $(APP_INC) $(NETWORK_INC) 

endif # ifndef $(COMMON_HEADER_MK)


