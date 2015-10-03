# makepng

* This is a simple program to convert any file into a "square .PNG" image file and also to get the original file back.
    * Non standard library dependency, libpng: expecting at /usr/lib.
    * encode and decode modes supported, to generate .PNG and to get the original file back.
    * release and debug targets.

| Version | Log |
| ------------- | ------------- |
| v1.6 | Cleanup |
| v1.5 | Added example script |
| v1.4 | More generic meta data text chunk added instead of particular ones e.g. sha1, filename |
| v1.3 | Removed SHA1 and filename support in text chunk. |
| v1.2 | SHA1 and filename support in text chunk. |
| v1.1 | Simple testing/validation added. |
| v1.0 | First working version with encode, decode and validate features. |
| v0.1 | Encoding working and cmdline options added. |

