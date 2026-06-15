#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <wayland-client.h>
#include <wayland-egl.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <png.h>

#include "ivi-application-client-protocol.h"

#define GL_INFO_LOG_SIZE	(256)	/* OpenGL log size */

struct window;

struct buffer {
	int width;
	int height;
	int format;
	int stride;
	char *virtAddr;

	EGLImageKHR egl_image;
	GLenum target;
	GLuint texture;
};

struct display {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct ivi_application *ivi_application;
	struct {
		EGLDisplay dpy;
		EGLContext ctx;
		EGLConfig conf;

		GLint proj_uniform;
		GLint tex_uniform;
	} egl;
	struct window *window;
	struct buffer buffer;
};

struct window {
	struct display *display;
	int width;
	int height;
	struct {
		GLuint rotation_uniform;
		GLuint pos;
		GLuint col;
	} gl;

	uint32_t frames;
	struct wl_egl_window *native;
	struct wl_surface *surface;
	struct ivi_surface *ivi_surface;
	uint32_t ivi_id;
	EGLSurface egl_surface;
};

/* vertex shader */
static const char vertex_shader[] =
	"uniform mat4 proj;\n"
	"attribute vec2 position;\n"
	"attribute vec2 texcoord;\n"
	"varying vec2 v_texcoord;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = proj * vec4(position, 0.0, 1.0);\n"
	"   v_texcoord = texcoord;\n"
	"}";

/* fragment shader */
static const char texture_fragment_shader_rgba[] =
	"precision mediump float;\n"
	"varying vec2 v_texcoord;\n"
	"uniform sampler2D tex;\n"
	"void main()\n"
	"{\n"
	"   gl_FragColor = texture2D(tex, v_texcoord);\n"
	"}\n";

static int running = 1;

static int init_egl(struct display *display, struct window *window)
{
	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_ALPHA_SIZE, 1,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	EGLint major, minor, n, count;
	EGLConfig *configs;
	EGLBoolean ret;

	display->egl.dpy = eglGetDisplay((EGLNativeDisplayType)display->display);
	if (display->egl.dpy == NULL)
	{
		fprintf(stderr, "eglGetDisplay error.\n");
		return -1;
	}

	ret = eglInitialize(display->egl.dpy, &major, &minor);
	if(ret != EGL_TRUE)
	{
		fprintf(stderr, "eglInitialize error.\n");
		return -1;
	}

	ret = eglBindAPI(EGL_OPENGL_ES_API);
	if(ret != EGL_TRUE)
	{
		fprintf(stderr, "eglBindAPI error.\n");
		return -1;
	}

	ret = eglGetConfigs(display->egl.dpy, NULL, 0, &count);
	if(ret != EGL_TRUE || count < 1)
	{
		fprintf(stderr, "eglGetConfigs error. ret=%d, count=%d\n", ret, count);
		return -1;
	}

	configs = calloc(count, sizeof *configs);
	if (configs == NULL)
	{
		fprintf(stderr, "calloc(%ld) error.\n", sizeof *configs);
		return -1;
	}

	ret = eglChooseConfig(display->egl.dpy, config_attribs, configs, count, &n);
	if(ret != EGL_TRUE || n < 1)
	{
		fprintf(stderr, "eglChooseConfig error. ret=%d, n=%d\n", ret, n);
		return -1;
	}

	display->egl.conf = configs[0];
	display->egl.ctx = eglCreateContext(display->egl.dpy, display->egl.conf, EGL_NO_CONTEXT, context_attribs);
	if (display->egl.ctx == 0)
	{
		fprintf(stderr, "eglCreateContext error.\n");
		return -1;
	}

	return 0;
}

static void
fini_egl(struct display *display)
{
	eglTerminate(display->egl.dpy);
	eglReleaseThread();
}

static GLuint
create_shader(struct window *window, const char *source, GLenum shader_type)
{
	GLuint shader;
	GLint status;
	char msg[GL_INFO_LOG_SIZE];

	shader = glCreateShader(shader_type);
	if (shader == GL_NONE)
	{
		fprintf(stderr, "eglCreateContext error.\n");
	}
	else
	{
		glShaderSource(shader, 1, (const char **) &source, NULL);
		glCompileShader(shader);
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (0 == status)
		{
			glGetShaderInfoLog(shader, GL_INFO_LOG_SIZE, NULL, msg);
			fprintf(stderr, "shader info: %s", msg);
			shader = GL_NONE;
		}
	}

	return shader;
}

static int
init_gl(struct window *window)
{
	int ret = 0;
	GLuint frag, vert;
	GLuint program;
	GLint status;
	char msg[GL_INFO_LOG_SIZE];

	frag = create_shader(window, texture_fragment_shader_rgba, GL_FRAGMENT_SHADER);
	vert = create_shader(window, vertex_shader, GL_VERTEX_SHADER);

	program = glCreateProgram();
	glAttachShader(program, frag);
	glAttachShader(program, vert);
	glBindAttribLocation(program, 0, "position");
	glBindAttribLocation(program, 1, "texcoord");
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (0 == status)
	{
		glGetProgramInfoLog(program, GL_INFO_LOG_SIZE, NULL, msg);
		fprintf(stderr, "program info: %s", msg);
		ret = -1;
	}

	window->display->egl.proj_uniform = glGetUniformLocation(program, "proj");
	window->display->egl.tex_uniform = glGetUniformLocation(program, "tex");

	glUseProgram(program);
	return ret;
}

static void
handle_ivi_surface_configure(void *data, struct ivi_surface *ivi_surface,
			     int32_t width, int32_t height)
{
	/* The width and height set by a controller are notified. */
	printf("%s(%d) (%dx%d)\n", __FUNCTION__, __LINE__, width, height);
}

static const struct ivi_surface_listener ivi_surface_listener = {
	handle_ivi_surface_configure,
};

static int
create_surface(struct window *window)
{
	struct display *display = window->display;
	EGLBoolean ret;

	window->surface = wl_compositor_create_surface(display->compositor);

	window->native = wl_egl_window_create(window->surface, window->width, window->height);
	window->egl_surface = eglCreateWindowSurface(display->egl.dpy, display->egl.conf, (EGLNativeWindowType) window->native, NULL);

	window->ivi_surface = ivi_application_surface_create(display->ivi_application, window->ivi_id, window->surface);
	ivi_surface_add_listener(window->ivi_surface, &ivi_surface_listener, window);

	ret = eglMakeCurrent(window->display->egl.dpy, window->egl_surface, window->egl_surface, window->display->egl.ctx);
	if (ret != EGL_TRUE)
	{
		fprintf(stderr, "eglMakeCurrent error.");
		return -1;
	}

	eglSwapInterval(display->egl.dpy, 1);
	return 0;
}

static void
destroy_surface(struct window *window)
{
	eglMakeCurrent(window->display->egl.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglDestroySurface(window->display->egl.dpy, window->egl_surface);
	wl_egl_window_destroy(window->native);

	wl_surface_destroy(window->surface);
}

#define PNG_HEADER_SIZE		(8)
static int read_png_file(struct buffer *buffer, const char *filename)
{
	int i;
	int ret = 0;
	FILE *fp;

	size_t memSiz;

	png_bytep *rowPtrTbl = NULL;

	/* libpng structure */
	png_structp pngPtr = NULL;
	png_infop	infPtr = NULL;

	/* buffer to check PNG signature (max 8 bytes) */
	png_byte header[PNG_HEADER_SIZE];

	png_byte color_type;
	png_byte bit_depth;

	fprintf(stderr, "load [%s].\n", filename);

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("file open error [%s]\n", filename);
		return -1;
	}

	do {
		/* check PNG signature */
		size_t read_size = fread(header, 1, PNG_HEADER_SIZE, fp);
		if (read_size != PNG_HEADER_SIZE)
		{
			fprintf(stderr, "header read size error. expected=%d, read=%ld\n", PNG_HEADER_SIZE, read_size);
			ret = -1;
			break;
		}

		if (png_sig_cmp(header, 0, PNG_HEADER_SIZE))
		{
			fprintf(stderr, "Not PngData.\n");
			ret = -1;
			break;
		}

		/* initialize libpng structure */
		pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!pngPtr)
		{
			fprintf(stderr, "png_create_read_struct failed\n");
			ret = -1;
			break;
		}

		infPtr = png_create_info_struct(pngPtr);
		if (!infPtr)
		{
			fprintf(stderr, "png_create_info_struct failed\n");
			ret = -1;
			break;
		}

		png_init_io(pngPtr, fp);
		png_set_sig_bytes(pngPtr, PNG_HEADER_SIZE);

		/* read the file header information */
		if (setjmp(png_jmpbuf(pngPtr)) == 0)
		{
			/* read information */
			png_read_info(pngPtr, infPtr);

			/* set acquired information in the texture structure. */
			buffer->width  = png_get_image_width(pngPtr, infPtr);
			buffer->height = png_get_image_height(pngPtr, infPtr);
			color_type = png_get_color_type(pngPtr, infPtr);
			bit_depth  = png_get_bit_depth(pngPtr, infPtr);
		}
		else
		{
			fprintf(stderr, "Error during init_io\n");
			ret = -1;
			break;
		}

		/* memory allocation for line tables. */
		memSiz = sizeof(png_bytep) * buffer->height;
		rowPtrTbl = (png_bytep*)malloc(memSiz);
		if (rowPtrTbl == NULL)
		{
			fprintf(stderr, "malloc failed\n");
			ret = -1;
			break;
		}
		(void)memset(rowPtrTbl, 0, (size_t)memSiz);

		/* memory allocation for line data (for all lines at once). */
		buffer->stride = buffer->width * 4;
		memSiz = buffer->stride * buffer->height;
		buffer->virtAddr = (char*) malloc(memSiz);
		if (buffer->virtAddr == NULL)
		{
			fprintf(stderr, "malloc failed\n");
			ret = -1;
			break;
		}
		(void)memset(buffer->virtAddr, 0, (size_t)memSiz);

		/* set each line address in the table. */
		for (i = 0; i < buffer->height; i++)
		{
			rowPtrTbl[i] = (png_bytep)&buffer->virtAddr[buffer->stride * i];
		}

		/* make bit depth 8. */
		if (bit_depth == 8)
		{
			/* nothing to do */
		}
		else if (bit_depth == 16)
		{
			png_set_strip_16(pngPtr);
		}
		else if (bit_depth < 8)
		{
			png_set_packing(pngPtr);
		}
		else
		{
			/* not supported */
			fprintf(stderr, "Error depth type\n");
			ret = -1;
			break;
		}
		
		/* check color type. */
		if (color_type == PNG_COLOR_TYPE_RGB ||		/* (2) : RGB       */
			color_type == PNG_COLOR_TYPE_RGB_ALPHA)	/* (6) : RGB+alpha */
		{
			buffer->format = GL_RGBA;
			/* add an opaque alpha channel if any alpha channel is not provided. */
			if (color_type == PNG_COLOR_TYPE_RGB)
			{
				png_set_filler(pngPtr, 0xff, PNG_FILLER_AFTER);
			}
		}
		else
		{
			fprintf(stderr, "format type[%d] is not supported\n", color_type);
			ret = -1;
			break;
		}

		/* read from file */
		if (setjmp(png_jmpbuf(pngPtr)) == 0)
		{
			png_read_image(pngPtr, rowPtrTbl);
		}
		else
		{
			fprintf(stderr, "Error during read_image\n");
			ret = -1;
			break;
		}

	} while(0);

	/* free work memory */
	if ((pngPtr != NULL) && (infPtr != NULL))
	{
		png_destroy_read_struct(&pngPtr, &infPtr, (png_infopp)NULL);
	}
	else if (pngPtr != NULL)
	{
		png_destroy_read_struct(&pngPtr, NULL, (png_infopp)NULL);
	}

	if (rowPtrTbl != NULL)
	{
		free(rowPtrTbl);
	}

	fclose(fp);

	if (ret == 0)
		fprintf(stderr, "successfully use file[%s]\n", filename);

	return ret;
}

static int create_buffer(struct buffer *buffer)
{
	buffer->target = GL_TEXTURE_2D;

	glGenTextures(1, &buffer->texture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(buffer->target, buffer->texture);


	glTexImage2D(buffer->target, 0, buffer->format, buffer->width, buffer->height, 0, buffer->format, GL_UNSIGNED_BYTE, buffer->virtAddr);
	return 0;
}

static void destroy_buffer(struct buffer *buffer)
{
	/* unbind texture */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* free buffer */
	glTexImage2D(buffer->target, 0, buffer->format, 0, 0, 0, buffer->format, GL_UNSIGNED_BYTE, NULL);	

	glBindTexture(buffer->target, 0);
	glDeleteTextures(1, &buffer->texture);

	if (buffer->virtAddr != NULL)
	{
		free(buffer->virtAddr);
		buffer->virtAddr = NULL;
	}
}

static void set_proj_matrix(GLint proj_uniform, int32_t width, int32_t height, int inverted)
{
	float matrix[] = {
		 2.0f, 0.0f, 0.0f, 0.0f,
		 0.0f, 2.0f, 0.0f, 0.0f,
		 0.0f, 0.0f, 0.0f, 0.0f,
		-1.0f,-1.0f, 0.0f, 1.0f,
	};

	/* invert upside down */
	if (0 == inverted)
	{
		matrix[5] = -2.0f;
		matrix[13] = 1.0f;
	}

	matrix[0] /= (float)width;
	matrix[5] /= (float)height;

	glUniformMatrix4fv(proj_uniform, 1, GL_FALSE, matrix);
}

static void redraw(struct window *window)
{
	struct display *display = window->display;
	struct buffer *buffer = &display->buffer;

	GLfloat vrtcoord[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	GLfloat texcoord[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
	GLushort indices[] = { 0, 1, 2, 3 };	/* vertex of a rectangle. 8 values for each (x,y) */
	EGLBoolean eglb;

	/* draw the whole window */
	glViewport(0, 0, buffer->width, buffer->height);

	glBlendFunc(GL_ONE, GL_ZERO);

	set_proj_matrix(display->egl.proj_uniform, buffer->width, buffer->height, 0);

	/* set texture */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(buffer->target, buffer->texture);
	glUniform1i(display->egl.tex_uniform, 0);

	glTexParameteri(buffer->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(buffer->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(buffer->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(buffer->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* disable transparency */
	glDisable(GL_BLEND);

	/* draw the whole area */
	vrtcoord[0] = 0.0f;
	vrtcoord[1] = 0.0f;
	vrtcoord[2] = (GLfloat) buffer->width;
	vrtcoord[3] = 0.0f;
	vrtcoord[4] = 0.0f;
	vrtcoord[5] = (GLfloat) buffer->height;
	vrtcoord[6] = (GLfloat) buffer->width;
	vrtcoord[7] = (GLfloat) buffer->height;

	/* draw poligons */
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vrtcoord);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texcoord);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices);

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	eglb = eglSwapBuffers(display->egl.dpy, window->egl_surface);
	if (EGL_FALSE == eglb)
	{
		fprintf(stderr, "eglSwapBuffers error.\n");
	}
}

static void
registry_handle_global(void *data, struct wl_registry *registry,
		       uint32_t name, const char *interface, uint32_t version)
{
	struct display *d = data;

	if (strcmp(interface, "wl_compositor") == 0) {
		d->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
	} else if (strcmp(interface, "ivi_application") == 0) {
		d->ivi_application = wl_registry_bind(registry, name, &ivi_application_interface, 1);
	}
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
			      uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
	registry_handle_global,
	registry_handle_global_remove
};

static void
signal_int(int signum)
{
	running = 0;
}

static void usage(void)
{
	fprintf(stderr, "Usage: halcp_video_client [OPTIONS]\n"
			"  -S <surface ID>\n"
			"  -F <filename>\n");
}

int
main(int argc, char **argv)
{
	struct sigaction sigint;
	struct display display = { 0 };
	struct window  window  = { 0 };
	int i, ret = 0;
	char *filename = NULL;

	window.display = &display;
	display.window = &window;

	for (i = 1; i < argc; i++) {
		if (strcmp("-F", argv[i]) == 0) {
			filename = argv[++i];
		} else if (strcmp("-S", argv[i]) == 0 && i+1 < argc) {
			window.ivi_id = atoi(argv[++i]);
		} else if (strcmp("-h", argv[i]) == 0) {
			usage();
			return 0;
		} else {
			usage();
			return -1;
		}
	}

	display.display = wl_display_connect(NULL);
	if (display.display == NULL)
	{
		fprintf(stderr, "wl_display_connect error.\n");
		return -1;
	}

	display.registry = wl_display_get_registry(display.display);
	wl_registry_add_listener(display.registry, &registry_listener, &display);

	wl_display_roundtrip(display.display);
	if (display.compositor == NULL || display.ivi_application == NULL)
	{
		fprintf(stderr, "wl_compositor=%p, ivi_application=%p.\n", display.compositor, display.ivi_application);
		return -1;
	}

	ret = read_png_file(&display.buffer, filename);
	if (0 > ret)
	{
		fprintf(stderr, "failed to load [%s].\n", filename);
		return -1;
	}
	window.width = display.buffer.width;
	window.height = display.buffer.height;

	ret = init_egl(&display, &window);
	if (0 > ret)
	{
		fprintf(stderr, "init_egl error.\n");
		return -1;
	}

	ret = create_surface(&window);
	if (0 > ret)
	{
		fprintf(stderr, "create_surface error.\n");
		return -1;
	}

	ret = init_gl(&window);
	if (0 > ret)
	{
		fprintf(stderr, "init_gl error.\n");
		return -1;
	}

	ret = create_buffer(&display.buffer);
	if (0 > ret)
	{
		fprintf(stderr, "failed to create buffer.\n");
		return -1;
	}

	sigint.sa_handler = signal_int;
	sigemptyset(&sigint.sa_mask);
	sigint.sa_flags = SA_RESETHAND;
	sigaction(SIGINT, &sigint, NULL);

	while (running && ret != -1) {
		redraw(&window);
		ret = wl_display_dispatch(display.display);
		sleep(1);
	}

	destroy_buffer(&display.buffer);
	destroy_surface(&window);
	fini_egl(&display);

	wl_compositor_destroy(display.compositor);
	wl_registry_destroy(display.registry);
	wl_display_flush(display.display);
	wl_display_disconnect(display.display);

	return 0;
}
