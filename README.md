# Grow Garden Automation

This repo contains a private-server automation toolkit for the Roblox game "[\xe2\x98\x80\xef\xb8\x8f] Grow a Garden \xf0\x9f\x8c\xb6". It includes three scripts:
**Use only in private servers or isolated development environments.**

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
