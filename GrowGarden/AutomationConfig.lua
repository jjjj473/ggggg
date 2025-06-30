local Config = {
    SeedType = "Basic Seed", -- default seed for AutoPlant
    PlantDelay = 0.15,       -- delay between planting attempts
}

-- expose globally for executors that load this file separately
getgenv().GrowGardenConfig = Config

return Config
