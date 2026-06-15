/*
/* Vhal動作確認用TP
 */
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <cstring>
#include <dlt/dlt.h>
#include <getopt.h>

#include "video_hal.h"


#define PROP_NUM_MAX		6		/* 同時に指定できるプロパティ数 */
#define MAX_WAIT_SECONDS	3600	/* プロパティ更新通知最大待ち時間（秒） */

#define PROP_VALUE_MAX		1024

#define VIDEO_HAL_DATA_TYPE_ULNUM			(4)		/* ulnumber (uint64_t) */

struct option_prop {
	std::string name;
	std::string type;
	std::string value;
};

struct option_operation {
	int num;
	struct option_prop prop[PROP_NUM_MAX];
};

struct option_data {
	struct option_operation read;
	struct option_operation write;
	struct option_operation notify;
	int wait_time;
	int flag_verbose;
	int flag_help;
} option;

cockpit::hal::video_hal::CtlObj vhal_obj;

typedef void (*cbFunc)(void);


static int get_type(std::string type_str)
{
	int type = 0;

	if (type_str == "num") {
		type = VIDEO_HAL_DATA_TYPE_NUM;
	}
	else if (type_str == "bool") {
		type = VIDEO_HAL_DATA_TYPE_BOOL;
	}
	else if (type_str == "str") {
		type = VIDEO_HAL_DATA_TYPE_STR;
	}
	else if (type_str == "ulnum") {
		type = VIDEO_HAL_DATA_TYPE_ULNUM;
	}

	return type;
}

static void callback(int i);
#define MAKE_CALLBACK_FUNC(a)	void callback##a(void) { callback(a); }
MAKE_CALLBACK_FUNC(0)
MAKE_CALLBACK_FUNC(1)
MAKE_CALLBACK_FUNC(2)
MAKE_CALLBACK_FUNC(3)
MAKE_CALLBACK_FUNC(4)
MAKE_CALLBACK_FUNC(5)
MAKE_CALLBACK_FUNC(6)
MAKE_CALLBACK_FUNC(7)
MAKE_CALLBACK_FUNC(8)
MAKE_CALLBACK_FUNC(9)
std::vector <cbFunc> cbFuncs = {
	callback0, callback1, callback2, callback3, callback4,
	callback5, callback6, callback7, callback8, callback9, 
};

//
// プロパティ文字列のパース
//
static void parse_property(char *arg, struct option_operation *op)
{
	int i = op->num;

	std::stringstream ss(arg);
	std::string part;

	while (std::getline(ss, part, ',')) {
		if (part.length() == 0) {
			continue;
		}
		size_t pos = part.find(":", 0);
		if (pos == std::string::npos) {
			pos = part.find("=", 0);
			if (pos == std::string::npos) {
				op->prop[i].name = part;
			} else {
				op->prop[i].name = part.substr(0, pos);
				op->prop[i].value = part.substr(pos + 1);
			}
		} else {
			op->prop[i].name = part.substr(0, pos);
			part.erase(0, pos + 1);
			pos = part.find("=", 0);
			if (pos == std::string::npos) {
				op->prop[i].type = part;
			} else {
				op->prop[i].type = part.substr(0, pos);
				op->prop[i].value = part.substr(pos + 1);
			}
		}
		i++;
		if (i == PROP_NUM_MAX) {
			break;
		}
	}

	op->num = i;
}

static std::string read_property(struct option_prop *p_prop)
{
	int ret;
	int type;
	int gettype;

	std::string str;
	int num;
	uint64_t ulnum;
	bool flg;

	void *p_buf;
	int size;
	std::string value;

	type = get_type(p_prop->type);
	gettype = type;
	switch (type)
	{
		case VIDEO_HAL_DATA_TYPE_NUM:
		{
			p_buf = &num;
			size = sizeof(num);
			break;
		}
		case VIDEO_HAL_DATA_TYPE_ULNUM:
		{
			p_buf = &ulnum;
			size = sizeof(ulnum);
			gettype = VIDEO_HAL_DATA_TYPE_NUM;
			break;
		}
		case VIDEO_HAL_DATA_TYPE_BOOL:
		{
			p_buf = &flg;
			size = sizeof(flg);
			break;
		}
		case VIDEO_HAL_DATA_TYPE_STR:
		default:
			p_buf = (void *)&str;
			size = sizeof(str);
			break;
	}

	ret = cockpit::hal::video_hal::GetValue(vhal_obj, p_prop->name, p_buf, size, gettype);
	if (VIDEO_HAL_API_SUCCESS != ret)
	{
		std::cerr << "cockpit::hal::video_hal::GetValue error. name=" << p_prop->name << " ret=" << ret << std::endl;
		return value;
	}

		switch (type)
		{
			case VIDEO_HAL_DATA_TYPE_NUM:
			{
				value = std::to_string(num);
				break;
			}
			case VIDEO_HAL_DATA_TYPE_ULNUM:
			{
				value = std::to_string(ulnum);
				break;
			}
			case VIDEO_HAL_DATA_TYPE_BOOL:
			{
				value = (flg==true)?"true":"false";
				break;
			}
			case VIDEO_HAL_DATA_TYPE_STR:
			default:
				value = str;
				break;
		}

	return value;
}

static void write_property(struct option_prop *p_prop)
{
	std::string str;
	int num;
	bool flg;

	void *p_buf;
	int size;

	int type = get_type(p_prop->type);
	switch (type)
	{
		case VIDEO_HAL_DATA_TYPE_NUM:
			p_buf = &num;
			size = sizeof(num);
			num = atoi(p_prop->value.c_str());
			break;
		case VIDEO_HAL_DATA_TYPE_BOOL:
		{
			p_buf = &flg;
			size = sizeof(flg);
			flg = (p_prop->value == "true")?true:false;
			break;
		}
		case VIDEO_HAL_DATA_TYPE_STR:
		default:
			p_buf = (void *)&str;
			str = p_prop->value;
			size = str.size() + 1;
			break;
	}
	int ret = cockpit::hal::video_hal::SetValue(vhal_obj, p_prop->name, p_buf, size, type);
	if (VIDEO_HAL_API_SUCCESS != ret)
	{
		std::cerr << "cockpit::hal::video_hal::SetValue error. ret=" << ret << std::endl;
	}
}

static void callback(int i)
{
	if (option.notify.prop[i].type.length() == 0) {
		std::cout << "notify[" << i << "] [" << option.notify.prop[i].name << "]" << std::endl;
	} else {
		std::string value = read_property(&option.notify.prop[i]);
		std::cout << "notify[" << i << "] [" << option.notify.prop[i].name << "] changed to [" << value << "]" << std::endl;
	}
}


//
// 使い方表示
//
static void print_help(void)
{
	std::cerr << "Usage: ivi_control <options>" << std::endl;
	std::cerr << "  valid options:" << std::endl;
	std::cerr << "    -R / --read=<name1:type1,name2:type2...>                : property names joined with comma to read" << std::endl;
	std::cerr << "    -W / --write=<name1:type1=value1,name2:type1=value2...> : property name and value pairs joined with comma to write" << std::endl;
	std::cerr << "    -N / --notify=<name1,name2...>                          : property names joined with comma to register notification" << std::endl;
	std::cerr << "    -T / --time=<seconds>                                   : time to wait notifications in seconds" << std::endl;
	std::cerr << "    -V / --verbose                                          : print additional information to stdout" << std::endl;
	std::cerr << "  property max is " << PROP_NUM_MAX << ", valid type: 'num','ulnum','bool','str'" << std::endl;;
}

//
// オプション取得
//
static int get_option(int argc, char *argv[], struct option_data *d)
{
	int ret;
	int result = 0;
	int opt;
	int longindex = -1;

	struct option longopts[] = {
		{ "read",    required_argument, NULL, 'R' },	/* 取得 */
		{ "write",   required_argument, NULL, 'W' },	/* 設定 */
		{ "notify",  required_argument, NULL, 'N' },	/* 通知登録 */
		{ "time",    required_argument, NULL, 'T' },	/* 待ち時間 */
		{ "verbose", no_argument,       NULL, 'V' },	/* 詳細情報表示 */
		{ "help",    no_argument,       NULL, 'H' },	/* ヘルプ表示 */
		{ 0,         0,                 0,     0  }
	};

	// オプションなしはヘルプ情報表示
	if (argc == 1) {
		d->flag_help = 1;
		return 0;
	}

	while ((opt = getopt_long(argc, argv, "R:W:N:T:VH", longopts, &longindex)) != -1) {
		std::cout << "opt[" << (char)opt << "] longindex[" << longindex << "]" << std::endl;
		switch (opt) {
			case 'R':
				parse_property(optarg, &d->read);
				if (d->read.num == 0)
				{
					result = -1;
				}
				break;
			case 'W':
				parse_property(optarg, &d->write);
				if (d->write.num == 0)
				{
					result = -1;
				}
				break;
			case 'N':
				parse_property(optarg, &d->notify);
				if (d->notify.num == 0)
				{
					result = -1;
				}
				break;
			case 'T':
				d->wait_time = atoi(optarg);
				if (d->wait_time < 0 || MAX_WAIT_SECONDS < d->wait_time) {
					std::cerr << "wait time error. optarg[" << optarg << "]" << std::endl;
					result = -1;
				}
				break;
			case 'V':
				d->flag_verbose = 1;
				break;
			case 'H':
				d->flag_help = 1;
				break;
			default:
				// std::cerr << "unknown option. opt[" << (char)opt << "] optarg[" << (optarg==nullptr?"":optarg) << "]" << std::endl;
				result = -1;
				break;
		}
		longindex = -1;
	}

	return result;
}

int vhal_control(void)
{
	int ret;

	ret = cockpit::hal::video_hal::Init(vhal_obj);
	if (VIDEO_HAL_API_SUCCESS != ret)
	{
		std::cerr << "cockpit::hal::video_hal::Init error. ret=" << ret << std::endl;
		return ret;
	}

	for (int i = 0; i < option.notify.num; i++)
	{
		ret = cockpit::hal::video_hal::RegisterCallback(vhal_obj, option.notify.prop[i].name, cbFuncs[i]);
		if (VIDEO_HAL_API_SUCCESS != ret)
		{
			std::cerr << "cockpit::hal::video_hal::RegistNtyCb error. name=" << option.notify.prop[i].name << " ret=" << ret << std::endl;
			return ret;
		}

		if (option.flag_verbose) std::cout << "watch name=" << option.notify.prop[i].name << std::endl;
	}

	for (int i = 0; i < option.read.num; i++)
	{
		std::string result = read_property(&option.read.prop[i]);

		if (option.flag_verbose) std::cout << "read name=" << option.read.prop[i].name << " value=";
		std::cout << result << std::endl;
	}

	for (int i = 0; i < option.write.num; i++)
	{
		write_property(&option.write.prop[i]);
		if (option.flag_verbose) std::cout << "write name=" << option.write.prop[i].name << " value=" << option.write.prop[i].value << std::endl;
	}

	if (option.write.num)
	{
		ret = cockpit::hal::video_hal::RequestUpdate(vhal_obj);
		if (VIDEO_HAL_API_SUCCESS != ret)
		{
			std::cerr << "cockpit::hal::video_hal::RequestUpdate error. ret=" << ret << std::endl;
			return ret;
		}
	}

	if (option.wait_time)
	{
		if (option.flag_verbose) std::cout << "# waiting events for " << option.wait_time << " seconds." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(option.wait_time));
		if (option.flag_verbose) std::cout << "# stop waiting" << std::endl;
	}

	for (int i = 0; i < option.notify.num; i++)
	{
		ret = cockpit::hal::video_hal::ClearCallback(vhal_obj, option.notify.prop[i].name, cbFuncs[i]);
		if (VIDEO_HAL_API_SUCCESS != ret)
		{
			std::cerr << "cockpit::hal::video_hal::RegistNtyCb error. name=" << option.notify.prop[i].name << " ret=" << ret << std::endl;
			return ret;
		}
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	ret = cockpit::hal::video_hal::Deinit(vhal_obj);
	if (VIDEO_HAL_API_SUCCESS != ret)
	{
		std::cerr << "cockpit::hal::video_hal::Deinit error. ret=" << ret << std::endl;
		return ret;
	}

	return VIDEO_HAL_API_SUCCESS;
}

int main(int argc, char **argv)
{
	int ret;

	// オプション取得
	ret = get_option(argc, argv, &option);
	if (ret < 0 || option.flag_help) {
		print_help();
		return -1;
	}

	DLT_REGISTER_APP("VHAL", "VideoHAL Control TP");

	ret = vhal_control();

	DLT_UNREGISTER_APP();

	return ret;
}

