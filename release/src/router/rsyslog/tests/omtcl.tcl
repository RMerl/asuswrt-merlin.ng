proc doAction {msg} {
  set fd [open $::env(RSYSLOG_OUT_LOG) a]
  puts $fd "message processed:"
  foreach {k v} $msg {
    puts $fd "  $k: <<$v>>"
  }
  puts $fd "  uppercase message: <<[string toupper [dict get $msg message]]>>"
  close $fd
}
