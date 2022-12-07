/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (c) 2000 - 2022 David Odin (aka DindinX) for MandrakeSoft
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/
/* <DESC>
 * use the libcurl in a gtk-threaded application
 * </DESC>
 */

#include <stdio.h>
#include <gtk/gtk.h>

#include <curl/curl.h>

GtkWidget *Bar;

static size_t my_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  return fwrite(ptr, size, nmemb, stream);
}

static size_t my_read_func(char *ptr, size_t size, size_t nmemb, FILE *stream)
{
  return fread(ptr, size, nmemb, stream);
}

static int my_progress_func(GtkWidget *bar,
                            double t, /* dltotal */
                            double d, /* dlnow */
                            double ultotal,
                            double ulnow)
{
/*  printf("%d / %d (%g %%)\n", d, t, d*100.0/t);*/
  gdk_threads_enter();
  gtk_progress_set_value(GTK_PROGRESS(bar), d*100.0/t);
  gdk_threads_leave();
  return 0;
}

static void *my_thread(void *ptr)
{
  CURL *curl;

  curl = curl_easy_init();
  if(curl) {
    gchar *url = ptr;
    const char *filename = "test.curl";
    FILE *outfile = fopen(filename, "wb");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, my_read_func);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, my_progress_func);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, Bar);

    curl_easy_perform(curl);

    fclose(outfile);
    /* always cleanup */
    curl_easy_cleanup(curl);
  }

  return NULL;
}

int main(int argc, char **argv)
{
  GtkWidget *Window, *Frame, *Frame2;
  GtkAdjustment *adj;

  /* Must initialize libcurl before any threads are started */
  curl_global_init(CURL_GLOBAL_ALL);

  /* Init thread */
  g_thread_init(NULL);

  gtk_init(&argc, &argv);
  Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  Frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(Frame), GTK_SHADOW_OUT);
  gtk_container_add(GTK_CONTAINER(Window), Frame);
  Frame2 = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(Frame2), GTK_SHADOW_IN);
  gtk_container_add(GTK_CONTAINER(Frame), Frame2);
  gtk_container_set_border_width(GTK_CONTAINER(Frame2), 5);
  adj = (GtkAdjustment*)gtk_adjustment_new(0, 0, 100, 0, 0, 0);
  Bar = gtk_progress_bar_new_with_adjustment(adj);
  gtk_container_add(GTK_CONTAINER(Frame2), Bar);
  gtk_widget_show_all(Window);

  if(!g_thread_create(&my_thread, argv[1], FALSE, NULL) != 0)
    g_warning("cannot create the thread");

  gdk_threads_enter();
  gtk_main();
  gdk_threads_leave();
  return 0;
}
