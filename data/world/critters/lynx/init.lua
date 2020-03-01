dirname = path.dirname(__file__)

world:new_critter_type{
   name = "lynx",
   descname = _ "Lynx",
   editor_category = "critters_carnivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 8, 14 }
      },
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 25,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 11, 21 }
      }
   }
}
