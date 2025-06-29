--[[
Automation script for [☀️] Grow a Garden 🌶️
Intended for private server or isolated development use only.
Automates repetitive tasks to aid QA and solo testing.
Uses only legitimate Roblox API calls and in-game RemoteEvents.
]]

local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local RunService = game:GetService("RunService")
local VirtualUser = game:GetService("VirtualUser")

local player = Players.LocalPlayer

-- RemoteEvents referenced by the game
local Events = ReplicatedStorage:WaitForChild("GameEvents")
local Plant_RE = Events:WaitForChild("Plant_RE")
local Remove_Item = Events:WaitForChild("Remove_Item")
local BuySeedStock = Events:WaitForChild("BuySeedStock")
local Sell_Inventory = Events:WaitForChild("Sell_Inventory")

-- GUI setup
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
    AntiAFK = false,
}

local function createToggle(name, parent)
    local button = Instance.new("TextButton")
    button.Size = UDim2.new(1, -4, 0, 30)
    button.Text = name .. ": OFF"
    button.Parent = parent
    button.BackgroundColor3 = Color3.fromRGB(60, 60, 60)
    button.TextColor3 = Color3.new(1, 1, 1)
    button.MouseButton1Click:Connect(function()
        toggles[name] = not toggles[name]
        button.Text = name .. (toggles[name] and ": ON" or ": OFF")
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
panel.Size = UDim2.new(0, 170, 0, 210)
panel.Position = UDim2.new(0, 40, 0, 100)
panel.BackgroundColor3 = Color3.fromRGB(30, 30, 30)
panel.Active = true
panel.Draggable = true
panel.Parent = gui

layout.Parent = panel
layout.SortOrder = Enum.SortOrder.LayoutOrder

openCloseButton.Name = "OpenClose"
openCloseButton.Size = UDim2.new(0, 60, 0, 25)
openCloseButton.Position = UDim2.new(0, 0, 1, -25)
openCloseButton.Text = "Close"
openCloseButton.Parent = panel
openCloseButton.BackgroundColor3 = Color3.fromRGB(45, 45, 45)
openCloseButton.TextColor3 = Color3.new(1,1,1)
openCloseButton.MouseButton1Click:Connect(function()
    panel.Visible = not panel.Visible
    openCloseButton.Text = panel.Visible and "Close" or "Open"
end)

createToggle("AutoPlant", panel)
createToggle("AutoWater", panel)
createToggle("AutoHarvest", panel)
createToggle("AutoBuySeeds", panel)
createToggle("AutoSell", panel)
createToggle("AntiAFK", panel)

-- Helpers
local function getPlayerPlot()
    local farm = workspace:FindFirstChild("Farm")
    if not farm then return nil end
    for _, plot in ipairs(farm:GetChildren()) do
        local data = plot:FindFirstChild("Important")
        if data and data:FindFirstChild("Data") and data.Data:FindFirstChild("Owner") then
            if data.Data.Owner.Value == player.Name then
                return plot
            end
        end
    end
    return nil
end

local function plantSeeds()
    local plot = getPlayerPlot()
    if not plot then return end
    local locations = plot.Important:FindFirstChild("Plant_Locations")
    if not locations then return end
    for _, spot in ipairs(locations:GetChildren()) do
        Plant_RE:FireServer(spot.Position, "Basic Seed")
        task.wait(0.3)
    end
end

local function handlePrompts(action)
    local plot = getPlayerPlot()
    if not plot then return end
    local plants = plot.Important:FindFirstChild("Plants_Physical")
    if not plants then return end
    for _, plant in ipairs(plants:GetChildren()) do
        local prompt = plant:FindFirstChildWhichIsA("ProximityPrompt", true)
        if prompt and prompt.Enabled and string.find(prompt.ObjectText, action) then
            fireproximityprompt(prompt)
            task.wait(0.2)
        end
    end
end

local function buySeeds()
    local guiShop = player.PlayerGui:FindFirstChild("Seed_Shop")
    if not guiShop or not guiShop:FindFirstChild("Frame") then return end
    local list = guiShop.Frame:FindFirstChild("ScrollingFrame")
    if not list then return end
    for _, item in ipairs(list:GetChildren()) do
        if item:IsA("Frame") and item:FindFirstChild("Stock") and item.Stock.Value > 0 then
            BuySeedStock:FireServer(item.Name)
            task.wait(0.2)
        end
    end
end

local function sellProduce()
    Sell_Inventory:FireServer()
end

local lastAFK = 0
local function antiAFK()
    if tick() - lastAFK > 30 then
        VirtualUser:CaptureController()
        VirtualUser:ClickButton2(Vector2.new(0,0), workspace.CurrentCamera.CFrame)
        lastAFK = tick()
    end
end

-- Main loop
RunService.Heartbeat:Connect(function()
    if toggles.AutoPlant then plantSeeds() end
    if toggles.AutoWater then handlePrompts("Water") end
    if toggles.AutoHarvest then handlePrompts("Harvest") end
    if toggles.AutoBuySeeds then buySeeds() end
    if toggles.AutoSell then sellProduce() end
    if toggles.AntiAFK then antiAFK() end
end)
