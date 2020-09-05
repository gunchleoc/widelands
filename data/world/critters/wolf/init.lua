push_textdomain("world")

wl.World():new_critter_type{
   name = "wolf",
   descname = _ "Wolf",
   animation_directory = path.dirname(__file__),
   size = 5,
   reproduction_rate = 80,
   appetite = 100,
   carnivore = true,

   programs = {
      remove = { "remove" },
   },

   spritesheets = {
      idle = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 8, 15 },
         sound_effect = {
            path = "sound/animals/wolf",
            priority = 3
         }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 8, 15 },
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 19, 19 }
      }
   }
}

pop_textdomain()
