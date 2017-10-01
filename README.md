# SkiRoutingServer

Routing server that calculates routes based on two GPS positions using data gathered from Open Street map. The data of the resort Skiwelt Wilder Kaiser is contained in the repository.

You can start the server with the command `SkiServer -g Data/kaiser.gr.bin`.
You can query the server via `host:15235/query?slat=0.0&slng=0.0&tlat=1.0&tlng=1.0`, which yields a JSON representation of the route from slat/slng to tlat/tlng.
