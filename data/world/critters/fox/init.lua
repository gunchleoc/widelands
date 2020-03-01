dirname = path.dirname(__file__)

world:new_critter_type{
   name = "fox",
   descname = _ "Fox",
   editor_category = "critters_carnivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 13 }
      },
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 25,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 11, 14 }
      }
   }
}
