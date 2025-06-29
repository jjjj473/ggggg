--[[
    Modular GUI-based automation script for "Prison Life".
    Intended for private servers or Roblox Studio testing only.
    Provides toggleable utilities for teleporting, tool granting,
    speed adjustment, arresting criminals, auto respawn, and anti-AFK.
    All actions use legitimate client methods and official RemoteEvents.
    Do not use in public matches.
]]

---------------------- Services & References ----------------------
local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Workspace = game:GetService("Workspace")
local RunService = game:GetService("RunService")
local VirtualUser = game:GetService("VirtualUser")

local LocalPlayer = Players.LocalPlayer

local ItemHandler = ReplicatedStorage:WaitForChild("ItemHandler")
local ArrestRemote = Workspace:WaitForChild("Remote"):WaitForChild("arrest")

---------------------- Configuration ----------------------
local LOCATIONS = {
    Yard = CFrame.new(-75, 22, 110),
    Cafeteria = CFrame.new(45, 22, 70),
    Armory = CFrame.new(-110, 22, 50),
    CriminalBase = CFrame.new(-943, 94, 2060),
}

local Automation = {
    WalkSpeeds = {16, 30, 60},
    WalkIndex = 1,
    Toggles = {
        AutoRespawn = false,
        AntiAFK = false,
    },
    GUI = {},
    Connections = {},
    SpawnCFrame = nil,
    LastAFK = 0,
}

---------------------- Utility Functions ----------------------
function Automation:Teleport(cf)
    local char = LocalPlayer.Character
    if char and char:FindFirstChild("HumanoidRootPart") then
        char.HumanoidRootPart.CFrame = cf
    end
end

function Automation:CycleWalkSpeed()
    local char = LocalPlayer.Character
    if char and char:FindFirstChildOfClass("Humanoid") then
        self.WalkIndex = self.WalkIndex % #self.WalkSpeeds + 1
        char.Humanoid.WalkSpeed = self.WalkSpeeds[self.WalkIndex]
        if self.GUI.WalkSpeedBtn then
            self.GUI.WalkSpeedBtn.Text = "WalkSpeed: " .. self.WalkSpeeds[self.WalkIndex]
        end
    end
end

function Automation:GiveTool(toolName)
    ItemHandler:InvokeServer(toolName)
end

local function getNearestCriminal()
    local hrp = LocalPlayer.Character and LocalPlayer.Character:FindFirstChild("HumanoidRootPart")
    if not hrp then return nil end
    local nearest, dist = nil, math.huge
    for _, plr in ipairs(Players:GetPlayers()) do
        if plr ~= LocalPlayer and plr.Team and plr.Team.Name == "Criminals" then
            local char = plr.Character
            local root = char and char:FindFirstChild("HumanoidRootPart")
            if root then
                local d = (root.Position - hrp.Position).Magnitude
                if d < dist then
                    dist = d
                    nearest = plr
                end
            end
        end
    end
    return nearest
end

function Automation:ArrestNearest()
    if not (LocalPlayer.Team and LocalPlayer.Team.Name == "Guards") then return end
    local target = getNearestCriminal()
    if target and target.Character and target.Character:FindFirstChild("HumanoidRootPart") then
        ArrestRemote:InvokeServer(target.Character.HumanoidRootPart)
    end
end

function Automation:EnableAutoRespawn(state)
    self.Toggles.AutoRespawn = state
    if self.GUI.AutoRespawnBtn then
        self.GUI.AutoRespawnBtn.Text = "Auto Respawn: " .. (state and "ON" or "OFF")
    end
    if state then
        local char = LocalPlayer.Character or LocalPlayer.CharacterAdded:Wait()
        self.SpawnCFrame = char:WaitForChild("HumanoidRootPart").CFrame
        self.Connections.Death = char:FindFirstChildOfClass("Humanoid").Died:Connect(function()
            LocalPlayer.CharacterAdded:Wait():WaitForChild("HumanoidRootPart").CFrame = self.SpawnCFrame
        end)
    else
        if self.Connections.Death then
            self.Connections.Death:Disconnect()
            self.Connections.Death = nil
        end
    end
end

function Automation:EnableAntiAFK(state)
    self.Toggles.AntiAFK = state
    if self.GUI.AntiAFKBtn then
        self.GUI.AntiAFKBtn.Text = "Anti AFK: " .. (state and "ON" or "OFF")
    end
end

function Automation:HandleAntiAFK()
    if self.Toggles.AntiAFK and tick() - self.LastAFK > 60 then
        VirtualUser:CaptureController()
        VirtualUser:ClickButton2(Vector2.new(0, 0), Workspace.CurrentCamera.CFrame)
        self.LastAFK = tick()
    end
end

---------------------- GUI Construction ----------------------
function Automation:CreateButton(text, callback)
    local btn = Instance.new("TextButton")
    btn.Size = UDim2.new(1, -8, 0, 28)
    btn.Text = text
    btn.BackgroundColor3 = Color3.fromRGB(55, 55, 55)
    btn.TextColor3 = Color3.new(1, 1, 1)
    btn.Parent = self.GUI.Panel
    btn.MouseButton1Click:Connect(callback)
    return btn
end

function Automation:BuildGUI()
    local screen = Instance.new("ScreenGui")
    screen.Name = "PrisonLifeAutomationGUI"
    screen.ResetOnSpawn = false
    screen.Parent = game:GetService("CoreGui")

    local panel = Instance.new("Frame")
    panel.Size = UDim2.new(0, 200, 0, 330)
    panel.Position = UDim2.new(0, 60, 0.5, -165)
    panel.BackgroundColor3 = Color3.fromRGB(30, 30, 30)
    panel.Active = true
    panel.Draggable = true
    panel.Parent = screen

    local layout = Instance.new("UIListLayout")
    layout.Padding = UDim.new(0, 4)
    layout.Parent = panel

    self.GUI.Screen = screen
    self.GUI.Panel = panel

    self:CreateButton("Teleport: Yard", function() self:Teleport(LOCATIONS.Yard) end)
    self:CreateButton("Teleport: Cafeteria", function() self:Teleport(LOCATIONS.Cafeteria) end)
    self:CreateButton("Teleport: Armory", function() self:Teleport(LOCATIONS.Armory) end)
    self:CreateButton("Teleport: Criminal Base", function() self:Teleport(LOCATIONS.CriminalBase) end)

    self.GUI.WalkSpeedBtn = self:CreateButton("WalkSpeed: " .. self.WalkSpeeds[self.WalkIndex], function() self:CycleWalkSpeed() end)

    self:CreateButton("Give M9", function() self:GiveTool("M9") end)
    self:CreateButton("Give AK-47", function() self:GiveTool("AK-47") end)
    self:CreateButton("Give Remington 870", function() self:GiveTool("Remington 870") end)

    self:CreateButton("Arrest Nearest", function() self:ArrestNearest() end)

    self.GUI.AutoRespawnBtn = self:CreateButton("Auto Respawn: OFF", function()
        self:EnableAutoRespawn(not self.Toggles.AutoRespawn)
    end)

    self.GUI.AntiAFKBtn = self:CreateButton("Anti AFK: OFF", function()
        self:EnableAntiAFK(not self.Toggles.AntiAFK)
    end)

    local hideBtn = Instance.new("TextButton")
    hideBtn.Text = "Close"
    hideBtn.Size = UDim2.new(0, 60, 0, 26)
    hideBtn.Position = UDim2.new(1, 4, 0, 0)
    hideBtn.BackgroundColor3 = Color3.fromRGB(45, 45, 45)
    hideBtn.TextColor3 = Color3.new(1,1,1)
    hideBtn.Parent = panel
    hideBtn.MouseButton1Click:Connect(function()
        panel.Visible = false
    end)

    local openBtn = Instance.new("TextButton")
    openBtn.Text = "Open"
    openBtn.Size = UDim2.new(0, 60, 0, 26)
    openBtn.Position = UDim2.new(0, 0, 0.5, -13)
    openBtn.BackgroundColor3 = Color3.fromRGB(45,45,45)
    openBtn.TextColor3 = Color3.new(1,1,1)
    openBtn.Parent = screen
    openBtn.MouseButton1Click:Connect(function()
        panel.Visible = not panel.Visible
    end)
    self.GUI.OpenBtn = openBtn
end

---------------------- Initialization ----------------------
Automation:BuildGUI()

RunService.Heartbeat:Connect(function()
    Automation:HandleAntiAFK()
end)

return Automation
