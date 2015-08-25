/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2015 eduards <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-rtsp_sink
 *
 * FIXME:Describe rtsp_sink here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! rtsp_sink ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <gst/rtsp/rtsp.h>

#include "gstrtspsink.h"

GST_DEBUG_CATEGORY_STATIC (gst_rtspsink_debug);
#define GST_CAT_DEFAULT gst_rtspsink_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );
	


#define gst_rtspsink_parent_class parent_class
G_DEFINE_TYPE(GstRTSPsink, gst_rtspsink, GST_TYPE_BASE_SINK);
//G_DEFINE_TYPE (GstRTSPsink, gst_rtspsink, GST_TYPE_ELEMENT);
//G_DEFINE_TYPE(GstMySink, gst_my_sink, GST_TYPE_BASE_SINK);


static void gst_rtspsink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_rtspsink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_rtspsink_sink_event (GstPad * pad, GstObject * parent, GstEvent * event);
//static GstFlowReturn gst_rtspsink_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);

static GstFlowReturn  gst_rtsp_sink_preroll(GstBaseSink * bsink, GstBuffer * buffer);

static GstFlowReturn gst_rtsp_sink_render(GstBaseSink * bsink, GstBuffer * buf);


/* GObject vmethod implementations */



/* initialize the rtsp_sink's class */
static void gst_rtspsink_class_init (GstRTSPsinkClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSinkClass *gstbase_sink_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbase_sink_class = GST_BASE_SINK_CLASS(klass);  //(GstBaseSinkClass *)klass; //GST_BASE_SINK_CLASS(klass);



  gobject_class->set_property = gst_rtspsink_set_property;
  gobject_class->get_property = gst_rtspsink_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT, 
	                               g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
								   FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple(gstelement_class,
    "RTSPsink",
    "FIXME:Generic",
    "FIXME:Generic Template Element",
    "eduards <<user@hostname.org>>");

  //gst_element_class_add_pad_template (gstelement_class,    gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,    gst_static_pad_template_get (&sink_factory));

  klass->prepare = default_prepare; 

  gstbase_sink_class->render = (gst_rtsp_sink_render);

  gstbase_sink_class->preroll = (gst_rtsp_sink_preroll);

}


static GstFlowReturn gst_rtsp_sink_preroll(GstBaseSink * bsink, GstBuffer * buffer)
{
	GstRTSPsinkClass *sink = (GstRTSPsinkClass* )bsink;
	GstRTSPResult res; 
	GstRTSPConnection *conn ;
	const GstRTSPUrl * url ;
	GTimeVal timeout;
	guint8 data[4] = {1,2,3,4};
	guint size = 4;


	const gchar *urlstr = "rtsp://192.168.2.108";
	int port = 1935;
	timeout.tv_sec = 1; // set timeout to one second.
	timeout.tv_usec = 0;



	// set parameters
	GstRTSPResult res0 = gst_rtsp_url_parse(urlstr, &url);
	res0 = gst_rtsp_url_set_port(url, port);

	// create connection 
	res = gst_rtsp_connection_create(url, &conn);
	
	res =  gst_rtsp_connection_connect(conn, &timeout);


	res = gst_rtsp_connection_write(conn, data, size, &timeout);

	// close connection 
	res = gst_rtsp_connection_close(conn);


	return GST_FLOW_OK;
}


static gboolean default_prepare(GstBaseSink * media)
{

	return TRUE;
//	GstRTSPMediaPrivate *priv;
//	GstRTSPMediaClass *klass;
//	GstBus *bus;
//	GMainContext *context;
//	GSource *source;
//
//	priv = media->priv;
//
//	klass = GST_RTSP_MEDIA_GET_CLASS(media);
//
//	if (!klass->create_rtpbin)
//		goto no_create_rtpbin;
//
//	priv->rtpbin = klass->create_rtpbin(media);
//	if (priv->rtpbin != NULL) {
//		gboolean success = TRUE;
//
//		g_object_set(priv->rtpbin, "latency", priv->latency, NULL);
//
//		if (klass->setup_rtpbin)
//			success = klass->setup_rtpbin(media, priv->rtpbin);
//
//		if (success == FALSE) {
//			gst_object_unref(priv->rtpbin);
//			priv->rtpbin = NULL;
//		}
//	}
//	if (priv->rtpbin == NULL)
//		goto no_rtpbin;
//
//	priv->thread = thread;
//	context = (thread != NULL) ? (thread->context) : NULL;
//
//	bus = gst_pipeline_get_bus(GST_PIPELINE_CAST(priv->pipeline));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void gst_rtspsink_init (GstRTSPsink * filter)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink0");
  gst_pad_set_event_function (filter->sinkpad,    GST_DEBUG_FUNCPTR(gst_rtspsink_sink_event));
  //gst_pad_set_chain_function (filter->sinkpad,    GST_DEBUG_FUNCPTR(gst_rtspsink_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  //filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  //GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  //gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
  
  filter->silent = FALSE;
}


static GstFlowReturn gst_rtsp_sink_render(GstBaseSink * bsink, GstBuffer * buffer)
{
	GstMapInfo map;
	//buffer = gst_sample_get_buffer(sample);
	gst_buffer_map(buffer, &map, GST_MAP_READ);


	g_print("Data len %d\n", map.size);
	//g_memcpy(data, map.data, 4);
	//g_print("Data: %D %D %D %D", data[0], data[1], data[2], data[3] );


	gst_buffer_unmap(buffer, &map);
	//gst_sample_unref(sample);

	return GST_FLOW_OK;
}

static void
gst_rtspsink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstRTSPsink *filter = GST_RTSP_SINK (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_rtspsink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstRTSPsink *filter = GST_RTSP_SINK (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_rtspsink_sink_event (GstPad * pad, GstObject * parent, GstEvent * event)
{
  gboolean ret;
  GstRTSPsink *filter;

  filter = GST_RTSP_SINK (parent);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
      GstCaps * caps;

      gst_event_parse_caps (event, &caps);
      /* do something with the caps */

      /* and forward */
      ret = gst_pad_event_default (pad, parent, event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, parent, event);
      break;
  }
  return ret;
}

/* chain function
 * this function does the actual processing
 */
#if NOT_SINK
static GstFlowReturn
gst_rtspsink_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
  GstRTSPsink *filter;

  filter = GST_RTSP_SINK (parent);

  if (filter->silent == FALSE)
    g_print ("I'm plugged, therefore I'm in. EDU!\n");

  /* just push out the incoming buffer without touching it */
  return gst_pad_push (filter->srcpad, buf);
}
#endif


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
rtsp_sink_init (GstPlugin * rtsp_sink)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template rtsp_sink' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_rtspsink_debug, "rtsp_sink",
      0, "Template rtsp_sink");

  return gst_element_register (rtsp_sink, "rtsp_sink", GST_RANK_NONE,
      GST_TYPE_RTSP_SINK);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstrtsp_sink"
#endif


#define VERSION "0.0.1"


/* gstreamer looks for this structure to register rtsp_sinks
 *
 * exchange the string 'Template rtsp_sink' with your rtsp_sink description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    rtsp_sink,
    "Pushing rtsp sink",
    rtsp_sink_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "https://github.com/reporty/RTSPSink"
)
