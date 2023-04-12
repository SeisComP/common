LOCSAT is a locator with a travel-time interface in |scname| for computing
source time and hypocenter coordinates from phase picks considering:

* Pick time and pick uncertainty,
* Backazimuth and backazimuth uncertainty,
* Slowness and slowness uncertainty,
* Phase-specfic travel-time tables.

The LOCSAT :ref:`locator interface <locsat_li>` implements a wrapper for the
LocSAT locator by :cite:t:`bratt-1991` (according to the README file shipped
with the LocSAT distribution) referred to as **LOCSAT** in |scname|. The LOCSAT
:ref:`travel-time interface <locsat_tti>` provides travel time for specfic
phases, epicentral distance, soure depth and station elevation.


.. _locsat_li:

Locator Interface
=================

LOCSAT provides the hypocenter paramters through the locator interface.


.. _locsat_tti:

Travel-Time Interface
=====================

LOCSAT provides an interface for computing travel times based on coordinates and
depth. The times are plotted on waveforms, e.g., blue marks in
:ref:`scolv picker window <scolv-sec-waveform-review>`.

Use "LOCSAT" as a value for the travel-time interface when configurable, e.g.,
by :ref:`global_fixedhypocenter`.


.. _locsat_ttt:

Travel-Time Tables
==================

|scname| ships with two sets of predefined travel-time tables which are
made available as the profiles tab and iasp91.

The default profile is *iasp91*.

LOCSAT travel time tables are located as plain ascii files under
:file:`@DATADIR@/locsat/tables/`.
The tables provide the travel times for particular seismic phases at
given depth and epicentral distance in one file per Earth model and seismic
phase. E.g. P-wave arrival times in the iasp91 model are found in
:file:`@DATADIR@/locsat/tables/iasp91.P`. You may easily add your own tables
for any available Earth model and seismic phase by adopting existing ones in new
files which are added by :ref:`configuration <locsat_station_application>` to
your |scname| modules.


Limitations
-----------

#. Only phases for which a travel-time table exists can be considered.
#. LOCSAT currently considers travel-time tables for phases which are hard-coded

   * seismic body waves: P, Pg, Pb, Pn, Rg, pP, sP, PKP, PP, PKPab, PKPbc, PKPdf,
     SKPdf, PcP,
     S, Sg, Sb, Sn, Lg, SKS, SS, ScS,

     where P and S are the direct P and S phases, respectively, at all distances
     no matter the take-off angle at the source.
   * seismic surface waves: LQ, LR.
   * infrasound: Is, It, Iw.

#. The maximum number of distance and depth intervals per table file is
   currently 210 and 50, respectively.

   .. warning::

      * Travel-time tables with larger numbers of distance or depth samples are
        reported along with command-line error output (stderr). The travel-time
        tables should therefore be tested, e.g., with :ref:`scolv` before
        unsupervised application.
      * Travel times at distance and depth samples exceeding the limits are
        ignored. This may lead to undesired behavior during location.
      * Phase picks observed outside the distance and depth ranges defined by
        travel-time tables may lead to undesired behavior during location.

#. The considered minimum depth is 0 km. Elevations and depths above datum are
   not natively considered. The effects of station elevation can be
   :ref:`corrected for empirically <locsat_station_elevation>`.

.. _locsat_station_elevation:

Station elevations
------------------

LOCSAT does not natively support corrections of travel-time tables for station
elevations. At least checking the code:

.. code-block:: c

   sta_cor[i]  = 0.0;    /* FIX !!!!!!*/


However, the |scname| wrapper adds this feature. It allows to define a
:file:`.stacor` file which defines emperic corrections of observed travel times.
The corrections are provided in seconds and **subtracted** (not added) from
the observation time to be compatible with the NonLinLoc :cite:p:`nonlinloc`
station correction definitions.

Each LOCSAT profile (travel time table) can have one associated station
correction file. E.g. for adding station corrections to the iasp91 tables, the
file :file:`$SEISCOMP_ROOT/share/locsat/tables/iasp91.stacor` needs to be created.

A station correction table takes the form:

.. code-block:: params

   # LOCDELAY code phase numReadings delay
   LOCDELAY GE.MORC P 1 -0.1

with

- **code** (*string*) station code (after all alias evaluations)
- **phase** (*string*) phase type (any of the available travel time tables)
- **numReadings** (*integer*) number of residuals used to calculate mean residual/delay
  (not used by NLLoc, included for compatibility with the format of a summary,
  phase statistics file)
- **delay** (*float*) delay in seconds, subtracted from observed time

.. note::

   The fourth column (numReadings) is ignored and just provided for compatibility
   reasons with :ref:`NonLinLoc <global_nonlinloc>`.


.. _locsat_station_application:

Application and Setup
=====================

LOCSAT is the default and only locator for :ref:`scautoloc` with *iasp91* as the
default profile. However, LOCSAT can be used optionally in other modules such as
:ref:`scolv` or :ref:`screloc`.

#. Generate your travel-time tables from a custom Earth model, depth and
   distance intervals. Use the same format as the defaults as the *iasp91*
   tables. Tools such as :cite:t:`taup` allow the generation.
#. Add your custom travel-time tables along with station corrections to
   :file:`@DATADIR@/locsat/tables/`
#. Add your available custom LOCSAT travel-time tables in global configuration,
   e.g., to the list of tables of travel-time interfaces

   .. code-block:: params

      ttt.LOCSAT.tables = iasp91, tab, custom

   and to the list of locator profiles

   .. code-block:: params

      LOCSAT.profiles = iasp91, tab, custom

   and optionally to locators which make use of LOCSAT tables, e.g.,
   :ref:`global_fixedhypocenter`.

#. Configure a |scname| module with LOCSAT and a profile.

   * configure a profile in :ref:`scautoloc` for automatic locations,
   * configure *LOCSAT* along with a profile in :ref:`screloc` for automatically
     relocating,
   * configure *LOCSAT* along with a profile in :ref:`scolv` as defaults for
     interactive locations.
