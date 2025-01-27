The RouterLocator is a meta locator which selects an actual locator based on
region profiles configured in GeoJSON or BNA files.

The locator supports both, the initial location based on a pick set and the
relocation based on an existing origin. In case no origin is available an
initial solution is calculated by a configurable locator followed by a
relocation configured through region profiles.


Plugin
======

To enable the RouterLocator the plugin ``locrouter`` must be loaded.

.. code-block:: sh

   plugins = ${plugins}, locrouter


Region Configuration
====================

Regions may be configured and defined by files either in GeoJSON or BNA format
:cite:t:`gui`. Supported polygon attributes are:

* locator (mandatory): Name of the locator interface to use
* profile: Name of the locator-specific profile
* minDepth: Minimum depth in km the profile should be applied to
* maxDepth: Maximum depth in km the profile should be applied to

The configured features are sorted by rank and area. Larger ranks and smaller
areas are prioritized.

Example GeoJSON file:

```json
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
                "profile": "sil"
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
```

Example BNA file:

```text
"Iceland", "rank 1", "minDepth: 0, maxDepth: 30, locator: LOCSAT, profile: sil", 4
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
```


Commandline Parameters
======================

