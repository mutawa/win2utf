# win2utf
A Windows-1256 to UTF-8 converter

# How to compile
* Clone the repo to your local drive
```
    > git clone https://github.com/mutawa/win2utf
    > cd win2utf
    win2utf> g++ main.cpp -o utf
```
You will have an executable named `utf` which expects a filename as its arguement to work with.

The program will try to create a new file for the output conversion with a filename that is exactly the same as the input file name (except for the extesnion). Then the program will append to the file name `_arb` and will assign the default extension `.srt` to the overall filename.

# How to run the binary
Assuming that you named the executable `utf` as explained above, you can run the command:
```
    win2utf > utf The.Village.2005.srt
    Written: The.Village.2005_arb.srt
```

