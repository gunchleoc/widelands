dirname = path.dirname(__file__)

world:new_critter_type{
   name = "stag",
   descname = _ "Stag",
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
         hotspot = { 12, 30 },
         sound_effect = {
            -- Sound files with numbers starting for 10 are generating silence. Remove when we move the sound triggering to programs
            path = "sound/animals/stag",
         }
      },
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 30,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 25, 34 }
      }
   }
}
