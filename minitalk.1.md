% MINITALK(1) Minitalk User Manual | Minitalk 0.3.0
% Andrew Benson
% September 2019

# NAME

minitalk - small, simple chat system

# SYNOPSIS

minitalk FILE [NICK]

# DESCRIPTION

FILE
  ~ File to use as "room". Must have read/write permissions.

NICK
  ~ Specify nick to use. Default is current username. Must be 1-15 characters
in length, and must only contain numbers and letters.

# EXAMPLE

Join a room controlled via /var/chat/general.

	$ minitalk /var/chat/general

The same, but specifying the username "admin".

	$ minitalk /var/chat/general admin

# NOTES

The primary issue is a security concern. Anyone that needs to chat in the
"room" has to be able to write to the control file. Anyone that needs read from
the room needs read access. Anyone that has access to these "control" files can
also inject anything they want into the chat. This truly is built on the honor
system, and was designed around a single purpose: multi-user chat between
trusted users on a single host.

Don't expect this to be secure.


# AUTHORS

Andrew Benson.

# COPYRIGHT

Copyright © 2019 Andrew Benson. License: MIT
