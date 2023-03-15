#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>

#include <google/protobuf/io/coded_stream.h>
#include <argparse/argparse.hpp>

#include <csgopp/client.h>
#include <object/object.h>

#include "common.h"

using argparse::ArgumentParser;
using csgopp::client::Client;
using csgopp::client::User;
using csgopp::client::GameEvent;
using csgopp::client::entity::EntityType;
using csgopp::client::entity::EntityDatum;
using csgopp::client::entity::EntityConstReference;
using object::Accessor;
using object::ConstReference;

const char* describe_game_phase(uint32_t phase)
{
    switch (phase)
    {
        case 0:
            return "Init";
        case 1:
            return "Pregame";
        case 2:
            return "Start game phase";
        case 3:
            return "Team side switch";
        case 4:
            return "Game half ended";
        case 5:
            return "Game ended";
        case 6:
            return "Stale mate";
        case 7:
            return "Game over";
        default:
            return "Unknown";
    }
}

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

const char* describe_weapon(uint32_t weapon)
{
    switch (weapon)
    {
        case 1: return "weapon_deagle";
        case 2: return "weapon_elite";
        case 3: return "weapon_fiveseven";
        case 4: return "weapon_glock";
        case 7: return "weapon_ak47";
        case 8: return "weapon_aug";
        case 9: return "weapon_awp";
        case 10: return "weapon_famas";
        case 11: return "weapon_g3sg1";
        case 13: return "weapon_galilar";
        case 14: return "weapon_m249";
        case 16: return "weapon_m4a1";
        case 17: return "weapon_mac10";
        case 19: return "weapon_p90";
        case 20: return "weapon_zone_repulsor";
        case 23: return "weapon_mp5sd";
        case 24: return "weapon_ump45";
        case 25: return "weapon_xm1014";
        case 26: return "weapon_bizon";
        case 27: return "weapon_mag7";
        case 28: return "weapon_negev";
        case 29: return "weapon_sawedoff";
        case 30: return "weapon_tec9";
        case 31: return "weapon_taser";
        case 32: return "weapon_hkp2000";
        case 33: return "weapon_mp7";
        case 34: return "weapon_mp9";
        case 35: return "weapon_nova";
        case 36: return "weapon_p250";
        case 37: return "weapon_shield";
        case 38: return "weapon_scar20";
        case 39: return "weapon_sg556";
        case 40: return "weapon_ssg08";
        case 41: return "weapon_knifegg";
        case 42: return "weapon_knife";
        case 43: return "weapon_flashbang";
        case 44: return "weapon_hegrenade";
        case 45: return "weapon_smokegrenade";
        case 46: return "weapon_molotov";
        case 47: return "weapon_decoy";
        case 48: return "weapon_incgrenade";
        case 49: return "weapon_c4";
        case 50: return "item_kevlar";
        case 51: return "item_assaultsuit";
        case 52: return "item_heavyassaultsuit";
        case 54: return "item_nvg";
        case 55: return "item_defuser";
        case 56: return "item_cutters";
        case 57: return "weapon_healthshot";
        case 58: return "musickit_default";
        case 59: return "weapon_knife_t";
        case 60: return "weapon_m4a1_silencer";
        case 61: return "weapon_usp_silencer";
        case 62: return "Recipe Trade Up";
        case 63: return "weapon_cz75a";
        case 64: return "weapon_revolver";
        default: return "unknown";
    }
}

struct SummaryClient final : public Client
{
    using Client::Client;

    std::shared_ptr<const ServerClass> player_server_class;
    Accessor weapon_purchases_accessor;

    void on_server_class_creation(const std::shared_ptr<const ServerClass>& server_class) override
    {
        if (server_class->name == "CCSPlayer")
        {
            this->player_server_class = server_class;
            auto type = server_class->data_table->type();
            this->weapon_purchases_accessor = Accessor(type)["cslocaldata"]["m_iWeaponPurchasesThisRound"];
        }
    }

    void on_entity_update(
        const std::shared_ptr<const Entity>& entity,
        const std::vector<uint16_t>& indices
    ) override
    {
        if (entity->server_class == this->player_server_class)
        {
            for (uint16_t index : indices)
            {
                EntityConstReference ref = entity->at(index);
                if (this->weapon_purchases_accessor.is_strict_superset_of(ref))
                {
                    const std::shared_ptr<const User>& user = this->users().at_index(entity->id);
                    OK(user != nullptr);
                    int weapon = atoi(ref.property->name.c_str());
                    std::cout << user->name << " purchased " << describe_weapon(weapon) << std::endl;
                }
            }
        }
    }

    void on_game_event(GameEvent&& event) override
    {
        std::string_view name(event.type->name);
        if (name == "round_end")
        {
            std::cout << this->tick() << ": Round end: " << event["message"].is<std::string>() << std::endl;
            for (const std::shared_ptr<User>& user : this->users())
            {
                const std::shared_ptr<const Entity>& entity = this->entities().get(user->index);
                if (entity == nullptr)
                {
                    continue;
                }

                int32_t team_number = (*entity)["m_iTeamNum"].is<int32_t>();
                if (team_number != 2 && team_number != 3)
                {
                    continue;
                }

                std::cout << "  " << user->name << ": ";
                bool first = false;
                const auto* purchases = (*entity)["cslocaldata"]["m_iWeaponPurchasesThisRound"].as<uint32_t>();
                for (uint32_t i = 0; i < 64; ++i)
                {
                    if (purchases[i])
                    {
                        if (first)
                        {
                            std::cout << ", ";
                        }
                        std::cout << describe_weapon(i) << " (" << i << "=" << purchases[i] << ")";
                        first = true;
                    }
                }
                std::cout << std::endl;
            }
        }
        else if (name == "player_team")
        {
            const std::shared_ptr<const User>& user = this->users().at_id(event["userid"].is<int16_t>());
            const char* team = describe_team(event["team"].is<uint8_t>());
            std::cout << this->tick() << ": Player " << user->name << " joined " << team << std::endl;
        }
    }
};

struct SummaryCommand
{
    std::string name;
    ArgumentParser parser;

    explicit SummaryCommand(ArgumentParser& root) : name("summary"), parser(name)
    {
        this->parser.add_description("summarize the rounds in a demo and print a scoreboard");
        this->parser.add_argument("demo");
        root.add_subparser(this->parser);
    }

    [[nodiscard]] int main() const
    {
        Timer timer;
        std::string path = this->parser.get("demo");
        if (!std::filesystem::exists(path))
        {
            std::cerr << "No such file " << path << std::endl;
            return -1;
        }

        std::ifstream file_stream(path, std::ios::binary);
        IstreamInputStream file_input_stream(&file_stream);
        CodedInputStream coded_input_stream(&file_input_stream);

        try
        {
            SummaryClient client(coded_input_stream);
            while (client.advance(coded_input_stream));
        }
        catch (const csgopp::error::GameError& error)
        {
            std::cerr << error.message() << std::endl;
            return -1;
        }

        std::cout << "finished in " << timer << std::endl;
        return 0;
    }
};
