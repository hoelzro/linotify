linotify
========
A Lua binding for the Linux inotify library

Building
--------

To build `inotify.so`, simply type `make`.

Usage
-----

All of the constants are contained in the `inotify` table returned by
require.  Constants are named after their counterparts in the C header
file (for example: `inotify.IN_ACCESS`).

The only function to be found in the inotify table is `init`, which returns an
inotify handle.

`init` can optionally take a table a single argument.  This table should
contain attributes for the inotify handle's creation.  The supported
attributes are:

  * **blocking** - If set to false, the I/O operations performed on this
    inotify handle are non-blocking.  Otherwise, they are blocking.

Inotify handles have a variety of methods:

### handle:read()

Reads events from the handle, returning a table.  Each element of the table
is itself a table, with the members of the `inotify_event` struct as its
keys and values (except for len).  If the handle is in non-blocking mode and
no events are available, an empty table is returned. If an error occurs, `nil`,
the error message, and errno are returned.

### handle:events()

Returns an iterator that reads events from the handle, one at a time.
Each value yielded from the iterator is a table with the members of the
`inotify_event` struct as its keys and values (except for len).  If an
error occurs during reading, an error is thrown.  If this method is
run on a handle in non-blocking mode, it will yield events until no
more events are available without blocking.

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

```lua
-- Event masks passed as arguments
local handle = inotify.init()
local wd = handle:addwatch('/tmp/foo/', inotify.IN_CREATE, inotify.IN_MOVE)

-- Event masks passed as a single, manually OR'd variable
local handle = inotify.init()
local options = bit.bor(inotify.IN_CREATE, inotify.IN_MOVE)
local wd = handle:addwatch('/tmp/foo/', options)
```

### handle:rmwatch(watchid)

Removes the watch specified by watchid from the list of watches for this
inotify handle.  Returns true on success, and `nil, error, errno` on error.

### handle:fileno()

Returns the integer file descriptor for the given handle.  Useful when
used in combination with an event loop.

### handle:getfd()

Alias for [handle:fileno()](#handlefileno).

Example
-------

```lua
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
```

Example (Iterator)
------------------

```lua
local inotify = require 'inotify'
local handle = inotify.init()

-- Watch for new files and renames
local wd = handle:addwatch('/home/rob/', inotify.IN_CREATE, inotify.IN_MOVE)

for ev in handle:events() do
    print(ev.name .. ' was created or renamed')
end

-- Done automatically on close, I think, but kept to be thorough
handle:rmwatch(wd)

handle:close()
```

No More Global Table
-------------------

As of version 0.3, the global `inotify` table has been completely removed.
You now need to handle the return value from `require`, like so:

```lua
local inotify = require 'inotify'
```
