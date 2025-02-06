Router is a meta locator which selects an actual
:ref:`locator <concepts_locators>` based on region profiles configured in
GeoJSON or BNA files.

The locator supports both, the initial location based on a pick set and the
relocation based on an existing origin. In case no origin is available an
initial solution is calculated by a configurable locator followed by a
relocation configured through region profiles.


Setup
=====

The Router locator offers configuration by global module parameters.


Plugin
------

Add the plugin ``locrouter`` to :confval:`plugins` for activating the Router
locator. Example:

.. code-block:: sh

   plugins = ${plugins},locrouter


Initial locator
---------------

For routing, an initial source location is required. When only picks but no
origins are provided, the initial location is unknown but it can be defined by
an initial locator independent of location. Set
:confval:`RouterLocator.initial.locator` and
:confval:`RouterLocator.initial.profile` for defining the initial locator.


Region Configuration
--------------------

Regions may be considered by configuring their names in
:confval:`RouterLocator.regions`. The regions themselves are defined as polygons
files in either :ref:`GeoJSON <sec-gui_layers-vector-format-geojson>` or
:ref:`BNA <sec-gui_layers-vector-format-bna>` format. Supported polygon
attributes are:

* name (recommended): Name of polygon. An empty string is assumed if not given.
* locator (mandatory): Name of the locator interface to use.
* profile: Name of the locator-specific profile which must be configured
  according to the selected locator.
* minDepth: Minimum depth in km the profile should be applied to.
* maxDepth: Maximum depth in km the profile should be applied to.

The configured features are sorted by rank and area. Larger ranks and smaller
areas are prioritized.

Example :ref:`GeoJSON file<sec-gui_layers-vector-format-geojson>`:


.. code-block:: json

   {
       "type": "FeatureCollection",
       "features": [
           {
               "type": "Feature",
               "properties": {
                   "name": "Iceland",
                   "minDepth": 0,
                   "maxDepth": 30,
                   "locator": "LOCSAT",
                   "profile": "iceland"
               },
               "geometry": {
                   "type": "Polygon",
                   "coordinates": [
                       [
                           [
                               -24.5469, 63.3967
                           ],
                           [
                               -13.4958, 63.3967
                           ],
                           [
                               -13.4958, 66.5667
                           ],
                           [
                               -24.5469, 66.5667
                           ],
                           [
                               -24.5469, 63.3967
                           ]
                       ]
                   ]
               }
           },
           {
               "type": "Feature",
               "properties": {
                   "name": "World",
                   "locator": "LOCSAT",
                   "profile": "iasp91"
               },
               "geometry": {
                   "type": "Polygon",
                   "coordinates": [
                       [
                           [
                               -33, 90
                           ],
                           [
                               -180, 90
                           ],
                           [
                               -180, -90
                           ],
                           [
                               -33, -90
                           ],
                           [
                               33, -90
                           ],
                           [
                               180, -90
                           ],
                           [
                               180, 90
                           ],
                           [
                               33, 90
                           ],
                           [
                               -33, 90
                           ]
                       ]
                   ]
               }
           }
       ]
   }

Example :ref:`BNA file<sec-gui_layers-vector-format-bna>`:

.. code-block:: properties

   "Iceland", "rank 1", "minDepth: 0, maxDepth: 30, locator: LOCSAT, profile: iceland", 4
   -24.5469, 63.3967
   -13.4958, 63.3967
   -13.4958, 66.5667
   -24.5469, 66.5667
   "World", "rank 1", "locator: LOCSAT, profile: iasp91", 8
   -33, 90
   -180, 90
   -180, -90
   -33, -90
   33, -90
   180, -90
   180, 90
   33, 90


Application
===========

Once configured, the Router locator may be used by other |scname| modules such
as :ref:`scolv` or :ref:`screloc`. Refer to the locator as "Router".
