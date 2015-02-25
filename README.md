Based on:
https://github.com/McSimp/Borderlands2SDK

Early alpha stage.

Open with VS community 2013, compile (make sure the Release configuration is active), run.

bl2monitor (the server) will indicate that it is waiting for the target (borderlands2).
Launch bl2 (standalone or steam version), the server will update its status.
You can check that the server is working by pressing F1 in game, this will log a message in the server gui.
That's it.. for now.

Notes:
- removed dependency to detour
- new launcher (no need to launch the game from the gui)
- no d3d hook yet
- no lua capabilities yet
