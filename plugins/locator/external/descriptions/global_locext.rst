The ExternalLocator implements a wrapper for scripts which do the actual location
process. The input and output are represented as XML and communicated via the
input/output channels of the called process: stdin and stdout.

Plugin
======

To enable the ExternalLocator the plugin ``locext`` must be loaded.

Commandline parameters
======================

There are several commandline parameters passed to the script depending on
the locator configuration. The following table summarizes them.

=========================  ====================================================
Parameter                  Description
=========================  ====================================================
--max-dist=X               The cut-off distance if set
--ignore-initial-location  Whether to ignore the initial origin location or not
--fixed-depth=X            The depth in km to be fixed if enabled
=========================  ====================================================

Input
=====

The input document written to stdin of the child process is a valid SeisComP
XML document containing ``EventParameters``. The event parameters hold exactly
one origin to be relocated and all picks references from the origins arrivals.

Example:

.. code:: xml

   <?xml version="1.0" encoding="UTF-8"?>
   <seiscomp xmlns="http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/0.11" version="0.11">
     <EventParameters>
       <pick ...>...</pick>
       <pick ...>...</pick>
       ...
       <origin ...>
         ...
         <arrival>
           ...
         </arrival>
         <arrival>
           ...
         </arrival>
         ...
       </origin>
     </EventParameters>
   </seiscomp>

Output
======

The output is read from stdout and is expected to be a SeisComP XML document
just containing an origin.

Example:

.. code:: xml

   <?xml version="1.0" encoding="UTF-8"?>
   <seiscomp xmlns="http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/0.11" version="0.11">
     <Origin publicID="...">
     </Origin>
   </seiscomp>


Example configuration
=====================

.. code::

   ExternalLocator.profiles = locator1:"python /path/to/locator/script.py",\
                              locator2:/path/to/other/locator/script

