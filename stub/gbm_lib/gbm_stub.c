
#include "gbm.h"

/* gbm_priv.hから転載 */
#define GBM_ERROR_NONE                           0x0

/*---------------------------------------------------- */
/**
 * Perform supported queries
 * Input  : operation opcodes
 *        : Arguments depending on the operation
 * Return = gbm_error value
 *
 */
int gbm_perform(int operation,...)
{
	return GBM_ERROR_NONE;
}

int gbm_bo_get_fd(struct gbm_bo *bo)
{
	return -1;
}

/*---------------------------------------------------- */
