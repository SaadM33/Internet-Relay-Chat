# Internet Relay Chat server (`ircserv`)

A small IRC server written in C++98. It runs as a single process with a
single-threaded, non-blocking `poll(2)` event loop, accepts plain-TCP IRC
clients, and implements the core channel and messaging commands. It is
password protected: a client must send the server password with `PASS` before
it can register.

For the internals, see [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Features

- IPv4 TCP listener on a port of your choice, with a mandatory connection password
- Many simultaneous clients on one thread (`poll` plus non-blocking sockets)
- Registration with `PASS`, `NICK` and `USER`
- Channels (names start with `#`) with operators, topics, invites, keys and member limits
- Private messages to users, channels, and channel operators (`@#channel`)
- A small ASCII-art bot, triggered with `!artbot` in a channel
- Clean shutdown on `Ctrl+C` (SIGINT)

## Supported commands

| Command | Syntax | Notes |
|---|---|---|
| `CAP` | `CAP LS` | Replies with an empty capability list. Other subcommands are ignored. Allowed before registration. |
| `PASS` | `PASS <password>` | Must match the server password. A wrong password gets `464` and the connection is closed. |
| `NICK` | `NICK <nickname>` | Letters, digits and `- _ [ ] \ ` ^ { } \|`. Cannot start with `#` or `@`. Nicknames must be unique (`433` otherwise). After registration, a change is announced to the user and to everyone sharing a channel with them. |
| `USER` | `USER <username> <mode> <unused> :<realname>` | Needs three parameters plus a trailing real name. |
| `JOIN` | `JOIN <#chan>[,<#chan>...] [key[,key...]]` | Creates the channel if needed; the first member becomes operator. Checks `+i`, `+k` and `+l`. |
| `PART` | `PART <#chan>[,<#chan>...] [:reason]` | Empty channels are deleted. |
| `TOPIC` | `TOPIC <#chan> [:new topic]` | Without a trailing argument it shows the topic. With `+t`, only operators can set it. |
| `INVITE` | `INVITE <nick> <#chan>` | The inviter must be on the channel, and an operator if the channel is `+i`. The invite is consumed on `JOIN`. |
| `KICK` | `KICK <#chan> <nick>[,<nick>...] [:comment]` | Operators only. The comment defaults to the kicker's nickname. |
| `MODE` | `MODE <#chan> [+/-modes] [args]` | See below. With no mode string it reports the current modes (`324`). Only channel modes are handled. |
| `PRIVMSG` | `PRIVMSG <target>[,<target>...] :<text>` | Target is a nick, `#channel`, or `@#channel` (operators only). |
| `PING` | `PING <token>` | Replies `PONG :<token>`. |
| `QUIT` | `QUIT [:message]` | Notifies channel peers and closes the connection. |

Command names are case-insensitive. Anything else gets `421`. Every command
except `PASS`, `NICK`, `USER` and `CAP` needs a completed registration
(`451` otherwise).

### Channel modes

| Mode | Meaning | Argument |
|---|---|---|
| `i` | Invite-only | none |
| `t` | Only operators may change the topic | none |
| `k` | Channel key (password) | key when setting (`+k`) |
| `l` | Member limit | number when setting (`+l`) |
| `o` | Give or take operator status | nickname |

Several modes can be combined, e.g. `MODE #chan +it-k` or `MODE #chan +kl secret 20`.
Unknown mode characters are reported back to the sender.

### Bot

Send `!artbot` as a channel message (you must be a member). The server answers
with one of six ASCII pictures (rabbit, cat, computer, 1337, sword, flower),
picked at random, as `PRIVMSG` lines from `artbot`. Only the requester sees
the picture; it is not broadcast to the channel.

## Build

Requirements: a C++ compiler reachable as `c++` that supports `-std=c++98`
(g++ or clang++), and `make`. The code uses POSIX sockets, so Linux or macOS.

```sh
make          # build ./ircserv
make clean    # remove obj/
make fclean   # remove obj/ and ./ircserv
make re       # full rebuild
```

The Makefile builds with `-Wall -Wextra -Werror -g -std=c++98`.

## Run

```sh
./ircserv <port> <password>
```

- `port` is an integer from 1 to 65535 (ports below 1024 usually need root).
- `password` must not be empty.

Example:

```sh
./ircserv 6667 mysecret
# Server is listening on port 6667
```

The server binds to all interfaces (`0.0.0.0`). Stop it with `Ctrl+C`.

## Connect with a client

Use any IRC client and give it the server password.

**irssi**

```
irssi
/connect 127.0.0.1 6667 mysecret
/join #general
```

**netcat** (the server only understands `\r\n` line endings, so use `-C` for CRLF)

```sh
nc -C 127.0.0.1 6667
PASS mysecret
NICK alice
USER alice 0 * :Alice Example
JOIN #general
PRIVMSG #general :hello
```

After the three registration commands you get `001 Welcome to the IRC server Mortal!`.

## Project structure

```
.
├── main.cpp              Entry point: builds the Server and runs it
├── Makefile
├── includes/
│   ├── Server.hpp        Server class, command handler declarations
│   ├── Client.hpp        Per-connection state
│   ├── Channel.hpp       Channel state and mode helpers
│   └── utils.hpp         Includes, numeric reply codes, Message struct
├── src/
│   ├── Server.cpp        Setup, poll loop, accept/disconnect, replies
│   ├── Parsing.cpp       Line framing and message parsing
│   ├── ProcessCmd.cpp    Registration check and command dispatch
│   ├── Client.cpp
│   ├── Channel.cpp       Broadcast, join/leave, mode setters
│   ├── utils.cpp         Nickname helpers
│   └── cmds/             One file per command (Pass.cpp, Join.cpp, ...)
└── docs/
    └── ARCHITECTURE.md
```

## Known limitations

- IPv4 only, no TLS.
- Lines must end in `\r\n`; a bare `\n` is not treated as a line end.
- Outgoing data is sent with plain `send` and not queued, so a slow client
  may miss data.
- `QUIT` and disconnects are announced only for `QUIT`, not for dropped connections.
- Server replies use the fixed prefix `localhost`.
