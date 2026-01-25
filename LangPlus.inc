#if defined _inc_LangPlus
#undef _inc_LangPlus
#endif

#if defined _LangPlus_included
#endinput
#endif
#define _LangPlus_included

#if !defined PP_SYNTAX_FOR_POOL
    #define PP_SYNTAX_FOR_POOL
#endif

#if !defined PP_SYNTAX_FOR_MAP
    #define PP_SYNTAX_FOR_MAP
#endif

#include <open.mp>
#include <sscanf2>
#include <PawnPlus>
#include <strlib>
#tryinclude <YSI_Data\y_iterate>
#if !defined _Y_ITERATE_LOCAL_VERSION
    #tryinclude <foreach>
#endif
#tryinclude <logger>

#if !defined MAX_LANGUAGES
    #define MAX_LANGUAGES 4
#endif

#if !defined DELIMITER_CHAR
    #define DELIMITER_CHAR "="
#endif

// the directory language data is stored in
#if !defined DIRECTORY_LANGUAGES
	#define DIRECTORY_LANGUAGES "languages/"
#endif

// the maximum length of a language key
#if !defined MAX_LANGUAGE_KEY_LEN
	#define MAX_LANGUAGE_KEY_LEN (32)
#endif

// the maximum length of a language text entry
#if !defined MAX_LANGUAGE_ENTRY_LENGTH
	#define MAX_LANGUAGE_ENTRY_LENGTH (768)
#endif

// name limit for a language
#if !defined MAX_LANGUAGE_NAME
	#define MAX_LANGUAGE_NAME (32)
#endif

// Max file name length for a language file
#if !defined MAX_FILE_NAME
    #define MAX_FILE_NAME (64)
#endif

#if !defined MAX_REPLACEMENT_KEY_LEN
    #define MAX_REPLACEMENT_KEY_LEN (16)
#endif

#if !defined MAX_REPLACEMENT_VALUE_LEN
    #define MAX_REPLACEMENT_VALUE_LEN (16)
#endif

#define INVALID_LANGUAGE_ID (Language:-1)
#define DEFAULT_LANGUAGE_ID (Language:0)

#if !defined LANGPLUS_NO_MACROS
    #define @L(%0,%1) ReturnLanguageString(GetPlayerLanguage(%0), %1)
    #define @LS(%0,%1) ReturnLanguageString_s(GetPlayerLanguage(%0), %1)
#endif

static Map:s_LanguageMap[Language:MAX_LANGUAGES],
       Pool:s_LanguagePool,
       Map:s_LanguageNames,
       Map:s_StringReplacements,
       bool:s_LanguagePoolInitialized = false;

static Language:s_PlayerLanguage[MAX_PLAYERS];

static CallbackHandler:OnPlayerConnectHandler = INVALID_CALLBACK_HANDLER;

forward _pp@on_init@LangPlus();
public _pp@on_init@LangPlus() {
    s_LanguagePool = pool_new(MAX_LANGUAGES);
    s_LanguageNames = map_new();
    s_LanguagePoolInitialized = true;

    OnPlayerConnectHandler = pawn_register_callback("OnPlayerConnect", "LangPlus_OnPlayerConnect");
}

forward _pp@on_exit@PawnPlus();
public _pp@on_exit@PawnPlus() {
    if (!s_LanguagePoolInitialized) {
        return;
    }

    pawn_unregister_callback(OnPlayerConnectHandler);
    OnPlayerConnectHandler = INVALID_CALLBACK_HANDLER;

    // Delete all language maps
    for (new Language:i = DEFAULT_LANGUAGE_ID; i < Language:MAX_LANGUAGES; i++) {
        if (map_valid(s_LanguageMap[i])) {
            map_delete(s_LanguageMap[i]);
        }
    }

    // Delete language names map
    if (map_valid(s_LanguageNames)) {
        map_delete(s_LanguageNames);
    }

    // Delete string replacements map
    if (map_valid(s_StringReplacements)) {
        map_delete(s_StringReplacements);
    }

    // Delete language pool
    if (pool_valid(s_LanguagePool)) {
        pool_delete(s_LanguagePool);
    }

    s_LanguagePoolInitialized = false;
}

forward LangPlus_OnPlayerConnect(playerid);
public LangPlus_OnPlayerConnect(playerid) {
    s_PlayerLanguage[playerid] = DEFAULT_LANGUAGE_ID;
    return 1;
}

static Language:_InitLanguage(const string:code[], const string:name[]) {
    if (pool_size(s_LanguagePool) == MAX_LANGUAGES) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "too many languages",
                   Logger_S("code", code));
        #endif
        return INVALID_LANGUAGE_ID;
    }

    if (pool_find_str(s_LanguagePool, code) != -1) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "language already exists",
                   Logger_S("code", code));
        #endif
        return INVALID_LANGUAGE_ID;
    }

    new Language:languageid = Language:pool_add_str(s_LanguagePool, code);
    map_set_arr(s_LanguageNames, _:languageid, name, MAX_LANGUAGE_NAME);

    return languageid;
}

static bool:IsValidLanguage(Language:id) {
    if (!s_LanguagePoolInitialized) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "no languages have been initialized");
        #endif

        return false;
    }

    if (_:id < 0 || _:id >= pool_size(s_LanguagePool)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id",
                   Logger_I("languageid", _:id));
        #endif

        return false;
    }

    if (!map_valid(s_LanguageMap[id])) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "language hasn't been initialized ",
                   Logger_I("languageid", _:id));
        #endif

        return false;
    }

    return pool_has(s_LanguagePool, _:id);
}

AddLanguageEntry(const Language:languageid, const string:key[], const string:val[]) {
    if (!IsValidLanguage(languageid) || key[0] == EOS || val[0] == EOS) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id or key/value",
                   Logger_I("languageid", _:languageid),
                   Logger_S("key", key),
                   Logger_S("val", val));
        #endif
        return false;
    }

    if (map_has_str_key(s_LanguageMap[languageid], key)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "language key already exists",
                   Logger_S("key", key));
        #endif
        return false;
    }

    map_str_add_str(s_LanguageMap[languageid], key, val);

    return true;
}

// This one shouldn't be a stock.
// Importing the library and not using it is pointless
Language:LoadLanguage(const string:code[], const string:name[], const string:fileName[] = "") {
    if (code[0] == EOS || name[0] == EOS) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "code or name is empty");
        #endif
        return INVALID_LANGUAGE_ID;
    }

    new Language:languageid = _InitLanguage(code, name);
    if (languageid == INVALID_LANGUAGE_ID) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "unable to initialize language",
                   Logger_S("code", code),
                   Logger_S("name", name));
        #endif

        return INVALID_LANGUAGE_ID;
    }

    s_LanguageMap[languageid] = map_new();

    new fileToLoad[MAX_FILE_NAME],
        File:fHandler,
        line[MAX_LANGUAGE_KEY_LEN + 1 + MAX_LANGUAGE_ENTRY_LENGTH],
        lineNumber,
        lineLength,
        entriesAdded = 0;

    // Priority: (fileName > name > code).ini
    if (!isempty(fileName)) {
        format(fileToLoad, _, "%s%s", DIRECTORY_LANGUAGES, fileName);
    } else {
        format(fileToLoad, _, "%s%s.ini", DIRECTORY_LANGUAGES, name);
        
        if (!fexist(fileToLoad)) {
            format(fileToLoad, _, "%s%s.ini", DIRECTORY_LANGUAGES, code);
        }
    }

    fHandler = fopen(fileToLoad, io_read);

    if (!fHandler) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "unable to open file",
                   Logger_S("fileName", fileToLoad));
        #endif

        map_delete(s_LanguageMap[languageid]);
        map_remove(s_LanguageNames, _:languageid);
        pool_remove(s_LanguagePool, _:languageid);
        return INVALID_LANGUAGE_ID;
    }

    new key[MAX_LANGUAGE_KEY_LEN],
        val[MAX_LANGUAGE_ENTRY_LENGTH],
        valFormatted[MAX_LANGUAGE_ENTRY_LENGTH];

    while (fread(fHandler, line)) {
        lineNumber++;
        lineLength = strlen(line);

        if (lineLength < 3) {
            // Minimum length of a line is 3 characters: "a=b"
            continue;
        }

        if (
            (line[0] < 'a' || line[0] > 'z')
            && (line[0] < 'A' || line[0] > 'Z')
            && (line[0] != '!' && line[0] != '@')
            && (line[0] != '$' && line[0] != '&')
            && (line[0] < '0' || line[0] > '9')
        ) {
            // Skip lines that don't start with a letter or digit (most likely comments)
            continue;
        }

        if (sscanf(line, "p<"#DELIMITER_CHAR">s["#MAX_LANGUAGE_KEY_LEN"]s["#MAX_LANGUAGE_ENTRY_LENGTH"]", key, val)) {
            #if(defined _logger_included)
            Logger_Dbg("langplus", "invalid line format",
                       Logger_I("line", lineNumber),
                       Logger_S("file", fileToLoad),
                       Logger_S("line", line));
            #endif

            continue;
        }

        strfromliteral(valFormatted, val);

        if (map_valid(s_StringReplacements)) {
            for_map(it : s_StringReplacements) {
                new replacementKey[MAX_REPLACEMENT_KEY_LEN + 1],
                    replacementValue[MAX_REPLACEMENT_VALUE_LEN + 1];

                iter_get_key_str(it, replacementKey);
                iter_get_value_str(it, replacementValue);

                strreplace(valFormatted, replacementKey, replacementValue);
            }
        }

        if (AddLanguageEntry(languageid, key, valFormatted)) {
            entriesAdded++;
        } else {
            #if(defined _logger_included)
            Logger_Dbg("langplus", "failed to add language entry",
                       Logger_I("line", lineNumber),
                       Logger_S("file", fileToLoad),
                       Logger_S("key", key));
            #endif
        }
    }

    fclose(fHandler);

    if (entriesAdded == 0) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "no valid entries found in language file",
                   Logger_S("file", fileToLoad));
        #endif

        map_delete(s_LanguageMap[languageid]);
        map_remove(s_LanguageNames, _:languageid);
        pool_remove(s_LanguagePool, _:languageid);
        return INVALID_LANGUAGE_ID;
    }

    return languageid;
}

stock bool:SetStringReplacement(const string:key[], const string:value[]) {
    if (key[0] == EOS || value[0] == EOS) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid key/value",
                   Logger_S("key", key),
                   Logger_S("value", value));
        #endif

        return false;
    }

    if (strlen(key) > MAX_REPLACEMENT_KEY_LEN) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "key is too long",
                   Logger_S("key", key));
        #endif

        return false;
    }

    if (strlen(value) > MAX_REPLACEMENT_VALUE_LEN) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "value is too long",
                   Logger_S("value", value));
        #endif

        return false;
    }

    if (!map_valid(s_StringReplacements)) {
        s_StringReplacements = map_new();
    }

    if (map_has_str_key(s_StringReplacements, key)) {
        #if(defined _logger_included)
        Logger_Log("replacement key already exists",
                   Logger_S("key", key));
        #endif

        return false;
    }

    map_str_add_str(s_StringReplacements, key, value);

    return true;
}

stock GetLanguageCount() {
    if (!s_LanguagePoolInitialized) {
        return 0;
    }

    return pool_size(s_LanguagePool);
}

stock bool:DoesLanguageCodeExist(const string:code[]) {
    if (!s_LanguagePoolInitialized || code[0] == EOS) {
        return false;
    }

    return pool_find_str(s_LanguagePool, code) != -1;
}

stock bool:DoesLanguageNameExist(const string:name[]) {
    if (!s_LanguagePoolInitialized || name[0] == EOS) {
        return false;
    }

    for_pool(it : s_LanguagePool) {
        new Language:langid = Language:iter_get_key(it);
        new string:langName[MAX_LANGUAGE_NAME];
        map_get_arr(s_LanguageNames, _:langid, langName, MAX_LANGUAGE_NAME);
        
        if (strcmp(langName, name) == 0) {
            return true;
        }
    }

    return false;
}

stock Language:GetLanguageIdByCode(const string:code[]) {
    if (!s_LanguagePoolInitialized || code[0] == EOS) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "no languages have been initialized or empty code");
        #endif
        return INVALID_LANGUAGE_ID;
    }

    // Index of not existing languages is -1, so it equals INVALID_LANGUAGE_ID
    return Language:pool_find_str(s_LanguagePool, code);
}

stock Language:GetLanguageIdByName(const string:name[]) {
    if (!s_LanguagePoolInitialized || name[0] == EOS) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "no languages have been initialized or empty name",
                   Logger_S("name", name));
        #endif
        return INVALID_LANGUAGE_ID;
    }

    for_pool(it : s_LanguagePool) {
        new Language:langid = Language:iter_get_key(it);
        new string:langName[MAX_LANGUAGE_NAME];
        map_get_arr(s_LanguageNames, _:langid, langName, MAX_LANGUAGE_NAME);
        
        if (strcmp(langName, name) == 0) {
            return langid;
        }
    }

    return INVALID_LANGUAGE_ID;
}

stock GetLanguageCodeList(string:codes[][], maxSize = sizeof(codes)) {
    if (!s_LanguagePoolInitialized) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "no languages have been initialized");
        #endif
        return 0;
    }

    new index,
        poolSize = pool_size(s_LanguagePool),
        limit = (poolSize < maxSize) ? poolSize : maxSize;

    for_pool(it : s_LanguagePool) {
        if (index >= limit) {
            break;
        }
        new Language:langid = Language:iter_get_key(it);
        pool_get_value_str(s_LanguagePool, _:langid, codes[index], MAX_LANGUAGE_NAME);
        index++;
    }

    return index;
}

stock GetLanguageNameList(string:names[][], maxSize = sizeof(names)) {
    if (!s_LanguagePoolInitialized) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "no languages have been initialized");
        #endif
        return 0;
    }

    new index,
        poolSize = pool_size(s_LanguagePool),
        limit = (poolSize < maxSize) ? poolSize : maxSize;

    for_pool(it : s_LanguagePool) {
        if (index >= limit) {
            break;
        }
        new Language:langid = Language:iter_get_key(it);
        map_get_arr(s_LanguageNames, _:langid, names[index], MAX_LANGUAGE_NAME);
        index++;
    }

    return index;
}

stock bool:GetLanguageCode(Language:languageid, string:output[] = "", len = sizeof(output)) {
    if (!IsValidLanguage(languageid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id",
                   Logger_I("languageid", _:languageid));
        #endif

        return false;
    }

    pool_get_value_str(s_LanguagePool, _:languageid, output, len);

    return output[0] != '\0';
}

stock bool:GetLanguageName(Language:languageid, string:output[] = "", len = sizeof(output)) {
    if (!IsValidLanguage(languageid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id",
                   Logger_I("languageid", _:languageid));
        #endif

        return false;
    }

    map_get_arr(s_LanguageNames, _:languageid, output, len);

    return output[0] != '\0';
}

stock bool:GetLanguageString(Language:languageid, const string:key[], string:output[], len = sizeof(output)) {
    if (!IsValidLanguage(languageid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id",
                   Logger_I("languageid", _:languageid));
        #endif

        return false;
    }

    if (!map_has_str_key(s_LanguageMap[languageid], key)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "language key does not exist",
                   Logger_I("languageid", _:languageid),
                   Logger_S("key", key));
        #endif

        if (languageid != DEFAULT_LANGUAGE_ID && IsValidLanguage(DEFAULT_LANGUAGE_ID)) {
            return GetLanguageString(DEFAULT_LANGUAGE_ID, key, output, len);
        }

        #if(defined _logger_included)
        Logger_Log("language key does not exist in the default language",
                   Logger_I("languageid", _:languageid),
                   Logger_S("key", key));
        #endif

        strcopy(output, key, len);

        return false;
    }

    map_str_get_str(s_LanguageMap[languageid], key, output, len);

    return true;
}

stock ReturnLanguageString(Language:languageid, const string:key[]) {
    new string:result[MAX_LANGUAGE_ENTRY_LENGTH + 1];
    GetLanguageString(languageid, key, result);

    return result;
}

stock String:ReturnLanguageString_s(Language:languageid, const string:key[]) {
    new string:result[MAX_LANGUAGE_ENTRY_LENGTH + 1];
    GetLanguageString(languageid, key, result);

    return str_new(result);
}

stock bool:SetPlayerLanguage(playerid, Language:languageid) {
    if (!IsValidLanguage(languageid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id",
                   Logger_I("languageid", _:languageid));
        #endif

        return false;
    }

    if (!IsPlayerConnected(playerid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid player id",
                   Logger_I("playerid", playerid));
        #endif

        return false;
    }

    s_PlayerLanguage[playerid] = languageid;

    return true;
}

stock Language:GetPlayerLanguage(playerid) {
    if (!IsPlayerConnected(playerid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid player id",
                   Logger_I("playerid", playerid));
        #endif

        return INVALID_LANGUAGE_ID;
    }

    return s_PlayerLanguage[playerid];
}

stock bool:SetPlayerLanguageByName(playerid, const string:name[]) {
    return SetPlayerLanguage(playerid, GetLanguageIdByName(name));
}

stock bool:SetPlayerLanguageByCode(playerid, const string:code[]) {
    return SetPlayerLanguage(playerid, GetLanguageIdByCode(code));
}

stock Language:GetPlayerLanguageName(playerid, string:output[] = "", len = sizeof(output)) {
    if (!IsPlayerConnected(playerid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid player id",
                   Logger_I("playerid", playerid));
        #endif

        return INVALID_LANGUAGE_ID;
    }

    GetLanguageName(s_PlayerLanguage[playerid], output);

    return s_PlayerLanguage[playerid];
}

#if defined _INC_y_va
stock bool:SendLanguageMessage(playerid, colour, const string:key[], {Float, _}:...) {
    new Language:langid = GetPlayerLanguage(playerid);
    if (langid == INVALID_LANGUAGE_ID) {
        return false;
    }

    SendClientMessage(playerid, colour, @L(playerid, key), ___(3));
    return true;
}

stock bool:SendLanguageMessageToAll(colour, const string:key[], {Float, _}:...) {
    #if defined _foreach_included || defined foreach_included || defined _Y_ITERATE_LOCAL_VERSION
    foreach (new i : Player)
    #else
    for (new i = 0; i < GetMaxPlayers(); i++)
    #endif
    {
        #if !defined _foreach_included && !defined foreach_included && !defined _Y_ITERATE_LOCAL_VERSION
        if (!IsPlayerConnected(i)) {
            continue;
        }
        #endif

        new Language:langid = GetPlayerLanguage(i);
        if (langid == INVALID_LANGUAGE_ID) {
            continue;
        }

        SendClientMessage(i, colour, @L(i, key), ___(2));
    }
    return true;
}
#endif
