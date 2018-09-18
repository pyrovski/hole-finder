# hole-finder
Find aligned chunks of zeros in a file.

Example:


```sh
(head -c4096 /dev/urandom; head -c8192 /dev/zero; head -c123 /dev/urandom) > file
./zeros -c 1024 file 
```

yields:

```sh
file    4096    8192
```
