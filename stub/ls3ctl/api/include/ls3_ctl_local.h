#ifndef LS3_CTL_LOCAL_H
#define LS3_CTL_LOCAL_H

#include <ls3ctl_sif_log.h>
#include <sif_mem.h>

#define LS3_CTL_API_MAX_CALL UTL_RPC_MAX_CALL
#define LS3_CTL_SRV_END_POINT  "ls3ctl.server"

#define LS3CTL_MAX_CLIENT_ENDPOINT_NUM 8U

#define LS3_CTL_ENDPOINT_OK 0
#define LS3_CTL_ENDPOINT_NG 1

#define LS3CTL_SHARE_MEM_NAME_SIZE 24

INT32 ls3ctl_get_endpoint_name(char* endpoint_name,char* endpoint_file_name);
void ls3ctl_release_endpoint(char* endpoint_file_name);

#endif
