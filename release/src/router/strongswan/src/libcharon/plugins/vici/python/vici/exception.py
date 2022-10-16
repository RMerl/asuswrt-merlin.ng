"""Exception types that may be thrown by this library."""


class DeserializationException(Exception):
    """Encountered an unexpected byte sequence or missing element type."""


class SessionException(Exception):
    """Session request exception."""


class CommandException(Exception):
    """Command result exception."""


class EventUnknownException(Exception):
    """Event unknown exception."""
