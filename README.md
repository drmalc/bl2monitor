Based on:
https://github.com/McSimp/Borderlands2SDK

Binary snapshots:
https://mega.co.nz/#F!eEFmBL7L!3ahyYI8ImXZlC0wyNNRfLA

Early alpha stage.

Open with VS community 2013, compile (make sure the Release configuration is active), run.

bl2monitor (the server) will indicate that it is waiting for the target (borderlands2).
Launch bl2 (standalone or steam version), the server will update its status.

You will see a GUI overlay. That's it.. for now.

Notes:
- CEGUI is implemented. No Lua binding yet.
- lua is implemented, still needs a lot of work
- compilation requires DX9 SDK
