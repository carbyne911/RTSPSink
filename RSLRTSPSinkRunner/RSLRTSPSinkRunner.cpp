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
	pipeline = gst_parse_launch(" videotestsrc ! x264enc ! rtph264pay ! rtsp_sink host=192.168.2.108 silent=false port=1935 stream_name=live/1 auth-name=eduard auth-pass=123", &error);


	if (error) {
		g_print("failed initing pipeline with : %s", error->message) ;
	}

	GstStateChangeReturn returnValue = gst_element_set_state(pipeline, GST_STATE_PLAYING);
	if (returnValue == GST_STATE_CHANGE_FAILURE) {
		g_print("Pipe failed to change state.") ;
	}

	//////////////////////////  create out elements ////////////////////////////


	g_main_loop_run(loop);
	/* clean up */
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
	g_source_remove(watch_id);
	g_main_loop_unref(loop);
	return 0;
}
















