# Grow Garden Automation

This repo contains a private-server automation toolkit for the Roblox game "[\xe2\x98\x80\xef\xb8\x8f] Grow a Garden \xf0\x9f\x8c\xb6". It includes three scripts:

- `AutomationConfig.lua` - configuration options like seed type
- `AutomationHUD.lua` - simple on-screen counters for planted, watered, and harvested crops
- `GrowGardenAutomation.lua` - main automation logic with a toggle GUI

To use these scripts in your own testing place, you can load them directly from GitHub:

```lua
loadstring(game:HttpGet("https://github.com/yourname/ggggg/raw/main/AutomationConfig.lua"))()
loadstring(game:HttpGet("https://github.com/yourname/ggggg/raw/main/AutomationHUD.lua"))()
loadstring(game:HttpGet("https://github.com/yourname/ggggg/raw/main/GrowGardenAutomation.lua"))()
```

Replace `yourname/ggggg` with the actual GitHub path where the files are hosted. These commands insert the three scripts into your environment for private QA use.
