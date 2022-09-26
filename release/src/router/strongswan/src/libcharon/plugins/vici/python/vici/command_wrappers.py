class CommandWrappers(object):
    def version(self):
        """Retrieve daemon and system specific version information.

        :return: daemon and system specific version information
        :rtype: dict
        """
        return self.request("version")

    def stats(self):
        """Retrieve IKE daemon statistics and load information.

        :return: IKE daemon statistics and load information
        :rtype: dict
        """
        return self.request("stats")

    def reload_settings(self):
        """Reload strongswan.conf settings and any plugins supporting reload.
        """
        self.request("reload-settings")

    def initiate(self, sa):
        """Initiate an SA.

        :param sa: the SA to initiate
        :type sa: dict
        :return: generator for logs emitted as dict
        :rtype: generator
        """
        return self.streamed_request("initiate", "control-log", sa)

    def terminate(self, sa):
        """Terminate an SA.

        :param sa: the SA to terminate
        :type sa: dict
        :return: generator for logs emitted as dict
        :rtype: generator
        """
        return self.streamed_request("terminate", "control-log", sa)

    def rekey(self, sa):
        """Initiate the rekeying of an SA.

        .. versionadded:: 5.5.2

        :param sa: the SA to rekey
        :type sa: dict
        :return: number of matched SAs
        :rtype: dict
        """
        return self.request("rekey", sa)

    def redirect(self, sa):
        """Redirect an IKE_SA.

        .. versionchanged:: 5.5.2
           The number of matched SAs is returned.

        :param sa: the SA to redirect
        :type sa: dict
        :return: number of matched SAs
        :rtype: dict
        """
        return self.request("redirect", sa)

    def install(self, policy):
        """Install a trap, drop or bypass policy defined by a CHILD_SA config.

        :param policy: policy to install
        :type policy: dict
        """
        self.request("install", policy)

    def uninstall(self, policy):
        """Uninstall a trap, drop or bypass policy defined by a CHILD_SA config.

        :param policy: policy to uninstall
        :type policy: dict
        """
        self.request("uninstall", policy)

    def list_sas(self, filters=None):
        """Retrieve active IKE_SAs and associated CHILD_SAs.

        :param filters: retrieve only matching IKE_SAs (optional)
        :type filters: dict
        :return: generator for active IKE_SAs and associated CHILD_SAs as dict
        :rtype: generator
        """
        return self.streamed_request("list-sas", "list-sa", filters)

    def list_policies(self, filters=None):
        """Retrieve installed trap, drop and bypass policies.

        :param filters: retrieve only matching policies (optional)
        :type filters: dict
        :return: generator for installed trap, drop and bypass policies as dict
        :rtype: generator
        """
        return self.streamed_request("list-policies", "list-policy",
                                     filters)

    def list_conns(self, filters=None):
        """Retrieve loaded connections.

        :param filters: retrieve only matching configuration names (optional)
        :type filters: dict
        :return: generator for loaded connections as dict
        :rtype: generator
        """
        return self.streamed_request("list-conns", "list-conn",
                                     filters)

    def get_conns(self):
        """Retrieve connection names loaded exclusively over vici.

        :return: connection names
        :rtype: dict
        """
        return self.request("get-conns")

    def list_certs(self, filters=None):
        """Retrieve loaded certificates.

        :param filters: retrieve only matching certificates (optional)
        :type filters: dict
        :return: generator for loaded certificates as dict
        :rtype: generator
        """
        return self.streamed_request("list-certs", "list-cert", filters)

    def list_authorities(self, filters=None):
        """Retrieve loaded certification authority information.

        .. versionadded:: 5.3.3

        :param filters: retrieve only matching CAs (optional)
        :type filters: dict
        :return: generator for loaded CAs as dict
        :rtype: generator
        """
        return self.streamed_request("list-authorities", "list-authority",
                                     filters)

    def get_authorities(self):
        """Retrieve certification authority names loaded exclusively over vici.

        :return: CA names
        :rtype: dict
        """
        return self.request("get-authorities")

    def load_conn(self, connection):
        """Load a connection definition into the daemon.

        :param connection: connection definition
        :type connection: dict
        """
        self.request("load-conn", connection)

    def unload_conn(self, name):
        """Unload a connection definition.

        :param name: connection definition name
        :type name: dict
        """
        self.request("unload-conn", name)

    def load_cert(self, certificate):
        """Load a certificate into the daemon.

        :param certificate: PEM or DER encoded certificate
        :type certificate: dict
        """
        self.request("load-cert", certificate)

    def load_key(self, private_key):
        """Load a private key into the daemon.

        .. versionchanged:: 5.5.3
           The key identifier of the loaded key is returned.

        :param private_key: PEM or DER encoded key
        :type private_key: dict
        :return: key identifier
        :rtype: dict
        """
        return self.request("load-key", private_key)

    def unload_key(self, key_id):
        """Unload the private key with the given key identifier.

        .. versionadded:: 5.5.2

        :param key_id: key identifier
        :type key_id: dict
        """
        self.request("unload-key", key_id)

    def get_keys(self):
        """Retrieve identifiers of private keys loaded exclusively over vici.

        .. versionadded:: 5.5.2

        :return: key identifiers
        :rtype: dict
        """
        return self.request("get-keys")

    def load_token(self, token):
        """Load a private key located on a token into the daemon.

        .. versionadded:: 5.5.2

        :param token: token details
        :type token: dict
        :return: key identifier
        :rtype: dict
        """
        return self.request("load-token", token)

    def load_shared(self, secret):
        """Load a shared IKE PSK, EAP or XAuth secret into the daemon.

        .. versionchanged:: 5.5.2
           A unique identifier may be associated with the secret.

        :param secret: shared IKE PSK, EAP or XAuth secret
        :type secret: dict
        """
        self.request("load-shared", secret)

    def unload_shared(self, identifier):
        """Unload a previously loaded shared secret by its unique identifier.

        .. versionadded:: 5.5.2

        :param identifier: unique identifier
        :type secret: dict
        """
        self.request("unload-shared", identifier)

    def get_shared(self):
        """Retrieve identifiers of shared keys loaded exclusively over vici.

        .. versionadded:: 5.5.2

        :return: identifiers
        :rtype: dict
        """
        return self.request("get-shared")

    def flush_certs(self, filter=None):
        """Flush the volatile certificate cache.

        Flush the certificate stored temporarily in the cache. The filter
        allows to flush only a certain type of certificates, e.g. CRLs.

        :param filter: flush only certificates of a given type (optional)
        :type filter: dict
        """
        self.request("flush-certs", filter)

    def clear_creds(self):
        """Clear credentials loaded over vici.

        Clear all loaded certificate, private key and shared key credentials.
        This affects only credentials loaded over vici, but additionally
        flushes the credential cache.
        """
        self.request("clear-creds")

    def load_authority(self, ca):
        """Load a certification authority definition into the daemon.

        :param ca: certification authority definition
        :type ca: dict
        """
        self.request("load-authority", ca)

    def unload_authority(self, ca):
        """Unload a previously loaded certification authority by name.

        :param ca: certification authority name
        :type ca: dict
        """
        self.request("unload-authority", ca)

    def load_pool(self, pool):
        """Load a virtual IP pool.

        Load an in-memory virtual IP and configuration attribute pool.
        Existing pools with the same name get updated, if possible.

        :param pool: virtual IP and configuration attribute pool
        :type pool: dict
        """
        return self.request("load-pool", pool)

    def unload_pool(self, pool_name):
        """Unload a virtual IP pool.

        Unload a previously loaded virtual IP and configuration attribute pool.
        Unloading fails for pools with leases currently online.

        :param pool_name: pool by name
        :type pool_name: dict
        """
        self.request("unload-pool", pool_name)

    def get_pools(self, options=None):
        """Retrieve loaded pools.

        :param options: filter by name and/or retrieve leases (optional)
        :type options: dict
        :return: loaded pools
        :rtype: dict
        """
        return self.request("get-pools", options)

    def get_algorithms(self):
        """List of currently loaded algorithms and their implementation.

        .. versionadded:: 5.4.0

        :return: algorithms
        :rtype: dict
        """
        return self.request("get-algorithms")

    def get_counters(self, options=None):
        """List global or connection-specific counters for several IKE events.

        .. versionadded:: 5.6.1

        :param options: get global counters or those of all or one connection
        :type options: dict
        :return: counters
        :rtype: dict
        """
        return self.request("get-counters", options)

    def reset_counters(self, options=None):
        """Reset global or connection-specific IKE event counters.

        .. versionadded:: 5.6.1

        :param options: reset global counters or those of all or one connection
        :type options: dict
        """
        self.request("reset-counters", options)
