# csgopp

A generalized, high-performance library for working with CS:GO demos. All credit goes to [`markus-wa`'s `demoinfocs-golang`](https://github.com/markus-wa/demoinfocs-golang), without which this would have been an impossible undertaking.

This project provides a couple things:

- An efficient C++ framework for simulating CS:GO demos and observing high-level events with zero overhead.
- A CLI tool that serves as a proof of concept for the library.
- Ergonomic Python bindings for the C++ framework that facilitate observing events in Python callbacks.

## To Do

- [ ] PacketEntities
- [ ] GameEventList
- [ ] GameEvent
- [x] CreateStringTable
- [x] UpdateStringTable
- [ ] UserMessage
- [ ] ServerInfo
- [ ] SetConVar
- [ ] EncryptedData
