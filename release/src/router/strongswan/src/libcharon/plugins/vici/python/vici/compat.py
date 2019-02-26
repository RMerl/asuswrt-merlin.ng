# Help functions for compatibility between python version 2 and 3


# From http://legacy.python.org/dev/peps/pep-0469
try:
    dict.iteritems
except AttributeError:
    # python 3
    def iteritems(d):
        return iter(d.items())
else:
    # python 2
    def iteritems(d):
        return d.iteritems()
