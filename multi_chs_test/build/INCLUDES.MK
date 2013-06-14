# (c) Texas Instruments 

ifndef $(INCLUDES_MK)
INCLUDES_MK = 1

# all of src/inc code base dir

APP_DIR    = $(BASE_DIR)/../core
UTILS_DIR  = $(BASE_DIR)/../utils
OSA_DIR    = $(UTILS_DIR)/osa
ORTP_DIR   = $(UTILS_DIR)/ortp
COM_DIR    = $(UTILS_DIR)/com
NETWORK_DIR = $(BASE_DIR)/../network

# all of the inc dir

APP_INC     = -I$(APP_DIR)/inc
OSA_INC     = -I$(OSA_DIR)/inc
COM_INC     = -I$(COM_DIR)/inc
ORTP_INC    = -I$(ORTP_DIR)/inc 
NETWORK_INC = -I$(NETWORK_DIR)/inc


endif # ifndef $(INCLUDES_MK)

