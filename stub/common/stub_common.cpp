
#include "file_observer.h"
#include "stub_common.h"
#include <map>


namespace
{

using NotifyList = std::map<std::string, StubClientNotifyListener>;
NotifyList list_notify_;

using IdsList = std::map<std::string, std::vector<int32_t>>;
IdsList list_ids_;

}

void CallbackFileObserver(const std::string monitor_path)
{
	const auto iter = list_notify_.find(monitor_path);
	if (iter != list_notify_.end())
	{
		StubClientNotifyListener notify = *iter->second;
		notify(monitor_path.c_str());
	}
}



int StubClientRegisterListerner(const char* p_path, StubClientNotifyListener p_listener)
{
	if ( (nullptr == p_path) || (nullptr == p_listener) )
	{
		return -1;
	}

	/* CB関数ポインタ保持 */
	std::string str_path(p_path);
	list_notify_.insert(std::make_pair(str_path, p_listener));

	/* FileObserverにCB登録 */
	std::string monitor_path{p_path};
	std::vector<std::string> monitor_path_list{monitor_path};

	std::vector<int32_t> fileob_id_list = cockpit::bs::CFileObserver::GetInstance()->RegisterListener(CallbackFileObserver, monitor_path_list);

	list_ids_.insert(std::make_pair(str_path, fileob_id_list));

	return 0;
}

int StubClientUnRegisterListerner(const char* p_path)
{
	if ( nullptr == p_path )
	{
		return -1;
	}

	std::string str_path(p_path);
	const auto iter = list_ids_.find(str_path);
	if (iter != list_ids_.end())
	{
		std::vector<int32_t> ids = iter->second;
		std::vector<int32_t> fileob_id_list = cockpit::bs::CFileObserver::GetInstance()->UnRegisterListener(ids);
		list_ids_.erase(iter);
	}

	list_notify_.erase(str_path);

	return 0;
}

/* 動作未確認 */
int StubClientRead_i32(const char* p_path, int* value)
{
	int32_t fileob_ret{cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS};
	int32_t tmp_value{0};

	std::string monitor_path(p_path);
	fileob_ret = cockpit::bs::CFileObserver::GetInstance()->Read(monitor_path, static_cast<void*>(&tmp_value), sizeof(tmp_value));
	if( cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS != fileob_ret )
	{
		return -1;
	}
	*value = tmp_value;
	return 0;
}

int StubClientRead_bin(const char* p_path, int* value, int size)
{
	int32_t fileob_ret{cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS};

	std::string monitor_path(p_path);
	fileob_ret = cockpit::bs::CFileObserver::GetInstance()->Read(monitor_path, static_cast<void*>(value), size);
	if( cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS != fileob_ret )
	{
		return -1;
	}
	return 0;
}

int StubClientWrite_bin(const char* p_path, int* value, int size)
{
	int32_t fileob_ret{cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS};

	std::string monitor_path(p_path);
	fileob_ret = cockpit::bs::CFileObserver::GetInstance()->Write(monitor_path, static_cast<void*>(value), size);
	if( cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS != fileob_ret )
	{
		return -1;
	}
	return 0;
}

int StubClientRead_str(const char* p_path, char* p_str)
{
	int32_t fileob_ret{cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS};
	std::string monitor_path(p_path);
	std::string readStr;

	if(nullptr == p_str)
	{
		return -1;
	}

	fileob_ret = cockpit::bs::CFileObserver::GetInstance()->Read(monitor_path, readStr);
	if( cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS != fileob_ret )
	{
		return -1;
	}

	strncpy(p_str, readStr.c_str(), readStr.length()+1);

	return 0;
}

int StubClientWrite_str(const char* p_path, char* p_str)
{
	int32_t fileob_ret{cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS};
	std::string monitor_path(p_path);
	std::string writeStr(p_str);

	fileob_ret = cockpit::bs::CFileObserver::GetInstance()->Write(monitor_path, writeStr);
	if( cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS != fileob_ret )
	{
		return -1;
	}

	return 0;
}
