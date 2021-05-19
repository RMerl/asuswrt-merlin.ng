
/**
 * @file mainloop_state.inc
 * @brief Declare configuration options for the crypto_ops module.
 **/

/** Holds state for the mainloop, corresponding to part of the state
 * file in Tor's DataDirectory. */
BEGIN_CONF_STRUCT(mainloop_state_t)

/** Number of minutes since the last user-initiated request (as defined by
 * the dormant net-status system.) Set to zero if we are dormant. */
CONF_VAR(MinutesSinceUserActivity, POSINT, 0, NULL)

/** True if we were dormant when we last wrote the file; false if we
 * weren't.  "auto" on initial startup. */
CONF_VAR(Dormant, AUTOBOOL, 0, "auto")

END_CONF_STRUCT(mainloop_state_t)
