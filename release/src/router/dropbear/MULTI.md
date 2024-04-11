## Multi-Binary Compilation

To compile for systems without much space (floppy distributions etc), you can create a single binary.
This will save disk space by avoiding repeated code between the various parts.
If you are familiar with BusyBox, it's the same principle.

To compile the multi-binary, first `make clean` (if you've compiled previously), then

```sh
make PROGRAMS="programs you want here" MULTI=1
```

To use the binary, symlink it from the desired executable:

```sh
ln -s dropbearmulti dropbear
ln -s dropbearmulti dbclient
```
etc.

Then execute as normal:

```
./dropbear <options here>
```
