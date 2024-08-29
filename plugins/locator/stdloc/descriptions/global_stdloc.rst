StdLoc is a SeisComP locator plugin that combines standard location methods
and was developed with the focus on local seismicity, although the methods
are generic enough to work at larger scales as well.

Plugin
======

To enable StdLoc the plugin ``stdloc`` must be loaded.


How does it work?
=================

The locator can apply a multitude of location methods and it is particularly useful to
combine them to achieve better solutions:

- LeastSquares: this is the classic algorithm that solves the linearized problem of
  travel time residual minimization via iterative least squares. However an initial
  location estimate is required. This is the intended method to select when StdLoc
  is used in combination with a pick associator: it provides the initial location
  estimate and StdLoc will improve it. When used in :ref:`scolv` or :ref:`screloc`,
  the location of the origin to be relocated is used as starting estimate.
  The configuration doesn't require any mandatory parameters:

   .. code-block:: params

      method = LeastSquares

- GridSearch: finds the source parameters by evaluating the hypocenter probability
  of each point in a grid and returning the maximum likelihood hypocenter.
  Because the search space is fully evaluated there is no need for an initial
  location estimate and the location uncertainty is completely known. However the
  method is very slow. It can be used to relocate events in :ref:`scolv` that seem
  difficult to locate via other methods or to verify the uncertainty of a solution.
  The following example configuration computes a grid search around the average
  location of the picked stations. The grid points are spaced apart 0.5km
  horizontally and 2km vertically.

   .. code-block:: params

      method = GridSearch
      GridSearch.center = auto,auto,15
      GridSearch.size = 40,40,30
      GridSearch.numPoints = 81,81,16

- GridSearch+LeastSquares: this method can be used in very complex networks where
  a bad initial location estimates can get LeastSquares stuck in a local minimum.
  The method finds a LeastSquares solution for each cell in a (coarse) grid, using
  the cell centroid as initial location estimate. If finally returns the maximum
  likelihood solution. This method is intended to be used in :ref:`screloc` or 
  :ref:`scolv`  to relocate existing events.
  The following example configuration returns the best among the 75 (5x5x3)
  LeastSquares solutions, computed for every point in the grid.

   .. code-block:: params

      method = GridSearch+LeastSquares
      GridSearch.center = auto,auto,15
      GridSearch.size = 100,100,30
      GridSearch.numPoints = 5,5,3

- OctTree: this method produces similar results to GridSearch but it is extremely
  faster and it follows the NonLinLoc approach. The OctTree search starts by
  evaluating the hypocenter probability of each cell in a grid, computed as the
  probability density at the cell center coordinates times the cell volume. The
  search then continues by repeatedly fetching the  cell with highest probability
  and splitting it in 8 sub-cells. These 8 cells are then inserted in the pool of
  cells to fetch from at next iteration.
  The search terminates after either a maximum number of iterations or after
  reaching a minimum cell size. At that point the maximum likelihood hypocenter
  is selected. Because the algorithms splits only the cells with higher
  probability, the search space is sampled in a very efficient way and it makes
  the method way faster than GridSearch.
  This method is intended to be used in :ref:`screloc` or :ref:`scolv` to
  relocate existing events.
  The following example is a plausible configuration for the entire Swiss
  network:

   .. code-block:: params

      method = OctTree
      GridSearch.center = 47.0,8.5,50
      GridSearch.size = 700,700,100
      GridSearch.numPoints = 21,21,11
      OctTree.maxIterations = 100000
      OctTree.minCellSize = 0.001

  However in this example we are at the size limit for a flat earth study
  geometry and for bigger regions `GridSearch.center` should be set to
  `auto` and `GridSearch.size` to a smaller size. 

- OctTree+LeastSquares: this method allows the OctTree search to find the
  maximum probability cell in the network and uses that as the initial
  location estimate for LeastSquares. 
  This method is intended to be used in :ref:`screloc` or :ref:`scolv` to
  relocate existing events.
  The following example is a plausible configuration for the entire Swiss
  network:

   .. code-block:: params

      method = OctTree+LeastSquares
      GridSearch.center = 47.0,8.5,50
      GridSearch.size = 700,700,100
      GridSearch.numPoints = 21,21,11
      OctTree.maxIterations = 10000
      OctTree.minCellSize = 1.0

  However in this example we are at the size limit for a flat earth study
  geometry and for bigger regions `GridSearch.center` should be set to
  `auto` and `GridSearch.size` to a smaller size.

The algorithms implemented in StdLoc are standard methods described in "Routine Data
Processing in Earthquake Seismology" by Jens Havskov and Lars Ottemoller. The OctTree
search algorithm is based on NonLibLoc by Antony Lomax.



Why is stdloc suitable for local seismicity?
============================================

When dealing with very local seismicity (few kilometers or hundreds of meters) 
simplifications that are common for regional seismicity have to be removed. 
In particular the locator should take into consideration:

- station elevation and even negative elevation (e.g. borehole sensors)
- earthquake location can be above a seismic sensor (e.g. borehole sensors)
- possible negative earthquake depth (above surface)

More importantly the travel time tables used by the locator must be able to take
into consideration all the above too.


Travel Time Table
=================

StdLoc can be configured with any Travel Time Table type available in SeisComP,
however only the `homogeneous` type is able to take into consideration station
elevation, negative source depth and sources happening above stations. For this
reason `homogeneous` should be the preferred choice when working on very local
seismicity and especially with borehole sensors.



