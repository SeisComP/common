*****************
Global parameters
*****************

The SeisComP configuration uses a unified schema to configure modules.
Modules which use the SeisComP libraries can read this configuration
directly and share global configuration options like messaging connections,
database configurations, logging and much more.
There are still some modules that do not use the libraries and are called
**standalone** modules such as :ref:`seedlink` and
:ref:`slarchive`.
They need wrappers to generate their native configuration when
:command:`seiscomp update-config` is run.

Global parameters can be used by many modules. Thus multiple configuration can be avoided.
The parameters are read from file in the following order:

#. :file:`@SYSTEMCONFIGDIR@/global.cfg`
#. :file:`@CONFIGDIR@/global.cfg`

Parameters from :file:`@CONFIGDIR@` override parameters from :file:`@SYSTEMCONFIGDIR@`.

Most :term:`trunk` modules read the configuration from the global configuration which can be overridden
by the module configuration. Read the :ref:`concept section on modules <concepts_modules>`
and the :ref:`concept section on the configuraion <concepts_configuration>` for
more details.

Though it is easy to create the configuration by directly editing the configuration
files, it is even more convenient to use a configurator.
SeisComP ships with a graphical configurator and management tool (:ref:`scconfig`)
which makes it easy to maintain
module configurations and station bindings even for large networks. It has built-in
functionality to check the state of all registered modules and to start and stop them.

The configuration is divided into three parts: :ref:`stations <global-stations>`,
:ref:`modules <global_modules>` and :ref:`bindings <global_bindings>`.


.. _global-stations:

Station meta data
=================

Station meta-data is a fundamental requirement for a seismic processing system and
for SeisComP. Read the :ref:`inventory section<concepts_inventory>` in concepts for
more details.


.. _global_modules:

Modules
=======

The concepts of :ref:`modules <concepts_modules>` and
:ref:`their configuration <global_modules_config>` is described in the
:ref:`concepts section <concepts>`.


.. _global_bindings:

Bindings
========

Bindings provide specific configurations per
:ref:`module <global_modules_config>` and station and even stream. Read the
:ref:`bindings section <global_bindings_config>` in concepts for more details on
bindings.


Extensions
==========

Extensions add new configuration options to :term:`modules<module>`. It does
not matter how those extensions are used. Commonly a module loads a plugin,
which requires additional configuration parameters - these are provided by an
extension.

There are currently extensions for the following modules, corresponding to the
plugins shown:

.. include:: /base/extensions.doc

See the documentation for each module for further information about its
extensions.



.. _global-configuration:
