# NFS Pro Street - Test Track fixer
This is a simple patching utility to attempt converting the material chunks in the prototype/testing models to the final format.

It is not fully finished as it still has broken materials visible in the game.

Use this at your own risk.

# Usage
## Windows GUI way:
- Drag and drop a track stream you need to convert (e.g. STREAML6R_TestTrack.BUN) to the executable
- You should have a file with _patched at the end (e.g. STREAML6R_TestTrack_patched.BUN)

## CMD / Terminal way:
```
NFSPS_TestTrack_ModelFixer StreamBundle [OutFileName]
```
Afterwards:

- Backup your old file, and place your patched file in the TRACKS folder with the correct name (e.g. STREAML6R_TestTrack_patched.BUN back to STREAML6R_TestTrack.BUN)

# Credits
- heyitsleo / LeoCodes21 - figuring out the differences in the formats
