Rendering
=========
	+--------------------------------------+
	|       +-------+                      |
	|       |  . - -|------------------+   |
	|       |  .    |                  |   |
	|       |  . 3  |          +-------| - | -.
	|   1   |  .    |   2      | 4     |   |  .
	|       +-Portl-+          +-Prtl--| - | -.
	|          |                       |   |
	|          +---Portal--------------+   |
	|                                      |
	+----Screen----------------------------+


- Create graph of visible portals
	  / 3
	1
	  \ 2 -- 4

- Render current scene including portals into color and depth
- For each child node (portal)
	- Clear stencil
	- Render portal mesh to stencil
	- 'Clear' depth by drawing quad at far plane
		- This will reset the depth within the stencilled area
	- Set clip plane
	- Render destination scene

- Recurse to next set of children
	- Since the parent depth buffer is preserved, further portals
		should be obscured properly

Note: This method will not work well if destination scenes use the stencil buffer for their own
purposes. It might be worth looking into a method of rendering to a texture first.
So long as depth information is kept properly this should work fine.

It may also be worth figuring out how to extend this method to rendering mirrors also

