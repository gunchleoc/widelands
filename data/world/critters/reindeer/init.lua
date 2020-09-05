push_textdomain("world")

wl.World():new_critter_type{
   name = "reindeer",
   descname = _ "Reindeer",
   animation_directory = path.dirname(__file__),
   size = 4,
   reproduction_rate = 60,
   appetite = 60,
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
         hotspot = { 23, 18 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 23, 18 }
      },
      walk = {
         fps = 30,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 25, 27 }
      }
   }
}

pop_textdomain()
