# (c) Texas Instruments 

ifndef $(COMMON_FOOTER_MK)
COMMON_FOOTER_MK = 1

OBJS=$(subst .c,.o, $(FILES) )

OBJ_DIR=$(OBJ_BASE_DIR)/$(MODULE)/$(CONFIG)

vpath %.o $(OBJ_DIR) 

LIB = $(MODULE).a

EXE = $(EXE_BASE_DIR)/$(CONFIG)/$(MODULE).out

.c.o:
	@echo  $(MODULE): Compiling $< 
	$(CC) $(CC_OPTS) $(OPTI_OPTS) $(DEFINE) $(INCLUDE) -o$(OBJ_DIR)/$@ $<


lib : $(LIB_DIR)/$(LIB)

$(LIB_DIR)/$(LIB) : $(OBJS)
	@echo  $(MODULE): Creating archive $(LIB)
	$(AR)	$(AR_OPTS) $(LIB_DIR)/$(LIB) $(OBJ_DIR)/*.o

obj: $(OBJS)
		
clean:
	@echo $(MODULE): Deleting temporary files   
	-rm -f $(LIB_DIR)/$(LIB) 
	-rm -f $(OBJ_DIR)/*.*
	
depend:
	@echo $(MODULE): Making Directories, if not already created
	-mkdir -p $(LIB_DIR)	
	-mkdir -p $(OBJ_DIR)
	@echo $(MODULE): Building dependancies
	$(CC)	$(DEFINE) $(INCLUDE) $(FILES) -M 

exe:
	@echo $(MODULE): Linking
	$(LD) $(LD_OPTS) -o$(EXE) $(LIBS)
	@echo Final executable $(EXE) !!!	
	
install:
	
			 

endif # ifndef $(COMMON_FOOTER_MK)

