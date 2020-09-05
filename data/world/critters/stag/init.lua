push_textdomain("world")

wl.World():new_critter_type{
   name = "stag",
   descname = _ "Stag",
   animation_directory = path.dirname(__file__),
   size = 7,
   reproduction_rate = 30,
   appetite = 30,
   herbivore = {"field"},

   programs = {
      remove = { "remove" },
   },

   spritesheets = {
      idle = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 12, 30 },
         sound_effect = {
            path = "sound/animals/stag",
            priority = 2
         }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 12, 30 },
      },
      walk = {
         fps = 30,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 25, 34 }
      }
   }
}

pop_textdomain()
