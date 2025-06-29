# Grow Garden Automation

This repo contains a private-server automation toolkit for the Roblox game "[☀️] Grow a Garden 🌶️". It includes three scripts and is **for private servers only**.

This repository now organizes the Grow a Garden scripts inside a `GrowGarden` folder.

- `GrowGarden/AutomationConfig.lua` - configuration options like seed type
- `GrowGarden/AutomationHUD.lua` - simple on-screen counters for planted, watered, and harvested crops
- `GrowGarden/GrowGardenAutomation.lua` - main automation logic with a toggle GUI

To use these scripts in your own testing place, you can load them directly from GitHub:

```lua
loadstring(game:HttpGet("https://raw.githubusercontent.com/jjjj473/ggggg/codex/create-automation-script-for-grow-a-garden/GrowGarden/AutomationConfig.lua"))()
loadstring(game:HttpGet("https://raw.githubusercontent.com/jjjj473/ggggg/codex/create-automation-script-for-grow-a-garden/GrowGarden/AutomationHUD.lua"))()
loadstring(game:HttpGet("https://raw.githubusercontent.com/jjjj473/ggggg/codex/create-automation-script-for-grow-a-garden/GrowGarden/GrowGardenAutomation.lua"))()
```
These commands load the scripts directly from the repository's `codex/create-automation-script-for-grow-a-garden` branch for private QA use.

## Prison Life Automation

A separate script in this repo offers basic private-server utilities for the Roblox game "Prison Life." It features teleport shortcuts, tool granting, walk speed cycling, a quick-arrest action, an auto-respawn toggle, and an anti-AFK option. The script includes a draggable and collapsible GUI for easy testing.

Load it with:

```lua
loadstring(game:HttpGet("https://raw.githubusercontent.com/jjjj473/ggggg/work/PrisonLife/PrisonLifeAutomation.lua"))()
```

Like the Grow a Garden tools, this Prison Life automation is intended only for solo testing in private servers or Roblox Studio.
