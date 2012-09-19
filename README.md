linotify
========
A Lua binding for the Linux inotify library

Building
--------

To build `inotify.so`, simply type `make`.

Usage
-----

All of the constants are contained in the global `inotify` table.
Constants are named after their counterparts in the C header file
(for example: `inotify.IN_ACCESS`).

The only function to be found in the inotify table is `init`, which
takes no arguments and returns an inotify handle.

Inotify handles have a variety of methods:

### handle:read()

Reads events from the handle, returning a table.  Each element of the table
is itself a table, with the members of the `inotify_event` struct as its
keys and values (except for len).  If an error occurs, `nil`, the error
message, and errno are returned.

### handle:close()

Closes the inotify event handle.  This is done automatically on garbage
collection.

### handle:addwatch(path, [event_masks...])

Adds a watch on `event_masks` for the file located at path, returning a
watch identifier on success, and the traditional `nil, error, errno` triplet
on error.  `event_masks` is a variadic sequence of integer constants, taken
from `inotify.IN_*`.

All of the values in `event_masks` are OR'd together but this can also be done
manually with `bit.bor()`. The following two examples are equivalent:

    -- Event masks passed as arguments
    local handle = inotify.init()
    local wd = handle:addwatch('/tmp/foo/', inotify.IN_CREATE, inotify.IN_MOVE)

    -- Event masks passed as a single, manually OR'd variable
    local handle = inotify.init()
    local options = bit.bor(inotify.IN_CREATE, inotify.IN_MOVE)
    local wd = handle:addwatch('/tmp/foo/', options)

### handle:rmwatch(watchid)

Removes the watch specified by watchid from the list of watches for this
inotify handle.  Returns true on success, and `nil, error, errno` on error.

Example
-------

    local inotify = require 'inotify'
    local handle = inotify.init()

    -- Watch for new files and renames
    local wd = handle:addwatch('/home/rob/', inotify.IN_CREATE, inotify.IN_MOVE)

    local events = handle:read()

    for _, ev in ipairs(events) do
        print(ev.name .. ' was created or renamed')
    end

    -- Done automatically on close, I think, but kept to be thorough
    handle:rmwatch(wd)

    handle:close()
