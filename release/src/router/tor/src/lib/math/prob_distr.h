
/**
 * \file prob_distr.h
 *
 * \brief Header for prob_distr.c
 **/

#ifndef TOR_PROB_DISTR_H
#define TOR_PROB_DISTR_H

#include "lib/cc/compat_compiler.h"
#include "lib/cc/torint.h"
#include "lib/testsupport/testsupport.h"

/**
 * Container for distribution parameters for sampling, CDF, &c.
 */
struct dist {
  const struct dist_ops *ops;
};

/**
 * Untyped initializer element for struct dist using the specified
 * struct dist_ops pointer.  Don't actually use this directly -- use
 * the type-specific macro built out of DIST_BASE_TYPED below -- but if
 * you did use this directly, it would be something like:
 *
 *   struct weibull mydist = {
 *     DIST_BASE(&weibull_ops),
 *     .lambda = ...,
 *     .k = ...,
 *   };
 *
 * Note there is NO COMPILER FEEDBACK if you accidentally do something
 * like
 *
 *   struct geometric mydist = {
 *     DIST_BASE(&weibull_ops),
 *     ...
 *   };
 */
#define DIST_BASE(OPS)  { .ops = (OPS) }

/** A compile-time type-checking macro for use with DIST_BASE_TYPED.
 *
 *  This macro works by checking that &OBJ is a pointer type that is the same
 *  type (except for qualifiers) as (const TYPE *)&OBJ. It's a C constraint
 *  violation (which requires a diagnostic) if two pointers are different types
 *  and are subtracted. The sizeof() forces compile-time evaluation, and the
 *  multiplication by zero is to discard the result of the sizeof() from the
 *  expression.
 *
 *  We define this conditionally to suppress false positives from
 *  Coverity, which gets confused by the sizeof business.
 */
#ifdef __COVERITY__
#define TYPE_CHECK_OBJ(OPS, OBJ, TYPE) 0
#else
#define TYPE_CHECK_OBJ(OPS, OBJ, TYPE) \
  (0*sizeof(&(OBJ) - (const TYPE *)&(OBJ)))
#endif /* defined(__COVERITY__) */

/**
* Typed initializer element for struct dist using the specified struct
* dist_ops pointer.  Don't actually use this directly -- use a
* type-specific macro built out of it -- but if you did use this
* directly, it would be something like:
*
*   struct weibull mydist = {
*     DIST_BASE_TYPED(&weibull_ops, mydist, struct weibull),
*     .lambda = ...,
*     .k = ...,
*   };
*
* If you want to define a distribution type, define a canonical set of
* operations and define a type-specific initializer element like so:
*
*   struct foo {
*     struct dist base;
*     int omega;
*     double tau;
*     double phi;
*   };
*
*   struct dist_ops foo_ops = ...;
*
*   #define FOO(OBJ) DIST_BASE_TYPED(&foo_ops, OBJ, struct foo)
*
* Then users can do:
*
*   struct foo mydist = {
*     FOO(mydist),
*     .omega = ...,
*     .tau = ...,
*     .phi = ...,
*   };
*
* If you accidentally write
*
*   struct bar mydist = {
*     FOO(mydist),
*     ...
*   };
*
* then the compiler will report a type mismatch in the sizeof
* expression, which otherwise evaporates at runtime.
*/
#define DIST_BASE_TYPED(OPS, OBJ, TYPE)                         \
  DIST_BASE((OPS) + TYPE_CHECK_OBJ(OPS,OBJ,TYPE))

/**
 * Generic operations on distributions.  These simply defer to the
 * corresponding dist_ops function.  In the parlance of C++, these call
 * virtual member functions.
 */
const char *dist_name(const struct dist *);
double dist_sample(const struct dist *);
double dist_cdf(const struct dist *, double x);
double dist_sf(const struct dist *, double x);
double dist_icdf(const struct dist *, double p);
double dist_isf(const struct dist *, double p);

/**
 * Set of operations on a potentially parametric family of
 * distributions.  In the parlance of C++, this would be called a
 * `vtable' and the members are virtual member functions.
 */
struct dist_ops {
  const char *name;
  double (*sample)(const struct dist *);
  double (*cdf)(const struct dist *, double x);
  double (*sf)(const struct dist *, double x);
  double (*icdf)(const struct dist *, double p);
  double (*isf)(const struct dist *, double p);
};

/* Geometric distribution on positive number of trials before first success */

struct geometric {
  struct dist base;
  double p; /* success probability */
};

extern const struct dist_ops geometric_ops;

#define GEOMETRIC(OBJ)                                      \
  DIST_BASE_TYPED(&geometric_ops, OBJ, struct geometric)

/* Pareto distribution */

struct genpareto {
  struct dist base;
  double mu;
  double sigma;
  double xi;
};

extern const struct dist_ops genpareto_ops;

#define GENPARETO(OBJ)                                      \
  DIST_BASE_TYPED(&genpareto_ops, OBJ, struct genpareto)

/* Weibull distribution */

struct weibull {
  struct dist base;
  double lambda;
  double k;
};

extern const struct dist_ops weibull_ops;

#define WEIBULL(OBJ)                                    \
  DIST_BASE_TYPED(&weibull_ops, OBJ, struct weibull)

/* Log-logistic distribution */

struct log_logistic {
  struct dist base;
  double alpha;
  double beta;
};

extern const struct dist_ops log_logistic_ops;

#define LOG_LOGISTIC(OBJ)                                       \
  DIST_BASE_TYPED(&log_logistic_ops, OBJ, struct log_logistic)

/* Logistic distribution */

struct logistic {
  struct dist base;
  double mu;
  double sigma;
};

extern const struct dist_ops logistic_ops;

#define LOGISTIC(OBJ)                                   \
  DIST_BASE_TYPED(&logistic_ops, OBJ, struct logistic)

/* Uniform distribution */

struct uniform {
  struct dist base;
  double a;
  double b;
};

extern const struct dist_ops uniform_ops;

#define UNIFORM(OBJ)                                    \
  DIST_BASE_TYPED(&uniform_ops, OBJ, struct uniform)

/** Only by unittests */

#ifdef PROB_DISTR_PRIVATE

STATIC double logithalf(double p0);
STATIC double logit(double p);

STATIC double random_uniform_01(void);

STATIC double logistic(double x);
STATIC double cdf_logistic(double x, double mu, double sigma);
STATIC double sf_logistic(double x, double mu, double sigma);
STATIC double icdf_logistic(double p, double mu, double sigma);
STATIC double isf_logistic(double p, double mu, double sigma);
STATIC double sample_logistic(uint32_t s, double t, double p0);

STATIC double cdf_log_logistic(double x, double alpha, double beta);
STATIC double sf_log_logistic(double x, double alpha, double beta);
STATIC double icdf_log_logistic(double p, double alpha, double beta);
STATIC double isf_log_logistic(double p, double alpha, double beta);
STATIC double sample_log_logistic(uint32_t s, double p0);

STATIC double cdf_weibull(double x, double lambda, double k);
STATIC double sf_weibull(double x, double lambda, double k);
STATIC double icdf_weibull(double p, double lambda, double k);
STATIC double isf_weibull(double p, double lambda, double k);
STATIC double sample_weibull(uint32_t s, double p0, double lambda, double k);

STATIC double sample_uniform_interval(double p0, double a, double b);

STATIC double cdf_genpareto(double x, double mu, double sigma, double xi);
STATIC double sf_genpareto(double x, double mu, double sigma, double xi);
STATIC double icdf_genpareto(double p, double mu, double sigma, double xi);
STATIC double isf_genpareto(double p, double mu, double sigma, double xi);
STATIC double sample_genpareto(uint32_t s, double p0, double xi);

#endif /* defined(PROB_DISTR_PRIVATE) */

#endif /* !defined(TOR_PROB_DISTR_H) */
