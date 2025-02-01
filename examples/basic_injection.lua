-- Basic Lua Injection Example
-- This script demonstrates basic functionality and anti-detection techniques

-- Anti-detection timing check
local function checkWarden()
    local t1 = GetTime()
    local t2 = GetTime()
    if (t2 - t1) < 0.001 then
        return true -- Warden scan detected
    end
    return false
end

-- Safe print function with error handling
local function safePrint(message, value)
    local success, error = pcall(function()
        if value then
            print(message, value)
        else
            print(message)
        end
    end)
    if not success then
        -- Handle error silently to avoid detection
        return false
    end
    return true
end

-- Hook function example
local function hookSpellCast(self, spellId)
    if checkWarden() then return end
    safePrint("Spell cast detected", spellId)
end

-- Event handler example
local function onEvent(self, event)
    if checkWarden() then return end

    if event == "PLAYER_LOGIN" then
        safePrint("Player logged in successfully")
    elseif event == "SPELL_CAST_SUCCESS" then
        safePrint("Spell cast successful")
    end
end

-- Create frame for events
local frame = CreateFrame("Frame")
frame:RegisterEvent("PLAYER_LOGIN")
frame:RegisterEvent("SPELL_CAST_SUCCESS")
frame:SetScript("OnEvent", onEvent)

-- Hook secure functions
hooksecurefunc("CastSpell", hookSpellCast)

-- Main initialization
local function initialize()
    if checkWarden() then
        -- Retry initialization later if Warden is active
        C_Timer.After(0.1, initialize)
        return
    end

    safePrint("Injection initialized successfully")

    -- Example function modifications
    local oldCastSpell = CastSpell
    CastSpell = function(spellId)
        if checkWarden() then
            return oldCastSpell(spellId)
        end

        safePrint("Cast spell intercepted", spellId)
        return oldCastSpell(spellId)
    end
end

-- Start initialization
C_Timer.After(0.1, initialize)

-- Periodic safety check
C_Timer.NewTicker(1, function()
    if checkWarden() then return end

    -- Monitor player state
    local playerHealth = UnitHealth("player")
    local playerPower = UnitPower("player")

    safePrint("Player Health", playerHealth)
    safePrint("Player Power", playerPower)
end)

-- Command registration
SLASH_EXAMPLE1 = "/example"
SlashCmdList["EXAMPLE"] = function(msg)
    if checkWarden() then return end

    if msg == "status" then
        safePrint("Status check - everything working")
    elseif msg == "help" then
        safePrint("Available commands: status, help")
    end
end

-- Example utility functions
local function getPlayerInfo()
    if checkWarden() then return end

    local name = UnitName("player")
    local level = UnitLevel("player")
    local class = UnitClass("player")

    return {
        name = name,
        level = level,
        class = class
    }
end

local function monitorCombat()
    if checkWarden() then return end

    local inCombat = UnitAffectingCombat("player")
    if inCombat then
        safePrint("Player entered combat")
    else
        safePrint("Player left combat")
    end
end

-- Register for combat state changes
frame:RegisterEvent("PLAYER_REGEN_ENABLED")
frame:RegisterEvent("PLAYER_REGEN_DISABLED")

-- Example of secure hook usage
local function hookInventoryChange(bagID, slotID)
    if checkWarden() then return end
    safePrint("Inventory changed - Bag", bagID)
    safePrint("Slot", slotID)
end

hooksecurefunc("PickupContainerItem", hookInventoryChange)

-- Error handling wrapper
local function safeExecute(func)
    if type(func) ~= "function" then return end

    local success, result = pcall(func)
    if not success then
        -- Handle errors silently
        return nil
    end
    return result
end

-- Example usage
safeExecute(function()
    local info = getPlayerInfo()
    if info then
        safePrint("Player Name", info.name)
        safePrint("Player Level", info.level)
        safePrint("Player Class", info.class)
    end
end)

-- Register for additional events
frame:RegisterEvent("PLAYER_ENTERING_WORLD")
frame:RegisterEvent("PLAYER_LEAVING_WORLD")
frame:RegisterEvent("UNIT_SPELLCAST_SUCCEEDED")

-- Add event handler for new events
local function onWorldEvent(self, event)
    if checkWarden() then return end

    if event == "PLAYER_ENTERING_WORLD" then
        safePrint("Player entered world")
        initialize()
    elseif event == "PLAYER_LEAVING_WORLD" then
        safePrint("Player left world")
    end
end

frame:SetScript("OnEvent", onWorldEvent)
