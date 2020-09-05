push_textdomain("world")

wl.World():new_critter_type{
   name = "bunny",
   descname = _ "Bunny",
   animation_directory = path.dirname(__file__),
   size = 1,
   reproduction_rate = 100,
   appetite = 100,
   herbivore = {"field"},

   programs = {
      remove = { "remove" },
   },

   animations = {
      idle = {
         hotspot = { 4, 9 },
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 4, 9 },
      },
   },
   spritesheets = {
      walk = {
         frames = 2,
         rows = 2,
         columns = 1,
         directional = true,
         hotspot = { 5, 9 }
      }
   }
}

pop_textdomain()
