# PermissionAPI
Powerful permission API plugin for LiteLoaderBDS

## Installation
1. Download `Permission-X.X.X.zip` from [Releases](https://github.com/LiteLDev/PermissionAPI/releases)
2. Unzip the whole zip file into `/path/to/bds/plugins`
3. Start the server.

## Usage
### Command System
> Type `/help perm` to get the following usage in-game or in-console

```
perm:
Permission role system
Usage:
- /perm
- /perm add member <name1: string> <name2: string>
- /perm add perm <name1: string> <name2: string> [enabled: Boolean] [extraJson: text]
- /perm create perm <name1: string> [description: string]
- /perm create role <name1: string> [display name: string] [priority: int]
- /perm delete perm <name1: string>
- /perm delete role <name1: string>
- /perm list perm
- /perm list role
- /perm rm member <name1: string> <name2: string>
- /perm rm perm <name1: string> <name2: string>
- /perm role <name1: string>
- /perm role <name1: string> add member <name2: string>
- /perm role <name1: string> add perm <name2: string>
- /perm role <name1: string> add perm <name2: string> [enabled: Boolean] [extraJson: text]
- /perm role <name1: string> rm member <name2: string>
- /perm role <name1: string> set perm <name2: string> [enabled: Boolean] [extraJson: text]
- /perm role <name1: string> set priority [priority: int]
- /perm set perm <name1: string> <name2: string> [enabled: Boolean] [extraJson: text]
- /perm set priority <name1: string> [priority: int]
- /perm update player <name1: string> add role <name2: string>
- /perm update player <name1: string> rm role <name2: string>
- /perm update role <name1: string> add member <name2: string>
- /perm update role <name1: string> add perm <name2: string> [enabled: Boolean] [extraJson: text]
- /perm update role <name1: string> rm member <name2: string>
- /perm update role <name1: string> rm perm <name2: string>
- /perm update role <name1: string> set display_name [display name: string]
- /perm update role <name1: string> set perm <name2: string> [enabled: Boolean] [extraJson: text]
- /perm update role <name1: string> set priority [priority: int]
- /perm view perm <name1: string>
- /perm view player <name1: string>
- /perm view role <name1: string>
```

#### Command Parameters
- `name1`: `String`  
  The first target name. Whether this parameter is a role name or a player name or something else depends on the `TargetType` parameter preceding it.  
  For example, `/perm view role RoleName` `/perm create perm PermName` `/perm update role RoleName add member PlayerName`
- `name2`: `String`  
  The second target name, same as `name1`.
- `description`: `String`  
  The description of a role or permission.
- `display name`: `String`  
  The display name of a role.
- `priority`: `Integer`  
  The priority of a role.
- `enabled`: `Boolean`  
  Enable an permission or not for a role.
- `extraJson`: `RawText`  
  The extra json data for the permission in the role.

### Data File
> The data file of PermissionAPI is at `/path/to/bds/plugins/PermissionAPI/data.json`

The file content is like:
```json
{
    "permissionsInfo": {
        "PermissionAPI:cmd_control": {
            "desc": "Access to /perm commands"
        }
    },
    "roles": {
        "admin": {
            "permissions": {},
            "displayName": "§cadmin",
            "members": [
                "2535431384473425"
            ],
            "priority": 2147483647
        },
        "everyone": {
            "permissions": {},
            "displayName": "§7everyone",
            "priority": 0
        }
    }
}
```
Obviously, `json["permissionsInfo"]` stores information of all the permissions, including the description. And `json["roles]` stores all the roles.  
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
