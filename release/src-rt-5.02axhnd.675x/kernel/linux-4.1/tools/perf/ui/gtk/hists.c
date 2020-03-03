#include "../evlist.h"
#include "../cache.h"
#include "../evsel.h"
#include "../sort.h"
#include "../hist.h"
#include "../helpline.h"
#include "gtk.h"

#define MAX_COLUMNS			32

static int __percent_color_snprintf(struct perf_hpp *hpp, const char *fmt, ...)
{
	int ret = 0;
	int len;
	va_list args;
	double percent;
	const char *markup;
	char *buf = hpp->buf;
	size_t size = hpp->size;

	va_start(args, fmt);
	len = va_arg(args, int);
	percent = va_arg(args, double);
	va_end(args);

	markup = perf_gtk__get_percent_color(percent);
	if (markup)
		ret += scnprintf(buf, size, markup);

	ret += scnprintf(buf + ret, size - ret, fmt, len, percent);

	if (markup)
		ret += scnprintf(buf + ret, size - ret, "</span>");

	return ret;
}

#define __HPP_COLOR_PERCENT_FN(_type, _field)					\
static u64 he_get_##_field(struct hist_entry *he)				\
{										\
	return he->stat._field;							\
}										\
										\
static int perf_gtk__hpp_color_##_type(struct perf_hpp_fmt *fmt,		\
				       struct perf_hpp *hpp,			\
				       struct hist_entry *he)			\
{										\
	return hpp__fmt(fmt, hpp, he, he_get_##_field, " %*.2f%%",		\
			__percent_color_snprintf, true);			\
}

#define __HPP_COLOR_ACC_PERCENT_FN(_type, _field)				\
static u64 he_get_acc_##_field(struct hist_entry *he)				\
{										\
	return he->stat_acc->_field;						\
}										\
										\
static int perf_gtk__hpp_color_##_type(struct perf_hpp_fmt *fmt __maybe_unused,	\
				       struct perf_hpp *hpp,			\
				       struct hist_entry *he)			\
{										\
	return hpp__fmt_acc(fmt, hpp, he, he_get_acc_##_field, " %*.2f%%", 	\
			    __percent_color_snprintf, true);			\
}

__HPP_COLOR_PERCENT_FN(overhead, period)
__HPP_COLOR_PERCENT_FN(overhead_sys, period_sys)
__HPP_COLOR_PERCENT_FN(overhead_us, period_us)
__HPP_COLOR_PERCENT_FN(overhead_guest_sys, period_guest_sys)
__HPP_COLOR_PERCENT_FN(overhead_guest_us, period_guest_us)
__HPP_COLOR_ACC_PERCENT_FN(overhead_acc, period)

#undef __HPP_COLOR_PERCENT_FN


void perf_gtk__init_hpp(void)
{
	perf_hpp__format[PERF_HPP__OVERHEAD].color =
				perf_gtk__hpp_color_overhead;
	perf_hpp__format[PERF_HPP__OVERHEAD_SYS].color =
				perf_gtk__hpp_color_overhead_sys;
	perf_hpp__format[PERF_HPP__OVERHEAD_US].color =
				perf_gtk__hpp_color_overhead_us;
	perf_hpp__format[PERF_HPP__OVERHEAD_GUEST_SYS].color =
				perf_gtk__hpp_color_overhead_guest_sys;
	perf_hpp__format[PERF_HPP__OVERHEAD_GUEST_US].color =
				perf_gtk__hpp_color_overhead_guest_us;
	perf_hpp__format[PERF_HPP__OVERHEAD_ACC].color =
				perf_gtk__hpp_color_overhead_acc;
}

static void perf_gtk__add_callchain(struct rb_root *root, GtkTreeStore *store,
				    GtkTreeIter *parent, int col, u64 total)
{
	struct rb_node *nd;
	bool has_single_node = (rb_first(root) == rb_last(root));

	for (nd = rb_first(root); nd; nd = rb_next(nd)) {
		struct callchain_node *node;
		struct callchain_list *chain;
		GtkTreeIter iter, new_parent;
		bool need_new_parent;
		double percent;
		u64 hits, child_total;

		node = rb_entry(nd, struct callchain_node, rb_node);

		hits = callchain_cumul_hits(node);
		percent = 100.0 * hits / total;

		new_parent = *parent;
		need_new_parent = !has_single_node && (node->val_nr > 1);

		list_for_each_entry(chain, &node->val, list) {
			char buf[128];

			gtk_tree_store_append(store, &iter, &new_parent);

			scnprintf(buf, sizeof(buf), "%5.2f%%", percent);
			gtk_tree_store_set(store, &iter, 0, buf, -1);

			callchain_list__sym_name(chain, buf, sizeof(buf), false);
			gtk_tree_store_set(store, &iter, col, buf, -1);

			if (need_new_parent) {
				/*
				 * Only show the top-most symbol in a callchain
				 * if it's not the only callchain.
				 */
				new_parent = iter;
				need_new_parent = false;
			}
		}

		if (callchain_param.mode == CHAIN_GRAPH_REL)
			child_total = node->children_hit;
		else
			child_total = total;

		/* Now 'iter' contains info of the last callchain_list */
		perf_gtk__add_callchain(&node->rb_root, store, &iter, col,
					child_total);
	}
}

static void on_row_activated(GtkTreeView *view, GtkTreePath *path,
			     GtkTreeViewColumn *col __maybe_unused,
			     gpointer user_data __maybe_unused)
{
	bool expanded = gtk_tree_view_row_expanded(view, path);

	if (expanded)
		gtk_tree_view_collapse_row(view, path);
	else
		gtk_tree_view_expand_row(view, path, FALSE);
}

static void perf_gtk__show_hists(GtkWidget *window, struct hists *hists,
				 float min_pcnt)
{
	struct perf_hpp_fmt *fmt;
	GType col_types[MAX_COLUMNS];
	GtkCellRenderer *renderer;
	GtkTreeStore *store;
	struct rb_node *nd;
	GtkWidget *view;
	int col_idx;
	int sym_col = -1;
	int nr_cols;
	char s[512];

	struct perf_hpp hpp = {
		.buf		= s,
		.size		= sizeof(s),
	};

	nr_cols = 0;

	perf_hpp__for_each_format(fmt)
		col_types[nr_cols++] = G_TYPE_STRING;

	store = gtk_tree_store_newv(nr_cols, col_types);

	view = gtk_tree_view_new();

	renderer = gtk_cell_renderer_text_new();

	col_idx = 0;

	perf_hpp__for_each_format(fmt) {
		if (perf_hpp__should_skip(fmt))
			continue;

		/*
		 * XXX no way to determine where symcol column is..
		 *     Just use last column for now.
		 */
		if (perf_hpp__is_sort_entry(fmt))
			sym_col = col_idx;

		gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
							    -1, fmt->name,
							    renderer, "markup",
							    col_idx++, NULL);
	}

	for (col_idx = 0; col_idx < nr_cols; col_idx++) {
		GtkTreeViewColumn *column;

		column = gtk_tree_view_get_column(GTK_TREE_VIEW(view), col_idx);
		gtk_tree_view_column_set_resizable(column, TRUE);

		if (col_idx == sym_col) {
			gtk_tree_view_set_expander_column(GTK_TREE_VIEW(view),
							  column);
		}
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), GTK_TREE_MODEL(store));

	g_object_unref(GTK_TREE_MODEL(store));

	for (nd = rb_first(&hists->entries); nd; nd = rb_next(nd)) {
		struct hist_entry *h = rb_entry(nd, struct hist_entry, rb_node);
		GtkTreeIter iter;
		u64 total = hists__total_period(h->hists);
		float percent;

		if (h->filtered)
			continue;

		percent = hist_entry__get_percent_limit(h);
		if (percent < min_pcnt)
			continue;

		gtk_tree_store_append(store, &iter, NULL);

		col_idx = 0;

		perf_hpp__for_each_format(fmt) {
			if (perf_hpp__should_skip(fmt))
				continue;

			if (fmt->color)
				fmt->color(fmt, &hpp, h);
			else
				fmt->entry(fmt, &hpp, h);

			gtk_tree_store_set(store, &iter, col_idx++, s, -1);
		}

		if (symbol_conf.use_callchain && sort__has_sym) {
			if (callchain_param.mode == CHAIN_GRAPH_REL)
				total = symbol_conf.cumulate_callchain ?
					h->stat_acc->period : h->stat.period;

			perf_gtk__add_callchain(&h->sorted_chain, store, &iter,
						sym_col, total);
		}
	}

	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(view), TRUE);

	g_signal_connect(view, "row-activated",
			 G_CALLBACK(on_row_activated), NULL);
	gtk_container_add(GTK_CONTAINER(window), view);
}

int perf_evlist__gtk_browse_hists(struct perf_evlist *evlist,
				  const char *help,
				  struct hist_browser_timer *hbt __maybe_unused,
				  float min_pcnt)
{
	struct perf_evsel *pos;
	GtkWidget *vbox;
	GtkWidget *notebook;
	GtkWidget *info_bar;
	GtkWidget *statbar;
	GtkWidget *window;

	signal(SIGSEGV, perf_gtk__signal);
	signal(SIGFPE,  perf_gtk__signal);
	signal(SIGINT,  perf_gtk__signal);
	signal(SIGQUIT, perf_gtk__signal);
	signal(SIGTERM, perf_gtk__signal);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(window), "perf report");

	g_signal_connect(window, "delete_event", gtk_main_quit, NULL);

	pgctx = perf_gtk__activate_context(window);
	if (!pgctx)
		return -1;

	vbox = gtk_vbox_new(FALSE, 0);

	notebook = gtk_notebook_new();

	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

	info_bar = perf_gtk__setup_info_bar();
	if (info_bar)
		gtk_box_pack_start(GTK_BOX(vbox), info_bar, FALSE, FALSE, 0);

	statbar = perf_gtk__setup_statusbar();
	gtk_box_pack_start(GTK_BOX(vbox), statbar, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(window), vbox);

	evlist__for_each(evlist, pos) {
		struct hists *hists = evsel__hists(pos);
		const char *evname = perf_evsel__name(pos);
		GtkWidget *scrolled_window;
		GtkWidget *tab_label;
		char buf[512];
		size_t size = sizeof(buf);

		if (symbol_conf.event_group) {
			if (!perf_evsel__is_group_leader(pos))
				continue;

			if (pos->nr_members > 1) {
				perf_evsel__group_desc(pos, buf, size);
				evname = buf;
			}
		}

		scrolled_window = gtk_scrolled_window_new(NULL, NULL);

		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
							GTK_POLICY_AUTOMATIC,
							GTK_POLICY_AUTOMATIC);

		perf_gtk__show_hists(scrolled_window, hists, min_pcnt);

		tab_label = gtk_label_new(evname);

		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_window, tab_label);
	}

	gtk_widget_show_all(window);

	perf_gtk__resize_window(window);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	ui_helpline__push(help);

	gtk_main();

	perf_gtk__deactivate_context(&pgctx);

	return 0;
}
