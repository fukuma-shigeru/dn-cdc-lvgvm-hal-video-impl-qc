/*
 * pfm_core.c - platform module core
 */
#include <string.h>
#include <pthread.h> 
#include "pfm_tier1_public.h"
//#include "pfm_private.h"
//#include "pfm_debug.h"
//#include "hib_util.h"
//#include "sif_mem.h"
//#include "sif_log.h"
//#include "cenv_util_public.h"
//#include "spf_log_profile.h"

#define	PFM_OBJ_LIST_MAX	32
#define	PFM_NAME_MAX		4

static BOOL	_init_done=FALSE;
static BOOL _flag_server=FALSE;
static BOOL _flag_nohib=FALSE;
static BOOL _flag_nosig=FALSE;
static void *_myobj;
static pfm_obj_t *p_pfm_obj_list[PFM_OBJ_LIST_MAX];
static pthread_mutex_t    _pfm_mutex=PTHREAD_MUTEX_INITIALIZER;

static pfm_obj_t	g_pfm_obj_t;

/* internal functions */
static void init_obj_list(void)
{
	return;
}
static BOOL add_obj_list(pfm_obj_t* obj)
{
	return TRUE;	
}
static BOOL del_obj_list(pfm_obj_t* obj)
{
	return TRUE;	
}
static pfm_obj_t* search_obj_list(STRING* name)
{
	return p_pfm_obj_list[0];	
}
static pfm_obj_t* index_obj_list(INT32 index)
{
	return p_pfm_obj_list[0];	
}
static void call_pre_initialize(pfm_obj_t *obj, pfm_ops_t* ops, void* arg)
{	
	return;
}
static void call_initialize(pfm_obj_t *obj, pfm_ops_t* ops, void* arg)
{	
	return;
}
static void call_power_state(pfm_obj_t *obj, pfm_ops_t* ops, void* arg)
{
	return;
}
static void call_clear_data(pfm_obj_t *obj, pfm_ops_t* ops, void* arg)
{
	return;
}
static void call_show_dbginfo(pfm_obj_t *obj, pfm_ops_t* ops, void* arg)
{
	return;
}
static void call_shutdown(pfm_obj_t *obj, pfm_ops_t* ops, void* arg)
{	
	return;
}
static void call_terminate(pfm_obj_t *obj, pfm_ops_t* ops, void* arg)
{	
	return;
}
static pfm_ret_t call_functions(STRING* pfm_name, 
	void (*func)(pfm_obj_t*, pfm_ops_t*, void *), void* arg)
{
	return PFM_OK;
}
static void get_siginfo(UINT32* sig_num, int* sig_list)
{
	return;
}
static void dbg_parse_and_exec(STRING* s, pfm_dbg_cmd_t* list)
{
	return ;
}

PUBLIC pfm_ret_t pfm_register(pfm_obj_t* obj)
{
	memcpy(&g_pfm_obj_t, obj, sizeof(g_pfm_obj_t));
	return PFM_OK;
}

PUBLIC pfm_ret_t pfm_unregister(pfm_obj_t* obj)
{
	return PFM_OK;
}
	
PUBLIC pfm_ret_t pfm_get_resource(STRING* pfm_name, pfm_resource_t **res)
{	
	return PFM_OK; 
}

PUBLIC pfm_id_t pfm_getmid(STRING* name)
{
	return 0;
}

PRIVATE pfm_ret_t pfm_call_pre_initialize(STRING* pfm_name, void* arg)
{
	return PFM_OK; 
}

PRIVATE pfm_ret_t pfm_call_initialize(STRING* pfm_name, void* arg)
{
	return PFM_OK; 
}

PROTECTED pfm_ret_t pfm_call_power_state(STRING* pfm_name,
	pfm_pwr_sts_kind_t kind, void* arg)
{
	return PFM_OK; 
}

PROTECTED pfm_ret_t pfm_call_clear_data(STRING* pfm_name, 
	pfm_clear_kind_t kind, void* arg)
{
	return PFM_OK; 
}

PRIVATE pfm_ret_t pfm_call_show_dbginfo(STRING* pfm_name, void* arg)
{
	return PFM_OK; 
}

PROTECTED pfm_ret_t pfm_call_shutdown(STRING* pfm_name, void* arg)
{
	return PFM_OK; 
}

PRIVATE pfm_ret_t pfm_call_terminate(STRING* pfm_name, void* arg)
{
	return PFM_OK; 
}

#define	DBGCMD_PATH	"/var/volatile/tmp/dbgcmd"
#define	CMDLIST_MAX	256
#define	STREAM_MAX	256
#define UNUSED_PARAM(a) (void)a

static void dummy(int argc, char* argv[])
{
}


PRIVATE void pfm_show_dbginfo(void* arg)
{
}

PUBLIC void pfm_dbg_setcmd(pfm_dbg_cmd_t* list)
{
	return;
}

PRIVATE void pfm_inactivate(void *arg)
{
}

INT64 _chk_file_size(const char* fname)
{
    return(0);
}

size_t _file_read(const char* fname, char* buf, size_t size)
{
    return(0);
}

//static cenv_obj_t _cenv_obj;
static BOOL _cenv_mapped = FALSE;

#define 	PFM_SHM_NAME	"pfm.value"
#if 0
#define		PFM_CONF_FILE	"/usr/local/share/dten/etc/system.conf.bin"
#else
#define		PFM_CONF_FILE	"/opt/dc-ivi-pf/share/dten/etc/system.conf.bin"
#endif

static void _initvalue(void)
{
	return;
}
	
#if 0
static void* _getvalue(const STRING* name, CENV_TYPE type)
{
	void *val;
	return val;
}

static PRIVATE pfm_ret_t _setvalue(const STRING* name, CENV_TYPE type, void* val)
{
	return PFM_OK;
}
#endif

PUBLIC INT32 pfm_getvalue_d(STRING* name)
{
	INT32 *val;
	return *val;
}	

PUBLIC UINT32 pfm_getvalue_x(STRING* name)
{
	UINT32 *val;
	return *val;
}

PUBLIC STRING* pfm_getvalue_s(STRING* name)
{
	STRING *val;
	return val;
}

PUBLIC pfm_ret_t pfm_setvalue_d(STRING* name, INT32 d)
{
	return PFM_OK; 
}

PUBLIC pfm_ret_t pfm_setvalue_x(STRING* name, UINT32 x)
{
	return PFM_OK; 
}

PUBLIC pfm_ret_t pfm_setvalue_s(STRING* name, STRING* s)
{
	return PFM_OK; 
}

extern void (*pfm_user_probe)(void);
#ifdef __PFM_FORCE_PROBE__
__attribute__((constructor))
#endif

static PRIVATE void* run_power_state(void* arg)
{
	sleep(1);
	if (g_pfm_obj_t.res.ops_list->power_state)
	{
		g_pfm_obj_t.res.ops_list->power_state(NULL, PFM_PWR_STS_NML_BOOT_START, NULL);
	}
	return arg;
}

PUBLIC void pfm_activate(void)
{
	if (pfm_user_probe)
	{
		pfm_user_probe();

		if (g_pfm_obj_t.res.ops_list->pre_initialize)
		{
			g_pfm_obj_t.res.ops_list->pre_initialize(NULL, NULL);
		}

		if (g_pfm_obj_t.res.ops_list->initialize)
		{
			g_pfm_obj_t.res.ops_list->initialize(NULL, NULL);
		}
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, run_power_state, NULL);
		pthread_detach(thread_id);
	}

	return;
}

