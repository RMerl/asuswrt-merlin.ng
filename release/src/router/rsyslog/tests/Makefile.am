TEST_EXTENSIONS=.sh

if ENABLE_TESTBENCH

CLEANFILES=\
	*_*.conf \
	rsyslog*.started work-*.conf rsyslog.random.data \
	rsyslog*.pid.save xlate*.lkp_tbl \
	log log* \
	work \
	test-spool test-logdir stat-file1 \
	rsyslog.pipe rsyslog.input.* \
	rsyslog.input rsyslog.input.* imfile-state* omkafka-failed.data \
	rsyslog.input-symlink.log rsyslog-link.*.log targets \
	HOSTNAME \
	rstb_* \
	tmp.qi nocert

CLEANFILES+= \
	IN_AUTO_DEBUG
# IN_AUTO_DEBUG should be deleted each time make check is run, but
# there exists no such hook. Se we at least delete it on make clean.


pkglib_LTLIBRARIES =

pkglib_LTLIBRARIES += liboverride_gethostname.la
liboverride_gethostname_la_SOURCES = override_gethostname.c
liboverride_gethostname_la_CFLAGS =
liboverride_gethostname_la_LDFLAGS = -avoid-version -shared

pkglib_LTLIBRARIES += liboverride_gethostname_nonfqdn.la
liboverride_gethostname_nonfqdn_la_SOURCES = override_gethostname_nonfqdn.c
liboverride_gethostname_nonfqdn_la_CFLAGS =
liboverride_gethostname_nonfqdn_la_LDFLAGS = -avoid-version -shared

pkglib_LTLIBRARIES += liboverride_getaddrinfo.la
liboverride_getaddrinfo_la_SOURCES = override_getaddrinfo.c
liboverride_getaddrinfo_la_CFLAGS =
liboverride_getaddrinfo_la_LDFLAGS = -avoid-version -shared

# TODO: reenable TESTRUNS = rt_init rscript
check_PROGRAMS = $(TESTRUNS) ourtail tcpflood chkseq msleep randomgen \
	diagtalker uxsockrcvr syslog_caller inputfilegen minitcpsrv \
	omrelp_dflt_port \
	mangle_qi \
	have_relpSrvSetOversizeMode \
	test_id
if ENABLE_IMJOURNAL
check_PROGRAMS += journal_print
endif
TESTS = $(TESTRUNS)

if ENABLE_DEFAULT_TESTS
TESTS +=  \
	empty-hostname.sh \
	hostname-getaddrinfo-fail.sh \
	prop-programname.sh \
	prop-programname-with-slashes.sh \
	hostname-with-slash-pmrfc5424.sh \
	hostname-with-slash-pmrfc3164.sh \
	hostname-with-slash-dflt-invld.sh \
	hostname-with-slash-dflt-slash-valid.sh \
	stop-localvar.sh \
	stop-msgvar.sh \
	glbl-umask.sh \
	glbl-unloadmodules.sh \
	glbl-invld-param.sh \
	glbl_setenv_2_vars.sh \
	glbl_setenv_err.sh \
	glbl_setenv_err_too_long.sh \
	glbl_setenv.sh \
	nested-call-shutdown.sh \
	invalid_nested_include.sh \
	omfwd-keepalive.sh \
	omfile-read-only-errmsg.sh \
	omfile-null-filename.sh \
	omfile-whitespace-filename.sh \
	omfile-read-only.sh \
	omfile_both_files_set.sh \
	msgvar-concurrency.sh \
	localvar-concurrency.sh \
	exec_tpl-concurrency.sh \
	privdropuser.sh \
	privdropuserid.sh \
	privdropgroup.sh \
	privdropgroupid.sh \
	template-json.sh \
	template-pure-json.sh \
	template-pos-from-to.sh \
	template-pos-from-to-lowercase.sh \
	template-pos-from-to-oversize.sh \
	template-pos-from-to-oversize-lowercase.sh \
	template-pos-from-to-missing-jsonvar.sh \
	template-const-jsonf.sh \
	fac_authpriv.sh \
	fac_local0.sh \
	fac_local7.sh \
	fac_mail.sh \
	fac_news.sh \
	fac_ftp.sh \
	fac_ntp.sh \
	fac_uucp.sh \
	fac_invld1.sh \
	fac_invld2.sh \
	fac_invld3.sh \
	fac_invld4_rfc5424.sh \
	compresssp.sh \
	compresssp-stringtpl.sh \
	rawmsg-after-pri.sh \
	rfc5424parser.sh \
	pmrfc3164-msgFirstSpace.sh \
	pmrfc3164-AtSignsInHostname.sh \
	pmrfc3164-AtSignsInHostname_off.sh \
	pmrfc3164-tagEndingByColon.sh \
	pmrfc3164-defaultTag.sh \
	pmrfc3164-json.sh \
	tcp_forwarding_tpl.sh \
	tcp_forwarding_dflt_tpl.sh \
	tcp_forwarding_retries.sh \
	arrayqueue.sh \
	global_vars.sh \
	no-parser-errmsg.sh \
	da-mainmsg-q.sh \
	validation-run.sh \
	msgdup.sh \
	empty-ruleset.sh \
	imtcp-discard-truncated-msg.sh \
	imtcp-basic.sh \
	imtcp-maxFrameSize.sh \
	imtcp-msg-truncation-on-number.sh \
	imtcp-msg-truncation-on-number2.sh \
	imtcp-NUL.sh \
	imtcp-NUL-rawmsg.sh \
	imtcp-multiport.sh \
	imtcp_incomplete_frame_at_end.sh \
	daqueue-persist.sh \
	daqueue-invld-qi.sh \
	daqueue-dirty-shutdown.sh \
	diskq-rfc5424.sh \
	diskqueue.sh \
	diskqueue-fsync.sh \
	diskqueue-full.sh \
	rulesetmultiqueue.sh \
	rulesetmultiqueue-v6.sh \
	manytcp.sh \
	rsf_getenv.sh \
	imtcp_conndrop.sh \
	imtcp_addtlframedelim.sh \
	imtcp_no_octet_counted.sh \
	imtcp_spframingfix.sh \
	sndrcv.sh \
	sndrcv_failover.sh \
	sndrcv_gzip.sh \
	sndrcv_udp_nonstdpt.sh \
	sndrcv_udp_nonstdpt_v6.sh \
	imudp_thread_hang.sh \
	sndrcv_udp_nonstdpt_v6.sh \
	asynwr_simple.sh \
	asynwr_simple_2.sh \
	asynwr_timeout.sh \
	asynwr_timeout_2.sh \
	asynwr_small.sh \
	asynwr_tinybuf.sh \
	wr_large_async.sh \
	wr_large_sync.sh \
	asynwr_deadlock.sh \
	asynwr_deadlock_2.sh \
	asynwr_deadlock2.sh \
	asynwr_deadlock4.sh \
	abort-uncleancfg-goodcfg.sh \
	abort-uncleancfg-goodcfg-check.sh \
	abort-uncleancfg-badcfg-check.sh \
	abort-uncleancfg-badcfg-check_1.sh \
	variable_leading_underscore.sh \
	gzipwr_rscript.sh \
	gzipwr_flushInterval.sh \
	gzipwr_flushOnTXEnd.sh \
	gzipwr_large.sh \
	gzipwr_large_dynfile.sh \
	dynfile_invld_async.sh \
	dynfile_invld_sync.sh \
	dynfile_invalid2.sh \
	complex1.sh \
	queue-persist.sh \
	pipeaction.sh \
	execonlyonce.sh \
	execonlywhenprevsuspended.sh \
	execonlywhenprevsuspended2.sh \
	execonlywhenprevsuspended3.sh \
	execonlywhenprevsuspended4.sh \
	execonlywhenprevsuspended_multiwrkr.sh \
	execonlywhenprevsuspended-queue.sh \
	execonlywhenprevsuspended-nonsusp.sh \
	execonlywhenprevsuspended-nonsusp-queue.sh \
	pipe_noreader.sh \
	dircreate_dflt.sh \
	dircreate_off.sh \
	imuxsock_logger.sh \
	imuxsock_logger_ruleset.sh \
	imuxsock_logger_ruleset_ratelimit.sh \
	imuxsock_logger_err.sh \
	imuxsock_logger_parserchain.sh \
	imuxsock_traillf.sh \
	imuxsock_ccmiddle.sh \
	imuxsock_logger_syssock.sh \
	imuxsock_traillf_syssock.sh \
	imuxsock_ccmiddle_syssock.sh \
	discard-rptdmsg.sh \
	discard-allmark.sh \
	discard.sh \
	stop.sh \
	failover-async.sh \
	failover-double.sh \
	failover-basic.sh \
	failover-rptd.sh \
	failover-no-rptd.sh \
	failover-no-basic.sh \
	rcvr_fail_restore.sh \
	rscript_contains.sh \
	rscript_bare_var_root.sh \
	rscript_bare_var_root-empty.sh \
	rscript_ipv42num.sh \
	rscript_field.sh \
	rscript_stop.sh \
	rscript_stop2.sh \
	rscript_prifilt.sh \
	rscript_optimizer1.sh \
	rscript_ruleset_call.sh \
	rscript_ruleset_call_indirect-basic.sh \
	rscript_ruleset_call_indirect-var.sh \
	rscript_ruleset_call_indirect-invld.sh \
	rscript_set_unset_invalid_var.sh \
	rscript_set_modify.sh \
	rscript_unaffected_reset.sh \
	rscript_replace_complex.sh \
	rscript_wrap2.sh \
	rscript_wrap3.sh \
	rscript_re_extract.sh \
	rscript_re_match.sh \
	rscript_eq.sh \
	rscript_eq_var.sh \
	rscript_ge.sh \
	rscript_ge_var.sh \
	rscript_gt.sh \
	rscript_gt_var.sh \
	rscript_le.sh \
	rscript_le_var.sh \
	rscript_lt.sh \
	rscript_lt_var.sh \
	rscript_ne.sh \
	rscript_ne_var.sh \
	rscript_num2ipv4.sh \
	rscript_int2Hex.sh \
	rscript_trim.sh \
	rscript_substring.sh \
	rscript_format_time.sh \
	rscript_parse_time.sh \
	rscript_is_time.sh \
	rscript_script_error.sh \
	rscript_parse_json.sh \
	rscript_previous_action_suspended.sh \
	rscript_str2num_negative.sh \
	rscript-config_enable-on.sh \
	rs-cnum.sh \
	rs-substring.sh \
	rs-int2hex.sh \
	empty-prop-comparison.sh \
	rs_optimizer_pri.sh \
	cee_simple.sh \
	cee_diskqueue.sh \
	incltest.sh \
	incltest_dir.sh \
	incltest_dir_wildcard.sh \
	incltest_dir_empty_wildcard.sh \
	linkedlistqueue.sh \
	lookup_table.sh \
	lookup_table_no_hup_reload.sh \
	key_dereference_on_uninitialized_variable_space.sh \
	array_lookup_table.sh \
	sparse_array_lookup_table.sh \
	lookup_table_bad_configs.sh \
	lookup_table_rscript_reload.sh \
	lookup_table_rscript_reload_without_stub.sh \
	include-obj-text-from-file.sh \
	multiple_lookup_tables.sh
	parsertest-parse1.sh \
	parsertest-parse1-udp.sh \
	parsertest-parse2.sh \
	parsertest-parse2-udp.sh \
	parsertest-parse_8bit_escape.sh \
	parsertest-parse_8bit_escape-udp.sh \
	parsertest-parse3.sh \
	parsertest-parse3-udp.sh \
	parsertest-parse_invld_regex.sh \
	parsertest-parse_invld_regex-udp.sh \
	parsertest-parse-3164-buggyday.sh \
	parsertest-parse-3164-buggyday-udp.sh \
	parsertest-parse-nodate.sh \
	parsertest-parse-nodate-udp.sh \
	parsertest-snare_ccoff_udp.sh \
	parsertest-snare_ccoff_udp2.sh
if HAVE_VALGRIND
TESTS +=  \
	include-obj-outside-control-flow-vg.sh \
	include-obj-in-if-vg.sh \
	include-obj-text-vg.sh \
	rscript_parse_json-vg.sh \
	rscript_backticks-vg.sh \
	rscript-config_enable-off-vg.sh \
	prop-jsonmesg-vg.sh \
	mmexternal-SegFault-vg.sh \
	mmexternal-InvldProg-vg.sh \
	internal-errmsg-memleak-vg.sh \
	glbl-oversizeMsg-log-vg.sh \
	rscript_set_memleak-vg.sh \
	no-parser-vg.sh \
	discard-rptdmsg-vg.sh \
	discard-allmark-vg.sh \
	failover-basic-vg.sh \
	failover-rptd-vg.sh \
	failover-no-basic-vg.sh \
	failover-no-rptd-vg.sh \
	timereported-utc-vg.sh \
	udp-msgreduc-vg.sh \
	udp-msgreduc-orgmsg-vg.sh \
	tcp-msgreduc-vg.sh \
	rscript_field-vg.sh \
	unused_lookup_table-vg.sh \
	lookup_table-vg.sh \
	lookup_table_no_hup_reload-vg.sh \
	array_lookup_table-vg.sh \
	array_lookup_table_misuse-vg.sh \
	sparse_array_lookup_table-vg.sh \
	lookup_table_bad_configs-vg.sh \
	lookup_table_rscript_reload-vg.sh \
	lookup_table_rscript_reload_without_stub-vg.sh \
	multiple_lookup_tables-vg.sh \
	fac_local0-vg.sh \
	badqi.sh \
	threadingmq.sh \
	threadingmqaq.sh \
	rscript_trim-vg.sh
endif # HAVE_VALGRIND
endif # ENABLE_DEFAULT_TESTS


if ENABLE_MMANON
TESTS +=  \
	mmanon_random_32_ipv4.sh \
	mmanon_random_cons_32_ipv4.sh \
	mmanon_recognize_ipv4.sh \
	mmanon_zero_12_ipv4.sh \
	mmanon_zero_33_ipv4.sh \
	mmanon_zero_8_ipv4.sh \
	mmanon_simple_12_ipv4.sh \
	mmanon_simple_33_ipv4.sh \
	mmanon_simple_8_ipv4.sh \
	mmanon_random_128_ipv6.sh \
	mmanon_zero_128_ipv6.sh \
	mmanon_zero_96_ipv6.sh \
	mmanon_random_cons_128_ipv6.sh \
	mmanon_zero_50_ipv6.sh \
	mmanon_recognize_ipv6.sh \
	mmanon_zero_64_ipv6.sh \
	mmanon_both_modes_compatible.sh \
	mmanon_recognize_ipembedded.sh \
	mmanon_random_cons_128_ipembedded.sh
endif # if ENABLE_MMANON

if ENABLE_ELASTICSEARCH_TESTS
TESTS +=  \
	es-basic-server.sh \
	es-maxbytes-bulk.sh \
	es-basic-errfile-empty.sh \
	es-basic-errfile-popul.sh \
	es-bulk-errfile-empty.sh \
	es-bulk-errfile-popul.sh \
	es-writeoperation.sh

es-basic-server.log: es-basic-bulk.log
es-maxbytes-bulk.log: es-basic-server.log
es-basic-errfile-empty.log: es-maxbytes-bulk.log
es-basic-errfile-popul.log: es-basic-errfile-empty.log
es-bulk-errfile-empty.log: es-basic-errfile-popul.log
es-bulk-errfile-popul.log: es-bulk-errfile-empty.log
es-writeoperation.log: es-bulk-errfile-popul.log

if ENABLE_IMPSTATS
TESTS +=  \
	es-basic-ha.sh \
	es-bulk-retry.sh

es-basic-ha.log: es-writeoperation.log
es-bulk-retry.log: es-basic-ha.log
endif
if ENABLE_IMFILE
TESTS +=  \
	es-bulk-errfile-popul-def-format.sh \
	es-bulk-errfile-popul-erronly.sh \
	es-bulk-errfile-popul-erronly-interleaved.sh \
	es-bulk-errfile-popul-def-interleaved.sh

es-bulk-errfile-popul-def-format.log: es-bulk-retry.log
es-bulk-errfile-popul-erronly.log: es-bulk-errfile-popul-def-format.log
es-bulk-errfile-popul-erronly-interleaved.log: es-bulk-errfile-popul-erronly.log
es-bulk-errfile-popul-def-interleaved.log: es-bulk-errfile-popul-erronly-interleaved.log
endif
if HAVE_VALGRIND
TESTS +=  \
	es-basic-bulk-vg.sh \
	es-basic-ha-vg.sh

es-basic-bulk-vg.log: es-writeoperation.log
es-basic-bulk-vg.log: es-bulk-retry.log
es-basic-bulk-vg.log: es-bulk-errfile-popul-def-interleaved.log
es-basic-ha-vg.log: es-basic-bulk-vg.log
endif
endif # if ENABLE_ELASTICSEARCH_TESTS

if ENABLE_LIBFAKETIME
TESTS +=  \
	now_family_utc.sh \
	now-utc.sh \
	now-utc-ymd.sh \
	now-utc-casecmp.sh \
	timegenerated-ymd.sh \
	timegenerated-uxtimestamp.sh \
	timegenerated-uxtimestamp-invld.sh \
	timegenerated-dateordinal.sh \
	timegenerated-dateordinal-invld.sh \
	timegenerated-utc.sh \
	timegenerated-utc-legacy.sh \
	timereported-utc.sh \
	timereported-utc-legacy.sh
# now come faketime tests that utilize mmnormalize - aka "no endif here"
if ENABLE_MMNORMALIZE
TESTS += \
	mmnormalize_processing_test1.sh \
	mmnormalize_processing_test2.sh \
	mmnormalize_processing_test3.sh \
	mmnormalize_processing_test4.sh
endif
endif

if ENABLE_PGSQL
if ENABLE_PGSQL_TESTS
TESTS += \
	pgsql-basic.sh \
	pgsql-basic-cnf6.sh \
	pgsql-basic-threads-cnf6.sh \
	pgsql-template.sh \
	pgsql-template-cnf6.sh \
	pgsql-actq-mt-withpause.sh \
	pgsql-template-threads-cnf6.sh

pgsql-basic-cnf6.log: pgsql-basic.log
pgsql-basic-threads-cnf6.log: pgsql-basic-cnf6.log
pgsql-template.log: pgsql-basic-threads-cnf6.log
pgsql-template-cnf6.log: pgsql-template.log
pgsql-actq-mt-withpause.log: pgsql-template-cnf6.log
pgsql-template-threads-cnf6.log: pgsql-actq-mt-withpause.log
if HAVE_VALGRIND
TESTS += \
	pgsql-basic-vg.sh \
	pgsql-template-vg.sh \
	pgsql-basic-cnf6-vg.sh \
	pgsql-template-cnf6-vg.sh \
	pgsql-actq-mt-withpause-vg.sh

pgsql-basic-vg.log: pgsql-template-threads-cnf6.log
pgsql-template-vg.log: pgsql-basic-vg.log
pgsql-basic-cnf6-vg.log: pgsql-template-vg.log
pgsql-template-cnf6-vg.log: pgsql-basic-cnf6-vg.log
pgsql-actq-mt-withpause-vg.log: pgsql-template-cnf6-vg.log
endif
endif
endif

if ENABLE_MYSQL_TESTS
TESTS +=  \
	mysql-basic.sh \
	mysql-basic-cnf6.sh \
	mysql-asyn.sh \
	mysql-actq-mt.sh \
	mysql-actq-mt-withpause.sh \
	action-tx-single-processing.sh \
	action-tx-errfile.sh

mysql-basic-cnf6.log: mysql-basic.log
mysql-asyn.log: mysql-basic-cnf6.log
mysql-actq-mt.log: mysql-asyn.log
mysql-actq-mt-withpause.log: mysql-actq-mt.log
action-tx-single-processing.log: mysql-actq-mt-withpause.log
action-tx-errfile.log: action-tx-single-processing.log

if HAVE_VALGRIND
TESTS +=  \
	mysql-basic-vg.sh \
	mysql-asyn-vg.sh \
	mysql-actq-mt-withpause-vg.sh
mysql-basic-vg.log: action-tx-errfile.log
mysql-asyn-vg.log: mysql-basic-vg.log
mysql-actq-mt-withpause-vg.log: mysql-asyn-vg.log

libdbi-basic.log: mysql-actq-mt-withpause-vg.log # for next block!
endif

if ENABLE_OMLIBDBI # we piggy-back on MYSQL_TESTS as we need the same environment
TESTS +=  \
	libdbi-basic.sh \
	libdbi-asyn.sh
libdbi-basic.log: action-tx-errfile.log
libdbi-asyn.log: libdbi-basic.log
if HAVE_VALGRIND
TESTS +=  \
	libdbi-basic-vg.sh
libdbi-basic-vg.log: libdbi-asyn.log
endif
endif
endif #  MYSQL_TESTS

if ENABLE_FMHTTP
TESTS +=  \
	rscript_http_request.sh
if HAVE_VALGRIND
TESTS +=  \
	rscript_http_request-vg.sh
endif # HAVE_VALGRIND
endif # ENABLE_FMHTTP


if ENABLE_ROOT_TESTS
TESTS +=  \
	sndrcv_udp.sh \
	imuxsock_logger_root.sh \
	imuxsock_traillf_root.sh \
	imuxsock_ccmiddle_root.sh \
	imklog_permitnonkernelfacility_root.sh
if ENABLE_IP
TESTS += tcp_forwarding_ns_tpl.sh
endif
if ENABLE_RELP
TESTS += sndrcv_relp_dflt_pt.sh
endif
if HAVE_VALGRIND
TESTS +=  \
	mmexternal-SegFault-empty-jroot-vg.sh
endif
endif

if ENABLE_JOURNAL_TESTS
if ENABLE_IMJOURNAL
TESTS +=  \
	imjournal-basic.sh \
	imjournal-statefile.sh
if HAVE_VALGRIND
TESTS +=  \
	imjournal-basic-vg.sh
endif
endif

if ENABLE_OMJOURNAL
TESTS +=  \
	omjournal-abort-template.sh \
	omjournal-abort-no-template.sh \
	omjournal-basic-template.sh \
	omjournal-basic-no-template.sh
endif
endif #if ENABLE_JOURNAL_TESTS

if ENABLE_OMPROG
TESTS +=  \
	omprog-defaults.sh \
	omprog-output-capture.sh \
	omprog-output-capture-mt.sh \
	omprog-feedback.sh \
	omprog-feedback-mt.sh \
	omprog-feedback-timeout.sh \
	omprog-close-unresponsive.sh \
	omprog-close-unresponsive-noterm.sh \
	omprog-restart-terminated.sh \
	omprog-restart-terminated-outfile.sh \
	omprog-single-instance.sh \
	omprog-single-instance-outfile.sh \
	omprog-transactions.sh \
	omprog-transactions-failed-messages.sh \
	omprog-transactions-failed-commits.sh
if HAVE_VALGRIND
TESTS +=  \
	omprog-defaults-vg.sh \
	omprog-output-capture-vg.sh \
	omprog-feedback-vg.sh \
	omprog-close-unresponsive-vg.sh \
	omprog-restart-terminated-vg.sh \
	omprog-single-instance-vg.sh \
	omprog-transactions-vg.sh
endif
endif

if ENABLE_OMKAFKA
if ENABLE_IMKAFKA
if ENABLE_KAFKA_TESTS
TESTS += \
	omkafka.sh \
	imkafka.sh \
	imkafka_multi_single.sh \
	sndrcv_kafka.sh \
	sndrcv_kafka_multi_topics.sh
# Tests below need to be stable first!
#	sndrcv_kafka_fail.sh \
#	sndrcv_kafka_failresume.sh \
# needs properly to much mempory on arm devices!
#	sndrcv_kafka_multi.sh
if HAVE_VALGRIND
TESTS += \
	sndrcv_kafka-vg-rcvr.sh
#	sndrcv_kafka-vg-sender.sh
# Note: -sender had some issues. If the re-appear, see
# see https://github.com/rsyslog/rsyslog/issues/2434
endif
endif
endif
endif

if ENABLE_IMPSTATS
TESTS +=  \
	impstats-hup.sh \
	dynstats.sh \
	dynstats_overflow.sh \
	dynstats_reset.sh \
	dynstats_ctr_reset.sh \
	dynstats_nometric.sh \
	no-dynstats-json.sh \
	no-dynstats.sh \
	stats-json.sh \
	dynstats-json.sh \
	stats-cee.sh \
	stats-json-es.sh \
	dynstats_reset_without_pstats_reset.sh \
	dynstats_prevent_premature_eviction.sh
if HAVE_VALGRIND
TESTS +=  \
	dynstats-vg.sh \
	dynstats_reset-vg.sh \
	dynstats_overflow-vg.sh \
	dynstats-json-vg.sh \
	stats-json-vg.sh \
	stats-cee-vg.sh \
	dynstats_prevent_premature_eviction-vg.sh
endif
endif

if ENABLE_IMPTCP
# note that some tests simply USE imptcp, but they also
# need to be disabled if we do not have this module
TESTS +=  \
	manyptcp.sh \
	imptcp_framing_regex.sh \
	imptcp_framing_regex-oversize.sh \
	imptcp_large.sh \
	imptcp-connection-msg-disabled.sh \
	imptcp-connection-msg-received.sh \
	imptcp-discard-truncated-msg.sh \
	imptcp_addtlframedelim.sh \
	imptcp_conndrop.sh \
	imptcp_no_octet_counted.sh \
	imptcp_multi_line.sh \
	imptcp_spframingfix.sh \
	imptcp_nonProcessingPoller.sh \
	imptcp_veryLargeOctateCountedMessages.sh \
	imptcp-NUL.sh \
	imptcp-NUL-rawmsg.sh \
	rscript_random.sh \
	rscript_hash32.sh \
	rscript_hash64.sh \
	rscript_replace.sh
if HAVE_VALGRIND
TESTS +=  \
	imptcp_conndrop-vg.sh
if ENABLE_FMHASH
TESTS +=  \
	rscript_hash32-vg.sh \
	rscript_hash64-vg.sh
endif # ENABLE_FMHASH
endif # HAVE_VALGRIND
endif

if ENABLE_ELASTICSEARCH_TESTS_MINIMAL
TESTS +=  \
	es-basic.sh \
	es-basic-es6.0.sh \
	es-basic-bulk.sh
es-basic-es6.0.log: es-basic.log
es-basic-bulk.log: es-basic-es6.0.log
if HAVE_VALGRIND
TESTS += \
	es-basic-vg.sh \
	es-basic-vgthread.sh
es-basic-vg.log: es-basic-bulk.log
es-basic-vgthread.log: es-basic-vg.log
# for next if block:
es-basic-server.log: es-basic-vgthread.log
endif
endif

if ENABLE_MMPSTRUCDATA
TESTS +=  \
	mmpstrucdata.sh \
	mmpstrucdata-case.sh
if HAVE_VALGRIND
TESTS +=  \
	mmpstrucdata-vg.sh \
	mmpstrucdata-invalid-vg.sh
endif
endif

if ENABLE_MMRM1STSPACE
TESTS +=  \
	mmrm1stspace-basic.sh
endif

if ENABLE_PMNULL
TESTS +=  \
	pmnull-basic.sh \
	pmnull-withparams.sh
endif

if ENABLE_OMSTDOUT
TESTS +=  \
	omstdout-basic.sh
endif

if ENABLE_PMNORMALIZE
TESTS +=  \
	pmnormalize-basic.sh \
	pmnormalize-rule.sh
if HAVE_VALGRIND
TESTS += \
	pmnormalize-rule-vg.sh
endif
endif

if ENABLE_MMNORMALIZE
TESTS += msgvar-concurrency-array.sh \
	msgvar-concurrency-array-event.tags.sh \
	mmnormalize_rule_from_string.sh \
	mmnormalize_rule_from_array.sh

if ENABLE_IMPTCP
TESTS +=  \
	mmnormalize_regex_defaulted.sh \
	mmnormalize_regex_disabled.sh \
	mmnormalize_variable.sh \
	mmnormalize_tokenized.sh
endif

if LOGNORM_REGEX_SUPPORTED
TESTS += \
	mmnormalize_regex.sh
endif
endif

if ENABLE_MMJSONPARSE
TESTS += \
	mmjsonparse-w-o-cookie.sh \
	mmjsonparse-w-o-cookie-multi-spaces.sh
if ENABLE_IMPSTATS
TESTS += \
	mmjsonparse-invalid-containerName.sh \
	wtpShutdownAll-assertionFailure.sh
endif
if ENABLE_IMPTCP
TESTS +=  \
	mmjsonparse_simple.sh \
	imptcp-oversize-message-display.sh \
	imptcp-msg-truncation-on-number.sh \
	imptcp-msg-truncation-on-number2.sh \
	imptcp-maxFrameSize-parameter.sh \
	mmjsonparse_cim.sh \
	mmjsonparse_cim2.sh \
	mmjsonparse_localvar.sh \
	json_array_subscripting.sh \
	json_array_looping.sh \
	json_object_looping.sh \
	json_nonarray_looping.sh
endif
if HAVE_VALGRIND
TESTS +=  \
	json_null_array-vg.sh \
	json_object_looping-vg.sh \
	json_array_looping-vg.sh \
	json_object_suicide_in_loop-vg.sh \
	json_null-vg.sh
endif
TESTS +=  \
	stop_when_array_has_element.sh \
	json_null_array.sh \
	json_null.sh \
	json_var_cmpr.sh \
	json_var_case.sh
endif

if ENABLE_MMDBLOOKUP
TESTS += \
    mmdb.sh \
    mmdb-container.sh \
    mmdb-container-empty.sh
if HAVE_VALGRIND
TESTS += \
    mmdb-vg.sh \
    mmdb-multilevel-vg.sh
endif
endif

if ENABLE_GNUTLS
TESTS +=  \
	imtcp_conndrop_tls.sh \
	sndrcv_tls_anon_hostname.sh \
	sndrcv_tls_anon_ipv4.sh \
	sndrcv_tls_priorityString.sh \
	imtcp-tls-basic.sh \
	sndrcv_tls_anon_rebind.sh
if HAVE_VALGRIND
TESTS += \
	imtcp-tls-basic-vg.sh \
	imtcp_conndrop_tls-vg.sh \
	manytcp-too-few-tls-vg.sh
endif
endif

if ENABLE_OPENSSL
TESTS +=  \
	imtcp-tls-ossl-basic.sh \
	sndrcv_tls_ossl_anon_ipv4.sh \
	sndrcv_tls_ossl_anon_ipv6.sh \
	sndrcv_tls_ossl_anon_rebind.sh \
	imtcp-tls-ossl-x509valid.sh \
	imtcp-tls-ossl-x509name.sh \
	imtcp-tls-ossl-x509fingerprint.sh
if HAVE_VALGRIND
TESTS += \
	imtcp-tls-ossl-basic-vg.sh
endif
endif


if ENABLE_OMUXSOCK
TESTS += uxsock_simple.sh
endif

if ENABLE_RELP
TESTS += sndrcv_relp.sh \
	 sndrcv_relp_rebind.sh \
	 imrelp-basic.sh \
	 imrelp-manyconn.sh \
	 imrelp-maxDataSize-error.sh \
	 imrelp-long-msg.sh \
	 imrelp-oversizeMode-truncate.sh \
	 imrelp-oversizeMode-accept.sh \
	 glbl-oversizeMsg-log.sh \
	 glbl-oversizeMsg-truncate.sh \
	 glbl-oversizeMsg-split.sh
if ENABLE_GNUTLS
TESTS += \
         sndrcv_relp_tls.sh \
         sndrcv_relp_tls_certvalid.sh \
         sndrcv_relp_tls_prio.sh \
         relp_tls_certificate_not_found.sh \
	 omrelp_wrong_authmode.sh
endif
endif

if ENABLE_OMUDPSPOOF
TESTS += \
	sndrcv_omudpspoof.sh \
	sndrcv_omudpspoof_nonstdpt.sh
endif

#disabled as array-passing mode is no longer supported:	omod-if-array.sh 
#	omod-if-array.sh 
#	omod-if-array-udp.sh 
if ENABLE_IMPTCP
TESTS += \
	tabescape_dflt.sh \
	tabescape_dflt-udp.sh \
	tabescape_off.sh \
	tabescape_off-udp.sh \
	inputname-imtcp.sh \
	fieldtest.sh \
	fieldtest-udp.sh \
	proprepltest-nolimittag-udp.sh \
	proprepltest-nolimittag.sh \
	proprepltest-rfctag-udp.sh \
	proprepltest-rfctag.sh \
	timestamp-3164-udp.sh \
	timestamp-3164.sh \
	timestamp-3339-udp.sh \
	timestamp-3339.sh \
	timestamp-mysql-udp.sh \
	timestamp-mysql.sh \
	timestamp-pgsql-udp.sh \
	timestamp-pgsql.sh \
	timestamp-subseconds-udp.sh \
	timestamp-subseconds.sh

endif

if ENABLE_OMRULESET
TESTS += \
	omruleset.sh \
	omruleset-queue.sh
endif

if ENABLE_PMSNARE
TESTS += \
	pmsnare-default.sh \
	pmsnare-default-udp.sh \
	pmsnare-ccoff.sh \
	pmsnare-ccoff-udp.sh \
	pmsnare-ccdefault.sh \
	pmsnare-ccdefault-udp.sh \
	pmsnare-cccstyle.sh \
	pmsnare-cccstyle-udp.sh \
	pmsnare-ccbackslash.sh \
	pmsnare-ccbackslash-udp.sh \
	pmsnare-modoverride.sh \
	pmsnare-modoverride-udp.sh
endif

if ENABLE_PMLASTMSG
TESTS += \
	pmlastmsg.sh \
	pmlastmsg-udp.sh
endif

if ENABLE_EXTENDED_TESTS
# random.sh is temporarily disabled as it needs some work
# to rsyslog core to complete in reasonable time
#TESTS += random.sh
endif

if ENABLE_IMFILE
TESTS += \
	imfile-basic.sh \
	imfile-discard-truncated-line.sh \
	imfile-truncate-line.sh \
	imfile-file-not-found-error.sh \
	imfile-fileNotFoundError-parameter.sh \
	imfile-error-not-repeated.sh \
	imfile-truncate.sh \
	imfile-readmode2.sh \
	imfile-readmode2-polling.sh \
	imfile-readmode2-with-persists-data-during-stop.sh \
	imfile-readmode2-with-persists.sh \
	imfile-endregex.sh \
	imfile-endregex-save-lf.sh \
	imfile-endregex-save-lf-persist.sh \
	imfile-endregex-timeout-none-polling.sh \
	imfile-endregex-timeout-polling.sh \
	imfile-endregex-timeout.sh \
	imfile-endregex-timeout-none.sh \
	imfile-endregex-timeout-with-shutdown.sh \
	imfile-endregex-timeout-with-shutdown-polling.sh \
	imfile-endmsg.regex.sh \
	imfile-persist-state-1.sh \
	imfile-freshStartTail1.sh \
	imfile-freshStartTail2.sh \
	imfile-freshStartTail3.sh \
	imfile-wildcards.sh \
	imfile-wildcards-dirs.sh \
	imfile-wildcards-dirs2.sh \
	imfile-wildcards-dirs-multi.sh \
	imfile-wildcards-dirs-multi2.sh \
	imfile-wildcards-dirs-multi3.sh \
	imfile-wildcards-dirs-multi4.sh \
	imfile-wildcards-dirs-multi5.sh \
	imfile-wildcards-dirs-multi5-polling.sh \
	imfile-old-state-file.sh \
	imfile-rename-while-stopped.sh \
	imfile-rename.sh \
	imfile-symlink.sh \
	imfile-symlink-multi.sh \
	imfile-growing-file-id.sh \
	glbl-oversizeMsg-truncate-imfile.sh

if ENABLE_MMNORMALIZE
TESTS += \
	imfile-endmsg.regex-with-example.sh
endif

if HAVE_VALGRIND
TESTS += \
	imfile-basic-vg.sh \
	imfile-endregex-vg.sh \
	imfile-endmsg.regex-vg.sh \
	imfile-readmode0-vg.sh \
	imfile-readmode2-vg.sh \
	imfile-basic-vgthread.sh

if ENABLE_MMNORMALIZE
TESTS += \
	imfile-endmsg.regex-with-example-vg.sh
endif

endif

endif

if ENABLE_OMTCL
TESTS += \
	 omtcl.sh
endif

if ENABLE_MMKUBERNETES
if ENABLE_MMJSONPARSE
if ENABLE_IMFILE
TESTS += \
	mmkubernetes-basic.sh
if HAVE_VALGRIND
TESTS += \
	mmkubernetes-basic-vg.sh
endif
endif
endif
endif

# test samples...
#empty-hostname.log: hostname-getaddrinfo-fail.log
#hostname-getaddrinfo-fail.log: empty-hostname.log

endif # if ENABLE_TESTBENCH

TESTS_ENVIRONMENT = RSYSLOG_MODDIR='$(abs_top_builddir)'/runtime/.libs/
TESTS_ENVIRONMENT+= TOP_BUILDDIR='$(top_builddir)'
TESTS_ENVIRONMENT+= TESTTOOL_DIR='$(abs_top_builddir)/tests'
test_files = testbench.h runtime-dummy.c
DISTCLEANFILES=rsyslog.pid

distclean-local:
	rm -rf .dep_cache .dep_wrk

EXTRA_DIST= \
	internal-errmsg-memleak-vg.sh \
	glbl-oversizeMsg-log-vg.sh \
	empty-hostname.sh \
	hostname-getaddrinfo-fail.sh \
	hostname-with-slash-pmrfc5424.sh \
	hostname-with-slash-pmrfc3164.sh \
	pmrfc3164-msgFirstSpace.sh \
	pmrfc3164-AtSignsInHostname.sh \
	pmrfc3164-AtSignsInHostname_off.sh \
	pmrfc3164-tagEndingByColon.sh \
	pmrfc3164-defaultTag.sh \
	pmrfc3164-json.sh \
	hostname-with-slash-dflt-invld.sh \
	hostname-with-slash-dflt-slash-valid.sh \
	glbl-umask.sh \
	glbl-unloadmodules.sh \
	glbl-invld-param.sh \
	glbl_setenv_2_vars.sh \
	glbl_setenv_err.sh \
	glbl_setenv_err_too_long.sh \
	glbl_setenv.sh \
	mmexternal-SegFault-vg.sh \
	mmexternal-SegFault-empty-jroot-vg.sh \
	testsuites/mmexternal-SegFault-mm-python.py \
	mmexternal-InvldProg-vg.sh \
	nested-call-shutdown.sh \
	1.rstest 2.rstest 3.rstest err1.rstest \
	invalid_nested_include.sh \
	validation-run.sh \
	tls-certs/ca-key.pem \
	tls-certs/ca.pem \
	tls-certs/cert.pem \
	tls-certs/key.pem \
	testsuites/x.509/ca.pem \
	testsuites/x.509/ca-key.pem \
	testsuites/x.509/client-cert.pem \
	testsuites/x.509/client-key.pem \
	testsuites/x.509/machine-cert.pem \
	testsuites/x.509/machine-key.pem \
	cfg.sh \
	cfg1.cfgtest \
	cfg1.testin \
	cfg2.cfgtest \
	cfg2.testin \
	cfg3.cfgtest \
	cfg3.testin \
	cfg4.cfgtest \
	cfg4.testin \
	DevNull.cfgtest \
	err1.rstest \
	NoExistFile.cfgtest \
	tcp_forwarding_tpl.sh \
	tcp_forwarding_ns_tpl.sh \
	tcp_forwarding_dflt_tpl.sh \
	tcp_forwarding_retries.sh \
	killrsyslog.sh \
	parsertest-parse1.sh \
	parsertest-parse1-udp.sh \
	parsertest-parse2.sh \
	parsertest-parse2-udp.sh \
	parsertest-parse_8bit_escape.sh \
	parsertest-parse_8bit_escape-udp.sh \
	parsertest-parse3.sh \
	parsertest-parse3-udp.sh \
	parsertest-parse_invld_regex.sh \
	parsertest-parse_invld_regex-udp.sh \
	parsertest-parse-3164-buggyday.sh \
	parsertest-parse-3164-buggyday-udp.sh \
	parsertest-parse-nodate.sh \
	parsertest-parse-nodate-udp.sh \
	parsertest-snare_ccoff_udp.sh \
	parsertest-snare_ccoff_udp2.sh \
	fieldtest.sh \
	fieldtest-udp.sh \
	proprepltest-nolimittag-udp.sh \
	proprepltest-nolimittag.sh \
	proprepltest-rfctag-udp.sh \
	proprepltest-rfctag.sh \
	timestamp-3164-udp.sh \
	timestamp-3164.sh \
	timestamp-3339-udp.sh \
	timestamp-3339.sh \
	timestamp-mysql-udp.sh \
	timestamp-mysql.sh \
	timestamp-pgsql-udp.sh \
	timestamp-pgsql.sh \
	timestamp-subseconds-udp.sh \
	timestamp-subseconds.sh \
	rsf_getenv.sh \
	diskq-rfc5424.sh \
	diskqueue-full.sh \
	diskqueue.sh \
	arrayqueue.sh \
	include-obj-text-from-file.sh \
	include-obj-outside-control-flow-vg.sh \
	include-obj-in-if-vg.sh \
	include-obj-text-vg.sh \
	rscript-config_enable-off-vg.sh \
	rscript-config_enable-on.sh \
	rscript_http_request.sh \
	rscript_http_request-vg.sh \
	rscript_bare_var_root.sh \
	rscript_bare_var_root-empty.sh \
	rscript_contains.sh \
	rscript_ipv42num.sh \
	rscript_field.sh \
	rscript_field-vg.sh \
	rscript_stop.sh \
	rscript_stop2.sh \
	stop.sh \
	rscript_le.sh \
	rscript_le_var.sh \
	rscript_ge.sh \
	rscript_ge_var.sh \
	rscript_lt.sh \
	rscript_lt_var.sh \
	rscript_gt.sh \
	rscript_gt_var.sh \
	rscript_ne.sh \
	rscript_ne_var.sh \
	rscript_num2ipv4.sh \
	rscript_int2Hex.sh \
	rscript_trim.sh \
	rscript_substring.sh \
	rscript_format_time.sh \
	rscript_parse_time.sh \
	rscript_parse_time_get-ts.py \
	rscript_is_time.sh \
	rscript_script_error.sh \
	rscript_parse_json.sh \
	rscript_parse_json-vg.sh \
	rscript_backticks-vg.sh \
	rscript_previous_action_suspended.sh \
	rscript_str2num_negative.sh \
	rs-cnum.sh \
	rs-int2hex.sh \
	rs-substring.sh \
	mmanon_random_32_ipv4.sh \
	mmanon_random_cons_32_ipv4.sh \
	mmanon_recognize_ipv4.sh \
	mmanon_zero_12_ipv4.sh \
	mmanon_zero_33_ipv4.sh \
	mmanon_zero_8_ipv4.sh \
	mmanon_simple_12_ipv4.sh \
	mmanon_simple_33_ipv4.sh \
	mmanon_simple_8_ipv4.sh \
	mmanon_random_128_ipv6.sh \
	mmanon_zero_128_ipv6.sh \
	mmanon_zero_96_ipv6.sh \
	mmanon_random_cons_128_ipv6.sh \
	mmanon_zero_50_ipv6.sh \
	mmanon_recognize_ipv6.sh \
	mmanon_zero_64_ipv6.sh \
	mmanon_both_modes_compatible.sh \
	mmanon_recognize_ipembedded.sh \
	mmanon_random_cons_128_ipembedded.sh \
	rscript_eq.sh \
	rscript_eq_var.sh \
	rscript_set_memleak-vg.sh \
	rscript_set_unset_invalid_var.sh \
	rscript_set_modify.sh \
	stop-localvar.sh \
	stop-msgvar.sh \
	omfwd-keepalive.sh \
	omfile-read-only-errmsg.sh \
	omfile-null-filename.sh \
	omfile-whitespace-filename.sh \
	omfile-read-only.sh \
	omfile_both_files_set.sh \
	msgvar-concurrency.sh \
	msgvar-concurrency-array.sh \
	testsuites/msgvar-concurrency-array.rulebase \
	msgvar-concurrency-array-event.tags.sh \
	testsuites/msgvar-concurrency-array-event.tags.rulebase \
	localvar-concurrency.sh \
	exec_tpl-concurrency.sh \
	prop-jsonmesg-vg.sh \
	prop-all-json-concurrency.sh \
	no-parser-errmsg.sh \
	global_vars.sh \
	no-parser-errmsg.sh \
	no-parser-vg.sh \
	prop-programname.sh \
	prop-programname-with-slashes.sh \
	rfc5424parser.sh \
	privdrop_common.sh \
	privdropuser.sh \
	privdropuserid.sh \
	privdropgroup.sh \
	privdropgroupid.sh \
	template-json.sh \
	template-pure-json.sh \
	template-pos-from-to.sh \
	template-pos-from-to-lowercase.sh \
	template-pos-from-to-oversize.sh \
	template-pos-from-to-oversize-lowercase.sh \
	template-pos-from-to-missing-jsonvar.sh \
	template-const-jsonf.sh \
	fac_authpriv.sh \
	fac_local0.sh \
	fac_local0-vg.sh \
	fac_local7.sh \
	fac_mail.sh \
	fac_news.sh \
	fac_ftp.sh \
	fac_ntp.sh \
	fac_uucp.sh \
	fac_invld1.sh \
	fac_invld2.sh \
	fac_invld3.sh \
	fac_invld4_rfc5424.sh \
	compresssp.sh \
	compresssp-stringtpl.sh \
	now_family_utc.sh \
	now-utc-ymd.sh \
	now-utc-casecmp.sh \
	now-utc.sh \
	faketime_common.sh \
	imjournal-basic.sh \
	imjournal-statefile.sh \
	imjournal-basic-vg.sh \
	omjournal-abort-template.sh \
	omjournal-abort-no-template.sh \
	omjournal-basic-template.sh \
	omjournal-basic-no-template.sh \
	timegenerated-ymd.sh \
	timegenerated-uxtimestamp.sh \
	timegenerated-uxtimestamp-invld.sh \
	timegenerated-dateordinal.sh \
	timegenerated-dateordinal-invld.sh \
	timegenerated-utc.sh \
	timegenerated-utc-legacy.sh \
	timereported-utc.sh \
	timereported-utc-legacy.sh \
	timereported-utc-vg.sh \
	mmrm1stspace-basic.sh \
	mmnormalize_rule_from_string.sh \
	mmnormalize_rule_from_array.sh \
	pmnull-basic.sh \
	pmnull-withparams.sh \
	omstdout-basic.sh \
	testsuites/mmnormalize_processing_tests.rulebase \
	mmnormalize_processing_test1.sh \
	mmnormalize_processing_test2.sh \
	mmnormalize_processing_test3.sh \
	mmnormalize_processing_test4.sh \
	pmnormalize-rule.sh \
	pmnormalize-rule-vg.sh\
	testsuites/pmnormalize_basic.rulebase \
	pmnormalize-basic.sh \
	rawmsg-after-pri.sh \
	rs_optimizer_pri.sh \
	rscript_prifilt.sh \
	rscript_optimizer1.sh \
	rscript_ruleset_call.sh \
	rscript_ruleset_call_indirect-basic.sh \
	rscript_ruleset_call_indirect-var.sh \
	rscript_ruleset_call_indirect-invld.sh \
	cee_simple.sh \
	cee_diskqueue.sh \
	mmjsonparse-w-o-cookie.sh \
	mmjsonparse-w-o-cookie-multi-spaces.sh \
	mmjsonparse_simple.sh \
	mmjsonparse-invalid-containerName.sh \
	wtpShutdownAll-assertionFailure.sh \
	imptcp-oversize-message-display.sh \
	imptcp-msg-truncation-on-number.sh \
	imptcp-msg-truncation-on-number2.sh \
	imptcp-maxFrameSize-parameter.sh \
	mmjsonparse_cim.sh \
	mmjsonparse_cim2.sh \
	mmjsonparse_localvar.sh \
	mmdb.sh \
	mmdb.rb \
	test.mmdb \
	mmdb-vg.sh \
	mmdb-container.sh \
	mmdb-container-empty.sh \
	mmdb-multilevel-vg.sh \
	incltest.sh \
	incltest_dir.sh \
	incltest_dir_empty_wildcard.sh \
	incltest_dir_wildcard.sh \
	testsuites/es.yml \
	es_response_get_msgnum.py \
	elasticsearch-error-format-check.py \
	es-basic-es6.0.sh \
	es-basic.sh \
	es-basic-vgthread.sh \
	es-basic-server.sh \
	es-basic-ha.sh \
	es-basic-bulk.sh \
	es-basic-errfile-empty.sh \
	es-basic-errfile-popul.sh \
	es-bulk-errfile-empty.sh \
	es-bulk-errfile-popul.sh \
	es-bulk-errfile-popul-def-format.sh \
	es-bulk-errfile-popul-erronly.sh \
	es-bulk-errfile-popul-erronly-interleaved.sh \
	es-bulk-errfile-popul-def-interleaved.sh \
	es-basic-vg.sh \
	es-basic-bulk-vg.sh \
	es-basic-ha-vg.sh \
	es-maxbytes-bulk.sh \
	es-bulk-retry.sh \
	linkedlistqueue.sh \
	da-mainmsg-q.sh \
	diskqueue-fsync.sh \
	msgdup.sh \
	empty-ruleset.sh \
	imtcp-discard-truncated-msg.sh \
	imtcp-basic.sh \
	imtcp-maxFrameSize.sh \
	imtcp-msg-truncation-on-number.sh \
	imtcp-msg-truncation-on-number2.sh \
	imtcp-NUL.sh \
	imtcp-NUL-rawmsg.sh \
	imtcp-tls-basic.sh \
	imtcp-tls-basic-vg.sh \
	imtcp_incomplete_frame_at_end.sh \
	imtcp-multiport.sh \
	udp-msgreduc-orgmsg-vg.sh \
	udp-msgreduc-vg.sh \
	manytcp-too-few-tls-vg.sh \
	imtcp-tls-ossl-basic.sh \
	sndrcv_tls_ossl_anon_ipv4.sh \
	sndrcv_tls_ossl_anon_ipv6.sh \
	sndrcv_tls_ossl_anon_rebind.sh \
	imtcp-tls-ossl-x509valid.sh \
	imtcp-tls-ossl-x509name.sh \
	imtcp-tls-ossl-x509fingerprint.sh \
	imtcp-tls-ossl-basic-vg.sh \
	manytcp.sh \
	manyptcp.sh \
	imptcp-NUL.sh \
	imptcp-NUL-rawmsg.sh \
	imptcp_framing_regex.sh \
	testsuites/imptcp_framing_regex.testdata \
	imptcp_framing_regex-oversize.sh \
	testsuites/imptcp_framing_regex-oversize.testdata \
	imptcp_large.sh \
	imptcp-connection-msg-disabled.sh \
	imptcp-connection-msg-received.sh \
	imptcp-discard-truncated-msg.sh \
	imptcp_addtlframedelim.sh \
	imptcp_conndrop-vg.sh \
	imptcp_conndrop.sh \
	imptcp_multi_line.sh \
	testsuites/imptcp_multi_line.testdata \
	imptcp_no_octet_counted.sh \
	testsuites/no_octet_counted.testdata \
	imtcp_no_octet_counted.sh \
	testsuites/spframingfix.testdata \
	imtcp_spframingfix.sh \
	imptcp_spframingfix.sh \
	imtcp_conndrop.sh \
	imtcp_conndrop_tls.sh \
	imtcp_conndrop_tls-vg.sh \
	imtcp_addtlframedelim.sh \
	tcp-msgreduc-vg.sh \
	inputname-imtcp.sh \
	omod-if-array.sh \
	omod-if-array-udp.sh \
	discard.sh \
	failover-no-rptd.sh \
	failover-no-rptd-vg.sh \
	failover-no-basic.sh \
	failover-no-basic-vg.sh \
	failover-rptd.sh \
	failover-rptd-vg.sh \
	failover-basic.sh \
	failover-basic-vg.sh \
	failover-async.sh \
	failover-double.sh \
	discard-rptdmsg.sh \
	discard-rptdmsg-vg.sh \
	discard-allmark.sh \
	discard-allmark-vg.sh \
	diag.sh \
	rcvr_fail_restore.sh \
	daqueue-dirty-shutdown.sh \
	daqueue-invld-qi.sh \
	daqueue-persist.sh \
	daqueue-persist-drvr.sh \
	queue-persist.sh \
	queue-persist-drvr.sh \
	threadingmq.sh \
	threadingmqaq.sh \
	sndrcv_drvr.sh \
	sndrcv_drvr_noexit.sh \
	sndrcv_failover.sh \
	sndrcv.sh \
	imrelp-basic.sh \
	imrelp-manyconn.sh \
	imrelp-maxDataSize-error.sh \
	imrelp-long-msg.sh \
	imrelp-oversizeMode-truncate.sh \
	imrelp-oversizeMode-accept.sh \
	glbl-oversizeMsg-log.sh \
	glbl-oversizeMsg-truncate.sh \
	glbl-oversizeMsg-split.sh \
	sndrcv_relp.sh \
	sndrcv_relp_rebind.sh \
	sndrcv_relp_tls_prio.sh \
	sndrcv_relp_tls.sh \
        sndrcv_relp_tls_certvalid.sh \
	relp_tls_certificate_not_found.sh \
	omrelp_wrong_authmode.sh \
	sndrcv_relp_dflt_pt.sh \
	sndrcv_udp.sh \
	imudp_thread_hang.sh \
	sndrcv_udp_nonstdpt.sh \
	sndrcv_udp_nonstdpt_v6.sh \
	sndrcv_omudpspoof.sh \
	sndrcv_omudpspoof_nonstdpt.sh \
	sndrcv_gzip.sh \
	action-tx-single-processing.sh \
	action-tx-errfile.sh \
	testsuites/action-tx-errfile.result \
	pipeaction.sh \
	omprog-defaults.sh \
	omprog-defaults-vg.sh \
	omprog-output-capture.sh \
	omprog-output-capture-mt.sh \
	omprog-output-capture-vg.sh \
	omprog-feedback.sh \
	omprog-feedback-mt.sh \
	omprog-feedback-vg.sh \
	omprog-feedback-timeout.sh \
	omprog-close-unresponsive.sh \
	omprog-close-unresponsive-vg.sh \
	omprog-close-unresponsive-noterm.sh \
	omprog-restart-terminated.sh \
	omprog-restart-terminated-vg.sh \
	omprog-restart-terminated-outfile.sh \
	omprog-single-instance.sh \
	omprog-single-instance-vg.sh \
	omprog-single-instance-outfile.sh \
	omprog-transactions.sh \
	omprog-transactions-vg.sh \
	omprog-transactions-failed-messages.sh \
	omprog-transactions-failed-commits.sh \
	testsuites/omprog-defaults-bin.sh \
	testsuites/omprog-output-capture-bin.sh \
	testsuites/omprog-output-capture-mt-bin.py \
	testsuites/omprog-feedback-bin.sh \
	testsuites/omprog-feedback-mt-bin.sh \
	testsuites/omprog-feedback-timeout-bin.sh \
	testsuites/omprog-close-unresponsive-bin.sh \
	testsuites/omprog-restart-terminated-bin.sh \
	testsuites/omprog-single-instance-bin.sh \
	testsuites/omprog-transactions-bin.sh \
	pipe_noreader.sh \
	uxsock_simple.sh \
	asynwr_simple.sh \
	asynwr_simple_2.sh \
	asynwr_timeout.sh \
	asynwr_timeout_2.sh \
	asynwr_small.sh \
	asynwr_tinybuf.sh \
	wr_large_async.sh \
	wr_large_sync.sh \
	asynwr_deadlock.sh \
	asynwr_deadlock_2.sh \
	asynwr_deadlock2.sh \
	asynwr_deadlock4.sh \
	abort-uncleancfg-goodcfg.sh \
	abort-uncleancfg-goodcfg-check.sh \
	abort-uncleancfg-badcfg-check.sh \
	abort-uncleancfg-badcfg-check_1.sh \
	variable_leading_underscore.sh \
	gzipwr_rscript.sh \
	gzipwr_flushInterval.sh \
	gzipwr_flushOnTXEnd.sh \
	gzipwr_large.sh \
	gzipwr_large_dynfile.sh \
	complex1.sh \
	random.sh \
	testsuites/imfile-old-state-file_imfile-state_.-rsyslog.input \
	imfile-readmode0-vg.sh \
	imfile-readmode2.sh \
	imfile-readmode2-polling.sh \
	imfile-readmode2-vg.sh \
	imfile-readmode2-with-persists-data-during-stop.sh \
	imfile-readmode2-with-persists.sh \
	imfile-endregex-save-lf.sh \
	imfile-endregex-save-lf-persist.sh \
	imfile-endregex.sh \
	imfile-endregex-vg.sh \
	imfile-basic.sh \
	imfile-discard-truncated-line.sh \
	imfile-truncate-line.sh \
	imfile-file-not-found-error.sh \
	imfile-fileNotFoundError-parameter.sh \
	imfile-error-not-repeated.sh \
	imfile-basic-vg.sh \
	imfile-basic-vgthread.sh \
	imfile-endregex-timeout-none-polling.sh \
	imfile-endregex-timeout-polling.sh \
	imfile-endregex-timeout.sh \
	imfile-endregex-timeout-none.sh \
	imfile-endregex-timeout-with-shutdown.sh \
	imfile-endregex-timeout-with-shutdown-polling.sh \
	imfile-endmsg.regex.sh \
	imfile-endmsg.regex-vg.sh \
	imfile-endmsg.regex-with-example.sh \
	imfile-endmsg.regex-with-example-vg.sh \
	imfile-endmsg.regex.crio.rulebase \
	imfile-endmsg.regex.json.rulebase \
	imfile-persist-state-1.sh \
	imfile-freshStartTail1.sh \
	imfile-freshStartTail2.sh \
	imfile-freshStartTail3.sh \
	imfile-truncate.sh \
	imfile-wildcards.sh \
	imfile-wildcards-dirs.sh \
	imfile-wildcards-dirs2.sh \
	imfile-wildcards-dirs-multi.sh \
	imfile-wildcards-dirs-multi2.sh \
	imfile-wildcards-dirs-multi3.sh \
	imfile-wildcards-dirs-multi4.sh \
	imfile-wildcards-dirs-multi5.sh \
	imfile-wildcards-dirs-multi5-polling.sh \
	imfile-old-state-file.sh \
	imfile-rename-while-stopped.sh \
	imfile-rename.sh \
	imfile-symlink.sh \
	imfile-symlink-multi.sh \
	imfile-growing-file-id.sh \
	glbl-oversizeMsg-truncate-imfile.sh \
	dynfile_invld_async.sh \
	dynfile_invld_sync.sh \
	dynfile_invalid2.sh \
	rulesetmultiqueue.sh \
	rulesetmultiqueue-v6.sh \
	omruleset.sh \
	omruleset-queue.sh \
	badqi.sh \
	bad_qi/dbq.qi \
	execonlyonce.sh \
	testsuites/execonlyonce.data \
	execonlywhenprevsuspended.sh \
	execonlywhenprevsuspended2.sh \
	execonlywhenprevsuspended3.sh \
	execonlywhenprevsuspended4.sh \
	execonlywhenprevsuspended_multiwrkr.sh \
	execonlywhenprevsuspended-queue.sh \
	execonlywhenprevsuspended-nonsusp.sh \
	execonlywhenprevsuspended-nonsusp-queue.sh \
	tabescape_dflt.sh \
	tabescape_dflt-udp.sh \
	tabescape_off.sh \
	tabescape_off-udp.sh \
	dircreate_dflt.sh \
	dircreate_off.sh \
	imuxsock_logger_parserchain.sh \
	imuxsock_logger.sh \
	imuxsock_logger_ruleset.sh \
	imuxsock_logger_ruleset_ratelimit.sh \
	imuxsock_logger_err.sh \
	imuxsock_logger_root.sh \
	imuxsock_logger_syssock.sh \
	resultdata/imuxsock_logger.log \
	imuxsock_traillf.sh \
	imuxsock_traillf_root.sh \
	imuxsock_traillf_syssock.sh \
	resultdata/imuxsock_traillf.log \
	imuxsock_ccmiddle.sh \
	imuxsock_ccmiddle_root.sh \
	imklog_permitnonkernelfacility_root.sh \
	imuxsock_ccmiddle_syssock.sh \
	resultdata/imuxsock_ccmiddle.log \
	imuxsock_hostname.sh \
	resultdata/imuxsock_hostname.log \
	testsuites/mysql-truncate.sql \
	testsuites/mysql-select-msg.sql \
	libdbi-basic.sh \
	libdbi-asyn.sh \
	mysql-basic.sh \
	mysql-basic-cnf6.sh \
	mysql-basic-vg.sh \
	mysql-asyn.sh \
	mysql-asyn-vg.sh \
	mysql-actq-mt.sh \
	mysql-actq-mt-withpause.sh \
	mysql-actq-mt-withpause-vg.sh \
	omkafka.sh \
	imkafka.sh \
	imkafka_multi_single.sh \
	sndrcv_kafka.sh \
	sndrcv_kafka_multi_topics.sh \
	sndrcv_kafka-vg-rcvr.sh \
	sndrcv_kafka-vg-sender.sh \
	testsuites/kafka-server.properties \
	testsuites/kafka-server.dep_wrk1.properties \
	testsuites/kafka-server.dep_wrk2.properties \
	testsuites/kafka-server.dep_wrk3.properties \
	testsuites/zoo.cfg \
	testsuites/zoo.dep_wrk1.cfg \
	testsuites/zoo.dep_wrk2.cfg \
	testsuites/zoo.dep_wrk3.cfg \
	mmpstrucdata.sh \
	mmpstrucdata-case.sh \
	mmpstrucdata-vg.sh \
	mmpstrucdata-invalid-vg.sh \
	libdbi-basic-vg.sh \
	dynstats_ctr_reset.sh \
	dynstats_reset_without_pstats_reset.sh \
	dynstats_nometric.sh \
	dynstats_overflow.sh \
	dynstats_overflow-vg.sh \
	dynstats_reset.sh \
	dynstats_reset-vg.sh \
	impstats-hup.sh \
	dynstats.sh \
	dynstats-vg.sh \
	dynstats_prevent_premature_eviction.sh \
	dynstats_prevent_premature_eviction-vg.sh \
	testsuites/dynstats_empty_input \
	testsuites/dynstats_input \
	testsuites/dynstats_input_1 \
	testsuites/dynstats_input_2 \
	testsuites/dynstats_input_3 \
	testsuites/dynstats_input_more_0 \
	testsuites/dynstats_input_more_1 \
	testsuites/dynstats_input_more_2 \
	no-dynstats-json.sh \
	no-dynstats.sh \
	stats-json.sh \
	stats-json-vg.sh \
	stats-cee.sh \
	stats-cee-vg.sh \
	stats-json-es.sh \
	dynstats-json.sh \
	dynstats-json-vg.sh \
	mmnormalize_variable.sh \
	mmnormalize_tokenized.sh \
	testsuites/mmnormalize_variable.rulebase \
	testsuites/date_time_msg \
	testsuites/mmnormalize_tokenized.rulebase \
	testsuites/tokenized_input \
	rscript_random.sh \
	rscript_hash32.sh \
	rscript_hash32-vg.sh \
	rscript_hash64.sh \
	rscript_hash64-vg.sh \
	rscript_replace.sh \
	rscript_replace_complex.sh \
	testsuites/complex_replace_input \
	rscript_unaffected_reset.sh \
	rscript_wrap2.sh \
	rscript_wrap3.sh \
	testsuites/wrap3_input\
	json_array_subscripting.sh \
	testsuites/json_array_input \
	testsuites/json_object_input \
	testsuites/json_nonarray_input \
	json_array_looping.sh \
	json_object_looping.sh \
	json_object_looping-vg.sh \
	json_array_looping-vg.sh \
	json_object_suicide_in_loop-vg.sh \
	json_nonarray_looping.sh \
	json_null.sh \
	json_null-vg.sh \
	json_null_array.sh \
	json_null_array-vg.sh \
	mmnormalize_regex.sh \
	testsuites/mmnormalize_regex.rulebase \
	testsuites/regex_input \
	mmnormalize_regex_disabled.sh \
	mmnormalize_regex_defaulted.sh \
	stop_when_array_has_element.sh \
	testsuites/stop_when_array_has_elem_input \
	key_dereference_on_uninitialized_variable_space.sh \
	rscript_re_extract.sh \
	rscript_re_match.sh \
	lookup_table.sh \
	lookup_table_no_hup_reload.sh \
	lookup_table_no_hup_reload-vg.sh \
	lookup_table_rscript_reload.sh \
	lookup_table_rscript_reload_without_stub.sh \
	lookup_table_rscript_reload-vg.sh \
	lookup_table_rscript_reload_without_stub-vg.sh \
	rscript_trim-vg.sh \
	testsuites/xlate.lkp_tbl \
	testsuites/xlate_more.lkp_tbl \
	unused_lookup_table-vg.sh \
	lookup_table-vg.sh \
	array_lookup_table.sh \
	array_lookup_table-vg.sh \
	array_lookup_table_misuse-vg.sh \
	multiple_lookup_tables.sh \
	multiple_lookup_tables-vg.sh \
	testsuites/xlate_array.lkp_tbl \
	testsuites/xlate_array_more.lkp_tbl \
	testsuites/xlate_array_misuse.lkp_tbl \
	testsuites/xlate_array_more_misuse.lkp_tbl \
	sparse_array_lookup_table.sh \
	sparse_array_lookup_table-vg.sh \
	testsuites/xlate_sparse_array.lkp_tbl \
	testsuites/xlate_sparse_array_more.lkp_tbl \
	lookup_table_bad_configs.sh \
	lookup_table_bad_configs-vg.sh \
	testsuites/xlate_array_empty_table.lkp_tbl \
	testsuites/xlate_array_no_index.lkp_tbl \
	testsuites/xlate_array_no_table.lkp_tbl \
	testsuites/xlate_array_no_value.lkp_tbl \
	testsuites/xlate_empty_file.lkp_tbl \
	testsuites/xlate_incorrect_type.lkp_tbl \
	testsuites/xlate_incorrect_version.lkp_tbl \
	testsuites/xlate_sparseArray_empty_table.lkp_tbl \
	testsuites/xlate_sparseArray_no_index.lkp_tbl \
	testsuites/xlate_sparseArray_no_table.lkp_tbl \
	testsuites/xlate_sparseArray_no_value.lkp_tbl \
	testsuites/xlate_string_empty_table.lkp_tbl \
	testsuites/xlate_string_no_index.lkp_tbl \
	testsuites/xlate_string_no_table.lkp_tbl \
	testsuites/xlate_string_no_value.lkp_tbl \
	testsuites/xlate_invalid_json.lkp_tbl \
	testsuites/xlate_array_more_with_duplicates_and_nomatch.lkp_tbl \
	testsuites/xlate_more_with_duplicates_and_nomatch.lkp_tbl \
	testsuites/xlate_sparse_array_more_with_duplicates_and_nomatch.lkp_tbl \
	json_var_cmpr.sh \
	imptcp_nonProcessingPoller.sh \
	imptcp_veryLargeOctateCountedMessages.sh \
	known_issues.supp \
	libmaxmindb.supp \
	travis/trusty.supp \
	linux_localtime_r.supp \
	CI/centos6-9.supp \
	json_var_case.sh \
	cfg.sh \
	empty-prop-comparison.sh \
	sndrcv_tls_anon_rebind.sh \
	sndrcv_tls_anon_hostname.sh \
	sndrcv_tls_anon_ipv4.sh \
	sndrcv_tls_anon_ipv6.sh \
	sndrcv_tls_priorityString.sh \
	omtcl.sh \
	omtcl.tcl \
	pmsnare-default.sh \
	pmsnare-default-udp.sh \
	pmsnare-ccoff.sh \
	pmsnare-ccoff-udp.sh \
	pmsnare-ccdefault.sh \
	pmsnare-ccdefault-udp.sh \
	pmsnare-cccstyle.sh \
	pmsnare-cccstyle-udp.sh \
	pmsnare-ccbackslash.sh \
	pmsnare-ccbackslash-udp.sh \
	pmsnare-modoverride.sh \
	pmsnare-modoverride-udp.sh \
	pmlastmsg.sh \
	pmlastmsg-udp.sh \
	pgsql-basic.sh \
	testsuites/pgsql-basic.sql \
	testsuites/pgsql-select-msg.sql \
	testsuites/pgsql-select-syslogtag.sql \
	pgsql-basic-cnf6.sh \
	pgsql-basic-threads-cnf6.sh \
	pgsql-template.sh \
	pgsql-template-cnf6.sh \
	pgsql-actq-mt-withpause.sh \
	pgsql-template-threads-cnf6.sh \
	pgsql-basic-vg.sh \
	pgsql-template-vg.sh \
	pgsql-basic-cnf6-vg.sh \
	pgsql-template-cnf6-vg.sh \
	pgsql-actq-mt-withpause-vg.sh \
	../devtools/prep-mysql-db.sh \
	mmkubernetes-basic.sh \
	mmkubernetes-basic-vg.sh \
	mmkubernetes_test_server.py \
	mmkubernetes-basic.out.json \
	es-writeoperation.sh \
	testsuites/incltest.d/include.conf \
	testsuites/abort-uncleancfg-goodcfg.conf \
	testsuites/include-std-omfile-action.conf \
	testsuites/invalid.conf \
	testsuites/valid.conf \
	testsuites/variable_leading_underscore.conf

ourtail_SOURCES = ourtail.c
msleep_SOURCES = msleep.c
omrelp_dflt_port_SOURCES = omrelp_dflt_port.c
mangle_qi_SOURCES = mangle_qi.c
chkseq_SOURCES = chkseq.c
have_relpSrvSetOversizeMode = have_relpSrvSetOversizeMode.c
test_id_SOURCES = test_id.c

uxsockrcvr_SOURCES = uxsockrcvr.c
uxsockrcvr_LDADD = $(SOL_LIBS)

tcpflood_SOURCES = tcpflood.c
tcpflood_CFLAGS = $(PTHREADS_CFLAGS) $(RELP_CFLAGS)
tcpflood_CPPFLAGS = $(PTHREADS_CFLAGS) $(RELP_CFLAGS)
tcpflood_LDADD = $(SOL_LIBS) $(PTHREADS_LIBS) $(RELP_LIBS)
if ENABLE_GNUTLS
tcpflood_CFLAGS += $(GNUTLS_CFLAGS)
tcpflood_CPPFLAGS += $(GNUTLS_CFLAGS)
tcpflood_LDADD += $(GNUTLS_LIBS)
endif
if ENABLE_OPENSSL
tcpflood_CFLAGS += $(OPENSSL_CFLAGS)
tcpflood_CPPFLAGS += $(OPENSSL_CFLAGS)
tcpflood_LDADD += $(OPENSSL_LIBS)
endif

minitcpsrv_SOURCES = minitcpsrvr.c
minitcpsrv_LDADD = $(SOL_LIBS)

syslog_caller_SOURCES = syslog_caller.c
syslog_caller_CPPFLAGS = $(LIBLOGGING_STDLOG_CFLAGS)
syslog_caller_LDADD = $(SOL_LIBS) $(LIBLOGGING_STDLOG_LIBS)

journal_print_SOURCES = journal_print.c
journal_print_CPPFLAGS = $(LIBSYSTEMD_JOURNAL_CFLAGS)
journal_print_LDADD = $(LIBSYSTEMD_JOURNAL_LIBS)

diagtalker_SOURCES = diagtalker.c
diagtalker_LDADD = $(SOL_LIBS)

randomgen_SOURCES = randomgen.c
randomgen_LDADD = $(SOL_LIBS)

inputfilegen_SOURCES = inputfilegen.c
inputfilegen_LDADD = $(SOL_LIBS)

# rtinit tests disabled for the moment - also questionable if they
# really provide value (after all, everything fails if rtinit fails...)
#rt_init_SOURCES = rt-init.c $(test_files)
#rt_init_CPPFLAGS =  -I$(top_srcdir) $(PTHREADS_CFLAGS) $(RSRT_CFLAGS)
#rt_init_LDADD = $(RSRT_LIBS) $(ZLIB_LIBS) $(PTHREADS_LIBS) $(SOL_LIBS)
#rt_init_LDFLAGS = -export-dynamic

# same for basic rscript tests
#rscript_SOURCES = rscript.c getline.c $(test_files)
#rscript_CPPFLAGS =  -I$(top_srcdir) $(PTHREADS_CFLAGS) $(RSRT_CFLAGS)
#rscript_LDADD = $(RSRT_LIBS) $(ZLIB_LIBS) $(PTHREADS_LIBS) $(SOL_LIBS)
#rscript_LDFLAGS = -export-dynamic
