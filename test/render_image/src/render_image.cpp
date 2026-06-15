/*
 * NV12描画サンプル
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <png.h>
#if 1	// for BEVstep3
#include "drm_fourcc.h"
#include "gbm.h"
#endif
#include "wl_renderer_public.h"


#define MAX_BUFFER_PLANES			2
#define MAX_BUFFER_COUNT			5

/* ログ出力 */
#if 0
#define PRINT_LOGE(fmt, ...)	fprintf(stderr, "[E] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#define PRINT_LOGI(fmt, ...)	fprintf(stderr, "[D] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#define PRINT_LOGD(fmt, ...)
#else
#define PRINT_LOGE(fmt, ...)	fprintf(stderr, "[E] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#define PRINT_LOGI(fmt, ...)	fprintf(stderr, "[I] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#define PRINT_LOGD(fmt, ...)	fprintf(stderr, "[D] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#endif

enum e_buf_fmt1 {
	fmt1_shm = 0,
	fmt1_dma,
	fmt1_gbm,
	fmt1_max
};

enum e_buf_fmt2 {
	fmt2_xrgb = 0,
	fmt2_nv12,
	fmt2_uyvy,
	fmt2_yuy2,
	fmt2_max
};

constexpr bool	g_shm_only = RENDERER_SHM_ONLY;
constexpr int	g_buf_fmt[fmt1_max][fmt2_max] = {
	{ WL_SHM_FORMAT_XRGB8888,	WL_SHM_FORMAT_NV12,	WL_SHM_FORMAT_UYVY,	WL_SHM_FORMAT_YUYV	 },
	{ DRM_FORMAT_XRGB8888,		DRM_FORMAT_NV12,	DRM_FORMAT_UYVY,	DRM_FORMAT_YUYV		 },
	{ GBM_FORMAT_XRGB8888,		GBM_FORMAT_NV12,	GBM_FORMAT_UYVY,	GBM_FORMAT_YUYV		 },
};

/* バッファのプレーン情報 */
struct plane {
	int dmabuf_fd;
	uint32_t stride;
	void *virtAddr;
};

/* バッファ情報 */
struct buffer {
	int width;
	int height;
	int format;
	int plane_count;

	struct plane plane[MAX_BUFFER_PLANES];

	wlrenderer::CWaylandRendererVideoBuffer* p_video_buffer_;
};

struct PngInfo
{
    png_uint_32 width;
    png_uint_32 height;
    unsigned char *data;
    bool has_alpha;
};

/* コマンドラインオプション */
struct option_data {
	char *wayland;
	int surface_front;
	int surface_rear;
	int surface_meter;
	int width;
	int height;

	int flag_help;
	int flag_debug;

	char *filename;
	int colorspace;
	int	file_format_idx;
};

/* クライアント全体データ構造体 */
struct client_data {
	wlrenderer::CWaylandRenderer *p_renderer_;
	wlrenderer::CWaylandRendererConfig* p_renderer_config_;
	wlrenderer::CWaylandRendererVideo*  p_renderer_video_;

	uint32_t buffer_count;

	struct buffer  buffer[MAX_BUFFER_COUNT];
};

/* 関数プロトタイプ */
static int initialize(struct client_data *p_data, const char *wayland, int width, int height, int surface_front, int surface_rear, int surface_meter, int file_format, int colorspace);
static void finalize(struct client_data *p_data);
static void redraw(struct client_data *p_data, int index);
static int loadNV12(struct buffer *p_buf, const char *filename);
static int loadPng(struct buffer *p_buf, const char *filename, struct PngInfo *pngInf);
static void option_dump(struct option_data *d);
static void print_help(void);
static int get_option(int argc, char *argv[], struct option_data *p_opt);




/* 全体の初期化処理 */
static int initialize(struct client_data *p_data, const char *wayland, int width, int height, int surface_front, int surface_rear, int surface_meter, int file_format, int colorspace)
{
	PRINT_LOGD("initialize start.");

	p_data->buffer_count = MAX_BUFFER_COUNT;

	p_data->p_renderer_ = new wlrenderer::CWaylandRenderer;
	p_data->p_renderer_->Initialize();

	p_data->p_renderer_config_ = new wlrenderer::CWaylandRendererConfig();
#if 0	// for BEVstep3
	p_data->p_renderer_config_->SetBufferType(wlrenderer::CWaylandRendererVideo::VIDEO_BUFFER_TYPE_DMA);
#else
	if (true == g_shm_only)
	{
		PRINT_LOGD("buffer_type = SHM");
		p_data->p_renderer_config_->SetBufferType(wlrenderer::CWaylandRendererVideo::VIDEO_BUFFER_TYPE_SHM);
	}
	else
	{
		if (0 < colorspace)
		{
			PRINT_LOGD("buffer_type = GBM colorspace=(%d)", colorspace);
			p_data->p_renderer_config_->SetBufferType(wlrenderer::CWaylandRendererVideo::VIDEO_BUFFER_TYPE_GBM);
			p_data->p_renderer_config_->SetColorSpace(colorspace);
		}
		else
		{
			PRINT_LOGD("buffer_type = DMA");
			p_data->p_renderer_config_->SetBufferType(wlrenderer::CWaylandRendererVideo::VIDEO_BUFFER_TYPE_DMA);
		}
	}
#endif
	p_data->p_renderer_config_->SetBufferCount(p_data->buffer_count);
	p_data->p_renderer_config_->SetWidth(width);
	p_data->p_renderer_config_->SetHeight(height);
	p_data->p_renderer_config_->SetFormat(file_format);
#if 0	// for BEVstep3
	p_data->p_renderer_config_->AddSurfaceId(surface_front);
	p_data->p_renderer_config_->SetLoopBuffer(false);
#else
	p_data->p_renderer_config_->SetLoopBuffer(true);
	if (0 <= surface_front)
	{
		p_data->p_renderer_config_->AddSurfaceId(surface_front);
	}
#endif
	if (0 <= surface_rear)
	{
		p_data->p_renderer_config_->AddSurfaceId(surface_rear);
	}
	if (0 <= surface_meter)
	{
		p_data->p_renderer_config_->AddSurfaceId(surface_meter);
	}

	/* 映像バッファ領域の取得 */
	p_data->p_renderer_video_ = p_data->p_renderer_->CreateRendererVideo(*p_data->p_renderer_config_);
	if (nullptr == p_data->p_renderer_video_)
	{
		PRINT_LOGE("CreateRendererVideo failed.");
		return -1;
	}

	/* バッファ数分 */
	for (uint32_t i = 0; i < p_data->buffer_count; ++i)
	{
		p_data->buffer[i].width = width;
		p_data->buffer[i].height= height;
		p_data->buffer[i].format= file_format;
		p_data->buffer[i].p_video_buffer_ = p_data->p_renderer_video_->GetBuffer();
		uint32_t plane_count = p_data->buffer[i].p_video_buffer_->GetPlaneCount();	/* プレーン配列数 */
		p_data->buffer[i].plane_count = plane_count;
		if (nullptr == p_data->buffer[i].p_video_buffer_)
		{
			PRINT_LOGE("GetBuffer failed idx=%d",i);
			return -1;
		}

		PRINT_LOGD("bufp[%d]=[%p] plane=[%d]", i, p_data->buffer[i].p_video_buffer_, plane_count);
		/* バッファプレーンの設定 */
		for (uint32_t planeNo = 0; planeNo < plane_count; ++planeNo)
		{
			p_data->buffer[i].plane[planeNo].dmabuf_fd = p_data->buffer[i].p_video_buffer_->GetDmafd(planeNo);
			p_data->buffer[i].plane[planeNo].stride    = p_data->buffer[i].p_video_buffer_->GetStride(planeNo);
			p_data->buffer[i].plane[planeNo].virtAddr  = p_data->buffer[i].p_video_buffer_->GetPlaneAddr(planeNo);

			PRINT_LOGD("bufno[%d] plane[%d] p_addr[%p] FD[%d]", i, planeNo, p_data->buffer[i].plane[planeNo].virtAddr, p_data->buffer[i].plane[planeNo].dmabuf_fd);
//			memset(p_addr, 0xff, p_buffer->planes[planeNo].size);
		}

	}

	PRINT_LOGD("initialize end (success).");
	return 0;
}




/* 全体の終了処理 */
static void finalize(struct client_data *p_data)
{
	PRINT_LOGD("finalize start.");

	delete p_data->p_renderer_config_;

	p_data->p_renderer_->RemoveRendererVideo(p_data->p_renderer_video_);

	if ( nullptr != p_data->p_renderer_ )
	{
		p_data->p_renderer_->Finalize();
		delete p_data->p_renderer_;
	}

	PRINT_LOGD("finalize end.");
}

/* OpenGL描画処理 */
static void redraw(struct client_data *p_data, int index)
{

//	PRINT_LOGD("redraw start.");

	if (nullptr != p_data->buffer[index].p_video_buffer_)
	{
		std::vector<int32_t>	surface_ids{};
		p_data->p_renderer_config_->GetSurfaceIds(surface_ids);
		p_data->p_renderer_video_->SendBuffer(p_data->buffer[index].p_video_buffer_, surface_ids);
	}

//	PRINT_LOGD("redraw end.");
}

/* NV12ファイル読み込み */
static int loadNV12(struct buffer *p_buf, const char *filename)
{
	int32_t ret = 0;
	FILE* fp = NULL;

	PRINT_LOGD("loading image file %s.", filename);

	fp = fopen(filename, "rb");
	if (NULL == fp) {
		PRINT_LOGE("img file open error(%s)", filename);
		return -1;
	}

	/* YUV(NV12)用Yプレーン読み込み */
	uint8_t *p_addr0 = static_cast<uint8_t *>(p_buf->plane[0].virtAddr);
	PRINT_LOGD("p_addr(0):%p stride:%u width:%d height:%d", p_addr0, p_buf->plane[0].stride, p_buf->width, p_buf->height);
	if(NULL != p_addr0) {
		for(int32_t i = 0; i < p_buf->height; i++) {
			ret = fread(p_addr0 + (p_buf->plane[0].stride * i), p_buf->width, 1, fp);
			if (ret <= 0) {
				PRINT_LOGE("file read(y plane) error(%d) errno=%d", ret, errno);
				break;
			}
		}
	}

	/* YUV(NV12)用Cプレーン読み込み */
	uint8_t *p_addr1 = static_cast<uint8_t *>(p_buf->plane[1].virtAddr);
	PRINT_LOGD("p_addr(1):%p stride:%u width:%d height:%d", p_addr1, p_buf->plane[1].stride, p_buf->width, p_buf->height);
	if(NULL != p_addr1) {
		for(int32_t i = 0; i < p_buf->height / 2; i++) {
			ret = fread(p_addr1 + (p_buf->plane[1].stride * i), p_buf->width, 1, fp);
			if (ret <= 0) {
				PRINT_LOGE("file read(c plane) error(%d) errno=%d", ret, errno);
				break;
			}
		}
	}

	fclose(fp);

	PRINT_LOGD("loaded.");
	return 0;
}

/* PNGファイル読み込み */
static int loadPng(struct buffer *p_buf, const char *filename, struct PngInfo *pngInf)
{
	int ret;
	FILE *fp;
	png_structp pngPtr = NULL;
	png_infop   infPtr = NULL;

	int bit_depth          = 0;
	int color_type         = 0;
	int interlace_type     = 0;
	int compression_type   = 0;
	int filter_method      = 0;
	unsigned int row_bytes = 0;
	//unsigned int memSiz    = 0;
	png_bytepp rows        = NULL;

	if (!pngInf)
	{
		PRINT_LOGE("Err:pngInf is NULL");
		return -1;
	}

	if (!filename)
	{
		PRINT_LOGE("Err:file is NULL");
		return -1;
	}

	PRINT_LOGD("loading image file %s.", filename);

	fp = fopen(filename, "rb");
	if (!fp)
	{
		PRINT_LOGE("Err:file open %s", filename);
		return -1;
	}

	do
	{
		pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!pngPtr)
		{
			PRINT_LOGE("Err:png_create_read_struct");
			ret = -1;
			break;
		}
		infPtr = png_create_info_struct(pngPtr);
		if (!infPtr)
		{
			PRINT_LOGE("Err:png_create_info_struct");
			ret = -1;
			break;
		}

		/* ファイルヘッダ情報の読み込み */
		if (setjmp(png_jmpbuf(pngPtr)) == 0)
		{
			png_init_io(pngPtr, fp);
			png_set_sig_bytes(pngPtr, 0);

			png_read_png(pngPtr, infPtr, (PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND), NULL);

			/* 読み込んだ情報をテクスチャ構造体に設定 */
			png_get_IHDR(pngPtr, infPtr, &pngInf->width, &pngInf->height, &bit_depth, &color_type, &interlace_type,
						&compression_type, &filter_method);

			//row_bytes = (unsigned int)png_get_rowbytes(pngPtr, infPtr);
			//memSiz = row_bytes * pngInf->height;
			//pngInf->data = malloc(memSiz);
			row_bytes = p_buf->plane[0].stride;
			pngInf->data = static_cast<uint8_t *>(p_buf->plane[0].virtAddr);
			rows = png_get_rows(pngPtr, infPtr);
			//PRINT_LOGD("png_size.   width:%d, height:%d, stride:%d", pngInf->width, pngInf->height, row_bytes);
			
			for (unsigned int i = 0; i < pngInf->height; ++i)
			{
				memcpy(pngInf->data + (row_bytes * i), rows[i], row_bytes);
				/* ARGBデータのRB交換 */
				for (unsigned int j = 0; j < row_bytes; j+=4)
				{
				    unsigned char *pos = (pngInf->data + (row_bytes * i) + j);
					unsigned char tmp = pos[2];
					pos[2] = pos[0];
					pos[0] = tmp;
				}
			}

			png_destroy_read_struct(&pngPtr, &infPtr, NULL);

			if (color_type == PNG_COLOR_TYPE_RGBA)
			{
				pngInf->has_alpha = true;
			}
			else
			{
				pngInf->has_alpha = false;
			}
			ret = 0;
		}
		else
		{
			PRINT_LOGE("Err:png_jmpbuf");
			ret = -1;
			break;
		}
	} while(0);

	if (fp != NULL)
	{
		fclose(fp);
	}

	PRINT_LOGD("loaded.");
	return ret;
}

// for BEVstep3
static int loadYUV422(struct buffer *p_buf, const char *filename, int idx2)
{
	int32_t ret = 0;
	FILE* fp = NULL;

	PRINT_LOGD("loading image file %s.", filename);

	fp = fopen(filename, "rb");
	if (NULL == fp) {
		PRINT_LOGE("img file open error(%s)", filename);
		return -1;
	}

	/* UYVY or YUYVプレーン読み込み : プレーン数は1つのみ */
	uint8_t *p_addr0 = static_cast<uint8_t *>(p_buf->plane[0].virtAddr);
	PRINT_LOGD("p_addr(0):%p", p_addr0);
	if(NULL != p_addr0) {
		for(int32_t i = 0; i < p_buf->height; i++) {
			ret = fread(p_addr0 + (p_buf->plane[0].stride * i), p_buf->plane[0].stride, 1, fp);
			if (ret <= 0) {
				PRINT_LOGE("file read error(%d) errno=%d", ret, errno);
				break;
			}
			/* DockerのUYVYは非対応フォーマットのため、YUY2に変換する */
			if ((true == g_shm_only) && (fmt2_uyvy == idx2)) {
				uint8_t	tmp;
				uint32_t	pos;
				for (uint32_t j = 0; j < p_buf->plane[0].stride; j += 2)
				{
					pos = (p_buf->plane[0].stride * i) + j;
					tmp = *(p_addr0 + pos);
					*(p_addr0 + pos) = *(p_addr0 + pos + 1);
					*(p_addr0 + pos + 1) = tmp;
				}
			}
		}
	}

	fclose(fp);

	PRINT_LOGD("loaded.");
	return 0;
}

/* オプション構造体ダンプ（デバッグ用） */
static void option_dump(struct option_data *p_opt)
{
	PRINT_LOGI("----- option -----\n"
		"wayland[%s], surface_front[%d], surface_rear[%d], surface_meter[%d], dim(%dx%d), file(%s), flag_help[%d], flag_debug[%d]",
		p_opt->wayland, p_opt->surface_front, p_opt->surface_rear, p_opt->surface_meter, p_opt->width, p_opt->height, p_opt->filename,p_opt->flag_help, p_opt->flag_debug);
}

/* 使い方表示 */
static void print_help(void)
{
	PRINT_LOGI("Usage: render_image <options>\n"
		"  valid options:\n"
		"    -W / --wayland=<wayland-server>  : wayland-server socket name \n"
		"    -F / --surface_f=<surface ID>    : front ivi-surface ID       \n"
		"    -R / --surface_r=<surface ID>    : rear ivi-surface ID        \n"
		"    -M / --surface_m=<surface ID>    : meter ivi-surface ID       \n"
		"    -D / --dimension=<w,h>           : buffer dimension           \n"
		"    -I / --infile=<filename>         : file to be read            \n"
		"    -Y / --YUV file-type=<type>      : YUV format file_type       \n"
		"    -C / --GBM colorspace=<cs>       : colorspace                 \n"
	);
}

/* コマンドラインオプション取得 */
static int get_option(int argc, char *argv[], struct option_data *p_opt)
{
	int ret;
	int result = 0;
	int opt;
	int longindex = -1;

	struct option longopts[] = {
		{ "wayland",        required_argument, NULL, 'W' },	/* 接続socket名 */
		{ "front surface",  required_argument, NULL, 'F' },	/* ivi-surface ID */
		{ "rear surface",   required_argument, NULL, 'R' },	/* ivi-surface ID */
		{ "meter surface",  required_argument, NULL, 'M' },	/* ivi-surface ID */
		{ "dimension",      required_argument, NULL, 'D' },	/* バッファの縦横寸法 */
		{ "input file",     required_argument, NULL, 'I' }, /* データファイル */
		{ "colorspace",     required_argument, NULL, 'C' }, /* GBMカラースペース */
		{ "yuv file-type",  required_argument, NULL, 'Y' }, /* YUV ファイル種別 */
		{ "help",           no_argument,       NULL, 'H' },	/* ヘルプ表示 */
		{ 0,                0,                 0,     0  }
	};

	/* オプションなしは使い方表示 */
	if (argc == 1)
		return -1;

	while ((opt = getopt_long(argc, argv, "W:F:R:M:D:C:Y:I:HX", longopts, &longindex)) != -1) {
		PRINT_LOGD("opt[%c] longindex[%d]", opt, longindex);
		switch (opt) {
			case 'W':
				p_opt->wayland = optarg;
				break;
			case 'F':
				p_opt->surface_front = atoi(optarg);
				break;
			case 'R':
				p_opt->surface_rear = atoi(optarg);
				break;
			case 'M':
				p_opt->surface_meter = atoi(optarg);
				break;
			case 'D':
				ret = sscanf(optarg, "%d,%d", &p_opt->width, &p_opt->height);
				if (ret != 2) {
					ret = sscanf(optarg, "%dx%d", &p_opt->width, &p_opt->height);
					if (ret != 2) {
						PRINT_LOGE("dimension error. opt[%c] optarg[%s]", opt, optarg);
						result = -1;
					}
				}
				break;
			case 'I':
				p_opt->filename = optarg;
				break;
			case 'C':
				if ('6' == optarg[0])			// GBM R_601系
				{
					p_opt->colorspace = WL_RENDERER_COLOR_SPACE_ITU_R_601;
					if ('f' == tolower(optarg[1]))
					{
						p_opt->colorspace = WL_RENDERER_COLOR_SPACE_ITU_R_601_FR;
					}
				}
				else if ('7' == optarg[0])		// GBM R_709系
				{
					p_opt->colorspace = WL_RENDERER_COLOR_SPACE_ITU_R_709;
					if ('f' == tolower(optarg[1]))
					{
						p_opt->colorspace = WL_RENDERER_COLOR_SPACE_ITU_R_709_FR;
					}
				}
				else
				{
					p_opt->colorspace = 0;		// SHM/DMA
				}
				break;
			case 'Y':
				switch(tolower(optarg[0])) {
					case 'n' :
						p_opt->file_format_idx = fmt2_nv12;
						break;
					case 'u' :
						p_opt->file_format_idx = fmt2_uyvy;
						break;
					case 'y' :
						p_opt->file_format_idx = fmt2_yuy2;
						break;
					default :
						p_opt->file_format_idx = fmt2_xrgb;
						break;
				}
				break;
			case 'H':
				p_opt->flag_help = 1;
				break;
			case 'X':
				p_opt->flag_debug = 1;
				break;
			default:
				PRINT_LOGE("unknown option. opt[%c] optarg[%s]", opt, optarg);
				result = -1;
				break;
		}
		longindex = -1;
	}

	if (p_opt->flag_debug)
		option_dump(p_opt);

	return result;
}

/* render_imageサンプルプログラムメイン関数 */
int main(int argc, char *argv[])
{
	int ret;
	int file_format;
	struct client_data data;
	struct option_data option = {NULL, -1, -1, -1, 800, 480, 0, 0, NULL, 0, 0};	/* コマンドラインオプションデフォルト値 */

	PRINT_LOGD("starting...");

	memset(&data, 0, sizeof(data));

	/* コマンドラインオプション取得 */
	ret = get_option(argc, argv, &option);
	if (ret < 0 || option.flag_help) {
		print_help();
		return -1;
	}

	/* 読み込むファイル拡張子で分岐 */
#if 1	// for BEVstep3
	int  idx1 = (true == g_shm_only ? fmt1_shm :
				0 < option.colorspace ? fmt1_gbm : fmt1_dma);
	int  idx2 = option.file_format_idx;
	char* p_ext = strrchr(option.filename, '.');
	/* 拡張子で確定させる場合 */
	if (nullptr != p_ext) {
		p_ext++;
		const std::pair<const char*, int>	ext_tbl[] = {
			{ "png"  , fmt2_xrgb },
			{ "nv12" , fmt2_nv12 },
			{ "uyvy" , fmt2_uyvy },
			{ "yuyv" , fmt2_yuy2 },
			{ "yuy2" , fmt2_yuy2 },
		};
		for (auto ext : ext_tbl) {
			if (0 == strcasecmp(p_ext, ext.first)) {
				idx2 = ext.second;
				break;
			}
		}
	}
	file_format = g_buf_fmt[idx1][idx2];
	/* DockerのUYVYは非対応フォーマットのため、YUY2に変換する (ベストはwayland_clientがUYVYに対応してくれることだが…) */
	if ((true == g_shm_only) && (fmt2_uyvy == idx2)) {
		file_format = g_buf_fmt[idx1][fmt2_yuy2];
	}
	PRINT_LOGD("idx1=[%d] idx2=[%d]", idx1, idx2);

#else
//	const char *found = strstr(option.filename, ".png");
//	if (found) {
//		/* png */
//#if 0	// for BEVstep3
//		file_format = DRM_FORMAT_ARGB8888;
//#else
//		file_format = WL_SHM_FORMAT_XRGB8888;
//#endif
//    } else {
//		/* yuv */
//		file_format = DRM_FORMAT_NV12;
//    }
#endif

	/* 初期化 */
	ret = initialize(&data, option.wayland, option.width, option.height, option.surface_front, option.surface_rear, option.surface_meter, file_format, option.colorspace);
	if (ret < 0) {
		finalize(&data);
		return -1;
	}

//#if 0	// for BEVstep3
//	if (DRM_FORMAT_ARGB8888 == file_format) {
//#else
//	if (DRM_FORMAT_ARGB8888 == file_format || WL_SHM_FORMAT_XRGB8888 == file_format) {
//#endif
	if (idx2 == fmt2_xrgb) {
		/* PNGファイル読み込み */
		for( int32_t buf=0; buf < MAX_BUFFER_COUNT; ++buf)
		{
			struct PngInfo pngInf;
			ret = loadPng(&data.buffer[buf], option.filename, &pngInf);
			if (0 > ret)
			{
				PRINT_LOGE("failed to load PNG data. buf[%d]", buf);
				return -1;
			}
		}

	} else if (idx2 == fmt2_nv12) {
		/* NV12ファイル読み込み */
		for( int32_t buf=0; buf < MAX_BUFFER_COUNT; ++buf)
		{
			ret = loadNV12(&data.buffer[buf], option.filename);
			if (0 > ret)
			{
				PRINT_LOGE("failed to load NV12 data. buf[%d]", buf);
				return -1;
			}
		}
	} else {
		/* UYVY/YUYVファイル読み込み */
		for( int32_t buf=0; buf < MAX_BUFFER_COUNT; ++buf)
		{
			ret = loadYUV422(&data.buffer[buf], option.filename, idx2);
			if (0 > ret)
			{
				PRINT_LOGE("failed to load YUV422 data. buf[%d]", buf);
				return -1;
			}
		}
	}

	/* 画面描画 */
	int32_t count = 0;
	while( 1 )
	{
		redraw(&data, count);
		++count;
		if ( MAX_BUFFER_COUNT <= count )
		{
			count = 0;
		}
		sleep(1);
	}

	/* キー入力があるまで待ち。 */
	getchar();

	/*  終了 */
	PRINT_LOGD("ending...");
	finalize(&data);

	return 0;
}

