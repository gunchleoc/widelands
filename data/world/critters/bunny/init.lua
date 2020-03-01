dirname = path.dirname(__file__)

world:new_critter_type{
   name = "bunny",
   descname = _ "Bunny",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 4, 9 },
      },
   },
   spritesheets = {
      walk = {
         directory = dirname,
         basename = "walk",
         frames = 2,
         rows = 2,
         columns = 1,
         directional = true,
         hotspot = { 5, 9 }
      }
   }
}
