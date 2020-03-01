dirname = path.dirname(__file__)

world:new_critter_type{
   name = "wolf",
   descname = _ "Wolf",
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
         hotspot = { 8, 15 },
         sound_effect = {
            -- Sound files with numbers starting from 10 are generating silence.
            path = "sound/animals/wolf",
         }
      },
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 19, 19 }
      }
   }
}
