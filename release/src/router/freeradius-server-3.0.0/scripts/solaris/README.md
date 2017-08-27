# Solaris startup scripts

SMF is the Solaris version of upstart (or the reverse), it imports XML configuration file for services, and manages service dependencies. It will automatically restart daemons in they die, and provides a standard interface for checking the status of a service and administratively disabling/enabling it.

# Installation/configuration

## Solaris 10
Do the following as the root user ``sudo -s``.

Copy the service management script ``svc-radius`` to ``/lib/srv/method/``:

```bash
cp ./svc-radius /lib/svc/method/
chown root:bin /lib/svc/method/svc-radius
chmod 555 /lib/svc/method/svc-radius
```

Copy the ``radius.xml`` manifest to ``/var/svc/manifest/network/``, and import it into SMF:

```bash
cp ./radius.xml /var/svc/manifest/network/
svccfg import /var/svc/manifest/network/radius.xml
```
### Authorizing additional users

First create an authorisation entry for the radius service:
```bash
echo "solaris.smf.manage.radius/server:::FreeRADIUS Server management::" >> /etc/security/auth_attr
```

Next instruct SMF to use RBAC for authorising actions on this particular service (only works with local accounts):
```bash
svccfg -s radius setprop general/action_authorization=astring: 'solaris.smf.manage.radius/server'
```

Then assign this authorisation to our one or more users:
```bash
usermod -A solaris.smf.manage.radius/server <user>
```

And finally test with (as authorized user):
```bash
svcs radius
```
