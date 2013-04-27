LÖVELINESS is a port of LÖVE to Native Client. Wow!

Setting up the NaCl SDK
-----------------------

You'll probably need linux (or something linux-like).

These are the abbreviated steps, for more info see [here][sdk].

    # Find a good place to put the SDK
    $ wget http://storage.googleapis.com/nativeclient-mirror/nacl/nacl_sdk/nacl_sdk.zip
    $ unzip nacl_sdk.zip
    $ cd nacl_sdk
    $ ./naclsdk update pepper_canary
    $ export NACL_SDK_ROOT=$PWD/pepper_canary

Now you have the most recent SDK installed, and NACL_SDK_ROOT points to it.


Setting up ninja
----------------

[Ninja][ninja] is a build system like Make, but faster and quieter.

    # Find a good place to put ninja
    $ git clone git://github.com/martine/ninja.git
    $ cd ninja
    $ ./bootstrap.py
    $ # Move ./ninja in your PATH, or "export PATH=$PATH:$PWD"


Setting up the Repo
-------------------

    # Find a good place to put love-nacl
    $ git clone git://github.com/binji/love-nacl
    $ cd love-nacl
    $ git submodule init
    $ git submodule update


Building
--------

    $ make

The output is put in $PWD/out. The data needed for the package is in
$PWD/out/package.


Running
-------

    $ export CHROME_PATH=/path/to/chrome
    $ make run-package

[sdk]: https://developers.google.com/native-client/sdk/download
[ninja]: http://martine.github.io/ninja/
