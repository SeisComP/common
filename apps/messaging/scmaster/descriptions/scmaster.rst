scmaster is the implementation of the :ref:`messaging <concepts_messaging>`
mediator.


.. _section-scmaster-groups:

Message Groups
==============

scmaster provides the :ref:`message groups <messaging-groups>`. Configure

* :confval:`defaultGroups`: Add the groups which can be used by all queues.
* :confval:`queues.$name.groups`: Set all groups which are used by the given
  queue. You may inherit :confval:`defaultGroups`, e.g.: ::

     queues.production.groups = ${defaultGroups},L1PICK

  .. warning ::

     Setting any value without inheriting :confval:`defaultGroups` ignores all
     values of :confval:`defaultGroups`.


Queues
======

scmaster provides *queues* for separating the processing.
Typically, the default queue *production* is used. To add new queues

#. Define a new queue by adding a new profile with some name,
#. Configure the profile parameters :confval:`queues.$name.*`,
#. Register the queue in :confval:`queues`.


Scheme
======

scmaster provides unsecured and secured connection which is addressed by the
scheme values *scmp* and *scmps*, respectively, in :confval:`connection.server`
when connecting to the messaging.
Read the :ref:`concepts section <messaging-scheme>` for more details. *scmps*
is in use when configuring :confval:`interface.ssl.bind`.


Database Access
===============

scmaster reads from and writes to the database and reports the database connection
to the clients of the messaging system (compare with the :ref:`concepts section <messaging-db>`).

The database is configured per queue.


Single Machine
--------------

When running all |scname| modules on a single machine, the read and write
parameters are typically configured with *localhost* as a *host name*.

Example: ::

   queues.production.processors.messages.dbstore.read = sysop:sysop@localhost/seiscomp
   queues.production.processors.messages.dbstore.write = sysop:sysop@localhost/seiscomp


Multiple Machines
-----------------

If the clients are located on machines different from the messaging, the
*host name* of the read parameter
must be available on the client machine and the client machine must be able to
connect to the host with its name. If the database is on the same machine as the
messaging, the *host name* of the write connection typically remains *localhost*.

Example for connecting clients on computerB to the messaging on computerA (compare
with the :ref:`concepts section <messaging-distribution>`).

* Configuration of scmaster on computerA: ::

     queues.production.processors.messages.dbstore.read = sysop:sysop@computerA/seiscomp
     queues.production.processors.messages.dbstore.write = sysop:sysop@localhost/seiscomp

* Global configuration of client on computerB: ::

     connection.server = computerA/production

Database Proxy
--------------

scmaster can accept database requests and forward results to clients without
exposing the underlying database. That allows clients to connect to the database
of a particular queue via the Websocket HTTP protocol. No specific database
plugin is required at the client which reduces the complexity of configuration.

Be aware that due to the nature of a proxy which is another layer on top of the
actual database connection the performance is not as high as direct database
access.

To let scmaster return the proxy address of the database connection, set

.. code::

   queues.production.processors.messages.dbstore.proxy = true

in the configuration file.


Access Control
==============

scmaster does not provide any built-in access control to connecting clients.
The only exception is the possibility to verify client certificates against
the server certificate if SSL is enabled.

.. code::

   interface.ssl.verifyPeer = true

It is required that the client certificate is signed by the server certificate
otherwise the client connection will be rejected.
