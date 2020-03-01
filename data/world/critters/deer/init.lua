dirname = path.dirname(__file__)

world:new_critter_type{
   name = "deer",
   descname = _ "Deer",
   editor_category = "critters_herbivores",
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
         hotspot = { 1, 10 }
      },
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 10, 19 }
      }
   }
}
