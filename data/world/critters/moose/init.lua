push_textdomain("world")

wl.World():new_critter_type{
   name = "moose",
   descname = _ "Moose",
   animation_directory = path.dirname(__file__),
   size = 10,
   reproduction_rate = 30,
   appetite = 50,
   herbivore = {"field"},

   programs = {
      remove = { "remove" },
   },

   spritesheets = {
      idle = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 15, 26 },
         sound_effect = {
            path = "sound/animals/moose",
            priority = 5
         }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 15, 26 },
      },
      walk = {
         fps = 25,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 21, 33 }
      }
   }
}

pop_textdomain()
