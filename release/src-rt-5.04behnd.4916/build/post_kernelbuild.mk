include $(BUILD_DIR)/make.common

all:
	@echo
	@echo -------------------------------------------
	@echo ... kernel build at $(KERNEL_DIR)
	@echo PROFILE_KERNEL_VER is $(PROFILE_KERNEL_VER)
	-@cd $(INC_KERNEL_BASE); \
	if [ -f $(KERNEL_DIR)/kernel_repo_msg.txt ]; then cat $(KERNEL_DIR)/kernel_repo_msg.txt; rm $(KERNEL_DIR)/kernel_repo_msg.txt;   fi 
