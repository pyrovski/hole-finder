# hole-finder
Find aligned chunks of zeros in a file.

Example:


```sh
(head -c65536 /dev/urandom; head -c65536 /dev/zero; head -c123 /dev/urandom) > file
./zeros -c 65536 file
```

yields:

```sh
file    65536   65536
```

The tool only reads the first IO block (size defined by the file
system) of each chunk.  If it finds a zero block, it reads until it
encounters a nonzero byte, prints the interval to stdout, and seeks to
the next chunk boundary.