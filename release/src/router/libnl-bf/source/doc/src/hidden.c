/**
 * \cond skip
 * vim:syntax=doxygen
 * \endcond

\page auto_ack_warning Disabling Auto-ACK

\attention Disabling Auto-ACK (nl_socket_disable_auto_ack()) will cause this
           function to return immediately after sending the netlink message.
	   The function will not wait for an eventual error message. It is
	   the responsibility of the caller to handle any error messages or
	   ACKs returned.

\page pointer_lifetime_warning Pointer Lifetime

\attention The reference counter of the returned object is not incremented.
           Therefore, the returned pointer is only valid during the lifetime
	   of the parent object. Increment the reference counter if the object
	   is supposed to stay around after the parent object was freed.

\page private_struct Private Structure

\note The definition of this structure is private to allow modification
      without breaking API. Use the designated accessor functions to
      access individual object attributes.

\page read_only_attribute Read-Only Attribute

\note The attribute this accessor is modifying is a read-only attribute
      which can not be modified in the kernel. Any changes to the
      attribute only have an effect on the local copy of the object. The
      accessor function is provided solely for the purpose of creating
      objects for comparison and filtering.

*/
