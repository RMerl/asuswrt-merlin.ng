/*
 * Property Service contexts backend for labeling Android 
 * property keys
 */

#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "callbacks.h"
#include "label_internal.h"

/* A property security context specification. */
typedef struct spec {
	struct selabel_lookup_rec lr;	 /* holds contexts for lookup result */
	char *property_key;	         /* property key string */
} spec_t;

/* Our stored configuration */
struct saved_data {
	/*
	 * The array of specifications is sorted for longest
	 * prefix match
	 */
	spec_t *spec_arr;
	unsigned int nspec; /* total number of specifications */
};

static int cmp(const void *A, const void *B)
{
	const struct spec *sp1 = A, *sp2 = B;

	if (strncmp(sp1->property_key,"*",1) == 0)
		return 1;
	if (strncmp(sp2->property_key,"*",1) == 0)
		return -1;

	size_t L1 = strlen(sp1->property_key);
	size_t L2 = strlen(sp2->property_key);

	return (L1 < L2) - (L1 > L2);
}

/*
 * Warn about duplicate specifications.
 */
static int nodups_specs(struct saved_data *data, const char *path)
{
	int rc = 0;
	unsigned int ii, jj;
	struct spec *curr_spec, *spec_arr = data->spec_arr;

	for (ii = 0; ii < data->nspec; ii++) {
		curr_spec = &spec_arr[ii];
		for (jj = ii + 1; jj < data->nspec; jj++) {
			if ((!strcmp(spec_arr[jj].property_key, curr_spec->property_key))) {
				rc = -1;
				errno = EINVAL;
				if (strcmp
				    (spec_arr[jj].lr.ctx_raw,
				     curr_spec->lr.ctx_raw)) {
					selinux_log
						(SELINUX_ERROR,
						 "%s: Multiple different specifications for %s  (%s and %s).\n",
						 path, curr_spec->property_key,
						 spec_arr[jj].lr.ctx_raw,
						 curr_spec->lr.ctx_raw);
				} else {
					selinux_log
						(SELINUX_ERROR,
						 "%s: Multiple same specifications for %s.\n",
						 path, curr_spec->property_key);
				}
			}
		}
	}
	return rc;
}

static int process_line(struct selabel_handle *rec,
			const char *path, char *line_buf, 
			int pass, unsigned lineno)
{
	int items, len;
	char buf1[BUFSIZ], buf2[BUFSIZ];
	char *buf_p, *prop = buf1, *context = buf2;
	struct saved_data *data = (struct saved_data *)rec->data;
	spec_t *spec_arr = data->spec_arr;
	unsigned int nspec = data->nspec;

	len = strlen(line_buf);
	if (line_buf[len - 1] == '\n')
		line_buf[len - 1] = 0;
	buf_p = line_buf;
	while (isspace(*buf_p))
		buf_p++;
	/* Skip comment lines and empty lines. */
	if (*buf_p == '#' || *buf_p == 0)
		return 0;
	items = sscanf(line_buf, "%255s %255s", prop, context);
	if (items != 2) {
		selinux_log(SELINUX_WARNING,
			    "%s:  line %d is missing fields, skipping\n", path,
			    lineno);
		return 0;
	}

	if (pass == 1) {
		/* On the second pass, process and store the specification in spec. */
		spec_arr[nspec].property_key = strdup(prop);
		if (!spec_arr[nspec].property_key) {
			selinux_log(SELINUX_WARNING,
				    "%s:  out of memory at line %d on prop %s\n",
				    path, lineno, prop);
		return -1;
	    
		}

		spec_arr[nspec].lr.ctx_raw = strdup(context);
		if (!spec_arr[nspec].lr.ctx_raw) {
			selinux_log(SELINUX_WARNING,
				    "%s:  out of memory at line %d on context %s\n",
				    path, lineno, context);
		return -1;
		}

		if (rec->validating) {
		        if (selabel_validate(rec, &spec_arr[nspec].lr) < 0) {
			        selinux_log(SELINUX_WARNING,
					    "%s:  line %d has invalid context %s\n",
					    path, lineno, spec_arr[nspec].lr.ctx_raw);
			}
		}
     	}

	data->nspec = ++nspec;
	return 0;
}

static int init(struct selabel_handle *rec, const struct selinux_opt *opts,
		unsigned n)
{
	struct saved_data *data = (struct saved_data *)rec->data;
	const char *path = NULL;
	FILE *fp;
	char line_buf[BUFSIZ];
	unsigned int lineno = 0, maxnspec, pass;
	int status = -1;
	struct stat sb;

	/* Process arguments */
	while (n--)
		switch (opts[n].type) {
		case SELABEL_OPT_PATH:
			path = opts[n].value;
			break;
		}

	/* Open the specification file. */
	if ((fp = fopen(path, "r")) == NULL)
		return -1;

	if (fstat(fileno(fp), &sb) < 0)
		return -1;
	if (!S_ISREG(sb.st_mode)) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * Two passes of the specification file. First is to get the size.
	 * After the first pass, the spec array is malloced to the appropriate 
	 * size. Second pass is to populate the spec array and check for 
	 * dups.
	 */
	maxnspec = UINT_MAX / sizeof(spec_t);
	for (pass = 0; pass < 2; pass++) {
		data->nspec = 0;

		while (fgets(line_buf, sizeof line_buf - 1, fp)
		       && data->nspec < maxnspec) {
			if (process_line(rec, path, line_buf, pass, ++lineno) != 0) {
				goto finish;
			}
		}

		if (pass == 1) {
			status = nodups_specs(data, path);
	    
			if (status)
				goto finish;
		}

		if (pass == 0) {

			if (data->nspec == 0) {
				status = 0;
				goto finish;
			}

			if (NULL == (data->spec_arr =
				     malloc(sizeof(spec_t) * data->nspec)))
				goto finish;

			memset(data->spec_arr, 0, sizeof(spec_t)*data->nspec);
			maxnspec = data->nspec;
			rewind(fp);
		}
	}

	qsort(data->spec_arr, data->nspec, sizeof(struct spec), cmp);

	status = 0;
finish:
	fclose(fp);
	return status;
}

/*
 * Backend interface routines
 */
static void closef(struct selabel_handle *rec)
{
	struct saved_data *data = (struct saved_data *)rec->data;
	struct spec *spec;
	unsigned int i;

	for (i = 0; i < data->nspec; i++) {
		spec = &data->spec_arr[i];
		free(spec->property_key);
		free(spec->lr.ctx_raw);
		free(spec->lr.ctx_trans);
	}

	if (data->spec_arr)
		free(data->spec_arr);
	
	free(data);
}

static struct selabel_lookup_rec *lookup(struct selabel_handle *rec, 
					 const char *key, 
					 int __attribute__((unused)) type)
{
	struct saved_data *data = (struct saved_data *)rec->data;
	spec_t *spec_arr = data->spec_arr;
	unsigned int i;
	struct selabel_lookup_rec *ret = NULL;

	if (!data->nspec) {
		errno = ENOENT;
		goto finish;
	}

	for (i = 0; i < data->nspec; i++) {
		if (strncmp(spec_arr[i].property_key, key, 
		    strlen(spec_arr[i].property_key)) == 0) {
			break;
		}
		if (strncmp(spec_arr[i].property_key, "*", 1) == 0)
			break;
	}

	if (i >= data->nspec) {
		/* No matching specification. */
		errno = ENOENT;
		goto finish;
	}

	ret = &spec_arr[i].lr;

finish:
	return ret;
}

static void stats(struct selabel_handle __attribute__((unused)) *rec)
{
	selinux_log(SELINUX_WARNING, "'stats' functionality not implemented.\n");
}

int selabel_property_init(struct selabel_handle *rec,
			  const struct selinux_opt *opts,
			  unsigned nopts)
{
	struct saved_data *data;

	data = (struct saved_data *)malloc(sizeof(*data));
	if (!data)
		return -1;
	memset(data, 0, sizeof(*data));

	rec->data = data;
	rec->func_close = &closef;
	rec->func_stats = &stats;
	rec->func_lookup = &lookup;

	return init(rec, opts, nopts);
}
