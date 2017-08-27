/**
 * \file seq/seq.c
 * \brief Sequencer Interface
 * \author Jaroslav Kysela <perex@perex.cz>
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \author Takashi Iwai <tiwai@suse.de>
 * \date 2000-2001
 *
 * See \ref seq page for more details.
 */

/* 
 *  Sequencer Interface - main file
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

/*! \page seq Sequencer interface

\section seq_general General

The ALSA sequencer interface is designed to deliver the MIDI-like
events between clients/ports.
A typical usage is the MIDI patch-bay.  A MIDI application can be
connected arbitrarily from/to the other MIDI clients.
The routing between clients can be changed dynamically, so the
application can handle incoming or outgoing MIDI events regardless of
the devices or the application connections.

The sequencer core stuff only takes care of two things:
scheduling events and dispatching them to the destination at the
right time.  All processing of MIDI events has to be done within the clients.
The event can be dispatched immediately without queueing, too.
The event scheduling can be done either on a MIDI tempo queue or
on a wallclock-time queue.

\section seq_client Client and Port

A <i>client</i> is created at each time #snd_seq_open() is called.
Later on, the attributes of client such as its name string can be changed
via #snd_seq_set_client_info().  There are helper functions for ease of use,
e.g. #snd_seq_set_client_name() and #snd_seq_set_client_event_filter().
A typical code would be like below:
\code
// create a new client
snd_seq_t *open_client()
{
        snd_seq_t *handle;
        int err;
        err = snd_seq_open(&handle, "default", SND_SEQ_OPEN_INPUT, 0);
        if (err < 0)
                return NULL;
        snd_seq_set_client_name(handle, "My Client");
	return handle;
}
\endcode

You'll need to know the id number of the client eventually, for example,
when accessing to a certain port (see the section \ref seq_subs).
The client id can be obtained by #snd_seq_client_id() function.

A client can have one or more <i>ports</i> to communicate between other
clients.  A port is corresponding to the MIDI port in the case of MIDI device,
but in general it is nothing but the access point between other clients.
Each port may have capability flags, which specify the read/write
accessibility and subscription permissions of the port.
For creation of a port, call #snd_seq_create_port()
with the appropriate port attribute specified in #snd_seq_port_info_t
record.

For creating a port for the normal use, there is a helper function
#snd_seq_create_simple_port().  An example with this function is like below.
\code
// create a new port; return the port id
// port will be writable and accept the write-subscription.
int my_new_port(snd_seq_t *handle)
{
	return snd_seq_create_simple_port(handle, "my port",
			SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
			SND_SEQ_PORT_TYPE_MIDI_GENERIC);
}
\endcode

\section seq_memory Memory Pool

Each client owns memory pools on kernel space
for each input and output events.
Here, input and output mean
input (read) from other clients and output (write) to others, respectively.
Since memory pool of each client is independent from others,
it avoids such a situation that a client eats the whole events pool
and interfere other clients' response.

The all scheduled output events or input events from dispatcher are stored
on these pools until delivered to other clients or extracted to user space.
The size of input/output pools can be changed independently.
The output pool has also a room size, which is used to wake up the
thread when it falls into sleep in blocking write mode.

Note that ports on the same client share the same memory pool.
If a port fills the memory pool, another can't use it any more.
For avoiding this, multiple clients can be used.

For chancing the pool size and the condition, access to #snd_seq_client_pool_t
record.  There are helper functions, #snd_seq_set_client_pool_output(),
#snd_seq_set_client_pool_output_room() and #snd_seq_set_client_pool_input(),
for setting the total output-pool size, the output-room size and the input-pool
size, respectively.

\section seq_subs Subscription

One of the new features in ALSA sequencer system is <i>subscription</i> of ports.
In general, subscription is a connection between two sequencer ports.
Even though an event can be delivered to a port without subscription
using an explicit destination address,
the subscription mechanism provides us more abstraction.

Suppose a MIDI input device which sends events from a keyboard.
The port associated with this device has READ capability - which means
this port is readable from other ports.
If a user program wants to capture events from keyboard and store them
as MIDI stream, this program must subscribe itself to the MIDI port
for read.
Then, a connection from MIDI input port to this program is established.
From this time, events from keyboard are automatically sent to this program.
Timestamps will be updated according to the subscribed queue.
\code
        MIDI input port (keyboard)
            |
            V
        ALSA sequencer - update timestamp
            |
            V
        application port
\endcode

There is another subscription type for opposite direction:
Suppose a MIDI sequencer program which sends events to a MIDI output device.
In ALSA system, MIDI device is not opened until the associated MIDI port
is accessed.  Thus, in order to activate MIDI device, we have to subscribe
to MIDI port for write.
After this connection is established, events will be properly sent
to MIDI output device.
\code
        application port
            |
            V
        ALSA sequencer - events are scheduled
            |
            V
        MIDI output port (WaveTable etc.)
\endcode

From the viewpoint of subscription, the examples above are special cases.
Basically, subscription means the connection between two arbitrary ports.
For example, imagine a filter application which modifies
the MIDI events like program, velocity or chorus effects.
This application can accept arbitrary MIDI input
and send to arbitrary port, just like a Unix pipe application using
stdin and stdout files.
We can even connect several filter applications which work individually
in order to process the MIDI events.
Subscription can be used for this purpose.
The connection between ports can be done also by the "third" client.
Thus, filter applications have to manage
only input and output events regardless of receiver/sender addresses.
\code
        sequencer port #1
            |
            V
        ALSA sequencer (scheduled or real-time)
            |
            V
        sequencer port #2
\endcode

For the detail about subscription, see the section \ref seq_subs_more.

\section seq_events Sequencer Events

Messaging between clients is performed by sending events from one client to
another. These events contain high-level MIDI oriented messages or sequencer
specific messages.

All the sequencer events are stored in a sequencer event record,
#snd_seq_event_t type.
Application can send and receive these event records to/from other
clients via sequencer.
An event has several storage types according to its usage.
For example, a SYSEX message is stored on the variable length event,
and a large synth sample data is delivered using a user-space data pointer.


\subsection seq_ev_struct Structure of an event

An event consists of the following items:
<ul>
<li>The type of the event
<li>Event flags.  It describes various conditions:
  <ul>
  <li>time stamp; "real time" / "song ticks"
  <li>time mode; "absolute" / "relative to current time"
  </ul>
<li>Timestamp of the event.
<li>Scheduling queue id.
<li>Source address of the event, given by the combination
  of client id and port id numbers.
<li>Destination address of the event.
<li>The actual event data. (up to 12 bytes)
</ul>

The actual record is shown in #snd_seq_event_t.
The type field contains the type of the event
(1 byte).
The flags field consists of bit flags which
describe several conditions of the event (1 byte).
It includes the time-stamp mode, data storage type, and scheduling priority.
The tag field is an arbitrary tag.
This tag can used for removing a distinct event from the event queue
via #snd_seq_remove_events().
The queue field is the queue id for scheduling.
The source and dest fields are source and destination addresses.
The data field is a union of event data.

\subsection seq_ev_queue Scheduling queue

An event can be delivered either on scheduled or direct dispatch mode.
On the scheduling mode, an event is once stored on the priority queue
and delivered later (or even immediately) to the destination,
whereas on the direct dispatch mode, an event is passed to the destination
without any queue.

For a scheduled delivery, a queue to process the event must exist.
Usually, a client creates its own queue by
#snd_seq_alloc_queue() function.
Alternatively, a queue may be shared among several clients.
For scheduling an event on the specified queue,
a client needs to fill queue field
with the preferred queue id.

Meanwhile, for dispatching an event directly, just
use #SND_SEQ_QUEUE_DIRECT as the target queue id.
A macro #snd_seq_ev_set_direct() is provided for ease
and compatibility.

Note that scheduling at the current or earlier time is different
from the direct dispatch mode even though the event is delivered immediately.
On the former scheme, an event is once stored on priority queue, then
delivered actually.  Thus, it acquires a space from memory pool.
On the other hand, the latter is passed without using memory pool.
Although the direct dispatched event needs less memory, it means also
that the event cannot be resent if the destination is unable to receive it
momentarily.

\subsection seq_ev_time Time stamp

The timestamp of the event can either specified in
<i>real time</i> or in <i>song ticks</i>.
The former means the wallclock time while the latter corresponds to
the MIDI ticks.
Which format is used is determined by the event flags.

The resolution of real-time value is in nano second.
Since 64 bit length is required for the actual time calculation,
it is represented by
a structure of pair of second and nano second
defined as #snd_seq_real_time_t type.
The song tick is defined simply as a 32 bit integer,
defined as #snd_seq_tick_time_t type.
The time stored in an event record is a union of these two different
time values.

Note that the time format used for real time events is very similar to
timeval struct used for Unix system time.
The absurd resolution of the timestamps allows us to perform very accurate
conversions between songposition and real time. Round-off errors can be
neglected.

If a timestamp with a
<i>relative</i> timestamp is delivered to ALSA, the
specified timestamp will be used as an offset to the current time of the
queue the event is sent into.
An <i>absolute</i> timestamp is on the contrary the time
counted from the moment when the queue started.

An client that relies on these relative timestamps is the MIDI input port.
As each sequencer queue has it's own clock the only way to deliver events at
the right time is by using the relative timestamp format. When the event
arrives at the queue it is normalized to absolute format.

The timestamp format is specified in the flag bitfield masked by
#SND_SEQ_TIME_STAMP_MASK.
To schedule the event in a real-time queue or in a tick queue,
macros #snd_seq_ev_schedule_real() and
#snd_seq_ev_schedule_tick() are provided, respectively.

\subsection seq_ev_addr Source and destination addresses

To identify the source and destination of an event, the addressing field
contains a combination of client id and port id numbers, defined as
#snd_seq_addr_t type.
When an event is passed to sequencer from a client, sequencer fills
source.client field
with the sender's id automatically.
It is the responsibility of sender client to 
fill the port id of source.port and
both client and port of dest field.

If an existing address is set to the destination,
the event is simply delivered to it.
When #SND_SEQ_ADDRESS_SUBSCRIBERS is set to the destination client id,
the event is delivered to all the clients connected to the source port.


A sequencer core has two pre-defined system ports on the system client
#SND_SEQ_CLIENT_SYSTEM: #SND_SEQ_PORT_SYSTEM_TIMER and #SND_SEQ_PORT_SYSTEM_ANNOUNCE.
The #SND_SEQ_PORT_SYSTEM_TIMER is the system timer port,
and #SND_SEQ_PORT_SYSTEM_ANNOUNCE is the system
announce port.
In order to control a queue from a client, client should send a
queue-control event
like start, stop and continue queue, change tempo, etc.
to the system timer port.
Then the sequencer system handles the queue according to the received event.
This port supports subscription. The received timer events are 
broadcasted to all subscribed clients.

The latter port does not receive messages but supports subscription.
When each client or port is attached, detached or modified,
an announcement is sent to subscribers from this port.

\subsection seq_ev_data Data storage type

Some events like SYSEX message, however, need larger data space
than the standard data.
For such events, ALSA sequencer provides several different data storage types.
The data type is specified in the flag bits masked by #SND_SEQ_EVENT_LENGTH_MASK.
The following data types are available:

\par Fixed size data
Normal events stores their parameters on
data field (12 byte).
The flag-bit type is  #SND_SEQ_EVENT_LENGTH_FIXED.
A macro #snd_seq_ev_set_fixed() is provided to set this type.

\par Variable length data
SYSEX or a returned error use this type.
The actual data is stored on an extra allocated space.
On sequencer kernel, the whole extra-data is duplicated, so that the event
can be scheduled on queue.
The data contains only the length and the
pointer of extra-data.
The flag-bit type is  #SND_SEQ_EVENT_LENGTH_VARIABLE.
A macro #snd_seq_ev_set_variable() is provided to set this type.

\par User-space data
This type refers also an extra data space like variable length data,
but the extra-data is not duplicated but
but referred as a user-space data on kernel,
so that it reduces the time and resource for transferring
large bulk of data like synth sample wave.
This data type, however, can be used only for direct dispatch mode,
and supposed to be used only for a special purpose like a bulk data
transfer.
The data length and pointer are stored also in
data.ext field as well as variable length data.
The flag-bit type is  #SND_SEQ_EVENT_LENGTH_VARUSR.
A macro #snd_seq_ev_set_varusr() is provided to set this type.

\subsection seq_ev_sched Scheduling priority

There are two priorities for scheduling:
\par Normal priority
If an event with the same scheduling time is already present on the queue,
the new event is appended to the older.
\par High priority
If an event with the same scheduling time is already present on the queue,
the new event is inserted before others.

The scheduling priority is set in the flag bitfeld masked by #SND_SEQ_PRIORITY_MASK.
A macro #snd_seq_ev_set_priority() is provided to set the mode type.

\section seq_queue Event Queues
\subsection seq_ev_control Creation of a queue

Creating a queue is done usually by calling #snd_seq_alloc_queue.
You can create a queue with a certain name by #snd_seq_alloc_named_queue(), too.
\code
// create a queue and return its id
int my_queue(snd_seq_t *handle)
{
	return snd_seq_alloc_named_queue(handle, "my queue");
}
\endcode
These functions are the wrapper to the function #snd_seq_create_queue().
For releasing the allocated queue, call #snd_seq_free_queue() with the
obtained queue id.

Once when a queue is created, the two queues are associated to that
queue record in fact: one is the realtime queue and another is the
tick queue.  These two queues are bound together to work
synchronously.  Hence, when you schedule an event, you have to choose
which queue type is used as described in the section \ref
seq_ev_time.

\subsection seq_ev_tempo Setting queue tempo

The tempo (or the speed) of the scheduling queue is variable.
In the case of <i>tick</i> queue, the tempo is controlled
in the manner of MIDI.  There are two parameters to define the
actual tempo, PPQ (pulse per quarter note) and MIDI tempo.
The former defines the base resolution of the ticks, while
the latter defines the beat tempo in microseconds.
As default, 96 PPQ and 120 BPM are used, respectively.
That is, the tempo is set to 500000 (= 60 * 1000000 / 120).
Note that PPQ cannot be changed while the queue is running.
It must be set before the queue is started.

On the other hand, in the case of <i>realtime</i> queue, the
time resolution is fixed to nanoseconds.  There is, however,
a parameter to change the speed of this queue, called <i>skew</i>.
You can make the queue faster or slower by setting the skew value
bigger or smaller.  In the API, the skew is defined by two values,
the skew base and the skew value.  The actual skew is the fraction
of them, <i>value/base</i>.  As default, the skew base is set to 16bit
(0x10000) and the skew value is the identical, so that the queue is
processed as well as in the real world.

When the tempo of realtime queue is changed, the tempo of
the associated tick queue is changed together, too.
That's the reason why two queues are created always.
This feature can be used to synchronize the event queue with
the external synchronization source like SMPTE.  In such a case,
the realtime queue is skewed to match with the external source,
so that both the realtime timestamp and the MIDI timestamp are
synchronized.

For setting these tempo parameters, use #snd_seq_queue_tempo_t record.
For example, to set the tempo of the queue <code>q</code> to
48 PPQ, 60 BPM,
\code
void set_tempo(snd_seq_t *handle)
{
        snd_seq_queue_tempo_t *tempo;
        snd_seq_queue_tempo_alloca(&tempo);
        snd_seq_queue_tempo_set_tempo(tempo, 1000000); // 60 BPM
        snd_seq_queue_tempo_set_ppq(tempo, 48); // 48 PPQ
        snd_seq_set_queue_tempo(handle, tempo);
}
\endcode

For changing the (running) queue's tempo on the fly, you can either
set the tempo via #snd_seq_set_queue_tempo() or send a MIDI tempo event
to the system timer port.  For example,
\code
int change_tempo(snd_seq_t *handle, int q, unsigned int tempo)
{
	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);
	ev.dest.client = SND_SEQ_CLIENT_SYSTEM;
	ev.dest.port = SND_SEQ_PORT_SYSTEM_TIMER;
	ev.source.client = my_client_id;
	ev.source.port = my_port_id;
	ev.queue = SND_SEQ_QUEUE_DIRECT; // no scheduling
	ev.data.queue.queue = q;	// affected queue id
	ev.data.queue.value = tempo;	// new tempo in microsec.
	return snd_seq_event_output(handle, &ev);
}
\endcode
There is a helper function to do this easily,
#snd_seq_change_queue_tempo().
Set NULL to the last argument, if you don't need any
special settings.

In the above example, the tempo is changed immediately after
the buffer is flushed by #snd_seq_drain_output() call.
You can schedule the event in a certain queue so that the tempo
change happens at the scheduled time, too.

\subsection seq_ev_start Starting and stopping a queue

To start, stop, or continue a queue, you need to send a queue-control
event to the system timer port as well.  There are helper functions,
#snd_seq_start_queue(), #snd_seq_stop_queue() and
#snd_seq_continue_queue().
Note that if the last argument of these functions is NULL, the
event is sent (i.e. operated) immediately after the buffer flush.
If you want to schedule the event at the certain time, set up
the event record and provide the pointer of that event record as the
argument.

Only calling these functions doesn't deliver the event to the
sequencer core but only put to the output buffer.  You'll need to
call #snd_seq_drain_output() eventually.


\section seq_subs_more More inside the subscription

\subsection seq_subs_perm Permissions

Each ALSA port can have capability flags.
The most basic capability flags are
#SND_SEQ_PORT_CAP_READ and #SND_SEQ_PORT_CAP_WRITE.
The former means that the port allows to send events to other ports,
whereas the latter capability means
that the port allows to receive events from other ports.
You may have noticed that meanings of \c READ and \c WRITE
are permissions of the port from the viewpoint of other ports.

For allowing subscription from/to other clients, another capability
flags must be set together with read/write capabilities above.
For allowing read and write subscriptions,
#SND_SEQ_PORT_CAP_SUBS_READ and
#SND_SEQ_PORT_CAP_SUBS_WRITE are used,
respectively.
For example, the port with MIDI input device always has
#SND_SEQ_PORT_CAP_SUBS_READ capability,
and the port with MIDI output device always has
#SND_SEQ_PORT_CAP_SUBS_WRITE capability together with
#SND_SEQ_PORT_CAP_READ and #SND_SEQ_PORT_CAP_WRITE capabilities,
respectively.
Obviously, these flags have no influence
if \c READ or \c WRITE> capability is not set.

Note that these flags are not necessary if the client subscribes itself
to the specified port.
For example, when a port makes READ subscription
to MIDI input port, this port must have #SND_SEQ_PORT_CAP_WRITE capability,
but no #SND_SEQ_PORT_CAP_SUBS_WRITE capability is required.
Only MIDI input port must have #SND_SEQ_PORT_CAP_SUBS_READ capability.

As default, the connection of ports via the third client is always allowed
if proper read and write (subscription) capabilities are set both to the
source and destination ports.
For prohibiting this behavior, set a capability
#SND_SEQ_PORT_CAP_NO_EXPORT to the port.
If this flag is set, subscription must be done by sender or receiver
client itself.
It is useful to avoid unexpected disconnection.
The ports which won't accept subscription should have this capability
for better security.

\subsection seq_subs_handle Subscription handlers

In ALSA library, subscription is done via
#snd_seq_subscribe_port() function.
It takes the argument of #snd_seq_port_subscribe_t record pointer.
Suppose that you have a client which will receive data from
a MIDI input device.  The source and destination addresses
are like the below;
\code
snd_seq_addr_t sender, dest;
sender.client = MIDI_input_client;
sender.port = MIDI_input_port;
dest.client = my_client;
dest.port = my_port;
\endcode
To set these values as the connection call like this.
\code
snd_seq_port_subscribe_t *subs;
snd_seq_port_subscribe_alloca(&subs);
snd_seq_port_subscribe_set_sender(subs, &sender);
snd_seq_port_subscribe_set_dest(subs, &dest);
snd_seq_subscribe_port(handle, subs);
\endcode

When the connection should be exclusively done only between
a certain pair, set <i>exclusive</i> attribute to the subscription
record before calling #snd_seq_subscribe_port.
\code
snd_seq_port_subscribe_set_exclusive(subs, 1);
\endcode
The succeeding subscriptions will be refused.

The timestamp can be updated independently on each connection.
When set up, the timestamp of incoming queue to the destination port
is updated automatically to the time of the specified queue.
\code
snd_seq_port_subscribe_set_time_update(subs, 1);
snd_seq_port_subscribe_set_queue(subs, q);
\endcode
For getting the wallclock time (sec/nsec pair), set <i>real</i> attribute:
\code
snd_seq_port_subscribe_set_time_real(subs, 1);
\endcode
Otherwise, the timestamp is stored in tick unit.
This feature is useful when receiving events from MIDI input device.
The event time is automatically set in the event record.

Note that an outsider client may connect other ports.
In this case, however, the subscription may be refused
if #SND_SEQ_PORT_CAP_NO_EXPORT capability is set in either sender or receiver port.

\section seq_subs_ex Examples of subscription

\subsection seq_subs_ex_capt Capture from keyboard

Assume MIDI input port = 64:0, application port = 128:0, and
queue for timestamp = 1 with real-time stamp.
The application port must have capability #SND_SEQ_PORT_CAP_WRITE.
\code
void capture_keyboard(snd_seq_t *seq)
{
        snd_seq_addr_t sender, dest;
        snd_seq_port_subscribe_t *subs;
        sender.client = 64;
        sender.port = 0;
        dest.client = 128;
        dest.port = 0;
        snd_seq_port_subscribe_alloca(&subs);
        snd_seq_port_subscribe_set_sender(subs, &sender);
        snd_seq_port_subscribe_set_dest(subs, &dest);
        snd_seq_port_subscribe_set_queue(subs, 1);
        snd_seq_port_subscribe_set_time_update(subs, 1);
        snd_seq_port_subscribe_set_time_real(subs, 1);
        snd_seq_subscribe_port(seq, subs);
}
\endcode

\subsection seq_subs_ex_out Output to MIDI device

Assume MIDI output port = 65:1 and application port = 128:0.
The application port must have capability #SND_SEQ_PORT_CAP_READ.
\code
void subscribe_output(snd_seq_t *seq)
{
        snd_seq_addr_t sender, dest;
        snd_seq_port_subscribe_t *subs;
        sender.client = 128;
        sender.port = 0;
        dest.client = 65;
        dest.port = 1;
        snd_seq_port_subscribe_alloca(&subs);
        snd_seq_port_subscribe_set_sender(subs, &sender);
        snd_seq_port_subscribe_set_dest(subs, &dest);
        snd_seq_subscribe_port(seq, subs);
}
\endcode
This example can be simplified by using #snd_seq_connect_to() function.
\code
void subscribe_output(snd_seq_t *seq)
{
        snd_seq_connect_to(seq, 0, 65, 1);
}
\endcode

\subsection seq_subs_ex_arbit Arbitrary connection

Assume connection from application 128:0 to 129:0,
and that subscription is done by the third application (130:0).
The sender must have capabilities both
#SND_SEQ_PORT_CAP_READ and
#SND_SEQ_PORT_CAP_SUBS_READ,
and the receiver
#SND_SEQ_PORT_CAP_WRITE and
#SND_SEQ_PORT_CAP_SUBS_WRITE, respectively.
\code
// ..in the third application (130:0) ..
void coupling(snd_seq_t *seq)
{
        snd_seq_addr_t sender, dest;
        snd_seq_port_subscribe_t *subs;
        sender.client = 128;
        sender.port = 0;
        dest.client = 129;
        dest.port = 0;
        snd_seq_port_subscribe_alloca(&subs);
        snd_seq_port_subscribe_set_sender(subs, &sender);
        snd_seq_port_subscribe_set_dest(subs, &dest);
        snd_seq_subscribe_port(seq, subs);
}
\endcode

\section seq_ex_event Event Processing

\subsection seq_ex_address Addressing

Now, two ports are connected by subscription.  Then how to send events?

The subscribed port doesn't have to know the exact sender address.
Instead, there is a special address for subscribers,
#SND_SEQ_ADDRESS_SUBSCRIBERS.
The sender must set this value as the destination client.
Destination port is ignored.

The other values in source and destination addresses are identical with
the normal event record.
If the event is scheduled, proper queue and timestamp values must be set.

There is a convenient function to set the address in an event record.
In order to set destination as subscribers, use
#snd_seq_ev_set_subs().

\subsection Scheduled Delivery

If we send an event at the scheduled time <code>t</code> (tick)
on the queue <code>Q</code>,
the sender must set both schedule queue and time in the
event record.
The program appears like this:
\code
void schedule_event(snd_seq_t *seq)
{
        snd_seq_event_t ev;

        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_source(&ev, my_port);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_schedule_tick(&ev, Q, 0, t);
        ... // set event type, data, so on..

        snd_seq_event_output(seq, &ev);
        ...
        snd_seq_drain_output(seq);  // if necessary
}
\endcode
Of course, you can use realtime stamp, too.

\subsection seq_ex_direct Direct Delivery

If the event is sent immediately without enqueued, the sender doesn't take
care of queue and timestamp.
As well as the case above, there is a function to set the direct delivery,
#snd_seq_ev_set_direct().
The program can be more simplified as follows:
\code
void direct_delivery(snd_seq_t *seq)
{
        snd_seq_event_t ev;

        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_source(&ev, port);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);
        ... // set event type, data, so on..

        snd_seq_event_output(seq, &ev);
        snd_seq_drain_output(seq);
}
\endcode
You should flush event soon after output event.
Otherwise, the event is enqueued on output queue of ALSA library
(not in the kernel!), and will be never processed until
this queue becomes full.

\subsection seq_ex_filter Filter Application

A typical filter program, which receives an event and sends it immediately
after some modification, will appear as following:
\code
void event_filter(snd_seq_t *seq, snd_seq_event_t *ev)
{
        while (snd_seq_event_input(seq, &ev) >= 0) {
                //.. modify input event ..

                snd_seq_ev_set_source(ev, my_port);
                snd_seq_ev_set_subs(ev);
                snd_seq_ev_set_direct(ev);
                snd_seq_event_output(seq, ev);
                snd_seq_drain_output(seq);
        }
}
\endcode

*/

#include <sys/poll.h>
#include "seq_local.h"

/****************************************************************************
 *                                                                          *
 *                                seq.h                                     *
 *                              Sequencer                                   *
 *                                                                          *
 ****************************************************************************/

/**
 * \brief get identifier of sequencer handle
 * \param seq sequencer handle
 * \return ASCII identifier of sequencer handle
 *
 * Returns the ASCII identifier of the given sequencer handle. It's the same
 * identifier specified in snd_seq_open().
 *
 * \sa snd_seq_open()
 */
const char *snd_seq_name(snd_seq_t *seq)
{
	assert(seq);
	return seq->name;
}

/**
 * \brief get type of sequencer handle
 * \param seq sequencer handle
 * \return type of sequencer handle
 *
 * Returns the type #snd_seq_type_t of the given sequencer handle.
 *
 * \sa snd_seq_open()
 */
snd_seq_type_t snd_seq_type(snd_seq_t *seq)
{
	assert(seq);
	return seq->type;
}

static int snd_seq_open_conf(snd_seq_t **seqp, const char *name,
			     snd_config_t *seq_root, snd_config_t *seq_conf,
			     int streams, int mode)
{
	const char *str;
	char buf[256];
	int err;
	snd_config_t *conf, *type_conf = NULL;
	snd_config_iterator_t i, next;
	const char *id;
	const char *lib = NULL, *open_name = NULL;
	int (*open_func)(snd_seq_t **, const char *,
			 snd_config_t *, snd_config_t *, 
			 int, int) = NULL;
#ifndef PIC
	extern void *snd_seq_open_symbols(void);
#endif
	void *h = NULL;
	if (snd_config_get_type(seq_conf) != SND_CONFIG_TYPE_COMPOUND) {
		if (name)
			SNDERR("Invalid type for SEQ %s definition", name);
		else
			SNDERR("Invalid type for SEQ definition");
		return -EINVAL;
	}
	err = snd_config_search(seq_conf, "type", &conf);
	if (err < 0) {
		SNDERR("type is not defined");
		return err;
	}
	err = snd_config_get_id(conf, &id);
	if (err < 0) {
		SNDERR("unable to get id");
		return err;
	}
	err = snd_config_get_string(conf, &str);
	if (err < 0) {
		SNDERR("Invalid type for %s", id);
		return err;
	}
	err = snd_config_search_definition(seq_root, "seq_type", str, &type_conf);
	if (err >= 0) {
		if (snd_config_get_type(type_conf) != SND_CONFIG_TYPE_COMPOUND) {
			SNDERR("Invalid type for SEQ type %s definition", str);
			goto _err;
		}
		snd_config_for_each(i, next, type_conf) {
			snd_config_t *n = snd_config_iterator_entry(i);
			const char *id;
			if (snd_config_get_id(n, &id) < 0)
				continue;
			if (strcmp(id, "comment") == 0)
				continue;
			if (strcmp(id, "lib") == 0) {
				err = snd_config_get_string(n, &lib);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					goto _err;
				}
				continue;
			}
			if (strcmp(id, "open") == 0) {
				err = snd_config_get_string(n, &open_name);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					goto _err;
				}
				continue;
			}
			SNDERR("Unknown field %s", id);
			err = -EINVAL;
			goto _err;
		}
	}
	if (!open_name) {
		open_name = buf;
		snprintf(buf, sizeof(buf), "_snd_seq_%s_open", str);
	}
#ifndef PIC
	snd_seq_open_symbols();
#endif
	h = snd_dlopen(lib, RTLD_NOW);
	if (h)
		open_func = snd_dlsym(h, open_name, SND_DLSYM_VERSION(SND_SEQ_DLSYM_VERSION));
	err = 0;
	if (!h) {
		SNDERR("Cannot open shared library %s", lib);
		err = -ENOENT;
	} else if (!open_func) {
		SNDERR("symbol %s is not defined inside %s", open_name, lib);
		snd_dlclose(h);
		err = -ENXIO;
	}
       _err:
	if (type_conf)
		snd_config_delete(type_conf);
	if (! err) {
		err = open_func(seqp, name, seq_root, seq_conf, streams, mode);
		if (err < 0)
			snd_dlclose(h);
		else
			(*seqp)->dl_handle = h;
	}
	return err;
}

static int snd_seq_open_noupdate(snd_seq_t **seqp, snd_config_t *root,
				 const char *name, int streams, int mode,
				 int hop)
{
	int err;
	snd_config_t *seq_conf;
	err = snd_config_search_definition(root, "seq", name, &seq_conf);
	if (err < 0) {
		SNDERR("Unknown SEQ %s", name);
		return err;
	}
	snd_config_set_hop(seq_conf, hop);
	err = snd_seq_open_conf(seqp, name, root, seq_conf, streams, mode);
	snd_config_delete(seq_conf);
	return err;
}


/**
 * \brief Open the ALSA sequencer
 *
 * \param seqp Pointer to a snd_seq_t pointer.  This pointer must be
 * kept and passed to most of the other sequencer functions.
 * \param name The sequencer's "name".  This is \em not a name you make
 * up for your own purposes; it has special significance to the ALSA
 * library.  Usually you need to pass \c "default" here.
 * \param streams The read/write mode of the sequencer.  Can be one of
 * three values:
 * - #SND_SEQ_OPEN_OUTPUT - open the sequencer for output only
 * - #SND_SEQ_OPEN_INPUT - open the sequencer for input only
 * - #SND_SEQ_OPEN_DUPLEX - open the sequencer for output and input
 * \note Internally, these are translated to \c O_WRONLY, \c O_RDONLY and
 * \c O_RDWR respectively and used as the second argument to the C library
 * open() call.
 * \param mode Optional modifier.  Can be either 0, or
 * #SND_SEQ_NONBLOCK, which will make read/write operations
 * non-blocking.  This can also be set later using #snd_seq_nonblock().
 * \return 0 on success otherwise a negative error code
 *
 * Creates a new handle and opens a connection to the kernel
 * sequencer interface.
 * After a client is created successfully, an event
 * with #SND_SEQ_EVENT_CLIENT_START is broadcast to announce port.
 *
 * \sa snd_seq_open_lconf(), snd_seq_close(), snd_seq_type(), snd_seq_name(),
 *     snd_seq_nonblock(), snd_seq_client_id()
 */
int snd_seq_open(snd_seq_t **seqp, const char *name, 
		 int streams, int mode)
{
	int err;
	assert(seqp && name);
	err = snd_config_update();
	if (err < 0)
		return err;
	return snd_seq_open_noupdate(seqp, snd_config, name, streams, mode, 0);
}

/**
 * \brief Open the ALSA sequencer using local configuration
 *
 * \param seqp Pointer to a snd_seq_t pointer.
 * \param name The name to open
 * \param streams The read/write mode of the sequencer.
 * \param mode Optional modifier
 * \param lconf Local configuration
 * \return 0 on success otherwise a negative error code
 *
 * See the snd_seq_open() function for further details. The extension
 * is that the given configuration is used to resolve abstract name.
 *
 * \sa snd_seq_open()
 */
int snd_seq_open_lconf(snd_seq_t **seqp, const char *name, 
		       int streams, int mode, snd_config_t *lconf)
{
	assert(seqp && name && lconf);
	return snd_seq_open_noupdate(seqp, lconf, name, streams, mode, 0);
}

#ifndef DOC_HIDDEN
int _snd_seq_open_lconf(snd_seq_t **seqp, const char *name, 
			int streams, int mode, snd_config_t *lconf,
			snd_config_t *parent_conf)
{
	int hop;
	assert(seqp && name && lconf);
	if ((hop = snd_config_check_hop(parent_conf)) < 0)
		return hop;
	return snd_seq_open_noupdate(seqp, lconf, name, streams, mode, hop + 1);
}
#endif

/**
 * \brief Close the sequencer
 * \param seq Handle returned from #snd_seq_open()
 * \return 0 on success otherwise a negative error code
 *
 * Closes the sequencer client and releases its resources.
 * After a client is closed, an event with
 * #SND_SEQ_EVENT_CLIENT_EXIT is broadcast to announce port.
 * The connection between other clients are disconnected.
 * Call this just before exiting your program.
 *
 * \sa snd_seq_close()
 */
int snd_seq_close(snd_seq_t *seq)
{
	int err;
	assert(seq);
	err = seq->ops->close(seq);
	if (seq->dl_handle)
		snd_dlclose(seq->dl_handle);
	free(seq->obuf);
	free(seq->ibuf);
	free(seq->tmpbuf);
	free(seq->name);
	free(seq);
	return err;
}

/**
 * \brief Returns the number of poll descriptors
 * \param seq sequencer handle
 * \param events the poll events to be checked (\c POLLIN and \c POLLOUT)
 * \return the number of poll descriptors.
 *
 * Get the number of poll descriptors.  The polling events to be checked
 * can be specified by the second argument.  When both input and output
 * are checked, pass \c POLLIN|POLLOUT
 *
 * \sa snd_seq_poll_descriptors()
 */
int snd_seq_poll_descriptors_count(snd_seq_t *seq, short events)
{
	int result = 0;
	assert(seq);
	if (events & POLLIN) {
		assert(seq->streams & SND_SEQ_OPEN_INPUT);
		result++;
	}
	if (events & POLLOUT) {
		assert(seq->streams & SND_SEQ_OPEN_OUTPUT);
		result++;
	}
	return result ? 1 : 0;
}

/**
 * \brief Get poll descriptors
 * \param seq sequencer handle
 * \param pfds array of poll descriptors
 * \param space space in the poll descriptor array
 * \param events polling events to be checked (\c POLLIN and \c POLLOUT)
 * \return count of filled descriptors
 *
 * Get poll descriptors assigned to the sequencer handle.
 * Since a sequencer handle can duplex streams, you need to set which direction(s)
 * is/are polled in \a events argument.  When \c POLLIN bit is specified,
 * the incoming events to the ports are checked.
 *
 * To check the returned poll-events, call #snd_seq_poll_descriptors_revents()
 * instead of reading the pollfd structs directly.
 *
 * \sa snd_seq_poll_descriptors_count(), snd_seq_poll_descriptors_revents()
 */
int snd_seq_poll_descriptors(snd_seq_t *seq, struct pollfd *pfds, unsigned int space, short events)
{
	short revents = 0;

	assert(seq);
	if ((events & POLLIN) && space >= 1) {
		assert(seq->streams & SND_SEQ_OPEN_INPUT);
		revents |= POLLIN|POLLERR|POLLNVAL;
	}
	if ((events & POLLOUT) && space >= 1) {
		assert(seq->streams & SND_SEQ_OPEN_OUTPUT);
		revents |= POLLOUT|POLLERR|POLLNVAL;
	}
	if (!revents)
		return 0;
	pfds->fd = seq->poll_fd;
	pfds->events = revents;
	return 1;
}

/**
 * \brief get returned events from poll descriptors
 * \param seq sequencer handle
 * \param pfds array of poll descriptors
 * \param nfds count of poll descriptors
 * \param revents returned events
 * \return zero if success, otherwise a negative error code
 *
 * \sa snd_seq_poll_descriptors()
 */
int snd_seq_poll_descriptors_revents(snd_seq_t *seq, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
        assert(seq && pfds && revents);
        if (nfds == 1) {
                *revents = pfds->revents;
                return 0;
        }
        return -EINVAL;
}

/**
 * \brief Set nonblock mode
 * \param seq sequencer handle
 * \param nonblock 0 = block, 1 = nonblock mode
 * \return 0 on success otherwise a negative error code
 *
 * Change the blocking mode of the given client.
 * In block mode, the client falls into sleep when it fills the
 * output memory pool with full events.  The client will be woken up
 * after a certain amount of free space becomes available.
 *
 * \sa snd_seq_open()
 */
int snd_seq_nonblock(snd_seq_t *seq, int nonblock)
{
	int err;
	assert(seq);
	err = seq->ops->nonblock(seq, nonblock);
	if (err < 0)
		return err;
	if (nonblock)
		seq->mode |= SND_SEQ_NONBLOCK;
	else
		seq->mode &= ~SND_SEQ_NONBLOCK;
	return 0;
}

/**
 * \brief Get the client id
 * \param seq sequencer handle
 * \return the client id
 *
 * Returns the id of the specified client.
 * If an error occurs, function returns the negative error code.
 * A client id is necessary to inquiry or to set the client information.
 * A user client is assigned from 128 to 191.
 *
 * \sa snd_seq_open()
 */
int snd_seq_client_id(snd_seq_t *seq)
{
	assert(seq);
	return seq->client;
}

/**
 * \brief Return the size of output buffer
 * \param seq sequencer handle
 * \return the size of output buffer in bytes
 *
 * Obtains the size of output buffer.
 * This buffer is used to store decoded byte-stream of output events
 * before transferring to sequencer.
 *
 * \sa snd_seq_set_output_buffer_size()
 */
size_t snd_seq_get_output_buffer_size(snd_seq_t *seq)
{
	assert(seq);
	if (!seq->obuf)
		return 0;
	return seq->obufsize;
}

/**
 * \brief Return the size of input buffer
 * \param seq sequencer handle
 * \return the size of input buffer in bytes
 *
 * Obtains the size of input buffer.
 * This buffer is used to read byte-stream of input events from sequencer.
 *
 * \sa snd_seq_set_input_buffer_size()
 */
size_t snd_seq_get_input_buffer_size(snd_seq_t *seq)
{
	assert(seq);
	if (!seq->ibuf)
		return 0;
	return seq->ibufsize * sizeof(snd_seq_event_t);
}

/**
 * \brief Change the size of output buffer
 * \param seq sequencer handle
 * \param size the size of output buffer to be changed in bytes
 * \return 0 on success otherwise a negative error code
 *
 * Changes the size of output buffer.
 *
 * \sa snd_seq_get_output_buffer_size()
 */
int snd_seq_set_output_buffer_size(snd_seq_t *seq, size_t size)
{
	assert(seq && seq->obuf);
	assert(size >= sizeof(snd_seq_event_t));
	snd_seq_drop_output(seq);
	if (size != seq->obufsize) {
		char *newbuf;
		newbuf = calloc(1, size);
		if (newbuf == NULL)
			return -ENOMEM;
		free(seq->obuf);
		seq->obuf = newbuf;
		seq->obufsize = size;
	}
	return 0;
}

/**
 * \brief Resize the input buffer
 * \param seq sequencer handle
 * \param size the size of input buffer to be changed in bytes
 * \return 0 on success otherwise a negative error code
 *
 * Changes the size of input buffer.
 *
 * \sa snd_seq_get_input_buffer_size()
 */
int snd_seq_set_input_buffer_size(snd_seq_t *seq, size_t size)
{
	assert(seq && seq->ibuf);
	assert(size >= sizeof(snd_seq_event_t));
	snd_seq_drop_input(seq);
	size = (size + sizeof(snd_seq_event_t) - 1) / sizeof(snd_seq_event_t);
	if (size != seq->ibufsize) {
		snd_seq_event_t *newbuf;
		newbuf = calloc(sizeof(snd_seq_event_t), size);
		if (newbuf == NULL)
			return -ENOMEM;
		free(seq->ibuf);
		seq->ibuf = newbuf;
		seq->ibufsize = size;
	}
	return 0;
}


/**
 * \brief Get size of #snd_seq_system_info_t
 * \return size in bytes
 */
size_t snd_seq_system_info_sizeof()
{
	return sizeof(snd_seq_system_info_t);
}

/**
 * \brief Allocate an empty #snd_seq_system_info_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_seq_system_info_malloc(snd_seq_system_info_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_seq_system_info_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief Frees a previously allocated #snd_seq_system_info_t
 * \param obj pointer to object to free
 */
void snd_seq_system_info_free(snd_seq_system_info_t *obj)
{
	free(obj);
}

/**
 * \brief Copy one #snd_seq_system_info_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_seq_system_info_copy(snd_seq_system_info_t *dst, const snd_seq_system_info_t *src)
{
	assert(dst && src);
	*dst = *src;
}


/**
 * \brief Get maximum number of queues
 * \param info #snd_seq_system_info_t container
 * \return maximum number of queues
 *
 * \sa snd_seq_system_info()
 */
int snd_seq_system_info_get_queues(const snd_seq_system_info_t *info)
{
	assert(info);
	return info->queues;
}

/**
 * \brief Get maximum number of clients
 * \param info #snd_seq_system_info_t container
 * \return maximum number of clients
 *
 * \sa snd_seq_system_info()
 */
int snd_seq_system_info_get_clients(const snd_seq_system_info_t *info)
{
	assert(info);
	return info->clients;
}

/**
 * \brief Get maximum number of ports
 * \param info #snd_seq_system_info_t container
 * \return maximum number of ports
 *
 * \sa snd_seq_system_info()
 */
int snd_seq_system_info_get_ports(const snd_seq_system_info_t *info)
{
	assert(info);
	return info->ports;
}

/**
 * \brief Get maximum number of channels
 * \param info #snd_seq_system_info_t container
 * \return maximum number of channels
 *
 * \sa snd_seq_system_info()
 */
int snd_seq_system_info_get_channels(const snd_seq_system_info_t *info)
{
	assert(info);
	return info->channels;
}

/**
 * \brief Get the current number of clients
 * \param info #snd_seq_system_info_t container
 * \return current number of clients
 *
 * \sa snd_seq_system_info()
 */
int snd_seq_system_info_get_cur_clients(const snd_seq_system_info_t *info)
{
	assert(info);
	return info->cur_clients;
}

/**
 * \brief Get the current number of queues
 * \param info #snd_seq_system_info_t container
 * \return current number of queues
 *
 * \sa snd_seq_system_info()
 */
int snd_seq_system_info_get_cur_queues(const snd_seq_system_info_t *info)
{
	assert(info);
	return info->cur_queues;
}

/**
 * \brief obtain the sequencer system information
 * \param seq sequencer handle
 * \param info the pointer to be stored
 * \return 0 on success otherwise a negative error code
 *
 * Stores the global system information of ALSA sequencer system.
 * The returned data contains
 * the maximum available numbers of queues, clients, ports and channels.
 */
int snd_seq_system_info(snd_seq_t *seq, snd_seq_system_info_t * info)
{
	assert(seq && info);
	return seq->ops->system_info(seq, info);
}


/*----------------------------------------------------------------*/

/**
 * \brief get size of #snd_seq_client_info_t
 * \return size in bytes
 */
size_t snd_seq_client_info_sizeof()
{
	return sizeof(snd_seq_client_info_t);
}

/**
 * \brief allocate an empty #snd_seq_client_info_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_seq_client_info_malloc(snd_seq_client_info_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_seq_client_info_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees a previously allocated #snd_seq_client_info_t
 * \param obj pointer to object to free
 */
void snd_seq_client_info_free(snd_seq_client_info_t *obj)
{
	free(obj);
}

/**
 * \brief copy one #snd_seq_client_info_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_seq_client_info_copy(snd_seq_client_info_t *dst, const snd_seq_client_info_t *src)
{
	assert(dst && src);
	*dst = *src;
}


/**
 * \brief Get client id of a client_info container
 * \param info client_info container
 * \return client id
 *
 * \sa snd_seq_get_client_info(), snd_seq_client_info_set_client(), snd_seq_client_id()
 */
int snd_seq_client_info_get_client(const snd_seq_client_info_t *info)
{
	assert(info);
	return info->client;
}

/**
 * \brief Get client type of a client_info container
 * \param info client_info container
 * \return client type
 *
 * The client type is either #SND_SEQ_KERNEL_CLIENT or #SND_SEQ_USER_CLIENT
 * for kernel or user client respectively.
 *
 * \sa snd_seq_get_client_info()
 */
snd_seq_client_type_t snd_seq_client_info_get_type(const snd_seq_client_info_t *info)
{
	assert(info);
	return info->type;
}

/**
 * \brief Get the name of a client_info container
 * \param info client_info container
 * \return name string
 *
 * \sa snd_seq_get_client_info(), snd_seq_client_info_set_name()
 */
const char *snd_seq_client_info_get_name(snd_seq_client_info_t *info)
{
	assert(info);
	return info->name;
}

/**
 * \brief Get the broadcast filter usage of a client_info container
 * \param info client_info container
 * \return 1 if broadcast is accepted
 *
 * \sa snd_seq_get_client_info(), snd_seq_client_info_set_broadcast_filter()
 */
int snd_seq_client_info_get_broadcast_filter(const snd_seq_client_info_t *info)
{
	assert(info);
	return (info->filter & SNDRV_SEQ_FILTER_BROADCAST) ? 1 : 0;
}

/**
 * \brief Get the error-bounce usage of a client_info container
 * \param info client_info container
 * \return 1 if error-bounce is enabled
 *
 * \sa snd_seq_get_client_info(), snd_seq_client_info_set_error_bounce()
 */
int snd_seq_client_info_get_error_bounce(const snd_seq_client_info_t *info)
{
	assert(info);
	return (info->filter & SNDRV_SEQ_FILTER_BOUNCE) ? 1 : 0;
}

/**
 * \brief (DEPRECATED) Get the event filter bitmap of a client_info container
 * \param info client_info container
 * \return NULL if no event filter, or pointer to event filter bitmap
 *
 * Use #snd_seq_client_info_event_filter_check() instead.
 *
 * \sa snd_seq_client_info_event_filter_add(),
 *     snd_seq_client_info_event_filter_del(),
 *     snd_seq_client_info_event_filter_check(),
 *     snd_seq_client_info_event_filter_clear(),
 *     snd_seq_get_client_info()
 */
const unsigned char *snd_seq_client_info_get_event_filter(const snd_seq_client_info_t *info)
{
	assert(info);
	if (info->filter & SNDRV_SEQ_FILTER_USE_EVENT)
		return info->event_filter;
	else
		return NULL;
}

/**
 * \brief Disable event filtering of a client_info container
 * \param info client_info container
 *
 * Remove all event types added with #snd_seq_client_info_event_filter_add and clear
 * the event filtering flag of this client_info container.
 * 
 * \sa snd_seq_client_info_event_filter_add(),
 *     snd_seq_client_info_event_filter_del(),
 *     snd_seq_client_info_event_filter_check(),
 *     snd_seq_get_client_info(),
 *     snd_seq_set_client_info()
 */
void snd_seq_client_info_event_filter_clear(snd_seq_client_info_t *info)
{
       assert(info);
       info->filter &= ~SNDRV_SEQ_FILTER_USE_EVENT;
       memset(info->event_filter, 0, sizeof(info->event_filter));
}

/**
 * \brief Add an event type to the event filtering of a client_info container
 * \param info client_info container
 * \param event_type event type to be added
 * 
 * Set the event filtering flag of this client_info and add the specified event type to the 
 * filter bitmap of this client_info container.
 *
 * \sa snd_seq_get_client_info(),
 *     snd_seq_set_client_info(),
 *     snd_seq_client_info_event_filter_del(),
 *     snd_seq_client_info_event_filter_check(),
 *     snd_seq_client_info_event_filter_clear()
 */
void snd_seq_client_info_event_filter_add(snd_seq_client_info_t *info, int event_type)
{
       assert(info);
       info->filter |= SNDRV_SEQ_FILTER_USE_EVENT;
       snd_seq_set_bit(event_type, info->event_filter);
}

/**
 * \brief Remove an event type from the event filtering of a client_info container
 * \param info client_info container
 * \param event_type event type to be removed
 *
 * Removes the specified event from the filter bitmap of this client_info container. It will
 * not clear the event filtering flag, use #snd_seq_client_info_event_filter_clear instead.
 *
 * \sa snd_seq_get_client_info(),
 *     snd_seq_set_client_info(),
 *     snd_seq_client_info_event_filter_add(),
 *     snd_seq_client_info_event_filter_check(),
 *     snd_seq_client_info_event_filter_clear()
 */
void snd_seq_client_info_event_filter_del(snd_seq_client_info_t *info, int event_type)
{
       assert(info);
       snd_seq_unset_bit(event_type, info->event_filter);
}

/**
 * \brief Check if an event type is present in the event filtering of a client_info container
 * \param info client_info container
 * \param event_type event type to be checked
 * \return 1 if the event type is present, 0 otherwise
 *
 * Test if the event type is in the filter bitmap of this client_info container.
 *
 * \sa snd_seq_get_client_info(),
 *     snd_seq_set_client_info(),
 *     snd_seq_client_info_event_filter_add(),
 *     snd_seq_client_info_event_filter_del(),
 *     snd_seq_client_info_event_filter_clear()
 */
int snd_seq_client_info_event_filter_check(snd_seq_client_info_t *info, int event_type)
{
       assert(info);
       return snd_seq_get_bit(event_type, info->event_filter);
} 

/**
 * \brief Get the number of opened ports of a client_info container
 * \param info client_info container
 * \return number of opened ports
 *
 * \sa snd_seq_get_client_info()
 */
int snd_seq_client_info_get_num_ports(const snd_seq_client_info_t *info)
{
	assert(info);
	return info->num_ports;
}

/**
 * \brief Get the number of lost events of a client_info container
 * \param info client_info container
 * \return number of lost events
 *
 * \sa snd_seq_get_client_info()
 */
int snd_seq_client_info_get_event_lost(const snd_seq_client_info_t *info)
{
	assert(info);
	return info->event_lost;
}

/**
 * \brief Set the client id of a client_info container
 * \param info client_info container
 * \param client client id
 *
 * \sa snd_seq_get_client_info(), snd_seq_client_info_get_client()
 */
void snd_seq_client_info_set_client(snd_seq_client_info_t *info, int client)
{
	assert(info);
	info->client = client;
}

/**
 * \brief Set the name of a client_info container
 * \param info client_info container
 * \param name name string
 *
 * \sa snd_seq_get_client_info(), snd_seq_client_info_get_name(),
 *     snd_seq_set_client_name()
 */
void snd_seq_client_info_set_name(snd_seq_client_info_t *info, const char *name)
{
	assert(info && name);
	strncpy(info->name, name, sizeof(info->name));
}

/**
 * \brief Set the broadcast filter usage of a client_info container
 * \param info client_info container
 * \param val non-zero if broadcast is accepted
 *
 * \sa snd_seq_get_client_info(), snd_seq_client_info_get_broadcast_filter()
 */
void snd_seq_client_info_set_broadcast_filter(snd_seq_client_info_t *info, int val)
{
	assert(info);
	if (val)
		info->filter |= SNDRV_SEQ_FILTER_BROADCAST;
	else
		info->filter &= ~SNDRV_SEQ_FILTER_BROADCAST;
}

/**
 * \brief Set the error-bounce usage of a client_info container
 * \param info client_info container
 * \param val non-zero if error is bounced
 *
 * \sa snd_seq_get_client_info(), snd_seq_client_info_get_error_bounce()
 */
void snd_seq_client_info_set_error_bounce(snd_seq_client_info_t *info, int val)
{
	assert(info);
	if (val)
		info->filter |= SNDRV_SEQ_FILTER_BOUNCE;
	else
		info->filter &= ~SNDRV_SEQ_FILTER_BOUNCE;
}

/**
 * \brief (DEPRECATED) Set the event filter bitmap of a client_info container
 * \param info client_info container
 * \param filter event filter bitmap, pass NULL for no event filtering
 *
 * Use #snd_seq_client_info_event_filter_add instead.
 *
 * \sa snd_seq_client_info_event_filter_add(),
 *     snd_seq_client_info_event_filter_del(),
 *     snd_seq_client_info_event_filter_check(),
 *     snd_seq_client_info_event_filter_clear(),
 *     snd_seq_set_client_info()
 */
void snd_seq_client_info_set_event_filter(snd_seq_client_info_t *info, unsigned char *filter)
{
	assert(info);
	if (! filter)
		info->filter &= ~SNDRV_SEQ_FILTER_USE_EVENT;
	else {
		info->filter |= SNDRV_SEQ_FILTER_USE_EVENT;
		memcpy(info->event_filter, filter, sizeof(info->event_filter));
	}
}


/**
 * \brief obtain the information of the given client
 * \param seq sequencer handle
 * \param client client id
 * \param info the pointer to be stored
 * \return 0 on success otherwise a negative error code
 * 
 * Obtains the information of the client with a client id specified by
 * info argument.
 * The obtained information is written on info parameter.
 *
 * \sa snd_seq_get_client_info()
 */
int snd_seq_get_any_client_info(snd_seq_t *seq, int client, snd_seq_client_info_t *info)
{
	assert(seq && info && client >= 0);
	memset(info, 0, sizeof(snd_seq_client_info_t));
	info->client = client;
	return seq->ops->get_client_info(seq, info);
}

/**
 * \brief obtain the current client information
 * \param seq sequencer handle
 * \param info the pointer to be stored
 * \return 0 on success otherwise a negative error code
 *
 * Obtains the information of the current client stored on info.
 * client and type fields are ignored.
 *
 * \sa snd_seq_get_any_client_info(), snd_seq_set_client_info(),
 *     snd_seq_query_next_client()
 */
int snd_seq_get_client_info(snd_seq_t *seq, snd_seq_client_info_t *info)
{
	return snd_seq_get_any_client_info(seq, seq->client, info);
}

/**
 * \brief set the current client information
 * \param seq sequencer handle
 * \param info the client info data to set
 * \return 0 on success otherwise a negative error code
 *
 * Obtains the information of the current client stored on info.
 * client and type fields are ignored.
 *
 * \sa snd_seq_get_client_info()
 */
int snd_seq_set_client_info(snd_seq_t *seq, snd_seq_client_info_t *info)
{
	assert(seq && info);
	info->client = seq->client;
	info->type = USER_CLIENT;
	return seq->ops->set_client_info(seq, info);
}

/**
 * \brief query the next client
 * \param seq sequencer handle
 * \param info query pattern and result
 *
 * Queries the next client.
 * The search begins at the client with an id one greater than
 * client field in info.
 * If a client is found, its attributes are stored in info,
 * and zero is returned.
 * Otherwise returns a negative error code.
 *
 * \sa snd_seq_get_any_client_info()
 */
int snd_seq_query_next_client(snd_seq_t *seq, snd_seq_client_info_t *info)
{
	assert(seq && info);
	return seq->ops->query_next_client(seq, info);
}


/*----------------------------------------------------------------*/


/*
 * Port
 */

/**
 * \brief get size of #snd_seq_port_info_t
 * \return size in bytes
 */
size_t snd_seq_port_info_sizeof()
{
	return sizeof(snd_seq_port_info_t);
}

/**
 * \brief allocate an empty #snd_seq_port_info_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_seq_port_info_malloc(snd_seq_port_info_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_seq_port_info_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees a previously allocated #snd_seq_port_info_t
 * \param obj pointer to object to free
 */
void snd_seq_port_info_free(snd_seq_port_info_t *obj)
{
	free(obj);
}

/**
 * \brief copy one #snd_seq_port_info_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_seq_port_info_copy(snd_seq_port_info_t *dst, const snd_seq_port_info_t *src)
{
	assert(dst && src);
	*dst = *src;
}


/**
 * \brief Get client id of a port_info container
 * \param info port_info container
 * \return client id
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_client()
 */
int snd_seq_port_info_get_client(const snd_seq_port_info_t *info)
{
	assert(info);
	return info->addr.client;
}

/**
 * \brief Get port id of a port_info container
 * \param info port_info container
 * \return port id
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_port()
 */
int snd_seq_port_info_get_port(const snd_seq_port_info_t *info)
{
	assert(info);
	return info->addr.port;
}

/**
 * \brief Get client/port address of a port_info container
 * \param info port_info container
 * \return client/port address pointer
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_addr()
 */
const snd_seq_addr_t *snd_seq_port_info_get_addr(const snd_seq_port_info_t *info)
{
	assert(info);
	return &info->addr;
}

/**
 * \brief Get the name of a port_info container
 * \param info port_info container
 * \return name string
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_name()
 */
const char *snd_seq_port_info_get_name(const snd_seq_port_info_t *info)
{
	assert(info);
	return info->name;
}

/**
 * \brief Get the capability bits of a port_info container
 * \param info port_info container
 * \return capability bits
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_capability()
 */
unsigned int snd_seq_port_info_get_capability(const snd_seq_port_info_t *info)
{
	assert(info);
	return info->capability;
}

/**
 * \brief Get the type bits of a port_info container
 * \param info port_info container
 * \return port type bits
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_type()
 */
unsigned int snd_seq_port_info_get_type(const snd_seq_port_info_t *info)
{
	assert(info);
	return info->type;
}

/**
 * \brief Get the number of read subscriptions of a port_info container
 * \param info port_info container
 * \return number of read subscriptions
 *
 * \sa snd_seq_get_port_info()
 */
int snd_seq_port_info_get_read_use(const snd_seq_port_info_t *info)
{
	assert(info);
	return info->read_use;
}

/**
 * \brief Get the number of write subscriptions of a port_info container
 * \param info port_info container
 * \return number of write subscriptions
 *
 * \sa snd_seq_get_port_info()
 */
int snd_seq_port_info_get_write_use(const snd_seq_port_info_t *info)
{
	assert(info);
	return info->write_use;
}

/**
 * \brief Get the midi channels of a port_info container
 * \param info port_info container
 * \return number of midi channels (default 0)
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_midi_channels()
 */
int snd_seq_port_info_get_midi_channels(const snd_seq_port_info_t *info)
{
	assert(info);
	return info->midi_channels;
}

/**
 * \brief Get the midi voices of a port_info container
 * \param info port_info container
 * \return number of midi voices (default 0)
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_midi_voices()
 */
int snd_seq_port_info_get_midi_voices(const snd_seq_port_info_t *info)
{
	assert(info);
	return info->midi_voices;
}

/**
 * \brief Get the synth voices of a port_info container
 * \param info port_info container
 * \return number of synth voices (default 0)
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_synth_voices()
 */
int snd_seq_port_info_get_synth_voices(const snd_seq_port_info_t *info)
{
	assert(info);
	return info->synth_voices;
}

/**
 * \brief Get the port-specified mode of a port_info container
 * \param info port_info container
 * \return 1 if port id is specified at creation
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_port_specified()
 */
int snd_seq_port_info_get_port_specified(const snd_seq_port_info_t *info)
{
	assert(info);
	return (info->flags & SNDRV_SEQ_PORT_FLG_GIVEN_PORT) ? 1 : 0;
}

/**
 * \brief Get the time-stamping mode of the given port in a port_info container
 * \param info port_info container
 * \return 1 if the port updates timestamps of incoming events
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_timestamping()
 */
int snd_seq_port_info_get_timestamping(const snd_seq_port_info_t *info)
{
	assert(info);
	return (info->flags & SNDRV_SEQ_PORT_FLG_TIMESTAMP) ? 1 : 0;
}

/**
 * \brief Get whether the time-stamping of the given port is real-time mode
 * \param info port_info container
 * \return 1 if the time-stamping is in the real-time mode
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_timestamp_real()
 */
int snd_seq_port_info_get_timestamp_real(const snd_seq_port_info_t *info)
{
	assert(info);
	return (info->flags & SNDRV_SEQ_PORT_FLG_TIME_REAL) ? 1 : 0;
}

/**
 * \brief Get the queue id to update timestamps
 * \param info port_info container
 * \return the queue id to get the timestamps
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_set_timestamp_queue()
 */
int snd_seq_port_info_get_timestamp_queue(const snd_seq_port_info_t *info)
{
	assert(info);
	return info->time_queue;
}

/**
 * \brief Set the client id of a port_info container
 * \param info port_info container
 * \param client client id
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_client()
 */
void snd_seq_port_info_set_client(snd_seq_port_info_t *info, int client)
{
	assert(info);
	info->addr.client = client;
}

/**
 * \brief Set the port id of a port_info container
 * \param info port_info container
 * \param port port id
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_port()
 */
void snd_seq_port_info_set_port(snd_seq_port_info_t *info, int port)
{
	assert(info);
	info->addr.port = port;
}

/**
 * \brief Set the client/port address of a port_info container
 * \param info port_info container
 * \param addr client/port address
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_addr()
 */
void snd_seq_port_info_set_addr(snd_seq_port_info_t *info, const snd_seq_addr_t *addr)
{
	assert(info);
	info->addr = *addr;
}

/**
 * \brief Set the name of a port_info container
 * \param info port_info container
 * \param name name string
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_name()
 */
void snd_seq_port_info_set_name(snd_seq_port_info_t *info, const char *name)
{
	assert(info && name);
	strncpy(info->name, name, sizeof(info->name));
}

/**
 * \brief set the capability bits of a port_info container
 * \param info port_info container
 * \param capability capability bits
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_capability()
 */
void snd_seq_port_info_set_capability(snd_seq_port_info_t *info, unsigned int capability)
{
	assert(info);
	info->capability = capability;
}

/**
 * \brief Get the type bits of a port_info container
 * \param info port_info container
 * \param type port type bits
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_type()
 */
void snd_seq_port_info_set_type(snd_seq_port_info_t *info, unsigned int type)
{
	assert(info);
	info->type = type;
}

/**
 * \brief set the midi channels of a port_info container
 * \param info port_info container
 * \param channels midi channels (default 0)
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_midi_channels()
 */
void snd_seq_port_info_set_midi_channels(snd_seq_port_info_t *info, int channels)
{
	assert(info);
	info->midi_channels = channels;
}

/**
 * \brief set the midi voices of a port_info container
 * \param info port_info container
 * \param voices midi voices (default 0)
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_midi_voices()
 */
void snd_seq_port_info_set_midi_voices(snd_seq_port_info_t *info, int voices)
{
	assert(info);
	info->midi_voices = voices;
}

/**
 * \brief set the synth voices of a port_info container
 * \param info port_info container
 * \param voices synth voices (default 0)
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_synth_voice()
 */
void snd_seq_port_info_set_synth_voices(snd_seq_port_info_t *info, int voices)
{
	assert(info);
	info->synth_voices = voices;
}

/**
 * \brief Set the port-specified mode of a port_info container
 * \param info port_info container
 * \param val non-zero if specifying the port id at creation
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_port_specified()
 */
void snd_seq_port_info_set_port_specified(snd_seq_port_info_t *info, int val)
{
	assert(info);
	if (val)
		info->flags |= SNDRV_SEQ_PORT_FLG_GIVEN_PORT;
	else
		info->flags &= ~SNDRV_SEQ_PORT_FLG_GIVEN_PORT;
}

/**
 * \brief Set the time-stamping mode of the given port
 * \param info port_info container
 * \param enable non-zero if updating the timestamps of incoming events
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_timestamping()
 */
void snd_seq_port_info_set_timestamping(snd_seq_port_info_t *info, int enable)
{
	assert(info);
	if (enable)
		info->flags |= SNDRV_SEQ_PORT_FLG_TIMESTAMP;
	else
		info->flags &= ~SNDRV_SEQ_PORT_FLG_TIMESTAMP;
}

/**
 * \brief Set whether the timestime is updated in the real-time mode
 * \param info port_info container
 * \param enable non-zero if updating the timestamps in real-time mode
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_timestamp_real()
 */
void snd_seq_port_info_set_timestamp_real(snd_seq_port_info_t *info, int enable)
{
	assert(info);
	if (enable)
		info->flags |= SNDRV_SEQ_PORT_FLG_TIME_REAL;
	else
		info->flags &= ~SNDRV_SEQ_PORT_FLG_TIME_REAL;
}

/**
 * \brief Set the queue id for timestamping
 * \param info port_info container
 * \param queue the queue id to get timestamps
 *
 * \sa snd_seq_get_port_info(), snd_seq_port_info_get_timestamp_queue()
 */
void snd_seq_port_info_set_timestamp_queue(snd_seq_port_info_t *info, int queue)
{
	assert(info);
	info->time_queue = queue;
}


/**
 * \brief create a sequencer port on the current client
 * \param seq sequencer handle
 * \param port port information for the new port
 * \return 0 on success otherwise a negative error code
 *
 * Creates a sequencer port on the current client.
 * The attributes of created port is specified in \a info argument.
 *
 * The client field in \a info argument is overwritten with the current client id.
 * The port id to be created can be specified via #snd_seq_port_info_set_port_specified.
 * You can get the created port id by reading the port pointer via #snd_seq_port_info_get_port.
 *
 * Each port has the capability bit-masks to specify the access capability
 * of the port from other clients.
 * The capability bit flags are defined as follows:
 * - #SND_SEQ_PORT_CAP_READ Readable from this port
 * - #SND_SEQ_PORT_CAP_WRITE Writable to this port.
 * - #SND_SEQ_PORT_CAP_SYNC_READ For synchronization (not implemented)
 * - #SND_SEQ_PORT_CAP_SYNC_WRITE For synchronization (not implemented)
 * - #SND_SEQ_PORT_CAP_DUPLEX Read/write duplex access is supported
 * - #SND_SEQ_PORT_CAP_SUBS_READ Read subscription is allowed
 * - #SND_SEQ_PORT_CAP_SUBS_WRITE Write subscription is allowed
 * - #SND_SEQ_PORT_CAP_NO_EXPORT Subscription management from 3rd client is disallowed
 *
 * Each port has also the type bitmasks defined as follows:
 * - #SND_SEQ_PORT_TYPE_SPECIFIC Hardware specific port
 * - #SND_SEQ_PORT_TYPE_MIDI_GENERIC Generic MIDI device
 * - #SND_SEQ_PORT_TYPE_MIDI_GM General MIDI compatible device
 * - #SND_SEQ_PORT_TYPE_MIDI_GM2 General MIDI 2 compatible device
 * - #SND_SEQ_PORT_TYPE_MIDI_GS GS compatible device
 * - #SND_SEQ_PORT_TYPE_MIDI_XG XG compatible device
 * - #SND_SEQ_PORT_TYPE_MIDI_MT32 MT-32 compatible device
 * - #SND_SEQ_PORT_TYPE_HARDWARE Implemented in hardware
 * - #SND_SEQ_PORT_TYPE_SOFTWARE Implemented in software
 * - #SND_SEQ_PORT_TYPE_SYNTHESIZER Generates sound
 * - #SND_SEQ_PORT_TYPE_PORT Connects to other device(s)
 * - #SND_SEQ_PORT_TYPE_APPLICATION Application (sequencer/editor)
 *
 * A port may contain specific midi channels, midi voices and synth voices.
 * These values could be zero as default.
 *
 * \sa snd_seq_delete_port(), snd_seq_get_port_info(),
 *     snd_seq_create_simple_port()
 */
int snd_seq_create_port(snd_seq_t *seq, snd_seq_port_info_t * port)
{
	assert(seq && port);
	port->addr.client = seq->client;
	return seq->ops->create_port(seq, port);
}

/**
 * \brief delete a sequencer port on the current client
 * \param seq sequencer handle
 * \param port port to be deleted
 * \return 0 on success otherwise a negative error code
 *
 * Deletes the existing sequencer port on the current client.
 *
 * \sa snd_seq_create_port(), snd_seq_delete_simple_port()
 */
int snd_seq_delete_port(snd_seq_t *seq, int port)
{
	snd_seq_port_info_t pinfo;
	assert(seq);
	memset(&pinfo, 0, sizeof(pinfo));
	pinfo.addr.client = seq->client;
	pinfo.addr.port = port;
	return seq->ops->delete_port(seq, &pinfo);
}

/**
 * \brief obtain the information of a port on an arbitrary client
 * \param seq sequencer handle
 * \param client client id to get
 * \param port port id to get
 * \param info pointer information returns
 * \return 0 on success otherwise a negative error code
 *
 * \sa snd_seq_get_port_info()
 */
int snd_seq_get_any_port_info(snd_seq_t *seq, int client, int port, snd_seq_port_info_t * info)
{
	assert(seq && info && client >= 0 && port >= 0);
	memset(info, 0, sizeof(snd_seq_port_info_t));
	info->addr.client = client;
	info->addr.port = port;
	return seq->ops->get_port_info(seq, info);
}

/**
 * \brief obtain the information of a port on the current client
 * \param seq sequencer handle
 * \param port port id to get
 * \param info pointer information returns
 * \return 0 on success otherwise a negative error code
 *
 * \sa snd_seq_create_port(), snd_seq_get_any_port_info(), snd_seq_set_port_info(),
 *     snd_seq_query_next_port()
 */
int snd_seq_get_port_info(snd_seq_t *seq, int port, snd_seq_port_info_t * info)
{
	return snd_seq_get_any_port_info(seq, seq->client, port, info);
}

/**
 * \brief set the information of a port on the current client
 * \param seq sequencer handle
 * \param port port to be set
 * \param info port information to be set
 * \return 0 on success otherwise a negative error code
 *
 * \sa snd_seq_set_port_info()
 */
int snd_seq_set_port_info(snd_seq_t *seq, int port, snd_seq_port_info_t * info)
{
	assert(seq && info && port >= 0);
	info->addr.client = seq->client;
	info->addr.port = port;
	return seq->ops->set_port_info(seq, info);
}

/**
 * \brief query the next matching port
 * \param seq sequencer handle
 * \param info query pattern and result

 * Queries the next matching port on the client specified in
 * \a info argument.
 * The search begins at the next port specified in
 * port field of \a info argument.
 * For finding the first port at a certain client, give -1.
 *
 * If a matching port is found, its attributes are stored on
 * \a info and function returns zero.
 * Otherwise, a negative error code is returned.
 *
 * \sa snd_seq_get_port_info()
 */
int snd_seq_query_next_port(snd_seq_t *seq, snd_seq_port_info_t *info)
{
	assert(seq && info);
	return seq->ops->query_next_port(seq, info);
}


/*----------------------------------------------------------------*/

/*
 * subscription
 */


/**
 * \brief get size of #snd_seq_port_subscribe_t
 * \return size in bytes
 */
size_t snd_seq_port_subscribe_sizeof()
{
	return sizeof(snd_seq_port_subscribe_t);
}

/**
 * \brief allocate an empty #snd_seq_port_subscribe_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_seq_port_subscribe_malloc(snd_seq_port_subscribe_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_seq_port_subscribe_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees a previously allocated #snd_seq_port_subscribe_t
 * \param obj pointer to object to free
 */
void snd_seq_port_subscribe_free(snd_seq_port_subscribe_t *obj)
{
	free(obj);
}

/**
 * \brief copy one #snd_seq_port_subscribe_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_seq_port_subscribe_copy(snd_seq_port_subscribe_t *dst, const snd_seq_port_subscribe_t *src)
{
	assert(dst && src);
	*dst = *src;
}


/**
 * \brief Get sender address of a port_subscribe container
 * \param info port_subscribe container
 *
 * \sa snd_seq_subscribe_port(), snd_seq_port_subscribe_set_sender()
 */
const snd_seq_addr_t *snd_seq_port_subscribe_get_sender(const snd_seq_port_subscribe_t *info)
{
	assert(info);
	return &info->sender;
}

/**
 * \brief Get destination address of a port_subscribe container
 * \param info port_subscribe container
 *
 * \sa snd_seq_subscribe_port(), snd_seq_port_subscribe_set_dest()
 */
const snd_seq_addr_t *snd_seq_port_subscribe_get_dest(const snd_seq_port_subscribe_t *info)
{
	assert(info);
	return &info->dest;
}

/**
 * \brief Get the queue id of a port_subscribe container
 * \param info port_subscribe container
 * \return queue id
 *
 * \sa snd_seq_subscribe_port(), snd_seq_port_subscribe_set_queue()
 */
int snd_seq_port_subscribe_get_queue(const snd_seq_port_subscribe_t *info)
{
	assert(info);
	return info->queue;
}

/**
 * \brief Get the exclusive mode of a port_subscribe container
 * \param info port_subscribe container
 * \return 1 if exclusive mode
 *
 * \sa snd_seq_subscribe_port(), snd_seq_port_subscribe_set_exclusive()
 */
int snd_seq_port_subscribe_get_exclusive(const snd_seq_port_subscribe_t *info)
{
	assert(info);
	return (info->flags & SNDRV_SEQ_PORT_SUBS_EXCLUSIVE) ? 1 : 0;
}

/**
 * \brief Get the time-update mode of a port_subscribe container
 * \param info port_subscribe container
 * \return 1 if update timestamp
 *
 * \sa snd_seq_subscribe_port(), snd_seq_port_subscribe_set_time_update()
 */
int snd_seq_port_subscribe_get_time_update(const snd_seq_port_subscribe_t *info)
{
	assert(info);
	return (info->flags & SNDRV_SEQ_PORT_SUBS_TIMESTAMP) ? 1 : 0;
}

/**
 * \brief Get the real-time update mode of a port_subscribe container
 * \param info port_subscribe container
 * \return 1 if real-time update mode
 *
 * \sa snd_seq_subscribe_port(), snd_seq_port_subscribe_set_time_real()
 */
int snd_seq_port_subscribe_get_time_real(const snd_seq_port_subscribe_t *info)
{
	assert(info);
	return (info->flags & SNDRV_SEQ_PORT_SUBS_TIME_REAL) ? 1 : 0;
}

/**
 * \brief Set sender address of a port_subscribe container
 * \param info port_subscribe container
 * \param addr sender address
 *
 * \sa snd_seq_subscribe_port(), snd_seq_port_subscribe_get_sender()
 */
void snd_seq_port_subscribe_set_sender(snd_seq_port_subscribe_t *info, const snd_seq_addr_t *addr)
{
	assert(info);
	memcpy(&info->sender, addr, sizeof(*addr));
}
      
/**
 * \brief Set destination address of a port_subscribe container
 * \param info port_subscribe container
 * \param addr destination address
 *
 * \sa snd_seq_subscribe_port(), snd_seq_port_subscribe_get_dest()
 */
void snd_seq_port_subscribe_set_dest(snd_seq_port_subscribe_t *info, const snd_seq_addr_t *addr)
{
	assert(info);
	memcpy(&info->dest, addr, sizeof(*addr));
}

/**
 * \brief Set the queue id of a port_subscribe container
 * \param info port_subscribe container
 * \param q queue id
 *
 * \sa snd_seq_subscribe_port(), snd_seq_port_subscribe_get_queue()
 */
void snd_seq_port_subscribe_set_queue(snd_seq_port_subscribe_t *info, int q)
{
	assert(info);
	info->queue = q;
}

/**
 * \brief Set the exclusive mode of a port_subscribe container
 * \param info port_subscribe container
 * \param val non-zero to enable
 *
 * \sa snd_seq_subscribe_port(), snd_seq_port_subscribe_get_exclusive()
 */
void snd_seq_port_subscribe_set_exclusive(snd_seq_port_subscribe_t *info, int val)
{
	assert(info);
	if (val)
		info->flags |= SNDRV_SEQ_PORT_SUBS_EXCLUSIVE;
	else
		info->flags &= ~SNDRV_SEQ_PORT_SUBS_EXCLUSIVE;
}

/**
 * \brief Set the time-update mode of a port_subscribe container
 * \param info port_subscribe container
 * \param val non-zero to enable
 *
 * \sa snd_seq_subscribe_port(), snd_seq_port_subscribe_get_time_update()
 */
void snd_seq_port_subscribe_set_time_update(snd_seq_port_subscribe_t *info, int val)
{
	assert(info);
	if (val)
		info->flags |= SNDRV_SEQ_PORT_SUBS_TIMESTAMP;
	else
		info->flags &= ~SNDRV_SEQ_PORT_SUBS_TIMESTAMP;
}

/**
 * \brief Set the real-time mode of a port_subscribe container
 * \param info port_subscribe container
 * \param val non-zero to enable
 *
 * \sa snd_seq_subscribe_port(), snd_seq_port_subscribe_get_time_real()
 */
void snd_seq_port_subscribe_set_time_real(snd_seq_port_subscribe_t *info, int val)
{
	assert(info);
	if (val)
		info->flags |= SNDRV_SEQ_PORT_SUBS_TIME_REAL;
	else
		info->flags &= ~SNDRV_SEQ_PORT_SUBS_TIME_REAL;
}


/**
 * \brief obtain subscription information
 * \param seq sequencer handle
 * \param sub pointer to return the subscription information
 * \return 0 on success otherwise a negative error code
 *
 * \sa snd_seq_subscribe_port(), snd_seq_query_port_subscribers()
 */
int snd_seq_get_port_subscription(snd_seq_t *seq, snd_seq_port_subscribe_t * sub)
{
	assert(seq && sub);
	return seq->ops->get_port_subscription(seq, sub);
}

/**
 * \brief subscribe a port connection
 * \param seq sequencer handle
 * \param sub subscription information
 * \return 0 on success otherwise a negative error code
 *
 * Subscribes a connection between two ports.
 * The subscription information is stored in sub argument.
 *
 * \sa snd_seq_get_port_subscription(), snd_seq_unsubscribe_port(),
 *     snd_seq_connect_from(), snd_seq_connect_to()
 */
int snd_seq_subscribe_port(snd_seq_t *seq, snd_seq_port_subscribe_t * sub)
{
	assert(seq && sub);
	return seq->ops->subscribe_port(seq, sub);
}

/**
 * \brief unsubscribe a connection between ports
 * \param seq sequencer handle
 * \param sub subscription information to disconnect
 * \return 0 on success otherwise a negative error code
 *
 * Unsubscribes a connection between two ports,
 * described in sender and dest fields in sub argument.
 *
 * \sa snd_seq_subscribe_port(), snd_seq_disconnect_from(), snd_seq_disconnect_to()
 */
int snd_seq_unsubscribe_port(snd_seq_t *seq, snd_seq_port_subscribe_t * sub)
{
	assert(seq && sub);
	return seq->ops->unsubscribe_port(seq, sub);
}


/**
 * \brief get size of #snd_seq_query_subscribe_t
 * \return size in bytes
 */
size_t snd_seq_query_subscribe_sizeof()
{
	return sizeof(snd_seq_query_subscribe_t);
}

/**
 * \brief allocate an empty #snd_seq_query_subscribe_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_seq_query_subscribe_malloc(snd_seq_query_subscribe_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_seq_query_subscribe_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees a previously allocated #snd_seq_query_subscribe_t
 * \param obj pointer to object to free
 */
void snd_seq_query_subscribe_free(snd_seq_query_subscribe_t *obj)
{
	free(obj);
}

/**
 * \brief copy one #snd_seq_query_subscribe_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_seq_query_subscribe_copy(snd_seq_query_subscribe_t *dst, const snd_seq_query_subscribe_t *src)
{
	assert(dst && src);
	*dst = *src;
}


/**
 * \brief Get the client id of a query_subscribe container
 * \param info query_subscribe container
 * \return client id
 *
 * \sa snd_seq_query_port_subscribers(), snd_seq_query_subscribe_set_client()
 */
int snd_seq_query_subscribe_get_client(const snd_seq_query_subscribe_t *info)
{
	assert(info);
	return info->root.client;
}

/**
 * \brief Get the port id of a query_subscribe container
 * \param info query_subscribe container
 * \return port id
 *
 * \sa snd_seq_query_port_subscribers(), snd_seq_query_subscribe_set_port()
 */
int snd_seq_query_subscribe_get_port(const snd_seq_query_subscribe_t *info)
{
	assert(info);
	return info->root.port;
}

/**
 * \brief Get the client/port address of a query_subscribe container
 * \param info query_subscribe container
 * \return client/port address pointer
 *
 * \sa snd_seq_query_port_subscribers(), snd_seq_query_subscribe_set_root()
 */
const snd_seq_addr_t *snd_seq_query_subscribe_get_root(const snd_seq_query_subscribe_t *info)
{
	assert(info);
	return &info->root;
}

/**
 * \brief Get the query type of a query_subscribe container
 * \param info query_subscribe container
 * \return query type
 *
 * \sa snd_seq_query_port_subscribers(), snd_seq_query_subscribe_set_type()
 */
snd_seq_query_subs_type_t snd_seq_query_subscribe_get_type(const snd_seq_query_subscribe_t *info)
{
	assert(info);
	return info->type;
}

/**
 * \brief Get the index of subscriber of a query_subscribe container
 * \param info query_subscribe container
 * \return subscriber's index
 *
 * \sa snd_seq_query_port_subscribers(), snd_seq_query_subscribe_set_index()
 */
int snd_seq_query_subscribe_get_index(const snd_seq_query_subscribe_t *info)
{
	assert(info);
	return info->index;
}

/**
 * \brief Get the number of subscriptions of a query_subscribe container
 * \param info query_subscribe container
 * \return number of subscriptions
 *
 * \sa snd_seq_query_port_subscribers()
 */
int snd_seq_query_subscribe_get_num_subs(const snd_seq_query_subscribe_t *info)
{
	assert(info);
	return info->num_subs;
}	

/**
 * \brief Get the address of subscriber of a query_subscribe container
 * \param info query_subscribe container
 * \return subscriber's address pointer
 *
 * \sa snd_seq_query_port_subscribers()
 */
const snd_seq_addr_t *snd_seq_query_subscribe_get_addr(const snd_seq_query_subscribe_t *info)
{
	assert(info);
	return &info->addr;
}

/**
 * \brief Get the queue id of subscriber of a query_subscribe container
 * \param info query_subscribe container
 * \return subscriber's queue id
 *
 * \sa snd_seq_query_port_subscribers()
 */
int snd_seq_query_subscribe_get_queue(const snd_seq_query_subscribe_t *info)
{
	assert(info);
	return info->queue;
}

/**
 * \brief Get the exclusive mode of a query_subscribe container
 * \param info query_subscribe container
 * \return 1 if exclusive mode
 *
 * \sa snd_seq_query_port_subscribers()
 */
int snd_seq_query_subscribe_get_exclusive(const snd_seq_query_subscribe_t *info)
{
	assert(info);
	return (info->flags & SNDRV_SEQ_PORT_SUBS_EXCLUSIVE) ? 1 : 0;
}

/**
 * \brief Get the time-update mode of a query_subscribe container
 * \param info query_subscribe container
 * \return 1 if update timestamp
 *
 * \sa snd_seq_query_port_subscribers()
 */
int snd_seq_query_subscribe_get_time_update(const snd_seq_query_subscribe_t *info)
{
	assert(info);
	return (info->flags & SNDRV_SEQ_PORT_SUBS_TIMESTAMP) ? 1 : 0;
}

/**
 * \brief Get the real-time update mode of a query_subscribe container
 * \param info query_subscribe container
 * \return 1 if real-time update mode
 *
 * \sa snd_seq_query_port_subscribers()
 */
int snd_seq_query_subscribe_get_time_real(const snd_seq_query_subscribe_t *info)
{
	assert(info);
	return (info->flags & SNDRV_SEQ_PORT_SUBS_TIMESTAMP) ? 1 : 0;
}

/**
 * \brief Set the client id of a query_subscribe container
 * \param info query_subscribe container
 * \param client client id
 *
 * \sa snd_seq_query_port_subscribers(), snd_seq_query_subscribe_get_client()
 */
void snd_seq_query_subscribe_set_client(snd_seq_query_subscribe_t *info, int client)
{
	assert(info);
	info->root.client = client;
}

/**
 * \brief Set the port id of a query_subscribe container
 * \param info query_subscribe container
 * \param port port id
 *
 * \sa snd_seq_query_port_subscribers(), snd_seq_query_subscribe_get_port()
 */
void snd_seq_query_subscribe_set_port(snd_seq_query_subscribe_t *info, int port)
{
	assert(info);
	info->root.port = port;
}

/**
 * \brief Set the client/port address of a query_subscribe container
 * \param info query_subscribe container
 * \param addr client/port address pointer
 *
 * \sa snd_seq_query_port_subscribers(), snd_seq_query_subscribe_get_root()
 */
void snd_seq_query_subscribe_set_root(snd_seq_query_subscribe_t *info, const snd_seq_addr_t *addr)
{
	assert(info);
	info->root = *addr;
}

/**
 * \brief Set the query type of a query_subscribe container
 * \param info query_subscribe container
 * \param type query type
 *
 * \sa snd_seq_query_port_subscribers(), snd_seq_query_subscribe_get_type()
 */
void snd_seq_query_subscribe_set_type(snd_seq_query_subscribe_t *info, snd_seq_query_subs_type_t type)
{
	assert(info);
	info->type = type;
}

/**
 * \brief Set the subscriber's index to be queried
 * \param info query_subscribe container
 * \param index index to be queried
 *
 * \sa snd_seq_query_port_subscribers(), snd_seq_query_subscribe_get_index()
 */
void snd_seq_query_subscribe_set_index(snd_seq_query_subscribe_t *info, int index)
{
	assert(info);
	info->index = index;
}


/**
 * \brief query port subscriber list
 * \param seq sequencer handle
 * \param subs subscription to query
 * \return 0 on success otherwise a negative error code
 *
 * Queries the subscribers accessing to a port.
 * The query information is specified in subs argument.
 *
 * At least, the client id, the port id, the index number and
 * the query type must be set to perform a proper query.
 * As the query type, #SND_SEQ_QUERY_SUBS_READ or #SND_SEQ_QUERY_SUBS_WRITE
 * can be specified to check whether the readers or the writers to the port.
 * To query the first subscription, set 0 to the index number.  To list up
 * all the subscriptions, call this function with the index numbers from 0
 * until this returns a negative value.
 *
 * \sa snd_seq_get_port_subscription()
 */
int snd_seq_query_port_subscribers(snd_seq_t *seq, snd_seq_query_subscribe_t * subs)
{
	assert(seq && subs);
	return seq->ops->query_port_subscribers(seq, subs);
}

/*----------------------------------------------------------------*/

/*
 * queue handlers
 */

/**
 * \brief get size of #snd_seq_queue_info_t
 * \return size in bytes
 */
size_t snd_seq_queue_info_sizeof()
{
	return sizeof(snd_seq_queue_info_t);
}

/**
 * \brief allocate an empty #snd_seq_queue_info_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_seq_queue_info_malloc(snd_seq_queue_info_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_seq_queue_info_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees a previously allocated #snd_seq_queue_info_t
 * \param obj pointer to object to free
 */
void snd_seq_queue_info_free(snd_seq_queue_info_t *obj)
{
	free(obj);
}

/**
 * \brief copy one #snd_seq_queue_info_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_seq_queue_info_copy(snd_seq_queue_info_t *dst, const snd_seq_queue_info_t *src)
{
	assert(dst && src);
	*dst = *src;
}


/**
 * \brief Get the queue id of a queue_info container
 * \param info queue_info container
 * \return queue id
 *
 * \sa snd_seq_get_queue_info(), snd_seq_queue_info_set_queue()
 */
int snd_seq_queue_info_get_queue(const snd_seq_queue_info_t *info)
{
	assert(info);
	return info->queue;
}

/**
 * \brief Get the name of a queue_info container
 * \param info queue_info container
 * \return name string
 *
 * \sa snd_seq_get_queue_info(), snd_seq_queue_info_set_name()
 */
const char *snd_seq_queue_info_get_name(const snd_seq_queue_info_t *info)
{
	assert(info);
	return info->name;
}

/**
 * \brief Get the owner client id of a queue_info container
 * \param info queue_info container
 * \return owner client id
 *
 * \sa snd_seq_get_queue_info(), snd_seq_queue_info_set_owner()
 */
int snd_seq_queue_info_get_owner(const snd_seq_queue_info_t *info)
{
	assert(info);
	return info->owner;
}

/**
 * \brief Get the lock status of a queue_info container
 * \param info queue_info container
 * \return lock status --- non-zero = locked
 *
 * \sa snd_seq_get_queue_info(), snd_seq_queue_info_set_locked()
 */
int snd_seq_queue_info_get_locked(const snd_seq_queue_info_t *info)
{
	assert(info);
	return info->locked;
}

/**
 * \brief Get the conditional bit flags of a queue_info container
 * \param info queue_info container
 * \return conditional bit flags
 *
 * \sa snd_seq_get_queue_info(), snd_seq_queue_info_set_flags()
 */
unsigned int snd_seq_queue_info_get_flags(const snd_seq_queue_info_t *info)
{
	assert(info);
	return info->flags;
}

/**
 * \brief Set the name of a queue_info container
 * \param info queue_info container
 * \param name name string
 *
 * \sa snd_seq_get_queue_info(), snd_seq_queue_info_get_name()
 */
void snd_seq_queue_info_set_name(snd_seq_queue_info_t *info, const char *name)
{
	assert(info && name);
	strncpy(info->name, name, sizeof(info->name));
}

/**
 * \brief Set the owner client id of a queue_info container
 * \param info queue_info container
 * \param owner client id
 *
 * \sa snd_seq_get_queue_info(), snd_seq_queue_info_get_owner()
 */
void snd_seq_queue_info_set_owner(snd_seq_queue_info_t *info, int owner)
{
	assert(info);
	info->owner = owner;
}

/**
 * \brief Set the lock status of a queue_info container
 * \param info queue_info container
 * \param locked lock status
 *
 * \sa snd_seq_get_queue_info(), snd_seq_queue_info_get_locked()
 */
void snd_seq_queue_info_set_locked(snd_seq_queue_info_t *info, int locked)
{
	assert(info);
	info->locked = locked;
}

/**
 * \brief Set the conditional bit flags of a queue_info container
 * \param info queue_info container
 * \param flags conditional bit flags
 *
 * \sa snd_seq_get_queue_info(), snd_seq_queue_info_get_flags()
 */
void snd_seq_queue_info_set_flags(snd_seq_queue_info_t *info, unsigned int flags)
{
	assert(info);
	info->flags = flags;
}


/**
 * \brief create a queue
 * \param seq sequencer handle
 * \param info queue information to initialize
 * \return the queue id (zero or positive) on success otherwise a negative error code
 *
 * \sa snd_seq_alloc_queue()
 */
int snd_seq_create_queue(snd_seq_t *seq, snd_seq_queue_info_t *info)
{
	int err;
	assert(seq && info);
	info->owner = seq->client;
	err = seq->ops->create_queue(seq, info);
	if (err < 0)
		return err;
	return info->queue;
}

/**
 * \brief allocate a queue with the specified name
 * \param seq sequencer handle
 * \param name the name of the new queue
 * \return the queue id (zero or positive) on success otherwise a negative error code
 *
 * \sa snd_seq_alloc_queue()
 */ 
int snd_seq_alloc_named_queue(snd_seq_t *seq, const char *name)
{
	snd_seq_queue_info_t info;
	memset(&info, 0, sizeof(info));
	info.locked = 1;
	if (name)
		strncpy(info.name, name, sizeof(info.name) - 1);
	return snd_seq_create_queue(seq, &info);
}

/**
 * \brief allocate a queue
 * \param seq sequencer handle
 * \return the queue id (zero or positive) on success otherwise a negative error code
 *
 * \sa snd_seq_alloc_named_queue(), snd_seq_create_queue(), snd_seq_free_queue(),
 *     snd_seq_get_queue_info()
 */ 
int snd_seq_alloc_queue(snd_seq_t *seq)
{
	return snd_seq_alloc_named_queue(seq, NULL);
}

/**
 * \brief delete the specified queue
 * \param seq sequencer handle
 * \param q queue id to delete
 * \return 0 on success otherwise a negative error code
 *
 * \sa snd_seq_alloc_queue()
 */
int snd_seq_free_queue(snd_seq_t *seq, int q)
{
	snd_seq_queue_info_t info;
	assert(seq);
	memset(&info, 0, sizeof(info));
	info.queue = q;
	return seq->ops->delete_queue(seq, &info);
}

/**
 * \brief obtain queue attributes
 * \param seq sequencer handle
 * \param q queue id to query
 * \param info information returned
 * \return 0 on success otherwise a negative error code
 *
 * \sa snd_seq_alloc_queue(), snd_seq_set_queue_info(), snd_seq_query_named_queue()
 */
int snd_seq_get_queue_info(snd_seq_t *seq, int q, snd_seq_queue_info_t *info)
{
	assert(seq && info);
	info->queue = q;
	return seq->ops->get_queue_info(seq, info);
}

/**
 * \brief change the queue attributes
 * \param seq sequencer handle
 * \param q queue id to change
 * \param info information changed
 * \return 0 on success otherwise a negative error code
 *
 * \sa snd_seq_get_queue_info()
 */
int snd_seq_set_queue_info(snd_seq_t *seq, int q, snd_seq_queue_info_t *info)
{
	assert(seq && info);
	info->queue = q;
	return seq->ops->set_queue_info(seq, info);
}

/**
 * \brief query the matching queue with the specified name
 * \param seq sequencer handle
 * \param name the name string to query
 * \return the queue id if found or negative error code
 *
 * Searches the matching queue with the specified name string.
 *
 * \sa snd_seq_get_queue_info()
 */
int snd_seq_query_named_queue(snd_seq_t *seq, const char *name)
{
	int err;
	snd_seq_queue_info_t info;
	assert(seq && name);
	strncpy(info.name, name, sizeof(info.name));
	err = seq->ops->get_named_queue(seq, &info);
	if (err < 0)
		return err;
	return info.queue;
}

/**
 * \brief Get the queue usage flag to the client
 * \param seq sequencer handle
 * \param q queue id
 * \return 1 = client is allowed to access the queue, 0 = not allowed, 
 *     otherwise a negative error code
 *
 * \sa snd_seq_get_queue_info(), snd_seq_set_queue_usage()
 */
int snd_seq_get_queue_usage(snd_seq_t *seq, int q)
{
	struct sndrv_seq_queue_client info;
	int err;
	assert(seq);
	memset(&info, 0, sizeof(info));
	info.queue = q;
	info.client = seq->client;
	if ((err = seq->ops->get_queue_client(seq, &info)) < 0)
		return err;
	return info.used;
}

/**
 * \brief Set the queue usage flag to the client
 * \param seq sequencer handle
 * \param q queue id
 * \param used non-zero if the client is allowed
 * \return 0 on success otherwise a negative error code
 *
 * \sa snd_seq_get_queue_info(), snd_seq_set_queue_usage()
 */
int snd_seq_set_queue_usage(snd_seq_t *seq, int q, int used)
{
	struct sndrv_seq_queue_client info;
	assert(seq);
	memset(&info, 0, sizeof(info));
	info.queue = q;
	info.client = seq->client;
	info.used = used ? 1 : 0;
	return seq->ops->set_queue_client(seq, &info);
}


/**
 * \brief get size of #snd_seq_queue_status_t
 * \return size in bytes
 */
size_t snd_seq_queue_status_sizeof()
{
	return sizeof(snd_seq_queue_status_t);
}

/**
 * \brief allocate an empty #snd_seq_queue_status_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_seq_queue_status_malloc(snd_seq_queue_status_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_seq_queue_status_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees a previously allocated #snd_seq_queue_status_t
 * \param obj pointer to object to free
 */
void snd_seq_queue_status_free(snd_seq_queue_status_t *obj)
{
	free(obj);
}

/**
 * \brief copy one #snd_seq_queue_status_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_seq_queue_status_copy(snd_seq_queue_status_t *dst, const snd_seq_queue_status_t *src)
{
	assert(dst && src);
	*dst = *src;
}


/**
 * \brief Get the queue id of a queue_status container
 * \param info queue_status container
 * \return queue id
 *
 * \sa snd_seq_get_queue_status()
 */
int snd_seq_queue_status_get_queue(const snd_seq_queue_status_t *info)
{
	assert(info);
	return info->queue;
}

/**
 * \brief Get the number of events of a queue_status container
 * \param info queue_status container
 * \return number of events
 *
 * \sa snd_seq_get_queue_status()
 */
int snd_seq_queue_status_get_events(const snd_seq_queue_status_t *info)
{
	assert(info);
	return info->events;
}

/**
 * \brief Get the tick time of a queue_status container
 * \param info queue_status container
 * \return tick time
 *
 * \sa snd_seq_get_queue_status()
 */
snd_seq_tick_time_t snd_seq_queue_status_get_tick_time(const snd_seq_queue_status_t *info)
{
	assert(info);
	return info->tick;
}

/**
 * \brief Get the real time of a queue_status container
 * \param info queue_status container
 *
 * \sa snd_seq_get_queue_status()
 */
const snd_seq_real_time_t *snd_seq_queue_status_get_real_time(const snd_seq_queue_status_t *info)
{
	assert(info);
	return &info->time;
}

/**
 * \brief Get the running status bits of a queue_status container
 * \param info queue_status container
 * \return running status bits
 *
 * \sa snd_seq_get_queue_status()
 */
unsigned int snd_seq_queue_status_get_status(const snd_seq_queue_status_t *info)
{
	assert(info);
	return info->running;
}


/**
 * \brief obtain the running state of the queue
 * \param seq sequencer handle
 * \param q queue id to query
 * \param status pointer to store the current status
 * \return 0 on success otherwise a negative error code
 *
 * Obtains the running state of the specified queue q.
 */
int snd_seq_get_queue_status(snd_seq_t *seq, int q, snd_seq_queue_status_t * status)
{
	assert(seq && status);
	memset(status, 0, sizeof(snd_seq_queue_status_t));
	status->queue = q;
	return seq->ops->get_queue_status(seq, status);
}


/**
 * \brief get size of #snd_seq_queue_tempo_t
 * \return size in bytes
 */
size_t snd_seq_queue_tempo_sizeof()
{
	return sizeof(snd_seq_queue_tempo_t);
}

/**
 * \brief allocate an empty #snd_seq_queue_tempo_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_seq_queue_tempo_malloc(snd_seq_queue_tempo_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_seq_queue_tempo_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees a previously allocated #snd_seq_queue_tempo_t
 * \param obj pointer to object to free
 */
void snd_seq_queue_tempo_free(snd_seq_queue_tempo_t *obj)
{
	free(obj);
}

/**
 * \brief copy one #snd_seq_queue_tempo_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_seq_queue_tempo_copy(snd_seq_queue_tempo_t *dst, const snd_seq_queue_tempo_t *src)
{
	assert(dst && src);
	*dst = *src;
}


/**
 * \brief Get the queue id of a queue_status container
 * \param info queue_status container
 * \return queue id
 *
 * \sa snd_seq_get_queue_tempo()
 */
int snd_seq_queue_tempo_get_queue(const snd_seq_queue_tempo_t *info)
{
	assert(info);
	return info->queue;
}

/**
 * \brief Get the tempo of a queue_status container
 * \param info queue_status container
 * \return tempo value
 *
 * \sa snd_seq_get_queue_tempo()
 */
unsigned int snd_seq_queue_tempo_get_tempo(const snd_seq_queue_tempo_t *info)
{
	assert(info);
	return info->tempo;
}

/**
 * \brief Get the ppq of a queue_status container
 * \param info queue_status container
 * \return ppq value
 *
 * \sa snd_seq_get_queue_tempo()
 */
int snd_seq_queue_tempo_get_ppq(const snd_seq_queue_tempo_t *info)
{
	assert(info);
	return info->ppq;
}

/**
 * \brief Get the timer skew value of a queue_status container
 * \param info queue_status container
 * \return timer skew value
 *
 * \sa snd_seq_get_queue_tempo()
 */
unsigned int snd_seq_queue_tempo_get_skew(const snd_seq_queue_tempo_t *info)
{
	assert(info);
	return info->skew_value;
}

/**
 * \brief Get the timer skew base value of a queue_status container
 * \param info queue_status container
 * \return timer skew base value
 *
 * \sa snd_seq_get_queue_tempo()
 */
unsigned int snd_seq_queue_tempo_get_skew_base(const snd_seq_queue_tempo_t *info)
{
	assert(info);
	return info->skew_base;
}

/**
 * \brief Set the tempo of a queue_status container
 * \param info queue_status container
 * \param tempo tempo value
 *
 * \sa snd_seq_get_queue_tempo()
 */
void snd_seq_queue_tempo_set_tempo(snd_seq_queue_tempo_t *info, unsigned int tempo)
{
	assert(info);
	info->tempo = tempo;
}

/**
 * \brief Set the ppq of a queue_status container
 * \param info queue_status container
 * \param ppq ppq value
 *
 * \sa snd_seq_get_queue_tempo()
 */
void snd_seq_queue_tempo_set_ppq(snd_seq_queue_tempo_t *info, int ppq)
{
	assert(info);
	info->ppq = ppq;
}

/**
 * \brief Set the timer skew value of a queue_status container
 * \param info queue_status container
 * \param skew timer skew value
 *
 * The skew of timer is calculated as skew / base.
 * For example, to play with double speed, pass base * 2 as the skew value.
 *
 * \sa snd_seq_get_queue_tempo()
 */
void snd_seq_queue_tempo_set_skew(snd_seq_queue_tempo_t *info, unsigned int skew)
{
	assert(info);
	info->skew_value = skew;
}

/**
 * \brief Set the timer skew base value of a queue_status container
 * \param info queue_status container
 * \param base timer skew base value
 *
 * \sa snd_seq_get_queue_tempo()
 */
void snd_seq_queue_tempo_set_skew_base(snd_seq_queue_tempo_t *info, unsigned int base)
{
	assert(info);
	info->skew_base = base;
}

/**
 * \brief obtain the current tempo of the queue
 * \param seq sequencer handle
 * \param q queue id to be queried
 * \param tempo pointer to store the current tempo
 * \return 0 on success otherwise a negative error code
 *
 * \sa snd_seq_set_queue_tempo()
 */
int snd_seq_get_queue_tempo(snd_seq_t *seq, int q, snd_seq_queue_tempo_t * tempo)
{
	assert(seq && tempo);
	memset(tempo, 0, sizeof(snd_seq_queue_tempo_t));
	tempo->queue = q;
	return seq->ops->get_queue_tempo(seq, tempo);
}

/**
 * \brief set the tempo of the queue
 * \param seq sequencer handle
 * \param q queue id to change the tempo
 * \param tempo tempo information
 * \return 0 on success otherwise a negative error code
 *
 * \sa snd_seq_get_queue_tempo()
 */
int snd_seq_set_queue_tempo(snd_seq_t *seq, int q, snd_seq_queue_tempo_t * tempo)
{
	assert(seq && tempo);
	tempo->queue = q;
	return seq->ops->set_queue_tempo(seq, tempo);
}


/*----------------------------------------------------------------*/

/**
 * \brief get size of #snd_seq_queue_timer_t
 * \return size in bytes
 */
size_t snd_seq_queue_timer_sizeof()
{
	return sizeof(snd_seq_queue_timer_t);
}

/**
 * \brief allocate an empty #snd_seq_queue_timer_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_seq_queue_timer_malloc(snd_seq_queue_timer_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_seq_queue_timer_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees a previously allocated #snd_seq_queue_timer_t
 * \param obj pointer to object to free
 */
void snd_seq_queue_timer_free(snd_seq_queue_timer_t *obj)
{
	free(obj);
}

/**
 * \brief copy one #snd_seq_queue_timer_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_seq_queue_timer_copy(snd_seq_queue_timer_t *dst, const snd_seq_queue_timer_t *src)
{
	assert(dst && src);
	*dst = *src;
}


/**
 * \brief Get the queue id of a queue_timer container
 * \param info queue_timer container
 * \return queue id
 *
 * \sa snd_seq_get_queue_timer()
 */
int snd_seq_queue_timer_get_queue(const snd_seq_queue_timer_t *info)
{
	assert(info);
	return info->queue;
}

/**
 * \brief Get the timer type of a queue_timer container
 * \param info queue_timer container
 * \return timer type
 *
 * \sa snd_seq_get_queue_timer()
 */
snd_seq_queue_timer_type_t snd_seq_queue_timer_get_type(const snd_seq_queue_timer_t *info)
{
	assert(info);
	return (snd_seq_queue_timer_type_t)info->type;
}

/**
 * \brief Get the timer id of a queue_timer container
 * \param info queue_timer container
 * \return timer id pointer
 *
 * \sa snd_seq_get_queue_timer()
 */
const snd_timer_id_t *snd_seq_queue_timer_get_id(const snd_seq_queue_timer_t *info)
{
	assert(info);
	return &info->u.alsa.id;
}

/**
 * \brief Get the timer resolution of a queue_timer container
 * \param info queue_timer container
 * \return timer resolution
 *
 * \sa snd_seq_get_queue_timer()
 */
unsigned int snd_seq_queue_timer_get_resolution(const snd_seq_queue_timer_t *info)
{
	assert(info);
	return info->u.alsa.resolution;
}

/**
 * \brief Set the timer type of a queue_timer container
 * \param info queue_timer container
 * \param type timer type
 *
 * \sa snd_seq_get_queue_timer()
 */
void snd_seq_queue_timer_set_type(snd_seq_queue_timer_t *info, snd_seq_queue_timer_type_t type)
{
	assert(info);
	info->type = (int)type;
}
	
/**
 * \brief Set the timer id of a queue_timer container
 * \param info queue_timer container
 * \param id timer id pointer
 *
 * \sa snd_seq_get_queue_timer()
 */
void snd_seq_queue_timer_set_id(snd_seq_queue_timer_t *info, const snd_timer_id_t *id)
{
	assert(info && id);
	info->u.alsa.id = *id;
}

/**
 * \brief Set the timer resolution of a queue_timer container
 * \param info queue_timer container
 * \param resolution timer resolution
 *
 * \sa snd_seq_get_queue_timer()
 */
void snd_seq_queue_timer_set_resolution(snd_seq_queue_timer_t *info, unsigned int resolution)
{
	assert(info);
	info->u.alsa.resolution = resolution;
}


/**
 * \brief obtain the queue timer information
 * \param seq sequencer handle
 * \param q queue id to query
 * \param timer pointer to store the timer information
 * \return 0 on success otherwise a negative error code
 *
 * \sa snd_seq_set_queue_timer()
 */
int snd_seq_get_queue_timer(snd_seq_t *seq, int q, snd_seq_queue_timer_t * timer)
{
	assert(seq && timer);
	memset(timer, 0, sizeof(snd_seq_queue_timer_t));
	timer->queue = q;
	return seq->ops->get_queue_timer(seq, timer);
}

/**
 * \brief set the queue timer information
 * \param seq sequencer handle
 * \param q queue id to change the timer
 * \param timer timer information
 * \return 0 on success otherwise a negative error code
 *
 * \sa snd_seq_get_queue_timer()
 */
int snd_seq_set_queue_timer(snd_seq_t *seq, int q, snd_seq_queue_timer_t * timer)
{
	assert(seq && timer);
	timer->queue = q;
	return seq->ops->set_queue_timer(seq, timer);
}

/*----------------------------------------------------------------*/

#ifndef DOC_HIDDEN
/**
 * \brief (DEPRECATED) create an event cell
 * \return the cell pointer allocated
 *
 * create an event cell via malloc.  the returned pointer must be released
 * by the application itself via normal free() call,
 * not via snd_seq_free_event().
 */
snd_seq_event_t *snd_seq_create_event(void)
{
	return (snd_seq_event_t *) calloc(1, sizeof(snd_seq_event_t));
}
#endif

/**
 * \brief (DEPRECATED) free an event
 *
 * In the former version, this function was used to
 * release the event pointer which was allocated by snd_seq_event_input().
 * In the current version, the event record is not allocated, so
 * you don't have to call this function any more.
 */
#ifndef DOXYGEN
int snd_seq_free_event(snd_seq_event_t *ev ATTRIBUTE_UNUSED)
#else
int snd_seq_free_event(snd_seq_event_t *ev)
#endif
{
	return 0;
}

/**
 * \brief calculates the (encoded) byte-stream size of the event
 * \param ev the event
 * \return the size of decoded bytes
 */
ssize_t snd_seq_event_length(snd_seq_event_t *ev)
{
	ssize_t len = sizeof(snd_seq_event_t);
	assert(ev);
	if (snd_seq_ev_is_variable(ev))
		len += ev->data.ext.len;
	return len;
}

/*----------------------------------------------------------------*/

/*
 * output to sequencer
 */

/**
 * \brief output an event
 * \param seq sequencer handle
 * \param ev event to be output
 * \return the number of remaining events or a negative error code
 *
 * An event is once expanded on the output buffer.
 * The output buffer will be drained automatically if it becomes full.
 *
 * If events remain unprocessed on output buffer before drained,
 * the size of total byte data on output buffer is returned.
 * If the output buffer is empty, this returns zero.
 *
 * \sa snd_seq_event_output_direct(), snd_seq_event_output_buffer(),
 *    snd_seq_event_output_pending(), snd_seq_drain_output(),
 *    snd_seq_drop_output(), snd_seq_extract_output(),
 *    snd_seq_remove_events()
 */
int snd_seq_event_output(snd_seq_t *seq, snd_seq_event_t *ev)
{
	int result;

	result = snd_seq_event_output_buffer(seq, ev);
	if (result == -EAGAIN) {
		result = snd_seq_drain_output(seq);
		if (result < 0)
			return result;
		return snd_seq_event_output_buffer(seq, ev);
	}
	return result;
}

/**
 * \brief output an event onto the lib buffer without draining buffer
 * \param seq sequencer handle
 * \param ev event to be output
 * \return the byte size of remaining events. \c -EAGAIN if the buffer becomes full.
 *
 * This function doesn't drain buffer unlike snd_seq_event_output().
 *
 * \sa snd_seq_event_output()
 */
int snd_seq_event_output_buffer(snd_seq_t *seq, snd_seq_event_t *ev)
{
	int len;
	assert(seq && ev);
	len = snd_seq_event_length(ev);
	if (len < 0)
		return -EINVAL;
	if ((size_t) len >= seq->obufsize)
		return -EINVAL;
	if ((seq->obufsize - seq->obufused) < (size_t) len)
		return -EAGAIN;
	memcpy(seq->obuf + seq->obufused, ev, sizeof(snd_seq_event_t));
	seq->obufused += sizeof(snd_seq_event_t);
	if (snd_seq_ev_is_variable(ev)) {
		memcpy(seq->obuf + seq->obufused, ev->data.ext.ptr, ev->data.ext.len);
		seq->obufused += ev->data.ext.len;
	}
	return seq->obufused;
}

/*
 * allocate the temporary buffer
 */
static int alloc_tmpbuf(snd_seq_t *seq, size_t len)
{
	size_t size = ((len + sizeof(snd_seq_event_t) - 1) / sizeof(snd_seq_event_t));
	if (seq->tmpbuf == NULL) {
		if (size > DEFAULT_TMPBUF_SIZE)
			seq->tmpbufsize = size;
		else
			seq->tmpbufsize = DEFAULT_TMPBUF_SIZE;
		seq->tmpbuf = malloc(seq->tmpbufsize * sizeof(snd_seq_event_t));
		if (seq->tmpbuf == NULL)
			return -ENOMEM;
	}  else if (len > seq->tmpbufsize) {
		seq->tmpbuf = realloc(seq->tmpbuf, size * sizeof(snd_seq_event_t));
		if (seq->tmpbuf == NULL)
			return -ENOMEM;
		seq->tmpbufsize = size;
	}
	return 0;
}

/**
 * \brief output an event directly to the sequencer NOT through output buffer
 * \param seq sequencer handle
 * \param ev event to be output
 * \return the byte size sent to sequencer or a negative error code
 *
 * This function sends an event to the sequencer directly not through the
 * output buffer.  When the event is a variable length event, a temporary
 * buffer is allocated inside alsa-lib and the data is copied there before
 * actually sent.
 *
 * \sa snd_seq_event_output()
 */
int snd_seq_event_output_direct(snd_seq_t *seq, snd_seq_event_t *ev)
{
	ssize_t len;
	void *buf;

	len = snd_seq_event_length(ev);
	if (len < 0)
		return len;
	else if (len == sizeof(*ev)) {
		buf = ev;
	} else {
		if (alloc_tmpbuf(seq, (size_t)len) < 0)
			return -ENOMEM;
		*seq->tmpbuf = *ev;
		memcpy(seq->tmpbuf + 1, ev->data.ext.ptr, ev->data.ext.len);
		buf = seq->tmpbuf;
	}
	return seq->ops->write(seq, buf, (size_t) len);
}

/**
 * \brief return the size of pending events on output buffer
 * \param seq sequencer handle
 * \return the byte size of total of pending events
 *
 * \sa snd_seq_event_output()
 */
int snd_seq_event_output_pending(snd_seq_t *seq)
{
	assert(seq);
	return seq->obufused;
}

/**
 * \brief drain output buffer to sequencer
 * \param seq sequencer handle
 * \return 0 when all events are drained and sent to sequencer.
 *         When events still remain on the buffer, the byte size of remaining
 *         events are returned.  On error a negative error code is returned.
 *
 * This function drains all pending events on the output buffer.
 * The function returns immediately after the events are sent to the queues
 * regardless whether the events are processed or not.
 * To get synchronization with the all event processes, use
 * #snd_seq_sync_output_queue() after calling this function.
 *
 * \sa snd_seq_event_output(), snd_seq_sync_output_queue()
 */
int snd_seq_drain_output(snd_seq_t *seq)
{
	ssize_t result, processed = 0;
	assert(seq);
	while (seq->obufused > 0) {
		result = seq->ops->write(seq, seq->obuf, seq->obufused);
		if (result < 0) {
			if (result == -EAGAIN && processed)
				return seq->obufused;
			return result;
		}
		if ((size_t)result < seq->obufused)
			memmove(seq->obuf, seq->obuf + result, seq->obufused - result);
		seq->obufused -= result;
	}
	return 0;
}

/**
 * \brief extract the first event in output buffer
 * \param seq sequencer handle
 * \param ev_res event pointer to be extracted
 * \return 0 on success otherwise a negative error code
 *
 * Extracts the first event in output buffer.
 * If ev_res is NULL, just remove the event.
 *
 * \sa snd_seq_event_output()
 */
int snd_seq_extract_output(snd_seq_t *seq, snd_seq_event_t **ev_res)
{
	size_t len, olen;
	snd_seq_event_t ev;
	assert(seq);
	if (ev_res)
		*ev_res = NULL;
	if ((olen = seq->obufused) < sizeof(snd_seq_event_t))
		return -ENOENT;
	memcpy(&ev, seq->obuf, sizeof(snd_seq_event_t));
	len = snd_seq_event_length(&ev);
	if (ev_res) {
		/* extract the event */
		if (alloc_tmpbuf(seq, len) < 0)
			return -ENOMEM;
		memcpy(seq->tmpbuf, seq->obuf, len);
		*ev_res = seq->tmpbuf;
	}
	seq->obufused = olen - len;
	memmove(seq->obuf, seq->obuf + len, seq->obufused);
	return 0;
}

/*----------------------------------------------------------------*/

/*
 * input from sequencer
 */

/*
 * read from sequencer to input buffer
 */
static ssize_t snd_seq_event_read_buffer(snd_seq_t *seq)
{
	ssize_t len;
	len = (seq->ops->read)(seq, seq->ibuf, seq->ibufsize * sizeof(snd_seq_event_t));
	if (len < 0)
		return len;
	seq->ibuflen = len / sizeof(snd_seq_event_t);
	seq->ibufptr = 0;
	return seq->ibuflen;
}

static int snd_seq_event_retrieve_buffer(snd_seq_t *seq, snd_seq_event_t **retp)
{
	size_t ncells;
	snd_seq_event_t *ev;

	*retp = ev = &seq->ibuf[seq->ibufptr];
	seq->ibufptr++;
	seq->ibuflen--;
	if (! snd_seq_ev_is_variable(ev))
		return 1;
	ncells = (ev->data.ext.len + sizeof(snd_seq_event_t) - 1) / sizeof(snd_seq_event_t);
	if (seq->ibuflen < ncells) {
		seq->ibuflen = 0; /* clear buffer */
		*retp = NULL;
		return -EINVAL;
	}
	ev->data.ext.ptr = ev + 1;
	seq->ibuflen -= ncells;
	seq->ibufptr += ncells;
	return 1;
}

/**
 * \brief retrieve an event from sequencer
 * \param seq sequencer handle
 * \param ev event pointer to be stored
 * \return 
 *
 * Obtains an input event from sequencer.
 * The event is created via snd_seq_create_event(), and its pointer is stored on
 * ev argument.
 *
 * This function firstly receives the event byte-stream data from sequencer
 * as much as possible at once.  Then it retrieves the first event record
 * and store the pointer on ev.
 * By calling this function sequentially, events are extracted from the input buffer.
 *
 * If there is no input from sequencer, function falls into sleep
 * in blocking mode until an event is received,
 * or returns \c -EAGAIN error in non-blocking mode.
 * Occasionally, this function may return \c -ENOSPC error.
 * This means that the input FIFO of sequencer overran, and some events are
 * lost.
 * Once this error is returned, the input FIFO is cleared automatically.
 *
 * Function returns the byte size of remaining events on the input buffer
 * if an event is successfully received.
 * Application can determine from the returned value whether to call
 * input once more or not.
 *
 * \sa snd_seq_event_input_pending(), snd_seq_drop_input()
 */
int snd_seq_event_input(snd_seq_t *seq, snd_seq_event_t **ev)
{
	int err;
	assert(seq);
	*ev = NULL;
	if (seq->ibuflen <= 0) {
		if ((err = snd_seq_event_read_buffer(seq)) < 0)
			return err;
	}

	return snd_seq_event_retrieve_buffer(seq, ev);
}

/*
 * read input data from sequencer if available
 */
static int snd_seq_event_input_feed(snd_seq_t *seq, int timeout)
{
	struct pollfd pfd;
	int err;
	pfd.fd = seq->poll_fd;
	pfd.events = POLLIN;
	err = poll(&pfd, 1, timeout);
	if (err < 0) {
		SYSERR("poll");
		return -errno;
	}
	if (pfd.revents & POLLIN) 
		return snd_seq_event_read_buffer(seq);
	return seq->ibuflen;
}

/**
 * \brief check events in input buffer
 * \return the byte size of remaining input events on input buffer.
 *
 * If events remain on the input buffer of user-space, function returns
 * the total byte size of events on it.
 * If fetch_sequencer argument is non-zero,
 * this function checks the presence of events on sequencer FIFO
 * When events exist, they are transferred to the input buffer,
 * and the number of received events are returned.
 * If fetch_sequencer argument is zero and
 * no events remain on the input buffer, function simply returns zero.
 *
 * \sa snd_seq_event_input()
 */
int snd_seq_event_input_pending(snd_seq_t *seq, int fetch_sequencer)
{
	if (seq->ibuflen == 0 && fetch_sequencer) {
		return snd_seq_event_input_feed(seq, 0);
	}
	return seq->ibuflen;
}

/*----------------------------------------------------------------*/

/*
 * clear event buffers
 */

/**
 * \brief remove all events on user-space output buffer
 * \param seq sequencer handle
 *
 * Removes all events on user-space output buffer.
 * Unlike snd_seq_drain_output(), this function doesn't remove
 * events on output memory pool of sequencer.
 *
 * \sa snd_seq_drop_output()
 */
int snd_seq_drop_output_buffer(snd_seq_t *seq)
{
	assert(seq);
	seq->obufused = 0;
	return 0;
}

/**
 * \brief remove all events on user-space input FIFO
 * \param seq sequencer handle
 *
 * \sa snd_seq_drop_input()
 */
int snd_seq_drop_input_buffer(snd_seq_t *seq)
{
	assert(seq);
	seq->ibufptr = 0;
	seq->ibuflen = 0;
	return 0;
}

/**
 * \brief remove all events on output buffer
 * \param seq sequencer handle
 *
 * Removes all events on both user-space output buffer and
 * output memory pool on kernel.
 *
 * \sa snd_seq_drain_output(), snd_seq_drop_output_buffer(), snd_seq_remove_events()
 */
int snd_seq_drop_output(snd_seq_t *seq)
{
	snd_seq_remove_events_t rminfo;
	assert(seq);

	memset(&rminfo, 0, sizeof(rminfo));
	rminfo.remove_mode = SNDRV_SEQ_REMOVE_OUTPUT;

	return snd_seq_remove_events(seq, &rminfo);
}

/**
 * \brief clear input buffer and and remove events in sequencer queue
 * \param seq sequencer handle
 *
 * \sa snd_seq_drop_input_buffer(), snd_seq_remove_events()
 */
int snd_seq_drop_input(snd_seq_t *seq)
{
	snd_seq_remove_events_t rminfo;
	assert(seq);

	memset(&rminfo, 0, sizeof(rminfo));
	rminfo.remove_mode = SNDRV_SEQ_REMOVE_INPUT;

	return snd_seq_remove_events(seq, &rminfo);
}


/**
 * \brief get size of #snd_seq_remove_events_t
 * \return size in bytes
 */
size_t snd_seq_remove_events_sizeof()
{
	return sizeof(snd_seq_remove_events_t);
}

/**
 * \brief allocate an empty #snd_seq_remove_events_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_seq_remove_events_malloc(snd_seq_remove_events_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_seq_remove_events_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees a previously allocated #snd_seq_remove_events_t
 * \param obj pointer to object to free
 */
void snd_seq_remove_events_free(snd_seq_remove_events_t *obj)
{
	free(obj);
}

/**
 * \brief copy one #snd_seq_remove_events_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_seq_remove_events_copy(snd_seq_remove_events_t *dst, const snd_seq_remove_events_t *src)
{
	assert(dst && src);
	*dst = *src;
}


/**
 * \brief Get the removal condition bits
 * \param info remove_events container
 * \return removal condition bits
 *
 * \sa snd_seq_remove_events()
 */
unsigned int snd_seq_remove_events_get_condition(const snd_seq_remove_events_t *info)
{
	assert(info);
	return info->remove_mode;
}

/**
 * \brief Get the queue as removal condition
 * \param info remove_events container
 * \return queue id
 *
 * \sa snd_seq_remove_events()
 */
int snd_seq_remove_events_get_queue(const snd_seq_remove_events_t *info)
{
	assert(info);
	return info->queue;
}

/**
 * \brief Get the event timestamp as removal condition
 * \param info remove_events container
 * \return time stamp
 *
 * \sa snd_seq_remove_events()
 */
const snd_seq_timestamp_t *snd_seq_remove_events_get_time(const snd_seq_remove_events_t *info)
{
	assert(info);
	return &info->time;
}

/**
 * \brief Get the event destination address as removal condition
 * \param info remove_events container
 * \return destination address
 *
 * \sa snd_seq_remove_events()
 */
const snd_seq_addr_t *snd_seq_remove_events_get_dest(const snd_seq_remove_events_t *info)
{
	assert(info);
	return &info->dest;
}

/**
 * \brief Get the event channel as removal condition
 * \param info remove_events container
 * \return channel number
 *
 * \sa snd_seq_remove_events()
 */
int snd_seq_remove_events_get_channel(const snd_seq_remove_events_t *info)
{
	assert(info);
	return info->channel;
}

/**
 * \brief Get the event type as removal condition
 * \param info remove_events container
 * \return event type
 *
 * \sa snd_seq_remove_events()
 */
int snd_seq_remove_events_get_event_type(const snd_seq_remove_events_t *info)
{
	assert(info);
	return info->type;
}

/**
 * \brief Get the event tag id as removal condition
 * \param info remove_events container
 * \return tag id
 *
 * \sa snd_seq_remove_events()
 */
int snd_seq_remove_events_get_tag(const snd_seq_remove_events_t *info)
{
	assert(info);
	return info->tag;
}

/**
 * \brief Set the removal condition bits
 * \param info remove_events container
 * \param flags removal condition bits
 *
 * \sa snd_seq_remove_events()
 */
void snd_seq_remove_events_set_condition(snd_seq_remove_events_t *info, unsigned int flags)
{
	assert(info);
	info->remove_mode = flags;
}

/**
 * \brief Set the queue as removal condition
 * \param info remove_events container
 * \param queue queue id
 *
 * \sa snd_seq_remove_events()
 */
void snd_seq_remove_events_set_queue(snd_seq_remove_events_t *info, int queue)
{
	assert(info);
	info->queue = queue;
}

/**
 * \brief Set the timestamp as removal condition
 * \param info remove_events container
 * \param time timestamp pointer
 *
 * \sa snd_seq_remove_events()
 */
void snd_seq_remove_events_set_time(snd_seq_remove_events_t *info, const snd_seq_timestamp_t *time)
{
	assert(info);
	info->time = *time;
}

/**
 * \brief Set the destination address as removal condition
 * \param info remove_events container
 * \param addr destination address
 *
 * \sa snd_seq_remove_events()
 */
void snd_seq_remove_events_set_dest(snd_seq_remove_events_t *info, const snd_seq_addr_t *addr)
{
	assert(info);
	info->dest = *addr;
}

/**
 * \brief Set the channel as removal condition
 * \param info remove_events container
 * \param channel channel number
 *
 * \sa snd_seq_remove_events()
 */
void snd_seq_remove_events_set_channel(snd_seq_remove_events_t *info, int channel)
{
	assert(info);
	info->channel = channel;
}

/**
 * \brief Set the event type as removal condition
 * \param info remove_events container
 * \param type event type
 *
 * \sa snd_seq_remove_events()
 */
void snd_seq_remove_events_set_event_type(snd_seq_remove_events_t *info, int type)
{
	assert(info);
	info->type = type;
}

/**
 * \brief Set the event tag as removal condition
 * \param info remove_events container
 * \param tag tag id
 *
 * \sa snd_seq_remove_events()
 */
void snd_seq_remove_events_set_tag(snd_seq_remove_events_t *info, int tag)
{
	assert(info);
	info->tag = tag;
}


/* compare timestamp between events */
/* return 1 if a >= b; otherwise return 0 */
static inline int snd_seq_compare_tick_time(snd_seq_tick_time_t *a, snd_seq_tick_time_t *b)
{
	/* compare ticks */
	return (*a >= *b);
}

static inline int snd_seq_compare_real_time(snd_seq_real_time_t *a, snd_seq_real_time_t *b)
{
	/* compare real time */
	if (a->tv_sec > b->tv_sec)
		return 1;
	if ((a->tv_sec == b->tv_sec) && (a->tv_nsec >= b->tv_nsec))
		return 1;
	return 0;
}

/* Routine to match events to be removed */
static int remove_match(snd_seq_remove_events_t *info, snd_seq_event_t *ev)
{
	int res;

	if (info->remove_mode & SNDRV_SEQ_REMOVE_DEST) {
		if (ev->dest.client != info->dest.client ||
				ev->dest.port != info->dest.port)
			return 0;
	}
	if (info->remove_mode & SNDRV_SEQ_REMOVE_DEST_CHANNEL) {
		if (! snd_seq_ev_is_channel_type(ev))
			return 0;
		/* data.note.channel and data.control.channel are identical */
		if (ev->data.note.channel != info->channel)
			return 0;
	}
	if (info->remove_mode & SNDRV_SEQ_REMOVE_TIME_AFTER) {
		if (info->remove_mode & SNDRV_SEQ_REMOVE_TIME_TICK)
			res = snd_seq_compare_tick_time(&ev->time.tick, &info->time.tick);
		else
			res = snd_seq_compare_real_time(&ev->time.time, &info->time.time);
		if (!res)
			return 0;
	}
	if (info->remove_mode & SNDRV_SEQ_REMOVE_TIME_BEFORE) {
		if (info->remove_mode & SNDRV_SEQ_REMOVE_TIME_TICK)
			res = snd_seq_compare_tick_time(&ev->time.tick, &info->time.tick);
		else
			res = snd_seq_compare_real_time(&ev->time.time, &info->time.time);
		if (res)
			return 0;
	}
	if (info->remove_mode & SNDRV_SEQ_REMOVE_EVENT_TYPE) {
		if (ev->type != info->type)
			return 0;
	}
	if (info->remove_mode & SNDRV_SEQ_REMOVE_IGNORE_OFF) {
		/* Do not remove off events */
		switch (ev->type) {
		case SND_SEQ_EVENT_NOTEOFF:
		/* case SND_SEQ_EVENT_SAMPLE_STOP: */
			return 0;
		default:
			break;
		}
	}
	if (info->remove_mode & SNDRV_SEQ_REMOVE_TAG_MATCH) {
		if (info->tag != ev->tag)
			return 0;
	}

	return 1;
}

/**
 * \brief remove events on input/output buffers and pools
 * \param seq sequencer handle
 * \param rmp remove event container
 *
 * Removes matching events with the given condition from input/output buffers
 * and pools.
 * The removal condition is specified in \a rmp argument.
 *
 * \sa snd_seq_event_output(), snd_seq_drop_output(), snd_seq_reset_pool_output()
 */
int snd_seq_remove_events(snd_seq_t *seq, snd_seq_remove_events_t *rmp)
{
	if (rmp->remove_mode & SNDRV_SEQ_REMOVE_INPUT) {
		/*
		 * First deal with any events that are still buffered
		 * in the library.
		 */
		snd_seq_drop_input_buffer(seq);
	}

	if (rmp->remove_mode & SNDRV_SEQ_REMOVE_OUTPUT) {
		/*
		 * First deal with any events that are still buffered
		 * in the library.
		 */
		 if (! (rmp->remove_mode & ~(SNDRV_SEQ_REMOVE_INPUT|SNDRV_SEQ_REMOVE_OUTPUT))) {
			 /* The simple case - remove all */
			 snd_seq_drop_output_buffer(seq);
		} else {
			char *ep;
			size_t len;
			snd_seq_event_t *ev;

			ep = seq->obuf;
			while (ep - seq->obuf < (ssize_t)seq->obufused) {

				ev = (snd_seq_event_t *)ep;
				len = snd_seq_event_length(ev);

				if (remove_match(rmp, ev)) {
					/* Remove event */
					seq->obufused -= len;
					memmove(ep, ep + len, seq->obufused - (ep - seq->obuf));
				} else {
					ep += len;
				}
			}
		}
	}

	return seq->ops->remove_events(seq, rmp);
}

/*----------------------------------------------------------------*/

/*
 * client memory pool
 */

/**
 * \brief get size of #snd_seq_client_pool_t
 * \return size in bytes
 */
size_t snd_seq_client_pool_sizeof()
{
	return sizeof(snd_seq_client_pool_t);
}

/**
 * \brief allocate an empty #snd_seq_client_pool_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_seq_client_pool_malloc(snd_seq_client_pool_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_seq_client_pool_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees a previously allocated #snd_seq_client_pool_t
 * \param obj pointer to object to free
 */
void snd_seq_client_pool_free(snd_seq_client_pool_t *obj)
{
	free(obj);
}

/**
 * \brief copy one #snd_seq_client_pool_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_seq_client_pool_copy(snd_seq_client_pool_t *dst, const snd_seq_client_pool_t *src)
{
	assert(dst && src);
	*dst = *src;
}


/**
 * \brief Get the client id of a queue_info container
 * \param info client_pool container
 * \return client id
 */
int snd_seq_client_pool_get_client(const snd_seq_client_pool_t *info)
{
	assert(info);
	return info->client;
}

/**
 * \brief Get the output pool size of a queue_info container
 * \param info client_pool container
 * \return output pool size
 */
size_t snd_seq_client_pool_get_output_pool(const snd_seq_client_pool_t *info)
{
	assert(info);
	return info->output_pool;
}

/**
 * \brief Get the input pool size of a queue_info container
 * \param info client_pool container
 * \return input pool size
 */
size_t snd_seq_client_pool_get_input_pool(const snd_seq_client_pool_t *info)
{
	assert(info);
	return info->input_pool;
}

/**
 * \brief Get the output room size of a queue_info container
 * \param info client_pool container
 * \return output room size
 */
size_t snd_seq_client_pool_get_output_room(const snd_seq_client_pool_t *info)
{
	assert(info);
	return info->output_room;
}

/**
 * \brief Get the available size on output pool of a queue_info container
 * \param info client_pool container
 * \return available output size
 */
size_t snd_seq_client_pool_get_output_free(const snd_seq_client_pool_t *info)
{
	assert(info);
	return info->output_free;
}

/**
 * \brief Get the available size on input pool of a queue_info container
 * \param info client_pool container
 * \return available input size
 */
size_t snd_seq_client_pool_get_input_free(const snd_seq_client_pool_t *info)
{
	assert(info);
	return info->input_free;
}

/**
 * \brief Set the output pool size of a queue_info container
 * \param info client_pool container
 * \param size output pool size
 */
void snd_seq_client_pool_set_output_pool(snd_seq_client_pool_t *info, size_t size)
{
	assert(info);
	info->output_pool = size;
}

/**
 * \brief Set the input pool size of a queue_info container
 * \param info client_pool container
 * \param size input pool size
 */
void snd_seq_client_pool_set_input_pool(snd_seq_client_pool_t *info, size_t size)
{
	assert(info);
	info->input_pool = size;
}

/**
 * \brief Set the output room size of a queue_info container
 * \param info client_pool container
 * \param size output room size
 */
void snd_seq_client_pool_set_output_room(snd_seq_client_pool_t *info, size_t size)
{
	assert(info);
	info->output_room = size;
}


/**
 * \brief obtain the pool information of the current client
 * \param seq sequencer handle
 * \param info information to be stored
 */
int snd_seq_get_client_pool(snd_seq_t *seq, snd_seq_client_pool_t *info)
{
	assert(seq && info);
	info->client = seq->client;
	return seq->ops->get_client_pool(seq, info);
}

/**
 * \brief set the pool information
 * \param seq sequencer handle
 * \param info information to update
 *
 * Sets the pool information of the current client.
 * The client field in \a info is replaced automatically with the current id.
 */
int snd_seq_set_client_pool(snd_seq_t *seq, snd_seq_client_pool_t *info)
{
	assert(seq && info);
	info->client = seq->client;
	return seq->ops->set_client_pool(seq, info);
}

/*----------------------------------------------------------------*/

/*
 * misc.
 */

/**
 * \brief set a bit flag
 */
void snd_seq_set_bit(int nr, void *array)
{
	((unsigned int *)array)[nr >> 5] |= 1UL << (nr & 31);
}

/**
 * \brief unset a bit flag
 */
void snd_seq_unset_bit(int nr, void *array)
{
       ((unsigned int *)array)[nr >> 5] &= ~(1UL << (nr & 31));
}

/**
 * \brief change a bit flag
 */
int snd_seq_change_bit(int nr, void *array)
{
	int result;

	result = ((((unsigned int *)array)[nr >> 5]) & (1UL << (nr & 31))) ? 1 : 0;
	((unsigned int *)array)[nr >> 5] ^= 1UL << (nr & 31);
	return result;
}

/**
 * \brief get a bit flag state
 */
int snd_seq_get_bit(int nr, void *array)
{
	return ((((unsigned int *)array)[nr >> 5]) & (1UL << (nr & 31))) ? 1 : 0;
}
