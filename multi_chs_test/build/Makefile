# (c) Texas Instruments 

include RULES.MK
include $(BASE_DIR)/COMMON_HEADER.MK

exe:
	make depend
	make libs
	make -fMAKEFILE.MK -C$(APP_DIR)/src MODULE=app exe
	make install
		
libs:
	make -fMAKEFILE.MK -C$(OSA_DIR)/src MODULE=osa $(TARGET)
	make -fMAKEFILE.MK -C$(COM_DIR)/src MODULE=com $(TARGET)
	make -fMAKEFILE.MK -C$(ORTP_DIR)/src MODULE=ortp $(TARGET)
	make -fMAKEFILE.MK -C$(APP_DIR)/src MODULE=app $(TARGET)
	make -fMAKEFILE.MK -C$(NETWORK_DIR)/src MODULE=network $(TARGET)
	make -fMAKEFILE.MK -C$(UART_DIR)/src MODULE=uart $(TARGET)
all:
	make clean
	make exe

clean:
	make libs TARGET=clean
	-rm -f $(EXE_BASE_DIR)/*.out
	-rm -f $(TARGET_FS_DIR)/*.out
		
depend:
	-mkdir -p $(EXE_BASE_DIR)
	-mkdir -p $(TARGET_FS_DIR)	
	make libs TARGET=depend

ar_all:
	echo making ar , tar all .o files
	echo $(AR) $(AR_OPTS) all
	$(AR) $(AR_OPTS) $(LIB_BASE_DIR)/obj/ortp/*.o $(LIB_BASE_DIR)/obj/osa/*.o $(LIB_BASE_DIR)/obj/app/*.o 

install:
	make libs TARGET=install 
	cp $(EXE_BASE_DIR)/* $(TARGET_FS_DIR)
	
.PHONY : install clean depend all app 
