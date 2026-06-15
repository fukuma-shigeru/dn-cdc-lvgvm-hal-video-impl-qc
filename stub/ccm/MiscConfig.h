#ifndef MISC_CONFIG_H
#define MISC_CONFIG_H


/*Tab・Misc共通*/
#define STUB_PATH					"/run/arene/vehicle_fs/var/bev3/stub/ccm_client/"
#define STUB_PATH_FUNC				STUB_PATH "func_ret/"
#define STUB_FUNC(_fnc)				STUB_PATH_FUNC #_fnc
#define STUB_CALL(_fnc)				STUB_PATH #_fnc "_call"


/*Tab用*/
#define STUB_PATH_CB				STUB_PATH "TabCtrlApiPwCmdNtyCB/"
#define STUB_PARAM_STS(_no, _itm)	STUB_PATH_CB #_no "_" #_itm "_param"
#define STUB_PARAM_CTL(_no)			STUB_PATH_CB #_no "_control"


/* Misc用 */
#define STUB_PATH_CB2				STUB_PATH "MiscCtrlApiDataNtyCB/"
#define STUB_PATH_CB3				STUB_PATH "MiscCtrlApiStsCB/"
#define STUB_PARAM_STS2(_no, _itm)	STUB_PATH_CB2 #_no "_" #_itm "_param"
#define STUB_PARAM_STS3(_no, _itm)	STUB_PATH_CB3 #_no "_" #_itm "_param"
#define STUB_PARAM_CTL2(_no)		STUB_PATH_CB2 #_no "_control"
#define STUB_PARAM_CTL3(_no)		STUB_PATH_CB3 #_no "_control"


#endif // CONFIG_H
