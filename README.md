# PermissionAPI
Powerful permission group API plugin for LiteLoaderBDS

## Installation
1. Download `Permission-X.X.X.zip` from [Releases](https://github.com/LiteLDev/PermissionAPI/releases)
2. Unzip the whole zip file into `/path/to/bds/plugins`
3. Start the server.

## Usage
### Command System
> Type `/help perm` to get the following usage in-game or in-console

```
perm:
Permission group system
Usage:
- /perm
- /perm create ability <name1: string> [description: string]
- /perm create group <name1: string> [display name: string] [priority: int]
- /perm delete ability <name1: string>
- /perm delete group <name1: string>
- /perm list ability
- /perm list group
- /perm update group <name1: string> add ability <name2: string> [enabled: Boolean] [extraJson: text]
- /perm update group <name1: string> add member <name2: string>
- /perm update group <name1: string> rm ability <name2: string>
- /perm update group <name1: string> rm member <name2: string>
- /perm update group <name1: string> set ability <name2: string> [enabled: Boolean] [extraJson: text]
- /perm update group <name1: string> set display_name [display name: string]
- /perm update group <name1: string> set priority [priority: int]
- /perm update player <name1: string> add group <name2: string>
- /perm update player <name1: string> rm group <name2: string>
- /perm view ability <name1: string>
- /perm view group <name1: string>
- /perm view player <name1: string>
```

#### Command Parameters
- `name1`: `String`  
  The first target name. Whether this parameter is a group name or a player name or something else depends on the `TargetType` parameter preceding it.  
  For example, `/perm view group GroupName` `/perm create ability AbilityName` `/perm update group GroupName add member PlayerName`
- `name2`: `String`  
  The second target name, same as `name1`.
- `description`: `String`  
  The description of a group or ability.
- `display name`: `String`  
  The display name of a group.
- `priority`: `Integer`  
  The priority of a group.
- `enabled`: `Boolean`  
  Enable an ability or not for a group.
- `extraJson`: `RawText`  
  The extra json data for the ability in the group.

### Data File
> The data file of PermissionAPI is at `/path/to/bds/plugins/PermissionAPI/data.json`

The file content is like:
```json
{
    "abilitiesInfo": {
        "PermissionAPI:cmd_control": {
            "desc": "Access to /perm commands"
        }
    },
    "groups": {
        "admin": {
            "abilities": {},
            "displayName": "§cadmin",
            "members": [
                "2535431384473425"
            ],
            "priority": 2147483647
        },
        "everyone": {
            "abilities": {},
            "displayName": "§7everyone",
            "priority": 0
        }
    }
}
```
Obviously, `json["abilitiesInfo"]` stores information of all the abilities, including the description. And `json["groups]` stores all the groups.  
It is not recommended to modify the data file directly. Please use [command](#command-system)

### Internationalization

#### Language File
> The language file is at `/path/to/bds/plugins/PermissionAPI/lang.json`

It is like:
```json
{
    "lang_KEY": {
        "LANGKEY {}": "TRANSLATION {}"
    },
    "zh_CN": {
        "Player not found.": "找不到玩家"
    }
}
```
