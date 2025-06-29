--[[
    Modular automation tool for "[☀️] Grow a Garden 🌶️".
    Intended strictly for private server or studio testing to streamline QA tasks.
    All interactions rely on official RemoteEvents and player-simulated input only.

    Features:
      - Automated planting, watering, harvesting
      - Automated seed and tool purchasing
      - Automated selling of harvested produce
      - Anti-AFK for long test sessions
      - Draggable, collapsible GUI panel with per-feature toggles

    This script must not be used in public matches or to gain unfair advantages.
]]

---------------------- Services & References ----------------------
local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local RunService = game:GetService("RunService")
local VirtualUser = game:GetService("VirtualUser")

local LocalPlayer = Players.LocalPlayer

local Events = ReplicatedStorage:WaitForChild("GameEvents")
local Plant_RE = Events:WaitForChild("Plant_RE")
local Remove_Item = Events:WaitForChild("Remove_Item")
local BuySeedStock = Events:WaitForChild("BuySeedStock")
local BuyGearStock = Events:WaitForChild("BuyGearStock")
local Sell_Inventory = Events:WaitForChild("Sell_Inventory")

---------------------- Automation Table ----------------------
local Automation = {
    Toggles = {
        AutoPlant = false,
        AutoWater = false,
        AutoHarvest = false,
        AutoBuySeeds = false,
        AutoBuyTools = false,
        AutoSell = false,
        AntiAFK = false,
    },
    GUI = {},
    LastAFK = 0,
}

---------------------- Utility Functions ----------------------
function Automation:GetPlayerPlot()
    local farm = workspace:FindFirstChild("Farm")
    if not farm then return nil end
    for _, plot in ipairs(farm:GetChildren()) do
        local imp = plot:FindFirstChild("Important")
        if imp and imp:FindFirstChild("Data") and imp.Data:FindFirstChild("Owner") then
            if imp.Data.Owner.Value == LocalPlayer.Name then
                return plot
            end
        end
    end
    return nil
end

function Automation:PlantSeeds()
    local plot = self:GetPlayerPlot()
    if not plot then return end
    local locFolder = plot.Important:FindFirstChild("Plant_Locations")
    if not locFolder then return end
    for _, spot in ipairs(locFolder:GetChildren()) do
        Plant_RE:FireServer(spot.Position, "Basic Seed")
        task.wait(0.15)
    end
end

function Automation:HandlePrompts(keyword)
    local plot = self:GetPlayerPlot()
    if not plot then return end
    local plants = plot.Important:FindFirstChild("Plants_Physical")
    if not plants then return end
    for _, plant in ipairs(plants:GetChildren()) do
        local prompt = plant:FindFirstChildWhichIsA("ProximityPrompt", true)
        if prompt and prompt.Enabled and string.find(prompt.ObjectText, keyword) then
            fireproximityprompt(prompt)
            task.wait(0.1)
        end
    end
end

function Automation:BuySeeds()
    local guiShop = LocalPlayer.PlayerGui:FindFirstChild("Seed_Shop")
    if not guiShop then return end
    local list = guiShop:FindFirstChild("Frame") and guiShop.Frame:FindFirstChild("ScrollingFrame")
    if not list then return end
    for _, item in ipairs(list:GetChildren()) do
        if item:IsA("Frame") and item:FindFirstChild("Stock") and item.Stock.Value > 0 then
            BuySeedStock:FireServer(item.Name)
            task.wait(0.1)
        end
    end
end

function Automation:BuyTools()
    local guiShop = LocalPlayer.PlayerGui:FindFirstChild("Gear_Shop")
    if not guiShop then return end
    local list = guiShop:FindFirstChild("Frame") and guiShop.Frame:FindFirstChild("ScrollingFrame")
    if not list then return end
    for _, item in ipairs(list:GetChildren()) do
        if item:IsA("Frame") and item:FindFirstChild("Stock") and item.Stock.Value > 0 then
            BuyGearStock:FireServer(item.Name)
            task.wait(0.1)
        end
    end
end

function Automation:SellProduce()
    Sell_Inventory:FireServer()
end

function Automation:AntiAFK()
    if tick() - self.LastAFK > 60 then
        VirtualUser:CaptureController()
        VirtualUser:ClickButton2(Vector2.new(0, 0), workspace.CurrentCamera.CFrame)
        self.LastAFK = tick()
    end
end

---------------------- GUI Construction ----------------------
function Automation:CreateToggle(name, order)
    local button = Instance.new("TextButton")
    button.Size = UDim2.new(1, -8, 0, 28)
    button.Text = name .. ": OFF"
    button.LayoutOrder = order
    button.BackgroundColor3 = Color3.fromRGB(55, 55, 55)
    button.TextColor3 = Color3.new(1, 1, 1)
    button.Parent = self.GUI.Panel

    button.MouseButton1Click:Connect(function()
        self.Toggles[name] = not self.Toggles[name]
        button.Text = name .. (self.Toggles[name] and ": ON" or ": OFF")
    end)
end

function Automation:BuildGUI()
    local gui = Instance.new("ScreenGui")
    gui.Name = "GrowGardenAutomationGUI"
    gui.Parent = game:GetService("CoreGui")

    local panel = Instance.new("Frame")
    panel.Name = "Panel"
    panel.Size = UDim2.new(0, 180, 0, 240)
    panel.Position = UDim2.new(0, 40, 0, 120)
    panel.BackgroundColor3 = Color3.fromRGB(30, 30, 30)
    panel.Active = true
    panel.Draggable = true
    panel.Parent = gui

    local list = Instance.new("UIListLayout")
    list.Padding = UDim.new(0, 4)
    list.Parent = panel

    local openClose = Instance.new("TextButton")
    openClose.Name = "OpenClose"
    openClose.Size = UDim2.new(0, 60, 0, 24)
    openClose.Position = UDim2.new(0, 0, 1, -24)
    openClose.Text = "Close"
    openClose.BackgroundColor3 = Color3.fromRGB(45, 45, 45)
    openClose.TextColor3 = Color3.new(1, 1, 1)
    openClose.Parent = panel
    openClose.MouseButton1Click:Connect(function()
        panel.Visible = not panel.Visible
        openClose.Text = panel.Visible and "Close" or "Open"
    end)

    self.GUI.Screen = gui
    self.GUI.Panel = panel

    local order = 1
    for toggleName, _ in pairs(self.Toggles) do
        self:CreateToggle(toggleName, order)
        order += 1
    end
end

---------------------- Update Loop ----------------------
function Automation:Step()
    if self.Toggles.AutoPlant then self:PlantSeeds() end
    if self.Toggles.AutoWater then self:HandlePrompts("Water") end
    if self.Toggles.AutoHarvest then self:HandlePrompts("Harvest") end
    if self.Toggles.AutoBuySeeds then self:BuySeeds() end
    if self.Toggles.AutoBuyTools then self:BuyTools() end
    if self.Toggles.AutoSell then self:SellProduce() end
    if self.Toggles.AntiAFK then self:AntiAFK() end
end

---------------------- Initialization ----------------------
Automation:BuildGUI()

RunService.Heartbeat:Connect(function()
    Automation:Step()
end)

