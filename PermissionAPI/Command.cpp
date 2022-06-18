#include <RegCommandAPI.h>
#include <EventAPI.h>
#include "pch.h"

using namespace RegisterCommandHelper;

/*
/perm create group   <Name> [Priority]
/perm delete group   <Name>
/perm create ability <Name> [Desc]
/perm delete ability <Name>
/perm update group   <Name> add member   <RealName>
/perm update group   <Name> rm  member   <RealName>
/perm update group   <Name> add ability  <AbName> [Enabled] [ExtraJson(as RawText)]
/perm update group   <Name> set ability  <AbName> <Enabled> [ExtraJson(as RawText)]
/perm update group   <Name> rm  ability  <AbName>
/perm update group   <Name> set priority <Priority>
/perm update player  <Name> add group    <GroupName>
/perm update player  <Name> rm  group    <GroupName>
/perm view   player  <Name>
/perm view   group   <Name>
/perm view   ability <Name>
/perm list   group
/perm list   ability
*/

class PermCommand : public Command {

    enum class SubCommands : char {
        Create = 0,
        Delete,
        Update,
        View,
        List,
    } subcmd = (SubCommands)-1;
    enum class Action : char {
        Add,
        Set,
        Remove
    } action = (Action)-1;
    enum class TargetType : char {
        Group = 0,
        Ability,
        Player,
        Priority,
        Member
    } targetType = (TargetType)-1;

    std::string name1;
    std::string name2;
    std::string desc;
    CommandRawText extra;
    bool enabled = false;
    int priority = 0;

    bool enabled_set = false;
    bool extra_set = false;
    bool priority_set = false;
    bool desc_set = false;

public:

    void execute(CommandOrigin const& ori, CommandOutput& outp) const {


    }

    static void setup(CommandRegistry* reg) {
        if (reg == nullptr) return;
        reg->registerCommand("perm", "Permission group system",
                             CommandPermissionLevel::Any,
                             {CommandFlagValue::None},
                             {(CommandFlagValue)0x80});

        reg->addEnum<SubCommands>("Create", {{"create", SubCommands::Create}});
        reg->addEnum<SubCommands>("Delete", {{"delete", SubCommands::Delete}});
        reg->addEnum<SubCommands>("Update", {{"update", SubCommands::Update}});
        reg->addEnum<SubCommands>("View", {{"view", SubCommands::View}});
        reg->addEnum<SubCommands>("List", {{"list", SubCommands::List}});

        auto subcmd_create = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::subcmd, "Create", "Create");
        auto subcmd_delete = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::subcmd, "Delete", "Delete");
        auto subcmd_update = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::subcmd, "Update", "Update");
        auto subcmd_view = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::subcmd, "View", "View");
        auto subcmd_list = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::subcmd, "List", "List");

        reg->addEnum<Action>("Add", {{"add", Action::Add}});
        reg->addEnum<Action>("Set", {{"set", Action::Set}});
        reg->addEnum<Action>("Remove", {{"rm", Action::Remove}});

        auto action_add = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::action, "Add", "Add");
        auto action_set = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::action, "Set", "Set");
        auto action_remove = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::action, "Remove", "Remove");
        
        reg->addEnum<TargetType>("Group", {{"group", TargetType::Group}});
        reg->addEnum<TargetType>("Ability", {{"ability", TargetType::Ability}});
        reg->addEnum<TargetType>("Player", {{"player", TargetType::Player}});
        reg->addEnum<TargetType>("Priority", {{"priority", TargetType::Priority}});
        reg->addEnum<TargetType>("Member", {{"member", TargetType::Member}});

        auto target_group = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType, "Group", "Group");
        auto target_ability = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType, "Ability", "Ability");
        auto target_player = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType, "Player", "Player");
        auto target_priority = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType, "Priority", "Priority");
        auto target_member = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType, "Member", "Member");

        auto param_name1 = makeMandatory(&PermCommand::name1, "name1");
        auto param_name2 = makeMandatory(&PermCommand::name2, "name2");
        auto param_enabled = makeOptional(&PermCommand::enabled, "enabled", &PermCommand::enabled_set);
        auto param_priority = makeOptional(&PermCommand::priority, "priority", &PermCommand::priority_set);
        auto param_desc = makeOptional(&PermCommand::desc, "description", &PermCommand::desc_set);
        auto param_extra= makeOptional(&PermCommand::extra, "extraJson", &PermCommand::extra_set);

        reg->registerOverload<PermCommand>("perm");
        // perm create group <Name> [Priority]
        reg->registerOverload<PermCommand>("perm", subcmd_create, target_group, param_name1, param_priority);
        // perm delete ability <Name> [Desc]
        reg->registerOverload<PermCommand>("perm", subcmd_create, target_ability, param_name1, param_desc);
        // perm delete group <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_delete, target_group, param_name1);
        // perm delete ability <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_delete, target_ability, param_name1);
        // perm update group <Name> add member <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group, param_name1, action_add, target_member, param_name2);
        // perm update group <Name> rm member <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group, param_name1, action_remove, target_member, param_name2);
        // perm update group <Name> add ability <Name> [Enabled] [ExtraJson]
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group, param_name1, action_add, target_ability, param_name2, param_enabled, param_extra);
        // perm update group <Name> set ability <Name> [Enabled] [ExtraJson]
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group, param_name1, action_set, target_ability, param_name2, param_enabled, param_extra);
        // perm update group <Name> rm ability <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group, param_name1, action_remove, target_ability, param_name2);
        // perm update group <Name> set priority [Priority]
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group, param_name1, action_set, target_priority, param_priority);
        // perm update player <Name> add group <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_player, param_name1, action_add, target_group, param_name2);
        // perm update player <Name> rm group <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_player, param_name1, action_remove, target_group, param_name2);
        // perm view group <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_view, target_group, param_name1);
        // perm view ability <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_view, target_ability, param_name1);
        // perm view player <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_view, target_player, param_name1);
        // perm list group
        reg->registerOverload<PermCommand>("perm", subcmd_list, target_group);
        // perm list ability
        reg->registerOverload<PermCommand>("perm", subcmd_list, target_ability);
    }
};