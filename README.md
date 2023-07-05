# labrador

## Virtual Memory Layout

| Use | Start Address | End Address | Size |
| --- | --- | --- | --- |
| Userspace | `0x0000000000000000` | `0x00007fffffffffff` | |
| Direct Physical Memory Map | `0xffff800000000000` | Start + Physical Memory | Size of Physical Memory |
| Kernel Object Space | `0xffffff0000000000` | `0xffffffff7fffffff` | |
| Kernel/Modules | `0xffffffff80000000` | `0xffffffffffffffff` | 2 GB |