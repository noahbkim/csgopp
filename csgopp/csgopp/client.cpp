#include "client.h"

namespace csgopp::client
{

}

//        ingameTick           int
//        tState               common.TeamState
//        ctState              common.TeamState
//        playersByUserID      map[int]*common.Player            // Maps user-IDs to players
//        playersByEntityID    map[int]*common.Player            // Maps entity-IDs to players
//        playersBySteamID32   map[uint32]*common.Player         // Maps 32-bit-steam-IDs to players
//        playerResourceEntity st.Entity                         // CCSPlayerResource entity instance, contains scoreboard info and more
//        grenadeProjectiles   map[int]*common.GrenadeProjectile // Maps entity-IDs to active nade-projectiles. That's grenades that have been thrown, but have not yet detonated.
//        infernos             map[int]*common.Inferno           // Maps entity-IDs to active infernos.
//        weapons              map[int]*common.Equipment         // Maps entity IDs to weapons. Used to remember what a weapon is (p250 / cz etc.)
//        hostages             map[int]*common.Hostage           // Maps entity-IDs to hostages.
//        entities             map[int]st.Entity                 // Maps entity IDs to entities
//        bomb                 common.Bomb
//        totalRoundsPlayed    int
//        gamePhase            common.GamePhase
//        isWarmupPeriod       bool
//        isMatchStarted       bool
//        lastFlash            lastFlash                              // Information about the last flash that exploded, used to find the attacker and projectile for player_blind events
//        currentDefuser       *common.Player                         // Player currently defusing the bomb, if any
//        currentPlanter       *common.Player                         // Player currently planting the bomb, if any
//        thrownGrenades       map[*common.Player][]*common.Equipment // Information about every player's thrown grenades (from the moment they are thrown to the moment their effect is ended)
//        rules                gameRules
//        demoInfo             demoInfoProvider

// bombsiteA            bombsite
// bombsiteB            bombsite
// equipmentMapping     map[*st.ServerClass]common.EquipmentType        // Maps server classes to equipment-types
// rawPlayers           map[int]*common.PlayerInfo                      // Maps entity IDs to 'raw' player info
// modelPreCache        []string                                        // Used to find out whether a weapon is a p250 or cz for example (same id)
// triggers             map[int]*boundingBoxInformation                 // Maps entity IDs to triggers (used for bombsites)
// gameEventDescs       map[int32]*msg.CSVCMsg_GameEventListDescriptorT // Maps game-event IDs to descriptors
// grenadeModelIndices  map[int]common.EquipmentType                    // Used to map model indices to grenades (used for grenade projectiles)
// stringTables         []*msg.CSVCMsg_CreateStringTable                // Contains all created sendtables, needed when updating them
// delayedEventHandlers []func()