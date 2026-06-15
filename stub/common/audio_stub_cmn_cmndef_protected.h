/****************************************************************************/
/*	機能名称		：	AudioPolicyManger API<->サービス 内部共通ヘッダ		*/
/*	ファイル名称	:	audio_stub_cmn_cmndef_protected.h					*/
/*	作成者			：	 		大城						*/
/*	バージョン		：		2024年04月15日版					*/
/****************************************************************************/
#ifndef	_AUDIO_STUB_CMN_CMNDEF_PROTECTED_H_
#define	_AUDIO_STUB_CMN_CMNDEF_PROTECTED_H_


/****************************************************************************/
/*	インクルード															*/
/****************************************************************************/
#include "com_stddef.h"
#include "sif_log.h"


/****************************************************************************/
/*	定数マクロ定義															*/
/****************************************************************************/
/*	AudioPolicyプロパティ 共有メモリ名 -------------------------------------*/
#define	APOLICY_PROP_NAME		("/PROP_APOLICY")
#define	APOLICY_PROP_SIGNATURE	(0x50524150)				/* シグニチャ：PRoperty Audio Policyの略)	*/

/*	ログ記録メッセージ マクロ定義 ------------------------------------------*/
/*	ログ強制出力マクロ定義 -------------------------------------------------*/
/*	強制ログ出力(ログ記録レベル上書き)切り替えSW ---------------------------*/
/*	NOTE:	有効化はローカル環境限定とし、コミット時は常に「0」とすること 	*/
/*			設定値の説明は以下の通り。										*/
/*			- 0:SPFログ記録レベルに従う										*/
/*			- 1:強制的に当該レベルのログを出力する(SPFログ記録レベルは無効)	*/
#define	AUDIO_STUB_FORCE_ENABLE_LOGE	(TRUE)
#define	AUDIO_STUB_FORCE_ENABLE_LOGW	(TRUE)
#define	AUDIO_STUB_FORCE_ENABLE_LOGI	(TRUE)
#define	AUDIO_STUB_FORCE_ENABLE_LOGD	(TRUE)
#define	AUDIO_STUB_FORCE_ENABLE_LOGT	(TRUE)
#define	AUDIO_STUB_FORCE_ENABLE_LOGV	(FALSE)


/****************************************************************************/
/*	関数形式マクロ定義														*/
/****************************************************************************/
#define	APOLICY_LOG_USING_SIF_LOG	(1)

/*	ログ記録メッセージ マクロ定義 ------------------------------------------*/
/*	ログ記録レベル ERROR ---------------------------------------------------*/
#if ((SPF_LOG_LEVEL >= SPF_LOG_DEF_ERROR) || AUDIO_STUB_FORCE_ENABLE_LOGE)
	#if	APOLICY_LOG_USING_SIF_LOG
	#define AUDIO_STUB_LOGE(fmt,...)	sif_log_print(SPF_LOG_DEF_ERROR, _("[E][AUDIO_STUB] %s() "fmt),__FUNCTION__,##__VA_ARGS__);
	#else
	#define AUDIO_STUB_LOGE(fmt,...)	printf("[E][AUDIO_STUB] %s() "fmt" \n",__FUNCTION__,##__VA_ARGS__);
	#endif
#else 
	#define AUDIO_STUB_LOGE(...)
#endif

/*	ログ記録レベル WARNING -------------------------------------------------*/
#if ((SPF_LOG_LEVEL >= SPF_LOG_DEF_WARN) || AUDIO_STUB_FORCE_ENABLE_LOGW)
	#if	APOLICY_LOG_USING_SIF_LOG
	#define AUDIO_STUB_LOGW(fmt,...)	sif_log_print(SPF_LOG_DEF_WARN, _("[W][AUDIO_STUB] %s() "fmt),__FUNCTION__,##__VA_ARGS__);
	#else
	#define AUDIO_STUB_LOGW(fmt,...)	printf("[W][AUDIO_STUB] %s() "fmt" \n",__FUNCTION__,##__VA_ARGS__);
	#endif
#else 
	#define AUDIO_STUB_LOGW(...)
#endif

/*	ログ記録レベル INFORMATIVE ---------------------------------------------*/
#if ((SPF_LOG_LEVEL >= SPF_LOG_DEF_INFO) || AUDIO_STUB_FORCE_ENABLE_LOGI)
	#if	APOLICY_LOG_USING_SIF_LOG
	#define AUDIO_STUB_LOGI(fmt,...)	sif_log_print(SPF_LOG_DEF_INFO, _("[I][AUDIO_STUB] %s() "fmt),__FUNCTION__,##__VA_ARGS__);
	#else
	#define AUDIO_STUB_LOGI(fmt,...)	printf("[I][AUDIO_STUB] %s() "fmt" \n",__FUNCTION__,##__VA_ARGS__);
	#endif
#else 
	#define AUDIO_STUB_LOGI(...)
#endif

/*	ログ記録レベル DEBUG ---------------------------------------------------*/
#if ((SPF_LOG_LEVEL >= SPF_LOG_DEF_DEBUG) || AUDIO_STUB_FORCE_ENABLE_LOGD)
	#if	APOLICY_LOG_USING_SIF_LOG
	#define AUDIO_STUB_LOGD(fmt,...)	sif_log_print(SPF_LOG_DEF_DEBUG, "[D][AUDIO_STUB] %s() "fmt,__FUNCTION__,##__VA_ARGS__);
	#else
	#define AUDIO_STUB_LOGD(fmt,...)	printf("[D][AUDIO_STUB] %s() "fmt" \n",__FUNCTION__,##__VA_ARGS__);
	#endif
#else 
	#define AUDIO_STUB_LOGD(...)
#endif

/*	ログ記録レベル TRACE ---------------------------------------------------*/
#if ((SPF_LOG_LEVEL >= SPF_LOG_DEF_TRACE) || AUDIO_STUB_FORCE_ENABLE_LOGT)
	#if	APOLICY_LOG_USING_SIF_LOG
	#define AUDIO_STUB_LOGT(fmt,...)	sif_log_print(SPF_LOG_DEF_TRACE, "[T][AUDIO_STUB] %s() "fmt,__FUNCTION__,##__VA_ARGS__);
	#else
	#define AUDIO_STUB_LOGT(fmt,...)	printf("[T][AUDIO_STUB] %s() "fmt" \n",__FUNCTION__,##__VA_ARGS__);
	#endif
	#define AUDIO_STUB_LOGT_FUNC_IN	AUDIO_STUB_LOGT("%s", "FUNC IN");
	#define AUDIO_STUB_LOGT_FUNC_OUT	AUDIO_STUB_LOGT("%s", "FUNC OUT");
	#define AUDIO_STUB_LOGT_FUNC_PASS	AUDIO_STUB_LOGT("%s [line = %d]", "FUNC PASS",  __LINE__);
#else 
	#define AUDIO_STUB_LOGT(...)
	#define AUDIO_STUB_LOGT_FUNC_IN
	#define AUDIO_STUB_LOGT_FUNC_OUT
	#define AUDIO_STUB_LOGT_FUNC_PASS
#endif

/*	ログ記録レベル VERBOSE -------------------------------------------------*/
#if ((SPF_LOG_LEVEL >= SPF_LOG_DEF_VERBOSE) || AUDIO_STUB_FORCE_ENABLE_LOGV)
	#if	APOLICY_LOG_USING_SIF_LOG
	#define AUDIO_STUB_LOGV(fmt,...)	sif_log_print(SPF_LOG_DEF_VERBOSE, "[V][AUDIO_STUB] %s() "fmt,__FUNCTION__,##__VA_ARGS__);
	#else
	#define AUDIO_STUB_LOGV(fmt,...)	printf("[V][AUDIO_STUB] %s() "fmt" \n",__FUNCTION__,##__VA_ARGS__);
	#endif
#else 
	#define AUDIO_STUB_LOGV(...)
#endif

#endif	/* #ifndef	_AUDIO_STUB_CMN_CMNDEF_PROTECTED_H_ */
