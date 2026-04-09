## Building the Sphinx HTML

### Manual build commands

1. install `sphinx` `breathe` `doxygen`
2. run following commands in the sphinx directory
    ``` 
    $ mkdir build
    $ cd build
    $ cmake ..
    $ cmake --build . --target daric_sphinx
    ```

# Build script

Alternatively just use the `run-build` command, which can optionally create
a PDF if `latex(from xetex package)` and `graphviz` are also installed.

Use `run-build --help` to see details.
