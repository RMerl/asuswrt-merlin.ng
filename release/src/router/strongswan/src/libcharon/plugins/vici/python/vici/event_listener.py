from functools import wraps
import inspect

from .protocol import RECV_TIMEOUT_DEFAULT


class StopListening(Exception):
    """Exception that may be raised to stop listening for events."""


class EventListener(object):
    def __init__(self, session=None):
        """Create an event listener instance, which provides decorator methods
        to make listening for events and the disconnection of the vici session
        more convenient.

        The session is optional here, but one must be set via
        :func:`~set_session()` before calling :func:`~listen()`.

        :param session: optional vici session to use
        :type session: :class:`~vici.session.Session` or None
        """
        self.event_map = {}
        self.disconnect_list = []
        self.timeout_list = []
        self.session = session

    def set_session(self, session):
        """Set the session that's used to listen for events. Only has an effect
        when set before calling :func:`~listen()`.

        :param session: vici session to use
        :type session: :class:`~vici.session.Session`
        """
        self.session = session

    def on_events(self, events):
        """Decorator to mark a function as a listener for specific events.

        The decorated function is expected to receive the name of the event and
        the data as arguments. It may raise :class:`~StopListening` to stop
        listening and let :func:`~listen()` return.

        :param events: events to register and call decorated function for
        :type events: list
        :return: decorator function
        :rtype: any
        """
        def decorator(func):
            self.event_map.update({event: func for event in events})

            @wraps(func)
            def wrapper(*args, **kwargs):
                return func(*args, **kwargs)
            return wrapper
        return decorator

    def on_disconnected(self):
        """Decorator to mark a function as a listener for when the daemon
        disconnects the vici session.

        This listener instance is passed to the decorated function, which may
        be used to set a new session and continue listening. If no session is
        set, :func:`~listen()` will return after the decorated function has
        been called.

        :return: decorator function
        :rtype: any
        """
        def decorator(func):
            self.disconnect_list.append(func)

            @wraps(func)
            def wrapper(*args, **kwargs):
                return func(*args, **kwargs)
            return wrapper
        return decorator

    def on_timeout(self):
        """Decorator to mark a function as a listener for when a timeout occurs
        while waiting for events. Only has an effect if :func:`~listen()` is
        called with a timeout.

        The decorated function may either take no or two arguments (both will be
        set to `None`). So this may be applied to a function that's also
        decorated with :func:`~on_events()`. It may raise
        :class:`~StopListening` to stop listening and let :func:`~listen()`
        return.

        :return: decorator function
        :rtype: any
        """
        def decorator(func):
            self.timeout_list.append(func)

            @wraps(func)
            def wrapper(*args, **kwargs):
                return func(*args, **kwargs)
            return wrapper
        return decorator

    def listen(self, timeout=RECV_TIMEOUT_DEFAULT):
        """Dispatch events registered via decorators of this instance.

        An active session has to be set before calling this. After getting
        disconnected, a new session may be set via :func:`~set_session()` in
        a function decorated with :func:`~on_disconnected()` to resume
        listening for events.

        This method does not return unless :class:`~StopListening` or an
        unexpected exception is raised or if the current session is disconnected
        and no new session is set in a listener.

        The optional timeout allows calling functions decorated with
        :func:`~on_timeout()` if no event has been received for that time. Which
        may be used to abort listening or perform periodic tasks while
        continuing to listen for events.

        :param timeout: timeout to wait for events, in fractions of a second
        :type timeout: float
        :return: True if StopListening was raised, False if no session available
        :rtype: bool
        """
        while True:
            try:
                if self.session is None:
                    return False
                for label, event in self.session.listen(self.event_map.keys(),
                                                        timeout):
                    if label is None and event is None:
                        for func in self.timeout_list:
                            if len(inspect.signature(func).parameters) > 0:
                                func(label, event)
                            else:
                                func()
                        continue
                    name = label.decode()
                    if name in self.event_map:
                        self.event_map[name](name, event)
            except IOError:
                self.session = None
                for func in self.disconnect_list:
                    func(self)
                continue
            except StopListening:
                return True
