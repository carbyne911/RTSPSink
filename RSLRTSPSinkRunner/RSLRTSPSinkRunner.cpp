// RSLRTSPSinkRunner.cpp : Defines the entry point for the console application.
//
/*
#include <iostream>
#include <gst/gst.h>
#include <gst/gst.h>


int main(int argc, char* argv[])
{
	g_print("Changing state to %s", "Edu-test!");
	return 0;
}

*/

#define _CRT_SECURE_NO_WARNINGS 

#include <iostream>
#include <gst/gst.h>
#include <gst/gst.h>


#include <string.h>

static char* TAG = "RSL";

GST_DEBUG_CATEGORY_STATIC(debug_category);
#define GST_CAT_DEFAULT debug_category

#define VERIFY_ELSE_RETURN_ERROR(condition, message)         \
	if(!(condition))                                     \
			{                                                    \
		GST_ERROR((message));                            \
		return 1;                                 \
			}                                                    \

#define VERIFY_ELSE_RETURN_FALSE(condition, message)         \
	if(!(condition))                                     \
			{                                                    \
		GST_ERROR((message));                            \
		return JNI_FALSE;                                \
			}                                                    \

#define VERIFY_ELSE_LOG_ERROR(condition, message)            \
	if(!(condition))                                     \
			{                                                    \
		GST_ERROR((message));                            \
			}


static GstElement* pipeline;
static GstState current_state;

void set_error_message(const gchar *message)
{
	g_print("Setting message to: %s\n", message);
}

gboolean set_pipeline_state(GstState newState)
{
	g_print("Changing state to %s", gst_element_state_get_name(newState));
	const GstStateChangeReturn returnValue = gst_element_set_state(pipeline, newState);
	if (GST_STATE_CHANGE_FAILURE == returnValue)
	{
		g_print("Error changing state to: %s\n", gst_element_state_get_name(newState));
		return 1;
	}
	else
	{
		return 0;
	}
}

gboolean bus_call(GstBus* bus, GstMessage* msg, gpointer data)
{
	GMainLoop* loop = (GMainLoop*)data;
	const GstMessageType gstMessageType = GST_MESSAGE_TYPE(msg);
	switch (gstMessageType)
	{
	case GST_MESSAGE_EOS:
	{
		set_pipeline_state(GST_STATE_PAUSED);
		set_error_message("End of Stream");
		break;
	}
	case GST_MESSAGE_ERROR:
	{
		GError* err;
		gchar* debug_info;
		gchar* message_string;
		gst_message_parse_error(msg, &err, &debug_info);
		message_string = g_strdup_printf(
			"Error received from element %s: %s",
			GST_OBJECT_NAME(msg->src),
			err->message);
		g_clear_error(&err);
		g_free(debug_info);
		set_pipeline_state(GST_STATE_NULL);
		set_error_message(message_string);
		g_free(message_string);
		break;
	}
	case GST_MESSAGE_STATE_CHANGED:
	{
		GstState old_state, new_state, pending_state;
		gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
		/* Only pay attention to messages coming from the pipeline, not its children */
		if (GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline))
		{
			g_print("State changed to %s\n", gst_element_state_get_name(new_state));
			current_state = new_state;
			/* The Ready to Paused state change is particularly interesting: */
			if (new_state == GST_STATE_PLAYING)
			{
				GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS((GstBin *)pipeline, GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
			}
		}
		break;
	}
	case GST_MESSAGE_STREAM_STATUS:
	{
		GstStreamStatusType type;
		GstElement* owner;

		gst_message_parse_stream_status(msg,
			&type,
			&owner);

		g_print("streaming thread status %s - %d\n", owner->object.name, type);
		break;
	}
	case GST_MESSAGE_CLOCK_LOST:
	{
		set_pipeline_state(GST_STATE_PAUSED);
		set_pipeline_state(GST_STATE_PLAYING);
		break;
	}
	default:
	{
		g_print("bus callback default %d - %s\n", gstMessageType, gst_message_type_get_name(gstMessageType));
		break;
	}
	}
	return TRUE;
}

typedef enum
{
	GST_SPEEX_ENC_MODE_AUTO,
	GST_SPEEX_ENC_MODE_UWB,
	GST_SPEEX_ENC_MODE_WB,
	GST_SPEEX_ENC_MODE_NB
} GstSpeexMode;

enum GstQueueLeaky {
	GST_QUEUE_NO_LEAK = 0,
	GST_QUEUE_LEAK_UPSTREAM = 1,
	GST_QUEUE_LEAK_DOWNSTREAM = 2
};

/*
DWORD WINAPI Thread_no_1( LPVOID lpParam )
{
Sleep(5000);


//GstStateChangeReturn returnValue = gst_element_set_state (pipeline, GST_STATE_PLAYING);
//if (returnValue == GST_STATE_CHANGE_FAILURE)
//{
//	GstMessage *msg;
//	g_print ("Failed to start up pipeline!\n");

//}

return 0;
}
*/

int main(int argc, char *argv[])
{

	GError  *error = NULL;

	gst_init(&argc, &argv);

	GST_DEBUG_CATEGORY_INIT(debug_category, TAG, 0, TAG);
	gst_debug_set_threshold_for_name(TAG, GST_LEVEL_DEBUG);

	GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	pipeline = gst_pipeline_new("rsl-pipeline");

	GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	guint watch_id = gst_bus_add_watch(bus, bus_call, loop);
	gst_object_unref(bus);

	//pipeline = gst_parse_launch(" fakesrc ! rtsp_sink  ! fakesink ", &error);
	pipeline = gst_parse_launch(" videotestsrc ! x264enc ! rtph264pay ! rtsp_sink ", &error);


	if (error) {
		g_print("failed initing pipeline with : %s", error->message) ;
	}

	GstStateChangeReturn returnValue = gst_element_set_state(pipeline, GST_STATE_PLAYING);
	if (returnValue == GST_STATE_CHANGE_FAILURE) {
		g_print("Pipe failed to change state.") ;
	}

	//////////////////////////  create out elements ////////////////////////////
#if 0 
	GstElement* videoTestSrc = gst_element_factory_make("videotestsrc", "videotestsrc");
	VERIFY_ELSE_RETURN_ERROR((NULL != videoTestSrc), "error creating videoTestSrc GstElement");

	GstElement* x264enc = gst_element_factory_make("x264enc", "x264enc");
	VERIFY_ELSE_RETURN_ERROR((NULL != x264enc), "error creating x264enc GstElement");

	GstElement* rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
	VERIFY_ELSE_RETURN_ERROR((NULL != rtph264pay), "error creating rtph264pay GstElement");

	GstElement* rtspsink = gst_element_factory_make("rtsp_sink", "rtsp_sink");
	VERIFY_ELSE_RETURN_ERROR((NULL != rtspsink), "error creating rtspsink GstElement");


	gst_bin_add_many(
		GST_BIN(pipeline),
		videoTestSrc,
		x264enc,
		rtph264pay,
		rtspsink,
		NULL);

	// OUT elements linking
	gboolean result = gst_element_link_many(
		videoTestSrc,
		x264enc,
		rtph264pay,
		rtspsink,
		NULL);
	VERIFY_ELSE_RETURN_ERROR((0 != result), "error linking OUT first elements");

#endif 


#if 0



	//////////////////////////  create out elements ////////////////////////////

	GstElement* autoAudioSrc = gst_element_factory_make("directsoundsrc", "directsoundsrc0");
	VERIFY_ELSE_RETURN_ERROR((NULL != autoAudioSrc), "error creating autoAudioSrc GstElement");
	//GstElement* audioConvertOut = gst_element_factory_make("audioconvert", "audioconvert0");
	//VERIFY_ELSE_RETURN_ERROR((NULL != audioConvertOut), "error creating audioConvertOut GstElement");
	//GstElement* audioResampleOut = gst_element_factory_make("audioresample", "audioresample0");
	//VERIFY_ELSE_RETURN_ERROR((NULL != audioResampleOut), "error creating audioResampleOut GstElement");
	GstElement* capsFilterOut = gst_element_factory_make("capsfilter", "capsfilter0");
	VERIFY_ELSE_RETURN_ERROR((NULL != capsFilterOut), "error creating capsFilterOut GstElement");
	GstElement* queue_near_end_sink = gst_element_factory_make("queue", "queue_near_end_sink");
	VERIFY_ELSE_RETURN_ERROR((NULL != queue_near_end_sink), "error creating queue_near_end_sink GstElement");





	GstElement* speexAEC = gst_element_factory_make("rtsp_sink", "rtsp_sink0");
	VERIFY_ELSE_RETURN_ERROR((NULL != speexAEC), "error creating rtsp_sink GstElement");


	



	GstElement* queue_far_end_src = gst_element_factory_make("queue", "queue_far_end_src");
	VERIFY_ELSE_RETURN_ERROR((NULL != queue_far_end_src), "error creating queue_far_end_src GstElement");
	GstElement* speexEnc = gst_element_factory_make("speexenc", "speexenc0");
	VERIFY_ELSE_RETURN_ERROR((NULL != speexEnc), "error creating speexEnc GstElement");
	GstElement* rtpSpeexPay = gst_element_factory_make("rtpspeexpay", "rtpspeexpay0");
	VERIFY_ELSE_RETURN_ERROR((NULL != rtpSpeexPay), "error creating rtpSpeexPay GstElement");
	GstElement* udpSink = gst_element_factory_make("udpsink", "udpsink0");
	VERIFY_ELSE_RETURN_ERROR((NULL != udpSink), "error creating udpSink GstElement");

	GstCaps* capsFilterOutCaps = gst_caps_new_simple(
		"audio/x-raw",
		"rate", G_TYPE_INT, 8000,
		"channels", G_TYPE_INT, 1,
		NULL);
	VERIFY_ELSE_RETURN_ERROR((NULL != capsFilterOutCaps), "error creating capsFilterOutCaps GstCaps");

	g_object_set(autoAudioSrc, "buffer-time", (gint64)20000, NULL);
	g_object_set(capsFilterOut, "caps", capsFilterOutCaps, NULL);
	g_object_set(queue_near_end_sink, "leaky", GST_QUEUE_LEAK_DOWNSTREAM, NULL);
	g_object_set(queue_far_end_src, "leaky", GST_QUEUE_LEAK_DOWNSTREAM, NULL);
	g_object_set(speexEnc, "mode", GST_SPEEX_ENC_MODE_NB, NULL);
	g_object_set(udpSink, "host", "192.168.2.104", "port", 5004, NULL);

	////////////////////////// END - create out elements ////////////////////////////


	//////////////////////////  create in elements ////////////////////////////

	GstElement* udpSrc = gst_element_factory_make("udpsrc", "udpsrc0");
	VERIFY_ELSE_RETURN_ERROR((NULL != udpSrc), "error creating udpSrc GstElement");
	GstElement* rtpJitterBuffer = gst_element_factory_make("rtpjitterbuffer", "rtpjitterbuffer0");
	VERIFY_ELSE_RETURN_ERROR((NULL != rtpJitterBuffer), "error creating rtpJitterBuffer GstElement");
	GstElement* rtpSpeexDepay = gst_element_factory_make("rtpspeexdepay", "rtpspeexdepay0");
	VERIFY_ELSE_RETURN_ERROR((NULL != rtpSpeexDepay), "error creating rtpSpeexDepay GstElement");
	GstElement* speexDec = gst_element_factory_make("speexdec", "speexdec0");
	VERIFY_ELSE_RETURN_ERROR((NULL != speexDec), "error creating speexDec GstElement");
	//GstElement* tee = gst_element_factory_make("tee", "tee1");
	//VERIFY_ELSE_RETURN_ERROR((NULL != tee), "error creating tee GstElement");
	GstElement* queue_far_end_sink = gst_element_factory_make("queue", "queue_far_end_sink");
	VERIFY_ELSE_RETURN_ERROR((NULL != queue_far_end_sink), "error creating queue GstElement");
	//GstElement* queue1 = gst_element_factory_make("queue", "queue1");
	//VERIFY_ELSE_RETURN_ERROR((NULL != queue1), "error creating queue GstElement");
	//GstElement* audioConvertIn = gst_element_factory_make("audioconvert", "audioconvert1");
	//VERIFY_ELSE_RETURN_ERROR((NULL != audioConvertIn), "error creating audioConvertIn GstElement");
	//GstElement* audioResampleIn = gst_element_factory_make("audioresample", "audioresample1"); 
	//VERIFY_ELSE_RETURN_ERROR((NULL != audioResampleIn), "error creating audioResampleIn GstElement");
	//GstElement* capsFilterIn = gst_element_factory_make("capsfilter", "capsfilter1");
	//VERIFY_ELSE_RETURN_ERROR((NULL != capsFilterIn), "error creating capsFilterIn GstElement");
	GstElement* queue_near_end_src = gst_element_factory_make("queue", "queue_near_end_src");
	VERIFY_ELSE_RETURN_ERROR((NULL != queue_near_end_src), "error creating queue GstElement");
	GstElement* autoAudioSink = gst_element_factory_make("directsoundsink", "directsoundsink0");
	VERIFY_ELSE_RETURN_ERROR((NULL != autoAudioSink), "error creating autoAudioSink GstElement");

	GstCaps* udpSrcCaps = gst_caps_new_simple(
		"application/x-rtp",
		"media", G_TYPE_STRING, "audio",
		"clock-rate", G_TYPE_INT, 8000,
		"encoding-name", G_TYPE_STRING, "SPEEX",
		"encoding-params", G_TYPE_STRING, "1",
		"channels", G_TYPE_INT, 1,
		"payload", G_TYPE_INT, 110,
		NULL);
	VERIFY_ELSE_RETURN_ERROR((NULL != udpSrcCaps), "error creating udpSrcCaps GstCaps");

	GstCaps* capsFilterInCaps = gst_caps_new_simple(
		"audio/x-raw",
		"rate", G_TYPE_INT, 8000, // TODO:: in android change to 16000
		NULL);
	VERIFY_ELSE_RETURN_ERROR((NULL != capsFilterInCaps), "error creating capsFilterInCaps GstCaps");

	g_object_set(udpSrc, "uri", "udp://0.0.0.0:5004", "caps", udpSrcCaps, NULL);
	g_object_set(rtpJitterBuffer, "latency", 200, NULL);
	g_object_set(speexDec, "plc", true, NULL);
	//g_object_set(queue0, "leaky", GST_QUEUE_LEAK_DOWNSTREAM, NULL);
	g_object_set(queue_far_end_sink, "leaky", GST_QUEUE_LEAK_DOWNSTREAM, NULL);
	g_object_set(queue_near_end_src, "leaky", GST_QUEUE_LEAK_DOWNSTREAM, NULL);
	//g_object_set(queue1, "max-size-buffers", 2, NULL);
	//g_object_set(capsFilterIn, "caps", capsFilterInCaps, NULL);
	g_object_set(autoAudioSink, "buffer-time", (gint64)160000, NULL);

	////////////////////////// END - create in elements ////////////////////////////

	gst_bin_add_many(
		GST_BIN(pipeline),
		autoAudioSrc,
		//audioConvertOut,
		//audioResampleOut,
		capsFilterOut,
		queue_near_end_sink,
		speexAEC,
		queue_far_end_src,
		speexEnc,
		rtpSpeexPay,
		udpSink,
		udpSrc,
		rtpJitterBuffer,
		rtpSpeexDepay,
		speexDec,
		queue_far_end_sink,
		//tee,
		//queue0,
		queue_near_end_src,
		//audioConvertIn,
		//audioResampleIn,
		//capsFilterIn,
		autoAudioSink,
		NULL);

	// OUT elements linking
	gboolean result = gst_element_link_many(
		autoAudioSrc,
		//audioConvertOut, 
		//audioResampleOut, 
		capsFilterOut,
		queue_near_end_sink,
		NULL);
	VERIFY_ELSE_RETURN_ERROR((0 != result), "error linking OUT first elements");

	result = gst_element_link_pads(
		queue_near_end_sink,
		NULL,
		speexAEC,
		"near_end_sink");
	VERIFY_ELSE_RETURN_ERROR((0 != result), "error linking capsFilterOut and speexaec near end");


	result = gst_element_link_pads(
		speexAEC,
		"far_end_src",
		queue_far_end_src,
		NULL);
	VERIFY_ELSE_RETURN_ERROR((0 != result), "error linking speexAEC and queue0");

	result = gst_element_link_many(
		queue_far_end_src,
		speexEnc,
		rtpSpeexPay,
		udpSink,
		NULL);
	VERIFY_ELSE_RETURN_ERROR((0 != result), "error linking OUT last elements");

	// IN elements linking 
	result = gst_element_link_many(
		udpSrc,
		rtpJitterBuffer,
		rtpSpeexDepay,
		speexDec,
		queue_far_end_sink,
		NULL);
	VERIFY_ELSE_RETURN_ERROR((0 != result), "error linking IN first elements");


	result = gst_element_link_pads(
		queue_far_end_sink,
		NULL,
		speexAEC,
		"far_end_sink");
	VERIFY_ELSE_RETURN_ERROR((0 != result), "error linking tee and queue0 elements");

	result = gst_element_link_pads(
		speexAEC,
		"near_end_src",
		queue_near_end_src,
		NULL);
	VERIFY_ELSE_RETURN_ERROR((0 != result), "error linking tee and queue1 elements");

	//result = gst_element_link_pads(
	//	queue1,
	//	NULL,
	//	speexAEC,
	//	"far_end_sink");
	//VERIFY_ELSE_RETURN_ERROR((0 != result), "error linking queue1 and speexaec far end");

	result = gst_element_link_many(
		queue_near_end_src,
		//audioConvertIn,
		//audioResampleIn,
		//capsFilterIn,
		autoAudioSink,
		NULL);
	VERIFY_ELSE_RETURN_ERROR((0 != result), "error linking IN last elements");

	/* run */
	GstStateChangeReturn returnValue = gst_element_set_state(pipeline, GST_STATE_PLAYING);
	if (returnValue == GST_STATE_CHANGE_FAILURE)
	{
		GstMessage *msg;
		g_print("Failed to start up pipeline!\n");
		/* check if there is an error message with details on the bus */
		msg = gst_bus_poll(bus, GST_MESSAGE_ERROR, 0);
		if (msg)
		{
			GError *err = NULL;
			gst_message_parse_error(msg, &err, NULL);
			g_print("ERROR: %s\n", err->message);
			g_error_free(err);
			gst_message_unref(msg);
		}
		return -1;
	}
#endif

	g_main_loop_run(loop);
	/* clean up */
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
	g_source_remove(watch_id);
	g_main_loop_unref(loop);
	return 0;
}
















