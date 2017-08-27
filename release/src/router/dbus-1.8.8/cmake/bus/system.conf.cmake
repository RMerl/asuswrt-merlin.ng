<!-- This configuration file controls the systemwide message bus.
     Add a system-local.conf and edit that rather than changing this 
     file directly. -->

<!-- Note that there are any number of ways you can hose yourself
     security-wise by screwing up this file; in particular, you
     probably don't want to listen on any more addresses, add any more
     auth mechanisms, run as a different user, etc. -->

<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>

  <!-- Our well-known bus type, do not change this -->
  <type>system</type>

  <!-- Run as special user -->
  <user>@DBUS_USER@</user>

  <!-- Fork into daemon mode -->
  <fork/>

  <!-- Write a pid file -->
  <pidfile>@DBUS_SYSTEM_PID_FILE@</pidfile>

  <!-- Only allow socket-credentials-based authentication -->
  <auth>EXTERNAL</auth>

  <!-- Only listen on a local socket. (abstract=/path/to/socket 
       means use abstract namespace, don't really create filesystem 
       file; only Linux supports this. Use path=/whatever on other 
       systems.) -->
  <listen>@DBUS_SYSTEM_BUS_DEFAULT_ADDRESS@</listen>

  <policy context="default">
    <!-- Deny everything then punch holes -->
    <deny send_interface="*"/>
    <deny receive_interface="*"/>
    <deny own="*"/>
    <!-- But allow all users to connect -->
    <allow user="*"/>
    <!-- Allow anyone to talk to the message bus -->
    <!-- FIXME I think currently these allow rules are always implicit 
         even if they aren't in here -->
    <allow send_destination="org.freedesktop.DBus"/>
    <allow receive_sender="org.freedesktop.DBus"/>
    <!-- valid replies are always allowed -->
    <allow send_requested_reply="true"/>
    <allow receive_requested_reply="true"/>
  </policy>

  <!-- Config files are placed here that among other things, punch 
       holes in the above policy for specific services. -->
  <includedir>system.d</includedir>

  <!-- This is included last so local configuration can override what's 
       in this standard file -->
  <include ignore_missing="yes">system-local.conf</include>

  <include if_selinux_enabled="yes" selinux_root_relative="yes">contexts/dbus_contexts</include>

</busconfig>
