wl.World():new_terrain_type{
   name = "snow",
   descname = _ "Snow",
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 250,
   temperature = 25,
   humidity = 800,
   fertility = 100,

   enhancement = "taiga"
}
