---@meta Chatterino2

-- This file is intended to be used with LuaLS (https://luals.github.io/).
-- Add the folder this file is in to "Lua.workspace.library".

---@alias LogLevel integer

---@class CommandContext
---@field words string[] The words typed when executing the command. For example `/foo bar baz` will result in `{"/foo", "bar", "baz"}`.
---@field channel_name string The name of the channel the command was executed in.

---@class CompletionList
---@field values string[] The completions
---@field hide_others boolean Whether other completions from Chatterino should be hidden/ignored.

c2 = {}

---@type { Debug: LogLevel, Info: LogLevel, Warning: LogLevel, Critical: LogLevel }
c2.LogLevel = {}

--- Writes a message to the Chatterino log.
---@param level LogLevel The desired level.
---@param ... any Values to log. Should be convertible to a string with `tostring()`.
function c2.log(level, ...) end

--- Registers a new command called `name` which when executed will call `handler`.
---@param name string The name of the command.
---@param handler fun(ctx: CommandContext) The handler to be invoked when the command gets executed.
---@return boolean ok  Returns `true` if everything went ok, `false` if a command with this name exists.
function c2.register_command(name, handler) end

--- Sends a message to `channel` with the specified text. Also executes commands.
---
--- **Warning**: It is possible to trigger your own Lua command with this causing a potentially infinite loop.
---
---@param channel string The name of the Twitch channel
---@param text string The text to be sent
---@return boolean ok
function c2.send_msg(channel, text) end

--- Creates a system message (gray message) and adds it to the Twitch channel specified by `channel`.
---@param channel string
---@param text string
---@return boolean ok
function c2.system_msg(channel, text) end

--- Registers a callback to be invoked when completions for a term are requested.
---@param type "CompletionRequested"
---@param func fun(query: string, full_text_content: string, cursor_position: integer, is_first_word: boolean): CompletionList The callback to be invoked.
function c2.register_callback(type, func) end
