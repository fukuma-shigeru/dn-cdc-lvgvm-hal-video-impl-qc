#include <gst/gst.h>

#include "vhal_gst_player.h"
#include "vhal_log_wrapper.h"

static const bool kVhalRendererShm = RENDERER_SHM;

typedef enum {
    PLAYER_STATE_NONE = 0,
    PLAYER_STATE_PLAYING,
    PLAYER_STATE_FINISHED,
    PLAYER_STATE_FAILED,
} player_state_t;

struct vhal_player {
	GMainLoop *loop;
	GstElement *pipeline;

	pthread_t thread_id;
	sem_t start_sem;
	gboolean sem_posted;
	player_state_t state;

	const struct vhal_player_cb *callback;
	void *user_data;
};

static gboolean
bus_callback(GstBus *bus, GstMessage *message, gpointer data)
{
	struct vhal_player *player = data;
	gchar *dbg;
	GError *error;

	if (!player->sem_posted) {
		sem_post(&player->start_sem);
		player->sem_posted = TRUE;
	}

	switch(GST_MESSAGE_TYPE(message)) {
	case GST_MESSAGE_EOS:
		player->state = PLAYER_STATE_FINISHED;
		g_main_loop_quit(player->loop);
		VHAL_LOGD("Player: end of stream\n");
		break;
	case GST_MESSAGE_ERROR:
		player->state = PLAYER_STATE_FAILED;
		g_main_loop_quit(player->loop);
		gst_message_parse_error(message, &error, &dbg);
		VHAL_LOGE("Player: %s", error->message);
		g_free(dbg);
		g_error_free(error);
		break;
	case GST_MESSAGE_WARNING:
		gst_message_parse_warning(message, &error, &dbg);
#ifndef TEST_STRESS
		VHAL_LOGW("Player: %s", error->message);
#else
		fprintf(stderr, "[WARNING] Player: %s\n", error->message);
#endif
		g_free(dbg);
		g_error_free(error);
		break;
	default:
		/* ignore other messages */;
	}

	return TRUE;
}

static void
pad_added(GstElement *element, GstPad *pad, gpointer data)
{
	GstElement *parser = data;
	GstPad *sinkpad = gst_element_get_static_pad(parser, "sink");
	if (!sinkpad) {
		GST_ELEMENT_ERROR(parser, RESOURCE, FAILED,
				  ("Failed to get sinkpad."), NULL);
		return;
	}
	if (gst_pad_link(pad, sinkpad))
		GST_ELEMENT_ERROR(parser, RESOURCE, FAILED,
				  ("Failed to link pads."), NULL);

	gst_object_unref(sinkpad);
}

static void
handle_player_callback(struct vhal_player *player)
{
	switch (player->state) {
	case PLAYER_STATE_FINISHED:
		if (player->callback->completion)
			player->callback->completion(player->user_data);
		break;
	case PLAYER_STATE_FAILED:
		if (player->callback->error)
			player->callback->error(player->user_data);
		break;
	default:
		; /* nothing to do */
	}
}

static void *
player_thread(void *arg)
{
	VHAL_LOGD("Player: thread started");
	struct vhal_player *player = arg;

	if (gst_element_set_state(player->pipeline, GST_STATE_PLAYING)
	     != GST_STATE_CHANGE_FAILURE) {
		VHAL_LOGD("Player: playing");
		player->state = PLAYER_STATE_PLAYING;
		g_main_loop_run(player->loop);
	} else {
		VHAL_LOGE("Player: could not set state to GST_STATE_PLAYING");
		player->state = PLAYER_STATE_FAILED;
	}

	if (!player->sem_posted) {
		sem_post(&player->start_sem);
		player->sem_posted = TRUE;
	}

	if (player->callback)
		handle_player_callback(player);

	VHAL_LOGD("Player: thread end");
	return NULL;
}

static int
setup_pipeline(struct vhal_player *player, const char *file_path)
{
	VHAL_LOGD("Player: setup pipeline");

	player->pipeline = gst_pipeline_new("movie-player");
	GstElement *source = gst_element_factory_make("filesrc", NULL);
	GstElement *demuxer = gst_element_factory_make("qtdemux", NULL);
	GstElement *parser = gst_element_factory_make("h264parse", NULL);
	GstElement *queue1 = gst_element_factory_make("queue", NULL);
	GstElement *decorder = NULL;
	if (kVhalRendererShm)
	{
		decorder = gst_element_factory_make("avdec_h264", NULL);	/* Simulator */
	}
	else
	{
#ifndef TEST_PIPELINE_FAIL
		decorder = gst_element_factory_make("qcodec2h264dec", NULL);
#else
		decorder = gst_element_factory_make("omxh264dec", NULL);
#endif
	}
	GstElement *queue2 = gst_element_factory_make("queue", NULL);
	GstElement *sink = gst_element_factory_make("waylandsink", NULL);
	if (!source || !demuxer || !parser || !decorder || !sink) {
		return -1;
	}

	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(player->pipeline));
	gst_bus_add_watch(bus, bus_callback, player);
	gst_object_unref(bus);

	g_object_set(G_OBJECT(source), "location", file_path, NULL);

	gst_bin_add_many(GST_BIN(player->pipeline),
			 source, demuxer, parser, queue1, decorder, queue2,
			 sink, NULL);
	if (!gst_element_link(source, demuxer)) {
		VHAL_LOGE("Player: failed to link objects.");
		goto err1;
	}

	if (!gst_element_link_many(parser, queue1, decorder, queue2, sink, NULL)) {
		VHAL_LOGE("Player: failed to link many objects.");
		goto err1;
	}

	g_signal_connect(demuxer, "pad-added", G_CALLBACK(pad_added),
			 parser);

	return 0;

err2:
	if (sink)
		gst_object_unref(sink);
	if (queue2)
		gst_object_unref(queue2);
	if (decorder)
		gst_object_unref(decorder);
	if (queue1)
		gst_object_unref(queue1);
	if (parser)
		gst_object_unref(parser);
	if (demuxer)
		gst_object_unref(demuxer);
	if (source)
		gst_object_unref(source);
err1:
	gst_object_unref(player->pipeline);
	return -1;
}

#ifdef TEST_CALLOC_FAIL
void *my_calloc(size_t nmemb, size_t size){				 \
	return NULL;								\
}
#define calloc(a, b) my_calloc(a, b)
#endif

#ifdef TEST_PTHREAD_FAIL
int my_pthread_create(pthread_t *thread, const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg) { \
	VHAL_LOGD("Player: thread started");				\
	return 1;							\
}
#define pthread_create(a, b, c, d) my_pthread_create(a, b, c, d)
#endif

#ifdef TEST_ERROR_OCCUR
void generate_gst_error(struct vhal_player *player)
{
	GST_ELEMENT_ERROR(player->pipeline, RESOURCE, FAILED, ("TEST ERROR."), NULL);
}
#endif

struct vhal_player *
vhal_player_start(const char *file_path, const struct vhal_player_cb *callback,
		  void *user_data)
{
	if (!file_path) {
		VHAL_LOGE("Player: file_path is NULL.");
		return NULL;
	}

	VHAL_LOGD("Player: start");

	struct vhal_player *player = calloc(1, sizeof(*player));
	if (!player) {
		VHAL_LOGE("Player: memory allocation failed.");
		return NULL;
	}

	player->callback = callback;
	player->user_data = user_data;

	if (sem_init(&player->start_sem, 0, 0)) {
		VHAL_LOGE("Player: sem_init failed.");
		goto err;
	}

	gst_init(NULL, NULL);
	player->loop = g_main_loop_new(NULL, FALSE);
	if (setup_pipeline(player, file_path) < 0)
		goto err;

	if (pthread_create(&player->thread_id, NULL, player_thread,
			   player) != 0)
		goto err;

	sem_wait(&player->start_sem);

	return player;

err:
	free(player);
	return NULL;
}

void
vhal_player_stop(struct vhal_player *player)
{
	g_main_loop_quit(player->loop);

	pthread_join(player->thread_id, NULL);

	gst_element_set_state(player->pipeline, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(player->pipeline));

	g_main_loop_unref(player->loop);
	free(player);

	VHAL_LOGD("Player: stopped");
}
