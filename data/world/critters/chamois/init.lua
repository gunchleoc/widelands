push_textdomain("world")

wl.World():new_critter_type{
   name = "chamois",
   descname = _ "Chamois",
   animation_directory = path.dirname(__file__),
   size = 5,
   reproduction_rate = 60,
   appetite = 20,
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
         hotspot = { 11, 13 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 11, 13 }
      },
      walk = {
         fps = 40,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 12, 14 }
      }
   }
}

pop_textdomain()
