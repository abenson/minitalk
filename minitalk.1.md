% MINITALK(1) Minitalk User Manual
% Andrew Benson
% December 26, 2015

# NAME

minitalk - small, simple chat system

# SYNOPSIS

minitalk FILE [NICK]

# DESCRIPTION

FILE File to use as "room". Must have read/write permissions.
NICK Specify nick to use. Default is current username.

# EXAMPLES

Join a room controlled via /var/chat/general.

	$ minitalk /var/chat/general

The same, but specifying the username "admin".

	$ minitalk /var/chat/general admin

# AUTHOR

Written by Andrew Benson.

# COPYRIGHT

Copyright © 2015 Andrew Benson. License: MIT
