#ifndef __BP3_HAL_H
#define __BP3_HAL_H

#include <linux/types.h>
#include <bp3_license.h>

int BP3_SEC_HAL_Join(void);

int BP3_SEC_HAL_Leave(void);

int BP3_SEC_HAL_GetSessionToken(__u8 **session, __u32 *session_size);

int BP3_SEC_HAL_GetChipInfo(__u8 *feature_list, 
			    __u32 feature_list_size, 
			    __u32 *prod_id, 
			    __u32 *security_code, 
			    __u32 *bond_option, 
			    _Bool *provisioned);

int BP3_SEC_HAL_GetChipInfoEx(__u8 *feature_list, 
			    __u32 feature_list_size, 
			    __u32 *prod_id, 
			    __u32 *security_code, 
			    __u32 *bond_option, 
			    _Bool *provisioned,
			    __u8 **chip_sn, 
			    __u32 *chip_sn_size);

int BP3_SEC_HAL_GetProvisioningBlob(__u8 **provisioning_blob, 
                                    __u32 *provisioning_blob_size);

int BP3_SEC_HAL_ProvisionBp3(__u8 *ccf,
			     __u32 ccf_size, 
			     __u8 **log, 
			     __u32 *log_size, 
			     __u32 **status, 
			     __u32 *status_size,
			     __u8 **bin, 
			     __u32 *bin_size);

int BP3_SEC_HAL_ProvisionBP3Pak(char *file_name, __u32 *provision_status);

int BP3_SEC_HAL_RestoreBp3bin(char *file_name, __u32 *status);

int BP3_SEC_HAL_GetOtpId(__u32 *otp_id_high, __u32 *otp_id_low);

int BP3_SEC_HAL_CommitBp3Provision(__u8 *ccf, __u32 ccf_size);

int BP3_SEC_HAL_IsFeatureEnabled(__s32 feature, __u32 *enabled);

int BP3_SEC_HAL_GetFeaturesStatus(__u8 **feature_list, __u32 *feature_list_size);

int BP3_SEC_HAL_GetProductType(char *platform_name, 
                               __u32 platform_name_size, 
                               char *protocol_name, 
                               __u32 protocol_name_size);

#endif /* __BP3_HAL_H */
