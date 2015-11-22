# minitalk

This is the first working of a small chat program I wrote at the request of a coworker.

They had a very specific use case. Instead of being able to configure a standard chat system (I had ircd ready to go...) they ended up having to "chat" using a single file on a shared remote Linux server. They had two terminals open: one with a `cat >> file` for sending messages, another with `tail -f file` for receiving messages. Their primary complaint was having to use two windows, so I wrote this.

Here are some of the names that were suggested:

- Smalltalk
- Microtalk
- Minitalk "requires a minicomputer to run!", fitting since this would've done well back in the late 70s, early 80s...
- "Yet Another Chat Program It SeemS. Everyone will go crazy for YACPISS."
- "khat. Maybe if you had a KDE frontend?" "Because bolting on KDE support is what a <200 line program needs"
- mongochat. "down the slippery slope of bloat :)"

## Requirements

- C compiler.
- POSIX system
- GNU readline

## Usage

    $ minitest path/to/file

By default, it uses your username as the nick. You can override this by passing the username on the command line.

    $ minitest path/to/file mynick

To send a message, type it and hit enter. To quit, type `/quit`.

![A demo!](doc/minitalk.gif)

