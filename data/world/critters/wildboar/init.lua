push_textdomain("world")

wl.World():new_critter_type{
   name = "wildboar",
   descname = _ "Wild boar",
   animation_directory = path.dirname(__file__),
   size = 8,
   reproduction_rate = 10,
   appetite = 20,
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
         hotspot = { 10, 18 },
         sound_effect = {
            path = "sound/animals/boar",
            priority = 0.01
         }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 18 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 20, 22 }
      }
   }
}

pop_textdomain()
