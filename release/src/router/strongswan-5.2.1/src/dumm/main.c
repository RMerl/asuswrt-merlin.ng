/*
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "dumm.h"

#include <collections/linked_list.h>

#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>
#include <vte/reaper.h>

/**
 * notebook page with vte and guest
 */
typedef struct {
	gint num;
	GtkWidget *vte;
	guest_t *guest;
} page_t;

/**
 * Main window
 */
GtkWidget *window;

/**
 * notebook with guests, vtes
 */
GtkWidget *notebook;

/**
 * dumm context
 */
dumm_t *dumm;

/**
 * pages in notebook, page_t
 */
linked_list_t *pages;

/**
 * handle guest termination, SIGCHILD
 */
static void child_exited(VteReaper *vtereaper, gint pid, gint status)
{
	enumerator_t *enumerator;
	page_t *page;

	enumerator = pages->create_enumerator(pages);
	while (enumerator->enumerate(enumerator, (void**)&page))
	{
		if (page->guest->get_pid(page->guest) == pid)
		{
			page->guest->sigchild(page->guest);
			vte_terminal_feed(VTE_TERMINAL(page->vte),
							  "\n\r--- guest terminated ---\n\r", -1);
			break;
		}
	}
	enumerator->destroy(enumerator);
}

static page_t* get_page(int num)
{
	enumerator_t *enumerator;
	page_t *page, *found = NULL;

	enumerator = pages->create_enumerator(pages);
	while (enumerator->enumerate(enumerator, (void**)&page))
	{
		if (page->num == num)
		{
			found = page;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * Guest invocation callback
 */
static pid_t invoke(void *vte, guest_t *guest,
					char *args[], int argc)
{
	GPid pid;

	if (vte_terminal_fork_command_full(VTE_TERMINAL(vte),
						VTE_PTY_NO_LASTLOG | VTE_PTY_NO_UTMP | VTE_PTY_NO_WTMP,
						NULL, args, NULL,
						G_SPAWN_CHILD_INHERITS_STDIN | G_SPAWN_SEARCH_PATH,
						NULL, NULL, &pid, NULL))
	{
		return pid;
	}
	return 0;
}

void idle(void)
{
	gtk_main_iteration_do(FALSE);
	sched_yield();
}

static void start_guest()
{
	page_t *page;

	page = get_page(gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
	if (page && page->guest->get_state(page->guest) == GUEST_STOPPED)
	{
		vte_terminal_feed(VTE_TERMINAL(page->vte),
						  "--- starting guest ---\n\r", -1);
		page->guest->start(page->guest, invoke, VTE_TERMINAL(page->vte), idle);
	}
}

static void start_all_guests()
{
	enumerator_t *enumerator;
	page_t *page;

	enumerator = pages->create_enumerator(pages);
	while (enumerator->enumerate(enumerator, (void**)&page))
	{
		if (page->guest->get_state(page->guest) == GUEST_STOPPED)
		{
			vte_terminal_feed(VTE_TERMINAL(page->vte),
						  "--- starting all guests ---\n\r", -1);
			page->guest->start(page->guest, invoke,
							   VTE_TERMINAL(page->vte), idle);
		}
	}
	enumerator->destroy(enumerator);
}

static void stop_guest()
{
	page_t *page;

	page = get_page(gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
	if (page && page->guest->get_state(page->guest) == GUEST_RUNNING)
	{
		page->guest->stop(page->guest, idle);
	}
}

/**
 * quit signal handler
 */
static void quit()
{
	enumerator_t *enumerator;
	page_t *page;

	dumm->load_template(dumm, NULL);

	enumerator = pages->create_enumerator(pages);
	while (enumerator->enumerate(enumerator, &page))
	{
		if (page->guest->get_state(page->guest) != GUEST_STOPPED)
		{
			page->guest->stop(page->guest, idle);
		}
	}
	enumerator->destroy(enumerator);
	gtk_main_quit();
}

static void error_dialog(char *msg)
{
	GtkWidget *error;

	error = gtk_message_dialog_new(GTK_WINDOW(window),
							  GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
							  GTK_BUTTONS_CLOSE, msg);
	gtk_dialog_run(GTK_DIALOG(error));
	gtk_widget_destroy(error);
}

static void create_switch()
{
	GtkWidget *dialog, *table, *label, *name;
	bridge_t *bridge;

	dialog = gtk_dialog_new_with_buttons("Create new switch", GTK_WINDOW(window),
							GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
							GTK_STOCK_NEW, GTK_RESPONSE_ACCEPT, NULL);

	table = gtk_table_new(1, 2, TRUE);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);

	label = gtk_label_new("Switch name");
	gtk_table_attach(GTK_TABLE(table), label,  0, 1, 0, 1, 0, 0, 0, 0);
	gtk_widget_show(label);

	name = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table), name, 1, 2, 0, 1,
					 GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0, 0);
	gtk_widget_show(name);

	gtk_widget_show(table);

	while (TRUE)
	{
		switch (gtk_dialog_run(GTK_DIALOG(dialog)))
		{
			case GTK_RESPONSE_ACCEPT:
			{
				if (streq(gtk_entry_get_text(GTK_ENTRY(name)), ""))
				{
					continue;
				}
				bridge = dumm->create_bridge(dumm,
									(char*)gtk_entry_get_text(GTK_ENTRY(name)));
				if (!bridge)
				{
					error_dialog("creating bridge failed!");
					continue;
				}
				break;
			}
			default:
				break;
		}
		break;
	}
	gtk_widget_destroy(dialog);
}

static void delete_switch()
{

}

static void connect_guest()
{
	page_t *page;
	GtkWidget *dialog, *table, *label, *name, *box;
	bridge_t *bridge;
	iface_t *iface;
	enumerator_t *enumerator;

	page = get_page(gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
	if (!page || page->guest->get_state(page->guest) != GUEST_RUNNING)
	{
		return;
	}

	dialog = gtk_dialog_new_with_buttons("Connect guest", GTK_WINDOW(window),
							GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
							GTK_STOCK_NEW, GTK_RESPONSE_ACCEPT, NULL);

	table = gtk_table_new(2, 2, TRUE);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);

	label = gtk_label_new("Interface name");
	gtk_table_attach(GTK_TABLE(table), label,  0, 1, 0, 1, 0, 0, 0, 0);
	gtk_widget_show(label);

	name = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table), name, 1, 2, 0, 1,
					 GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0, 0);
	gtk_widget_show(name);

	label = gtk_label_new("Connected switch");
	gtk_table_attach(GTK_TABLE(table), label,  0, 1, 1, 2, 0, 0, 0, 0);
	gtk_widget_show(label);

	box = gtk_combo_box_new_text();
	gtk_table_attach(GTK_TABLE(table), box, 1, 2, 1, 2,
					 GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0, 0);
	enumerator = dumm->create_bridge_enumerator(dumm);
	while (enumerator->enumerate(enumerator, &bridge))
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(box), bridge->get_name(bridge));
	}
	enumerator->destroy(enumerator);
	gtk_widget_show(box);

	gtk_widget_show(table);

	while (TRUE)
	{
		switch (gtk_dialog_run(GTK_DIALOG(dialog)))
		{
			case GTK_RESPONSE_ACCEPT:
			{
				if (streq(gtk_entry_get_text(GTK_ENTRY(name)), ""))
				{
					continue;
				}

				iface = page->guest->create_iface(page->guest,
									(char*)gtk_entry_get_text(GTK_ENTRY(name)));
				if (!iface)
				{
					error_dialog("creating interface failed!");
					continue;
				}
				enumerator = dumm->create_bridge_enumerator(dumm);
				while (enumerator->enumerate(enumerator, &bridge))
				{
					if (!bridge->connect_iface(bridge, iface))
					{
						error_dialog("connecting interface failed!");
					}
					break;
				}
				enumerator->destroy(enumerator);
				break;
			}
			default:
				break;
		}
		break;
	}
	gtk_widget_destroy(dialog);
}

static void disconnect_guest()
{

}

static void delete_guest()
{
	page_t *page;

	page = get_page(gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
	if (page)
	{
		page->guest->stop(page->guest, idle);
		dumm->delete_guest(dumm, page->guest);
		gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), page->num);
		pages->remove(pages, page, NULL);
		g_free(page);
	}
}

/**
 * create a new page for a guest
 */
static page_t* create_page(guest_t *guest)
{
	GtkWidget *label;
	page_t *page;

	page = g_new(page_t, 1);
	page->guest = guest;
	page->vte = vte_terminal_new();
	label = gtk_label_new(guest->get_name(guest));
	page->num = gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
										 page->vte, label);
	gtk_widget_show(page->vte);
	pages->insert_last(pages, page);
	return page;
}

/**
 * create a new guest
 */
static void create_guest()
{
	guest_t *guest;
	GtkWidget *dialog, *table, *label, *name, *kernel, *master, *args;

	dialog = gtk_dialog_new_with_buttons("Create new guest", GTK_WINDOW(window),
							GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
							GTK_STOCK_NEW, GTK_RESPONSE_ACCEPT, NULL);

	table = gtk_table_new(4, 2, TRUE);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);

	label = gtk_label_new("Guest name");
	gtk_table_attach(GTK_TABLE(table), label,  0, 1, 0, 1, 0, 0, 0, 0);
	gtk_widget_show(label);

	label = gtk_label_new("UML kernel");
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2, 0, 0, 0, 0);
	gtk_widget_show(label);

	label = gtk_label_new("Master filesystem");
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, 0, 0, 0, 0);
	gtk_widget_show(label);

	label = gtk_label_new("Kernel arguments");
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4, 0, 0, 0, 0);
	gtk_widget_show(label);

	name = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table), name, 1, 2, 0, 1,
					 GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0, 0);
	gtk_widget_show(name);

	kernel = gtk_file_chooser_button_new("Select UML kernel image",
										 GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_table_attach(GTK_TABLE(table), kernel, 1, 2, 1, 2,
					 GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0, 0);
	gtk_widget_show(kernel);

	master = gtk_file_chooser_button_new("Select master filesystem",
										 GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_table_attach(GTK_TABLE(table), master, 1, 2, 2, 3,
					 GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0, 0);
	gtk_widget_show(master);

	args = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table), args, 1, 2, 3, 4,
					 GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0, 0);
	gtk_widget_show(args);

	gtk_widget_show(table);

	while (TRUE)
	{
		switch (gtk_dialog_run(GTK_DIALOG(dialog)))
		{
			case GTK_RESPONSE_ACCEPT:
			{
				char *sname, *skernel, *smaster, *sargs;
				page_t *page;

				sname = (char*)gtk_entry_get_text(GTK_ENTRY(name));
				skernel = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(kernel));
				smaster = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(master));
				sargs = (char*)gtk_entry_get_text(GTK_ENTRY(args));

				if (!sname[0] || !skernel || !smaster)
				{
					continue;
				}
				guest = dumm->create_guest(dumm, sname, skernel, smaster, sargs);
				if (!guest)
				{
					error_dialog("creating guest failed!");
					continue;
				}
				page = create_page(guest);
				gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), page->num);
				break;
			}
			default:
				break;
		}
		break;
	}
	gtk_widget_destroy(dialog);
}

/**
 * main routine, parses args and reads from console
 */
int main(int argc, char *argv[])
{
	GtkWidget *menubar, *menu, *menuitem, *vbox;
	GtkWidget *dummMenu, *guestMenu, *switchMenu;
	enumerator_t *enumerator;
	guest_t *guest;

	library_init(NULL, "dumm");
	gtk_init(&argc, &argv);

	pages = linked_list_create();
	dumm = dumm_create(NULL);

	/* setup window */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(quit), NULL);
	gtk_window_set_title(GTK_WINDOW (window), "Dumm");
	gtk_window_set_default_size(GTK_WINDOW (window), 1000, 500);
	g_signal_connect(G_OBJECT(vte_reaper_get()), "child-exited",
					 G_CALLBACK(child_exited), NULL);

	/* add vbox with menubar, notebook */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	menubar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, TRUE, 0);
	notebook = gtk_notebook_new();
	g_object_set(G_OBJECT(notebook), "homogeneous", TRUE, NULL);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_BOTTOM);
	gtk_container_add(GTK_CONTAINER(vbox), notebook);

	/* Dumm menu */
	menu = gtk_menu_new();
	dummMenu = gtk_menu_item_new_with_mnemonic("_Dumm");
	gtk_menu_bar_append(GTK_MENU_BAR(menubar), dummMenu);
	gtk_widget_show(dummMenu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(dummMenu), menu);

	/* Dumm -> exit */
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate",
					 G_CALLBACK(quit), NULL);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_widget_show(menuitem);

	/* Guest menu */
	menu = gtk_menu_new();
	guestMenu = gtk_menu_item_new_with_mnemonic("_Guest");
	gtk_menu_bar_append(GTK_MENU_BAR(menubar), guestMenu);
	gtk_widget_show(guestMenu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(guestMenu), menu);

	/* Guest -> new */
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate",
					 G_CALLBACK(create_guest), NULL);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_widget_show(menuitem);

	/* Guest -> delete */
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate",
					 G_CALLBACK(delete_guest), NULL);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_widget_show(menuitem);

	menuitem = gtk_separator_menu_item_new();
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_widget_show(menuitem);

	/* Guest -> start */
	menuitem = gtk_menu_item_new_with_mnemonic("_Start");
	g_signal_connect(G_OBJECT(menuitem), "activate",
					 G_CALLBACK(start_guest), NULL);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_widget_show(menuitem);

	/* Guest -> startall */
	menuitem = gtk_menu_item_new_with_mnemonic("Start _all");
	g_signal_connect(G_OBJECT(menuitem), "activate",
					 G_CALLBACK(start_all_guests), NULL);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_widget_show(menuitem);

	/* Guest -> stop */
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_STOP, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate",
					 G_CALLBACK(stop_guest), NULL);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_widget_show(menuitem);

	menuitem = gtk_separator_menu_item_new();
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_widget_show(menuitem);

	/* Guest -> connect */
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_CONNECT, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate",
					 G_CALLBACK(connect_guest), NULL);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_widget_show(menuitem);

	/* Guest -> disconnect */
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DISCONNECT, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate",
					 G_CALLBACK(disconnect_guest), NULL);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_widget_set_sensitive(menuitem, FALSE);
	gtk_widget_show(menuitem);

	/* Switch menu */
	menu = gtk_menu_new();
	switchMenu = gtk_menu_item_new_with_mnemonic("_Switch");
	gtk_menu_bar_append(GTK_MENU_BAR(menubar), switchMenu);
	gtk_widget_show(switchMenu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(switchMenu), menu);

	/* Switch -> new */
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate",
					 G_CALLBACK(create_switch), NULL);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_widget_show(menuitem);

	/* Switch -> delete */
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate",
					 G_CALLBACK(delete_switch), NULL);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_widget_set_sensitive(menuitem, FALSE);
	gtk_widget_show(menuitem);

	/* show widgets */
	gtk_widget_show(menubar);
	gtk_widget_show(notebook);
	gtk_widget_show(vbox);
	gtk_widget_show(window);

	/* fill notebook with guests */
	enumerator = dumm->create_guest_enumerator(dumm);
	while (enumerator->enumerate(enumerator, (void**)&guest))
	{
		create_page(guest);
	}
	enumerator->destroy(enumerator);

	gtk_main();

	dumm->destroy(dumm);
	pages->destroy_function(pages, g_free);

	library_deinit();
	return 0;
}

