# LangPlus

[![sampctl](https://img.shields.io/badge/sampctl-LangPlus-2f2f2f.svg?style=for-the-badge)](https://github.com/PerseweratywneImponderabilia/LangPlus)

A lightweight multilanguage library for open.mp servers. Uses INI files for translations and provides an easy-to-use API for managing player languages.


## Features
- **Fallback system** - Missing keys fall back to the default language; if that fails, the `key` itself is used
- **String replacements** - Use `SetStringReplacement()` for dynamic text substitution
- **Format specifiers** - Full support for format specifiers, such as: `%d`, `%s`, `%.2f`, etc.
- **Non-latin character support** - Support for non-latin characters, such as cyrillic alphabet; use proper file encoding for the desired characters


## Installation

```bash
sampctl install PerseweratywneImponderabilia/LangPlus
```

```pawn
#include <LangPlus>
```

### Note
Make sure these definitions are declared before you include `PawnPlus`, preferably at the very top of your main gamemode file:
```pawn
#define PP_SYNTAX_FOR_POOL
#define PP_SYNTAX_FOR_MAP
```

## Quick Start

**`gamemodes/main.pwn`**
```pawn
#include <LangPlus>

new Language:g_English, Language:g_Ukrainian;

public OnGameModeInit() {
    g_English = LoadLanguage("en", "English");
    g_Ukrainian = LoadLanguage("uk", "Ukrainian");
    return 1;
}

public OnPlayerConnect(playerid) {
    SetPlayerLanguage(playerid, g_Ukrainian);
    SendLanguageMessage(playerid, -1, "WELCOME_MESSAGE", playerid);
    SendLanguageMessageToAll(-1, "PLAYER_JOINED", playerid);
    return 1;
}
```

**`scriptfiles/languages/en.ini`**
```ini
WELCOME_MESSAGE=Welcome, player %d!
PLAYER_JOINED=Player %d joined
```

**`scriptfiles/languages/uk.ini`**
```ini
WELCOME_MESSAGE=Вітаємо, гравець %d!
PLAYER_JOINED=Гравець %d приєднався
```

## API Reference

| Function | Description |
|----------|-------------|
| `Language:LoadLanguage(code[], name[], fileName[])` | Load a language. Looks for the file in the `DIRECTORY_LANGUAGES` directory; file name is `fileName`, `name.ini`, `code.ini` respectively. |
| `SetPlayerLanguage(playerid, Language:id)` | Set player's language |
| `Language:GetPlayerLanguage(playerid)` | Get player's current language |
| `SendLanguageMessage(playerid, colour, key[], ...)` | Send localized message to player (supports format specifiers) |
| `SendLanguageMessageToAll(colour, key[], ...)` | Send localized message to all players in their language |
| `GetLanguageString(Language:id, key[], output[], len)` | Get translated string for a language |
| `ReturnLanguageString(Language:id, key[])` | Return translated string |
| `String:ReturnLanguageString_s(Language:id, key[])` | Return translated PawnPlus string |
| `GetLanguageCount()` | Get number of loaded languages |
| `bool:DoesLanguageCodeExist(code[])` | Check if language exists |
| `bool:DoesLanguageNameExist(code[])` | Check if language exists |
| `GetLanguageCodeList(string:codes[][], maxSize)` | Get array of language codes |
| `GetLanguageNameList(string:names[][], maxSize)` | Get array of language names |
| `SetStringReplacement(key[], value[])` | Define replacement for language loading (call before `LoadLanguage`) |
| `@L(playerid, key[])` | Macro for `ReturnLanguageString(GetPlayerLanguage(playerid), key)` |
| `@LS(playerid, key[])` | Macro for `ReturnLanguageString_s(GetPlayerLanguage(playerid), key)` |

## Configuration

Define before including the library:

| Macro | Default | Description |
|-------|---------|-------------|
| `MAX_LANGUAGES` | `4` | Maximum number of languages |
| `DELIMITER_CHAR` | `"="` | Key-value separator in INI files |
| `DIRECTORY_LANGUAGES` | `"languages/"` | Language files directory |
| `MAX_LANGUAGE_KEY_LEN` | `32` | Maximum key length |
| `MAX_LANGUAGE_ENTRY_LENGTH` | `768` | Maximum translation length |
| `MAX_LANGUAGE_NAME` | `32` | Maximum language name length |
| `MAX_LANGUAGE_CODE` | `8` | Maximum language code length |
| `MAX_FILE_NAME` | `64` | Maximum file name length |
| `MAX_REPLACEMENT_KEY_LEN` | `16` | Maximum replacement key length |
| `MAX_REPLACEMENT_VALUE_LEN` | `16` | Maximum replacement value length |
| `LANGPLUS_NO_MACROS` | `` | Define to disable @L and @LS macros |

Example:
```pawn
#define MAX_LANGUAGES 8
#define DIRECTORY_LANGUAGES "translations/"
#include <LangPlus>
```

## Language File Syntax

Language files use INI format with the following rules:

- **Key format**: Keys must start with a letter (a-z, A-Z), digit (0-9), or a certain symbol (`!`, `@`, `$`, `&`)
- **Delimiter**: Keys and values are separated by `=` (configurable via `DELIMITER_CHAR`)
- **Comments**: Lines starting with other characters are ignored by the parser (e.g., `;`, `#`, `[`)
- **Escape sequences**: Supports `\n`, `\t`, etc. in values
- **Format specifiers**: Values can contain `%d`, `%s`, `%f`, and other format specifiers

Example:
```ini
; This is a comment
WELCOME_MESSAGE=Welcome, player %d!
@ADMIN_COMMAND=Admin %s used command
$ERROR_MESSAGE=Error: %s
```

## Testing

```bash
sampctl run
```

## Credits

Inspired by [ScavengeSurvive/language](https://github.com/ScavengeSurvive/language)
