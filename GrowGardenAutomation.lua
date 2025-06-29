--[[
Automation script for [☀️] Grow a Garden 🌶️
Intended for private server or isolated development use only.
This script automates repetitive tasks for QA/testing purposes.
]]

local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local RunService = game:GetService("RunService")
local VirtualInputManager = game:GetService("VirtualInputManager")

local player = Players.LocalPlayer
local gui = Instance.new("ScreenGui")
local panel = Instance.new("Frame")
local openCloseButton = Instance.new("TextButton")
local layout = Instance.new("UIListLayout")

local toggles = {
    AutoPlant = false,
    AutoWater = false,
    AutoHarvest = false,
    AutoBuySeeds = false,
    AutoSell = false,
    AntiAFK = false
}

-- Utility function to create toggle buttons
local function createToggle(name, parent)
    local button = Instance.new("TextButton")
    button.Size = UDim2.new(1, -4, 0, 30)
    button.Text = name .. ": OFF"
    button.Parent = parent
    button.MouseButton1Click:Connect(function()
        toggles[name] = not toggles[name]
        if toggles[name] then
            button.Text = name .. ": ON"
        else
            button.Text = name .. ": OFF"
        end
    end)
end

-- Build GUI
gui.Name = "GrowGardenAutomationGUI"
if not game:GetService("CoreGui"):FindFirstChild(gui.Name) then
    gui.Parent = game:GetService("CoreGui")
else
    gui:Destroy()
    gui.Parent = game:GetService("CoreGui")
end

panel.Name = "Panel"
panel.Size = UDim2.new(0, 150, 0, 200)
panel.Position = UDim2.new(0, 50, 0, 100)
panel.BackgroundColor3 = Color3.fromRGB(30, 30, 30)
panel.Visible = true
panel.Active = true
panel.Draggable = true
panel.Parent = gui

layout.Parent = panel
layout.SortOrder = Enum.SortOrder.LayoutOrder

openCloseButton.Name = "OpenClose"
openCloseButton.Size = UDim2.new(0, 60, 0, 25)
openCloseButton.Position = UDim2.new(0, 0, 1, 0)
openCloseButton.Text = "Close"
openCloseButton.Parent = panel
openCloseButton.MouseButton1Click:Connect(function()
    panel.Visible = not panel.Visible
    if panel.Visible then
        openCloseButton.Text = "Close"
    else
        openCloseButton.Text = "Open"
    end
end)

-- Create toggle buttons
createToggle("AutoPlant", panel)
createToggle("AutoWater", panel)
createToggle("AutoHarvest", panel)
createToggle("AutoBuySeeds", panel)
createToggle("AutoSell", panel)
createToggle("AntiAFK", panel)

-- Helper functions (simplified examples)
local function plantSeed()
    -- Example: Fire remote event to plant a seed
    local remote = ReplicatedStorage:FindFirstChild("PlantSeed")
    if remote then
        remote:FireServer("Basic Seed")
    end
end

local function waterPlants()
    local remote = ReplicatedStorage:FindFirstChild("WaterPlant")
    if remote then
        remote:FireServer()
    end
end

local function harvestCrops()
    local remote = ReplicatedStorage:FindFirstChild("HarvestCrop")
    if remote then
        remote:FireServer()
    end
end

local function buySeeds()
    local remote = ReplicatedStorage:FindFirstChild("BuySeed")
    if remote then
        remote:FireServer("Basic Seed", 1)
    end
end

local function sellProduce()
    local remote = ReplicatedStorage:FindFirstChild("SellCrop")
    if remote then
        remote:FireServer()
    end
end

local function antiAFK()
    VirtualInputManager:SendKeyEvent(true, Enum.KeyCode.Space, false, game)
    wait(0.1)
    VirtualInputManager:SendKeyEvent(false, Enum.KeyCode.Space, false, game)
end

-- Automation loop
RunService.RenderStepped:Connect(function()
    if toggles.AutoPlant then
        plantSeed()
    end
    if toggles.AutoWater then
        waterPlants()
    end
    if toggles.AutoHarvest then
        harvestCrops()
    end
    if toggles.AutoBuySeeds then
        buySeeds()
    end
    if toggles.AutoSell then
        sellProduce()
    end
    if toggles.AntiAFK then
        antiAFK()
    end
end)

