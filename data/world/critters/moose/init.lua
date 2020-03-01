dirname = path.dirname(__file__)

world:new_critter_type{
   name = "moose",
   descname = _ "Moose",
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
         hotspot = { 15, 26 },
         sound_effect = {
            -- Sound files with numbers starting for 10 are generating silence. Remove when we move the sound triggering to programs
            path = "sound/animals/elk",
         }
      },
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 25,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 21, 33 }
      }
   }
}
