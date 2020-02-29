dirname = path.dirname(__file__)

world:new_critter_type{
   name = "brownbear",
   descname = _ "Brown bear",
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
         hotspot = { 20, 15 }
      },
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 35,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 24, 24 }
      }
   }
}
