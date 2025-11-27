|scname| applications access waveform data through the RecordStream interface.
The following tables lists available implementations:

.. csv-table::
   :header: "Name", "URL Scheme(s)", "Description"

   ":ref:`rs-balanced`", "``balanced``", "Distributes requests to multiple proxy streams"
   ":ref:`rs-routing`", "``routing``", "Distributes requests to multiple proxy streams according to user defined rules"
   ":ref:`rs-caps`", "``caps``, ``capss``", "Connects to a `gempa CAPS server <https://www.gempa.de/products/caps/>`_"
   ":ref:`rs-combined`", "``combined``", "Combines archive and real-time stream"
   ":ref:`rs-dec`", "``dec``", "Decimates (downsamples) a proxy stream"
   ":ref:`rs-fdsnws`", "``fdsnws``, ``fdsnwss``", "Connects to :ref:`FDSN web service <fdsnws>`"
   ":ref:`rs-file`", "``file``", "Reads records from file"
   ":ref:`rs-memory`", "``memory``", "Reads records from memory"
   ":ref:`rs-resample`", "``resample``", "Resamples (up or down) a proxy stream to a given sampling rate"
   ":ref:`rs-sdsarchive`", "``sdsarchive``", "Reads records from |scname| archive (:term:`SDS`)"
   ":ref:`rs-slink`", "``slink``", "Connects to :ref:`SeedLink server <seedlink>`"


Application
===========

The RecordStream parameters considered by an application are provided as a *URL*
in 2 alternative ways:

* Specification of the *URL* on the command line. Use the option ``-I URL``
* Configuration of the *URL* using the global parameter :confval:`recordstream`.

The URL scheme defines the specific RecordStream implementation. If the scheme
is omitted, the :ref:`rs-file` implementation is used as default.

.. note::

   Older SeisComP versions used to split the URL into the parameters
   :confval:`recordstream.service` and :confval:`recordstream.source`.
   These parameters are not being used anymore.


Implementations
===============

.. _rs-slink:


SeedLink
--------

This RecordStream fetches data from a SeedLink server.


Definition
^^^^^^^^^^

URL: ``slink://[host][:port][?parameter]``

The default host is set to `localhost`, the default port to `18000`. Optional
URL encoded parameters are:

- `timeout` - connection timeout in seconds, default: 300
- `retries` - number of connection retry attempts, default: -1
- `no-batch` - disables BATCH mode to request data, does not take a value


Examples
^^^^^^^^

- ``slink://``
- ``slink://geofon.gfz.de?timeout=60&retries=5``
- ``slink://localhost:18000``


FDSNWS
------

This RecordStream fetches data from a FDSN web service.


Definition
^^^^^^^^^^

URL: ``fdsnws[s]://host[:port][path]``

The host is a mandatory parameter. The default port depends on the URL scheme
used:

- `fdsnws`: `80` (HTTP)
- `fdsnwss`: `443` (HTTPS)

The default path is set to `/fdsnws/dataselect/1/query`. If a path is specified,
it needs to be complete up until the `query` resource.

Authentication via the `queryauth` resource is currently not supported.


Examples
^^^^^^^^

- ``fdsnws://service.iris.edu``
- ``fdsnws://service.iris.edu:80/fdsnws/dataselect/1/query``
- ``fdsnwss://geofon.gfz.de``

.. _rs-file:


File
----

This RecordStream reads data from a file.


Definition
^^^^^^^^^^

URL: ``file://path``

The path may be a absolute or relative path to a file. Environment variables
are **not** resolved. If path is set to ``-`` the data is read from `stdin`.

Supported files types are:

* miniSEED
* SAC
* XML
* binary

By default the record type is set to `mseed`. SAC data can be read using the *#sac*
descriptor. If a file name extension is available, then the record type is set as
follows:

========= ===========
Extension Record Type
========= ===========
`*.xml`   `xml`
`*.bin`   `binary`
`*.mseed` `mseed`
========= ===========

Optional descriptor:

- `sac` - input data are in SAC format.


Examples
^^^^^^^^

- ``file://-``
- ``file:///tmp/input.mseed``
- ``file:///tmp/input.sac#sac``

.. note ::

   When defining the File RecordStream on the command line using the option `-I`,
   the file name can also be passed without the URL scheme, e.g. ::

      -I -
      -I /tmp/input.mseed


.. _rs-sdsarchive:

SDSArchive
----------

This RecordStream reads data from one or more |scname| (:term:`SDS`) archives using the
:ref:`rs-file` RecordStream.


Definition
^^^^^^^^^^

URL: ``sdsarchive://[path[,path2[, ...]]]``

The default path is set to `$SEISCOMP_ROOT/var/lib/archive`.

In contrast to a formal URL definition, the URL path is interpreted as a directory path list
separated by commas.

.. note::

   When defining multiple directories separated by comma in a configuration
   file, please enclose the entire definition (including ``sdsarchive://`` with
   double quotes. Otherwise the configuration parser will interpret it as a list
   and will only return the first part up to the first comma.

Different SDS archives are not merged, but are read sequentially depending on
data existence. If a requested file is missing in the current SDS archive, it is
searched for in the archive next in the list. On success it will deliver all
the rest of files for the current channel from this SDS archive. On failure the
next SDS archive is searched.

This process is repeated for each requested channel individually. It always
starts to search data from the first given SDS to the last one, for each data
channel.


Examples
^^^^^^^^

- ``sdsarchive://``
- ``sdsarchive:///home/sysop/seiscomp/var/lib/archive``
- ``sdsarchive:///SDSA,/SDSB,/SDSC``

.. _rs-caps:


CAPS
----

This RecordStream reads data from a
`gempa CAPS server <https://www.gempa.de/products/caps/>`_.

Definition
^^^^^^^^^^

URL: ``caps[s]://[user:pass@][host[:port]][?parameters]``

The default host is set to `localhost`. The default port depends on the URL scheme
used:

- `caps`: `18002`
- `capss`: `18022` (SSL)

Optional URL encoded parameters are:

- `arch` - No parameter. Retrieve only archived data. In this mode the connection
   finished when all available data has been sent. It won't wait for additional
   real-time data.
- `ooo` - Allow out-of-order data
- `timeout` - The socket timeout in seconds
- `user` - **Deprecated:** The user name of an authenticated request. Please use
   the standard URL userinfo in front of the host instead.
- `pwd` - **Deprecated:** The password of an authenticated request. Please use
   the standard URL userinfo in front of the host instead.
- `request-file` - Use the given file to feed the request


Example
^^^^^^^

- ``caps://``
- ``caps://localhost:18002``
- ``capss://localhost:18022``
- ``caps://localhost:18002?arch``
- ``capss://user:mysecret@localhost``

.. _rs-memory:


Memory
------

This RecordStream reads data from memory and is only useful for developing
applications. For instance a record sequence stored in an internal buffer could
be passed to an instance of this RecordStream for reading.

.. _rs-combined:


Combined
--------

This RecordStream combines one archive and one real-time RecordStream, e.g.
:ref:`rs-arclink` and :ref:`rs-slink`. First the archive stream is read up to
the size of the real-time buffer. Then the acquisition is switched to the
real-time stream. The syntax for the source is similar to an URL:


Definition
^^^^^^^^^^

URL-like: ``combined://[real-time-stream];[archive-stream][??parameters]``

By default the real-time stream is set to :ref:`rs-slink` and the
archive-stream is set to :ref:`rs-arclink`. Any other streams may be configured.

The definition of the proxy streams has slightly changed: Scheme and source are
only separated by a slash, e.g. `slink://localhost` needs to be defined as
`slink/localhost`.

The URL parameters of the combined stream are separated by 2 question marks
(`??`) in order to distinguish them from the parameters used in the proxy
streams:

- `slinkMax|rtMax|1stMax` - Buffer size in seconds of the first stream
  (typically the real-time stream), default: 3600

  Time spans can be configured with an additional and optional suffix:

  ======  =============
  Suffix  Multiplicator
  ======  =============
  s       1
  m       60
  h       3600
  d       86400
  w       86400*7
  ======  =============

- `splitTime` - The absolute time of the separation of both sources. The argument
  is an ISO time string, e.g. 2018-05-10T12:00:00Z or a year, e.g. 2018, which is
  the same as 2018-01-01T00:00:00.000Z.
  `splitTime` can be used if the waveform archives are spread over several
  directories or hard disks. See also the :ref:`examples<rs_splitTime>`.

The combined record stream may be nested allowing the configuration of a
(theoretically) infinite number of archive streams. The URL syntax for a nested
configuration uses parenthesis:

``combined://real-time-stream;combined/(archive-stream1;archive-stream2??parameters)??parameters``

.. _rs_splitTime:


Examples
^^^^^^^^

.. csv-table::
   :header: "URL", "Description"

   "``combined://localhost:18000;localhost:18001``", "Seedlink on localhost:18000 combined with Arclink on localhost 18001"
   "``combined://slink/localhost:18000;arclink/localhost:18001``", "Same as above"
   "``combined://;``", "Same as above"
   "``combined://:18042;?user=foo&pwd=secret??rtMax=1800``", "Seedlink on localhost:18042 combined with Arclink on localhost 18001, real-time (SeedLink) buffer size set to 30min"
   "``combined://slink/localhost:18000;sdsarchive//home/sysop/seiscomp/var/lib/archive``", Seedlink combined with SDS archive
   "``combined://slink/localhost:18000;combined/(arclink/localhost:18001;arclink/localhost:18002??1stMax=30d)??1stMax=1h``", Seedlink combined with a combined record stream using two Arclink sources
   "``combined://slink/localhost:18000;combined/(sdsarchive//home/sysop/seiscomp/var/lib/archive;combined/(sdsarchive//home/sysop/seiscomp/var/lib/archive2017;sdsarchive//home/sysop/seiscomp/var/lib/archive2016??splitTime=2017)??splitTime=2018)``", "Seedlink combined with a combined recordStream providing access to 3 different SDS archives separated by time. The first SDS archive contains the most recent archived data. The other two contain the data from 2016 and 2017."
   "``combined://slink/localhost:18000;combined/(sdsarchive//home/sysop/seiscomp/var/lib/archive;combined/(sdsarchive//home/sysop/seiscomp/var/lib/archive2017;sdsarchive//home/sysop/seiscomp/var/lib/archive2016??splitTime=2017-06-01T00:00:00Z)??splitTime=2018-06-01T00:00:00Z)``", "Seedlink combined with a combined recordStream providing access to 3 different SDS archives separated by time. The first SDS archive contains the most recent archived data. The other two are separated in mid of 2016."

.. _rs-balanced:


Balanced
--------

This RecordStream distributes requests quasi-equally (but deterministically) to
multiple proxy streams. It can be used for load balancing and to improve failure
tolerance. The algorithm to choose a proxy stream (counting from 0) is based on
station code and can be expressed in Python as follows:

.. code-block:: python

   stationCode = "WLF"
   nproxies = 2

   x = 0
   for c in stationCode:
       x += ord(c)

   print("choosing proxy stream", x % nproxies)


Definition
^^^^^^^^^^

URL-like: ``balanced://proxy-stream[;proxy-stream2[; ...]]``

The definition of the proxy streams has slightly changed: Scheme and source
are only separated by a slash, e.g. `slink://localhost` needs to be defined as
`slink/localhost`.


Examples
^^^^^^^^

.. csv-table::
   :header: "URL", "Description"

   "``balanced://slink/server1:18000;slink/server2:18000``", "Distribute requests to 2 :ref:`rs-slink` RecordStreams"
   "``balanced://combined/(server1:18000;server1:18001);combined/(server2:18000;server2:18001)``", "Distribute requests to 2 :ref:`rs-combined` RecordStreams"

.. _rs-routing:


Routing
--------

This RecordStream distributes requests to multiple proxy streams according to
user supplied routing rules, which allow to route specific network, station,
location or channel codes to fixed proxy streams.

Definition
^^^^^^^^^^

URL-like: ``routing://proxy-stream??match=pattern[;proxy-stream2??match=pattern[; ...]]``
    
The definition of the proxy streams has slightly changed: Scheme and source
are only separated by a slash, e.g. `slink://localhost` needs to be defined as
`slink/localhost`.

The URL parameters of the routing stream are separated by 2 question marks
(`??`) in order to distinguish them from the parameters used in the proxy
streams.

`pattern` defines the rule used to route the request to the proxy stream and it is
in `NET.STA.LOC.CHA` format. The special characters `?` `*` `|` `(` `)` are allowed.

Examples
^^^^^^^^

.. csv-table::
   :header: "URL", "Description"

   "``routing://slink/server1:18000??match=(NET1|NET2).*.*.*;slink/server2:18000??match=*.*.*.*``", "Requests for network `NET1` and `NET2` go to server1, all the rest to server2"
   "``routing://slink/server1:18000??match=TMP?.*.*.*;slink/server2:18000??match=NET.*.*.*``", "Requests for network `TMPX` go to server1, for network `NET` go to server 2, all the rest are not fulfilled"
   "``routing://slink/server1:18000??match=*.*.*.(HH|EH)?;slink/server2:18000??match=*.*.*.*``", "Requests for channels `HH` and `EH` go to server1, all the rest to server2"   
   "``routing://combined/(server1:18000;server1:18001??rtMax=1800)??match=NET1.*.*.*;combined/(server2:18000;server2:18001??rtMax=1800)??match=NET2.*.*.*``", "Split requests to 2 :ref:`rs-combined` RecordStreams according to the network code `STA1` or `STA2`. Other network codes are not fullfilled"
   "``routing://combined/(slink/special-server:18000;sdsarchive//home/sysop/seiscomp/var/lib/special-archive)??match=SP.*.*.*;combined/(slink/default-server:18000;sdsarchive//home/sysop/seiscomp/var/lib/default-archive)??match=*.*.*.*``", "Requests for special network `SP` are fullfilled by seedlink `special-server` and sdsarchive `/home/sysop/seiscomp/var/lib/special-archive`, all the rest are fullfilled by seedlink `default-server` and archive `/home/sysop/seiscomp/var/lib/default-archive`"
   
.. _rs-dec:


Decimation
----------

This RecordStream decimates (downsamples) a proxy stream, e.g. :ref:`rs-slink`.


Definition
^^^^^^^^^^

URL-like: ``dec://proxy-stream-scheme[?dec-parameters]/[proxy-stream-source]``

The definition of the proxy streams has slightly changed: Scheme and source are
only separated by a slash, e.g. `slink://localhost` needs to be defined as
`slink/localhost`. Also optional decimation parameters directly follow the proxy
stream scheme.

Optional decimation parameters are:

- `rate` - target sampling rate in Hz, default: 1
- `fp` - default: 0.7
- `fs` - default: 0.9
- `cs` - coefficient scale, default: 10


Examples
^^^^^^^^

- ``dec://slink/localhost:18000``
- ``dec://file?rate=2/-``
- ``dec://combined/;``

.. _rs-resample:


Resample
--------

This RecordStream resamples (up or down) a proxy stream, e.g. :ref:`rs-slink`,
to a given sampling rate.


Definition
^^^^^^^^^^

URL-like: ``resample://proxy-stream-scheme[?dec-parameters]/[proxy-stream-source]``

The definition of the proxy streams has slightly changed: Scheme and source are
only separated by a slash, e.g. `slink://localhost` needs to be defined as
`slink/localhost`. Also optional decimation parameters directly follow the proxy
stream scheme.

Optional resample parameters are:

- `rate` - target sampling rate in Hz, default: 1
- `fp` - default: 0.7
- `fs` - default: 0.9
- `cs` - coefficient scale, default: 10
- `lw` - lanczos kernel width, default: 3
- `debug` - enables debug output, default: false


Examples
^^^^^^^^

- ``resample://slink/localhost:18000``
- ``resample://file?rate=2/-``
- ``resample://combined/;``
