dirname = path.dirname(__file__)

world:new_critter_type{
   name = "chamois",
   descname = _ "Chamois",
   editor_category = "critters_herbivores",
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
         hotspot = { 11, 13 }
      },
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 40,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 12, 14 }
      }
   }
}
