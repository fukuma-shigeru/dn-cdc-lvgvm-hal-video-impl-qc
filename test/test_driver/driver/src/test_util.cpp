
#include <string>
#include <unistd.h>
#include <experimental/source_location>
#include "test_main.h"
#include "test_util.h"
#include "file_observer.h"
#include <stdarg.h>
//#include <ios>
//#include <iomanip>
#include <time.h>

bool TestFileRead_bin(const char* p_path, void* p_value, size_t size)
{
	int32_t fileob_ret{cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS};

	std::string monitor_path(p_path);
	fileob_ret = cockpit::bs::CFileObserver::GetInstance()->Read(monitor_path, p_value, size);
	if( cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS != fileob_ret )
	{
		return false;
	}
	return true;
}

bool TestFileWrite_bin(const char* p_path, const void* p_value, size_t size)
{
	int32_t fileob_ret{cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS};

	std::string monitor_path(p_path);
	fileob_ret = cockpit::bs::CFileObserver::GetInstance()->Write(monitor_path, p_value, size);
	if( cockpit::bs::CFileObserver::FILEOB_RET_SUCCESS != fileob_ret )
	{
		return false;
	}
	return true;
}

void PrintMsg(const std::string msg, const std::experimental::source_location location = std::experimental::source_location::current())
{
	PrintMsg(msg.c_str(), location);
}

void PrintMsg(const char* msg, const std::experimental::source_location location = std::experimental::source_location::current())
{
	struct timespec	ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += (9 * 3600);	/* 本来なら環境変数経由などで+9時間を加算するべきだが高速化のため簡略 */
	ts.tv_sec %= (24 * 3600);	/* 時分秒しか使わないため年月日を削除 */
	char	tmbuf[64];
	snprintf(tmbuf, sizeof(tmbuf), "%02d:%02d:%02d.%03d ", (int)(ts.tv_sec / 3600), (int)((ts.tv_sec % 3600) / 60), (int)(ts.tv_sec % 60), (int)(ts.tv_nsec / 1000000));

	std::string absolute_path = location.file_name();
	std::size_t dir_pos = absolute_path.find_last_of("/");
	std::string file_name = absolute_path.substr(dir_pos + 1);
	std::cout << tmbuf << "[DEBUG][" << file_name << ":" << location.line() << 
		"][" << location.function_name() << "] " << msg << std::endl;
}

void PrintMsgV(const std::experimental::source_location location, const char* fmt, ...)
{
	char	buf[256];
	va_list ap;
	va_start(ap, fmt);
	(void)vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	PrintMsg(buf, location);
}
