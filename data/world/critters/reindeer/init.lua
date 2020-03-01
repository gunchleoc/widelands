dirname = path.dirname(__file__)

world:new_critter_type{
   name = "reindeer",
   descname = _ "Reindeer",
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
         hotspot = { 23, 18 }
      },
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 30,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 25, 27 }
      }
   }
}
