local HUD = {}
HUD.counters = {
    planted = 0,
    watered = 0,
    harvested = 0,
}

function HUD:Create()
    local gui = Instance.new("ScreenGui")
    gui.Name = "AutomationHUD"
    gui.Parent = game:GetService("CoreGui")

    local label = Instance.new("TextLabel")
    label.Name = "StatusLabel"
    label.Position = UDim2.new(1, -160, 0, 60)
    label.Size = UDim2.new(0, 150, 0, 60)
    label.BackgroundTransparency = 0.3
    label.BackgroundColor3 = Color3.fromRGB(20, 20, 20)
    label.TextColor3 = Color3.new(1, 1, 1)
    label.TextScaled = true
    label.Parent = gui

    self.GUI = { Screen = gui, Label = label }
    self:Update()
end

function HUD:Update()
    if not self.GUI then return end
    self.GUI.Label.Text = string.format(
        "Planted: %d\nWatered: %d\nHarvested: %d",
        self.counters.planted,
        self.counters.watered,
        self.counters.harvested
    )
end

return HUD
