# csgopp

A generalized, high-performance library for working with CS:GO demos. All credit goes to [`markus-wa`'s `demoinfocs-golang`](https://github.com/markus-wa/demoinfocs-golang), without which this would have been an impossible undertaking.

This project provides a couple things:

- An efficient C++ framework for simulating CS:GO demos and observing high-level events with zero overhead.
- A CLI tool that serves as a proof of concept for the library.
- (TODO) Ergonomic Python bindings that facilitate observing events in Python callbacks.

## Samples

Let's take a look at some examples of how to use `csgopp` as a static C++ library.
The entrypoint for user code is the `csgopp::client::Client`, which is parameterized on an `Observer` template.
This `Observer` objective is responsible for providing event handler classes that are constructed on the stack at runtime for performance.
You can skip a bunch of boilerplate by inheriting from the `csgopp::client::ClientObserverBase` as follows:

```cpp
#include <iostream>
#include <csgopp/client.h>

using csgopp::client::Client;
using csgopp::client::ClientObserverBase;

class MyObserver final : ClientObserverBase<MyObserver>
{
    using ClientObserverBase::ClientObserverBase;
};
```

You'll notice we have to pass the concrete type as a template parameter to the `ClientObserverBase`.
This ensures we can access the client we're bound to in callbacks.
To run the client with our observer, we have to instantiate a client.

```cpp
#include <filesystem>
#include <google/protobuf/io/coded_stream.h>

// Insert your own demo path
std::string path = "path/to/my/demo.dem";

// Create a CodedInputStream for protobuf deserialization
std::ifstream file_stream(path, std::ios::binary);
IstreamInputStream file_input_stream(&file_stream);
CodedInputStream coded_input_stream(&file_input_stream);

// Instantiate the client with our observer and advance until the demo ends
Client<MyObserver> client(coded_input_stream);
while (client.advance(coded_input_stream));
```

As an example, let's log when players join a team:

```cpp
const char* describe_team(uint8_t team)
{
    switch (team)
    {
    case 1:
        return "Spectators";
    case 2:
        return "Terrorists";
    case 3:
        return "Counter-terrorists";
    default:
        return "Unknown";
    }
}

class MyObserver final : ClientObserverBase<MyObserver>
{
    using ClientObserverBase::ClientObserverBase;
    
    void on_game_event(Client &client, GameEvent& event) override
    {
        if (event.name == "player_team")
        {
            const User* user = client.users().at_id(event["userid"].is<int16_t>());
            const char* team = describe_team(event["team"].is<uint8_t>());
            std::cout << client.tick() << ": Player " << user->name << " joined " << team << std::endl;
        }
    }
};
```

Before we talk about the `User` objective, we have to access data from the `GameEvent`.
We know what the layout of the `"player_team"` event looks like from the `csgopp.cli` command `generate`:

```
$ csgopp.cli generate --verify --directory . path/to/my/demo.dem
```

This emits all events and server classes in a demo as slightly invalid C++.
We can take a look at `*.events.h` and find the following:

```c++
struct player_team
{
    int16_t userid;
    uint8_t team;
    uint8_t oldteam;
    bool disconnect;
    bool autoteam;
    bool silent;
    bool isbot;
};
```

We can query this objective dynamically at runtime using `csgopp::common::objective` machinery, e.g. `event["userid"].is<int16_t>()`.
This allows us to then get the associated `User` via `client.users()`, which offers per-id lookup.
The rest is pretty straightforward.

As another example, let's take a look at what players are purchasing.
This requires delving into entity updates, specifically on the `cslocaldata.m_iWeaponPurchasesThisRound` field.

```cpp
const char* describe_weapon(uint32_t weapon)
{
    switch (weapon)
    {
        // See csgopp/cli/summary.h for complete list
        case 1: return "weapon_deagle";
        case 7: return "weapon_ak47";
        case 9: return "weapon_awp";
        case 16: return "weapon_m4a1";
        case 43: return "weapon_flashbang";
        case 44: return "weapon_hegrenade";
        case 45: return "weapon_smokegrenade";
        case 46: return "weapon_molotov";
        case 47: return "weapon_decoy";
        case 48: return "weapon_incgrenade";
        case 60: return "weapon_m4a1_silencer";
        default: return "something...";
    }
}

class MyObserver final : ClientObserverBase<MyObserver>
{
    using ClientObserverBase::ClientObserverBase;
    
    const ServerClass* player_server_class{nullptr};
    Accessor weapon_purchases_accessor;

    void on_server_class_creation(Client& client, const ServerClass* server_class) override
    {
        if (server_class->name == "CCSPlayer")
        {
            // Get a reference to the player entity server class 
            this->player_server_class = server_class;
            
            // Find the offset and type of the player entity's m_iWeaponPurchasesThisRound field
            const EntityType& type = *server_class->data_table->type();
            this->weapon_purchases_accessor = type["cslocaldata"]["m_iWeaponPurchasesThisRound"];
        }
    }
    
    void on_entity_update(Client& client, const Entity* entity, const std::vector<uint16_t>& indices) override
    {
        // Check if this is a player entity
        if (entity->server_class == this->player_server_class)
        {
            // Multiple fields are updated at a time; iterate through each updated field
            for (uint16_t index : indices)
            {
                // This is discrete, primitive(ish) value that was updated
                const EntityDatum& datum = entity->type->prioritized.at(index);
                
                // Check if it's within the bounds of the m_iWeaponPurchasesThisRound objective (> is overloaded)
                if (this->weapon_purchases_accessor > datum)
                {
                    const User* user = client.users().at_index(entity->id);
                    int weapon = atoi(datum.property->name.c_str());
                    std::cout << user->name << " purchased " << describe_weapon(weapon) << std::endl;
                }
            }
        }
    }
}
```

In order to figure out when a player purchases a weapon, we need to know the offset of the `m_iWeaponPurchasesThisRound` objective the player server class.
We figure this out in `on_server_class_creation`, which is called sometime near the start of the demo when server classes are registered.
We know the player server class is called `"CCSPlayer"`, so we save the pointer and find the accessor we're looking for.

When we get an entity update callback, we check whether the updated datum is within the byte range of the `m_iWeaponPurchasesThisRound` objective.
From the generated classes, we know that each member of this objective is named after its weapon index, e.g. `"001"`, `"002"`, etc., so the rest is as simple as parsing that integer and printing its corresponding weapon name.

## To Do

- [x] PacketEntities
- [x] GameEventList
- [x] GameEvent
- [x] CreateStringTable
- [x] UpdateStringTable
- [ ] UserMessage
- [ ] ServerInfo
- [ ] SetConVar
- [ ] EncryptedData

## Alternatives

- Go: https://github.com/markus-wa/demoinfocs-golang
- TypeScript: https://github.com/saul/demofile
- C++: https://github.com/ValveSoftware/csgo-demoinfo
