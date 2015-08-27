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



/*
Simple usage: 

1.Run Wowza server on machine. Make sure application name 'live' is present and no authentication present.

2. Run pipeline:
gstreamer-launch-1.0  videotestsrc ! x264enc ! rtph264pay ! rtsp_sink host=<Your-Wowza-IP> port=1935 stream_name=live/1 

3. Run VLC, press CTRL-N  to setup network stream
rtsp://<Your-Wowza-IP>:1935/live/1
*/



#ifndef __GST_RTSP_SINK_H__
#define __GST_RTSP_SINK_H__

#include <gst/gst.h>
#include <gst/base/gstbasesink.h>


G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GST_TYPE_RTSP_SINK \
  (gst_rtspsink_get_type())
#define GST_RTSP_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RTSP_SINK,GstRTSPsink))
#define GST_RTSP_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RTSP_SINK,GstRTSPsinkClass))
#define GST_IS_RTSP_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RTSP_SINK))
#define GST_IS_RTSP_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RTSP_SINK))

typedef struct _GstRTSPsink      GstRTSPsink;
typedef struct _GstRTSPsinkClass GstRTSPsinkClass;

struct _GstRTSPsink
{
  //GstElement element;
  GstBaseSink element; 

  GstPad *sinkpad;// , *srcpad;

  gboolean silent;

  GstRTSPConnection *conn;

  const gchar *host;  // The target host.  e.g Wowza ip.
  const gchar *stream_name;  // RTSP stream name e.g. Wowza application name
  int port; // 
  int    server_rtp_port;  // Host's RTSP port e.g. Wowza 1935
  gchar * user_agent;

  gchar *session_name;
  gchar *information;

  // RTP/UDP connection to server
  GSocket * socket ;
  GSocketAddress *sa;

  gboolean debug;

  // get this data from upstream pads capabilites.
  int payload, clock_rate;
  gchar * encoding_name;
};


static gboolean default_prepare(GstBaseSink * media);


struct _GstRTSPsinkClass 
{
//  GstElementClass parent_class;
  GstBaseSinkClass parent_class;

  gboolean(*prepare)         (GstRTSPsink *media);
};

GType gst_rtspsink_get_type (void);

G_END_DECLS

#endif /* __GST_RTSP_SINK_H__ */
