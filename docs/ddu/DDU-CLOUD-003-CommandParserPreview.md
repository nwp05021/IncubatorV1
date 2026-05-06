
# DDU-CLOUD-003 — Command Parser Preview

핵심 흐름:

AWS desired
    ↓
CmdParser
    ↓
Command
    ↓
CommandQueue
    ↓
AppController

목표:

- JSON → Command
- Queue Injection
- Validation
- Delta Handling
