import gdb
import glib
import gdb.backtrace
import gdb.command.backtrace

# This is not quite right, as local vars may override symname
def read_global_var (symname):
    return gdb.selected_frame().read_var(symname)

def g_type_to_name (gtype):
    def lookup_fundamental_type (typenode):
        if typenode == 0:
            return None
        val = read_global_var ("static_fundamental_type_nodes")
        if val == None:
            return None
        return val[typenode >> 2].address()

    gtype = long(gtype)
    typenode = gtype - gtype % 4
    if typenode > (255 << 2):
        typenode = gdb.Value(typenode).cast (gdb.lookup_type("TypeNode").pointer())
    else:
        typenode = lookup_fundamental_type (typenode)
    if typenode != None:
        return glib.g_quark_to_string (typenode["qname"])
    return None

def is_g_type_instance (val):
    def is_g_type_instance_helper (type):
        if str(type) == "GTypeInstance":
            return True

        while type.code == gdb.TYPE_CODE_TYPEDEF:
            type = type.target()

        if type.code != gdb.TYPE_CODE_STRUCT:
            return False

        fields = type.fields()
        if len (fields) < 1:
            return False

        first_field = fields[0]
        return is_g_type_instance_helper(first_field.type)

    type = val.type
    if type.code != gdb.TYPE_CODE_PTR:
        return False
    type = type.target()
    return is_g_type_instance_helper (type)

def g_type_name_from_instance (instance):
    if long(instance) != 0:
        try:
            inst = instance.cast (gdb.lookup_type("GTypeInstance").pointer())
            klass = inst["g_class"]
            gtype = klass["g_type"]
            name = g_type_to_name (gtype)
            return name
        except RuntimeError:
            pass
    return None

class GTypePrettyPrinter:
    "Prints a GType instance pointer"

    def __init__ (self, val):
        self.val = val

    def to_string (self):
        name = g_type_name_from_instance (self.val)
        if name:
            return ("0x%x [%s]")% (long(self.val), name)
        return  ("0x%x") % (long(self.val))

def pretty_printer_lookup (val):
    if is_g_type_instance (val):
        return GTypePrettyPrinter (val)

    return None

def get_signal_name (id):
  if id == None:
    return None
  id = long(id)
  if id == 0:
    return None
  val = read_global_var ("g_signal_nodes")
  max_s = read_global_var ("g_n_signal_nodes")
  max_s = long(max_s)
  if id < max_s:
    return val[id]["name"].string()
  return None

class GFrameWrapper:
    def __init__ (self, frame):
        self.frame = frame;

    def name (self):
        name = self.frame.name()
        if name and name.startswith("IA__"):
            return name[4:]
        return name

    def __getattr__ (self, name):
        return getattr (self.frame, name)

# Monkey patch FrameWrapper to avoid IA__ in symbol names
old__init__ = gdb.command.backtrace.FrameWrapper.__init__
def monkey_patched_init(self, frame):
    name = frame.name()
    if name and name.startswith("IA__"):
        frame = GFrameWrapper(frame)
    old__init__(self,frame)
gdb.command.backtrace.FrameWrapper.__init__ = monkey_patched_init

class DummyFrame:
    def __init__ (self, frame):
        self.frame = frame

    def name (self):
        return "signal-emission-dummy"

    def describe (self, stream, full):
        stream.write (" <...>\n")

    def __getattr__ (self, name):
        return getattr (self.frame, name)

class SignalFrame:
    def __init__ (self, frames):
        self.frame = frames[-1]
        self.frames = frames;

    def name (self):
        return "signal-emission"

    def read_var (self, frame, name, array = None):
        try:
            v = frame.read_var (name)
            if v == None or v.is_optimized_out:
                return None
            if array != None:
                array.append (v)
            return v
        except ValueError:
            return None

    def read_object (self, frame, name, array = None):
        try:
            v = frame.read_var (name)
            if v == None or v.is_optimized_out:
                return None
            v = v.cast (gdb.lookup_type("GObject").pointer())
            # Ensure this is a somewhat correct object pointer
            if v != None and g_type_name_from_instance (v):
                if array != None:
                    array.append (v)
                return v
            return None
        except ValueError:
            return None

    def append (self, array, obj):
        if obj != None:
            array.append (obj)

    def or_join_array (self, array):
        if len(array) == 0:
            return "???"

        v = {}
        for i in range(len(array)):
            v[str(array[i])] = 1
        array = v.keys()
        s = array[0]
        for i in range(1, len(array)):
            s = s + " or %s"%array[i]

        return s

    def describe (self, stream, full):
        instances = []
        signals = []

        for frame in self.frames:
            name = frame.name()
            if name == "signal_emit_unlocked_R":
                self.read_object (frame, "instance", instances)
                node = self.read_var (frame, "node")
                if node:
                    signal = node["name"].string()
                    detail = self.read_var (frame, "detail")
                    detail = glib.g_quark_to_string (detail)
                    if detail != None:
                        signal = signal + ":" + detail
                    self.append (signals, signal)

            if name == "g_signal_emitv":
                instance_and_params = self.read_var (frame, "instance_and_params")
                if instance_and_params:
                    instance = instance_and_params[0]["v_pointer"].cast (gdb.Type("GObject").pointer())
                    self.append (instances, instance)
                id = self.read_var (frame, "signal_id")
                signal = get_signal_name (id)
                if signal:
                    detail = self.read_var (frame, "detail")
                    detail = glib.g_quark_to_string (detail)
                    if detail != None:
                        signal = signal + ":" + detail
                    self.append (signals, signal)

            if name == "g_signal_emit_valist" or name == "g_signal_emit":
                self.read_object (frame, "instance", instances)
                id = self.read_var (frame, "signal_id")
                signal = get_signal_name (id)
                if signal:
                    detail = self.read_var (frame, "detail")
                    detail = glib.g_quark_to_string (detail)
                    if detail != None:
                        signal = signal + ":" + detail
                    self.append (signals, signal)

            if name == "g_signal_emit_by_name":
                self.read_object (frame, "instance", instances)
                self.read_var (frame, "detailed_signal", signals)
                break

        instance = self.or_join_array (instances)
        signal = self.or_join_array (signals)

        stream.write (" <emit signal %s on instance %s>\n" %  (signal, instance))

    def __getattr__ (self, name):
        return getattr (self.frame, name)

class GFrameFilter:
    def __init__ (self, iter):
        self.queue = []
        self.iter = iter

    def __iter__ (self):
        return self

    def fill (self):
        while len(self.queue) <= 6:
            try:
                f = self.iter.next ()
                self.queue.append (f)
            except StopIteration:
                return

    def find_signal_emission (self):
        for i in range (min (len(self.queue), 3)):
            if self.queue[i].name() == "signal_emit_unlocked_R":
                return i
        return -1

    def next (self):
        # Ensure we have enough frames for a full signal emission
        self.fill()

        # Are we at the end?
        if len(self.queue) == 0:
            raise StopIteration

        emission = self.find_signal_emission ()
        if emission > 0:
            start = emission
            while True:
                if start == 0:
                    break
                prev_name = self.queue[start-1].name()
                if prev_name.find("_marshal_") or prev_name == "g_closure_invoke":
                    start = start - 1
                else:
                    break
            end = emission + 1
            while end < len(self.queue):
                if self.queue[end].name() in ["g_signal_emitv",
                                              "g_signal_emit_valist",
                                              "g_signal_emit",
                                              "g_signal_emit_by_name"]:
                    end = end + 1
                else:
                    break

            signal_frames = self.queue[start:end]
            new_frames = []
            for i in range(len(signal_frames)-1):
                new_frames.append(DummyFrame(signal_frames[i]))
            new_frames.append(SignalFrame(signal_frames))

            self.queue[start:end] = new_frames

        return self.queue.pop(0)


def register (obj):
    if obj == None:
        obj = gdb

    gdb.backtrace.push_frame_filter (GFrameFilter)
    obj.pretty_printers.append(pretty_printer_lookup)
